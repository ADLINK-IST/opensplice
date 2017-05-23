/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v_kernel.h"
#include "v__entry.h"
#include "v__dataReaderEntry.h"
#include "v__dataReader.h"
#include "v__reader.h"
#include "v_filter.h"
#include "v__dataReaderInstance.h"
#include "v_dataView.h"
#include "v__observer.h"
#include "v_readerQos.h"
#include "v_index.h"
#include "v_public.h"
#include "v__dataReaderSample.h"
#include "v_policy.h"
#include "v_state.h"
#include "v_instance.h"
#include "v__deadLineInstanceList.h"
#include "v__lifespanAdmin.h"
#include "v__transaction.h"
#include "v__participant.h"
#include "v__subscriber.h"
#include "v_builtin.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "v__deliveryService.h"
#include "v__kernel.h"
#include "v_proxy.h"

#include "os_heap.h"
#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/
typedef struct v_lifespanArg_s {
   v_readerSampleAction action;
   c_voidp arg;
   v_index index;
   os_timeE time;
} *v_lifespanArg;

static c_bool
lifespanTakeAction(
    v_lifespanSample sample,
    c_voidp arg)
{
    c_bool result;
    v_lifespanArg lifespanArg = (v_lifespanArg)arg;
    v_dataReaderInstance instance;

    /* Data is expired, remove the sample from its instance
     * NOTE: the sampleCount of index is decreased by this function */
    if (lifespanArg->action != NULL) {
        v_actionResult actresult = lifespanArg->action(v_readerSample(sample), lifespanArg->arg);
        result = (actresult != V_STOP);
    } else {
        result = TRUE;
    }
    if (result) {
        instance = v_dataReaderInstance(v_readerSample(sample)->instance);
        v_dataReaderInstanceSampleRemove(instance, v_dataReaderSample(sample), FALSE);

        assert(v_dataReader(lifespanArg->index->reader)->resourceSampleCount >= 0);
        if (v_dataReaderInstanceEmpty(instance)) {
            v_dataReaderRemoveInstance(lifespanArg->index->reader, instance);
            /* No statistics, the index will update the statistics here */
        }
    }

    return result;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_dataReaderEntry
v_dataReaderEntryNew(
    v_dataReader dataReader,
    v_index index,
    v_topic topic,
    q_expr _where,
    const c_value *params[])
{
    v_kernel kernel;
    v_dataReaderEntry e;

    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(dataReader);
    e = v_dataReaderEntry(v_objectNew(kernel,K_DATAREADERENTRY));
    v_entryInit(v_entry(e), v_reader(dataReader));
    e->topic = c_keep(topic);

    v_filterSplit(topic, _where, *params, &e->filterInstance, &e->filterData, index);

    /* Aministration for lifespan of messages */
    e->lifespanAdmin = v_lifespanAdminNew(kernel);
    /* The time-ordered lists for autopurging and garbagecollection */
    e->purgeListNotEmpty = c_listNew(v_kernelType(kernel, K_PURGELISTITEM));
    e->purgeListDisposed = c_listNew(v_kernelType(kernel, K_PURGELISTITEM));
    if (v_reader(dataReader)->subQos->presentation.v.coherent_access) {
        v_transactionGroupAdmin gAdmin = NULL;
        if (v_reader(dataReader)->subQos->presentation.v.access_scope == V_PRESENTATION_GROUP)
        {
            gAdmin = v_subscriberLookupTransactionGroupAdmin(v_reader(dataReader)->subscriber);
            assert(gAdmin);
        }
        e->transactionAdmin = v_transactionAdminNew(v_object(dataReader), gAdmin, topic);
    } else {
        e->transactionAdmin = NULL;
    }

    return e;
}

/* Callback functions for the index write action */

static v_actionResult
onSampleDumpedAction(
    c_object sample,
    c_voidp arg)
{
    v_actionResult result = 0;
    OS_UNUSED_ARG(arg);

    v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
    v_actionResultSet(result, V_PROCEED);
    return result;
}

static void
onSampleRejected(
    v_dataReader dataReader,
    v_sampleRejectedKind kind,
    v_gid instanceGid)
{
    v_dataReaderNotifySampleRejected(dataReader, kind, instanceGid);

    /* update statistics */
    if (dataReader->statistics) {
        switch (kind) {
        case S_REJECTED_BY_INSTANCES_LIMIT:
            dataReader->statistics->numberOfSamplesRejectedByInstancesLimit++;
        break;
        case S_REJECTED_BY_SAMPLES_LIMIT:
            dataReader->statistics->numberOfSamplesRejectedBySamplesLimit++;
        break;
        case S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            /*NOT SUPPORTED*/
        break;
        case S_NOT_REJECTED:
            /*nothing to be done*/
        break;
        }
    }
}

static void
purgeListInsert(
    c_collection purgeList,
    v_dataReaderInstance instance)
{
    v_purgeListItem item;
    item = c_new(v_kernelType(v_objectKernel(instance),K_PURGELISTITEM));
    item->instance = c_keep(instance);
    item->insertionTime = os_timeMGet();
    if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
        item->genCount = instance->disposeCount;
    } else {
        item->genCount = instance->noWritersCount;
    }
    c_append(purgeList,item);
    instance->purgeInsertionTime = item->insertionTime;
    c_free(item);
}

static void
doInstanceAutoPurge(
    v_dataReader reader,
    v_dataReaderInstance instance,
    c_long disposedCount,
    c_long noWritersCount)
{
    c_long sampleCount;

    if (!v_dataReaderInstanceEmpty(instance)) {
        /* Remove all samples from the instance, where
       sample->disposed-/noWritersCount <= disposed-/noWritersCount
        */
        sampleCount = v_dataReaderInstanceSampleCount(instance);
        v_dataReaderInstancePurge(instance, disposedCount, noWritersCount);
        sampleCount -= v_dataReaderInstanceSampleCount(instance);
        assert(sampleCount >= 0);
        assert(reader->resourceSampleCount >= 0);
        if (v_dataReaderInstanceEmpty(instance)) {
            v_dataReaderRemoveInstance(reader,instance);
        }
    } else {
        v_dataReaderRemoveInstance(reader,instance);
    }
}

void
v_dataReaderEntryUpdatePurgeLists(
    v_dataReaderEntry entry)
{
    struct v_lifespanArg_s lifespanArg;
    os_timeM now, timestamp;
    c_list purgeList;
    os_compare equality;
    v_purgeListItem purgeListItem, testItem;
    v_dataReaderInstance purgeInstance;
    v_readerQos qos;
    v_dataReader reader;
    c_long count, delta; /* statistics */

    reader = v_dataReader(v_entry(entry)->reader);

    /* Purge all instances that are not alive and expired */
    if ((c_listCount(entry->purgeListNotEmpty) > 0) ||
        (c_listCount(entry->purgeListDisposed) > 0) ||
        (v_lifespanAdminSampleCount(entry->lifespanAdmin) > 0)) {
        now = os_timeMGet();
    } else {
        return;
    }

    if (v_lifespanAdminSampleCount(entry->lifespanAdmin) > 0) {
        /* Walk over samples in lifespan administration in order to determine if
         * they have to be discarded */
        lifespanArg.action = onSampleDumpedAction;
        lifespanArg.arg = reader;
        lifespanArg.index = entry->index;
        lifespanArg.time = os_timeEGet();

        /* get current sample count for statistics */
        count = reader->resourceSampleCount;

        v_lifespanAdminTakeExpired(entry->lifespanAdmin,
                                   lifespanArg.time,
                                   lifespanTakeAction,
                                   &lifespanArg);

        /* update statistics */
        if (reader->statistics) {
            delta = count - reader->resourceSampleCount;
            if (delta) {
                assert(delta > 0);
                reader->statistics->numberOfSamplesExpired += (c_ulong) delta;
            }
        }
    }

    /* This routine walks over the purgeLists and checks if any actions
     * have to be done with the instances in the lists. */
    qos = v_reader(reader)->qos;
    purgeList = entry->purgeListNotEmpty;
    if (c_listCount(purgeList) > 0) {
        timestamp = os_timeMSub(now, qos->lifecycle.v.autopurge_nowriter_samples_delay);
        count = reader->resourceSampleCount; /* statistics */
        purgeListItem = c_removeAt(purgeList, 0);
        while (purgeListItem != NULL) {
            equality = os_timeMCompare(purgeListItem->insertionTime,timestamp);
            if ((equality == OS_LESS) || (equality == OS_EQUAL)) {
                purgeInstance = purgeListItem->instance;

                if (os_timeMCompare(purgeListItem->insertionTime,
                                    purgeInstance->purgeInsertionTime) == OS_EQUAL){
                    doInstanceAutoPurge(reader, purgeInstance, -1, purgeListItem->genCount);
                }

                c_free(purgeListItem);
                purgeListItem = c_removeAt(purgeList, 0);
            } else {
               /* the taken instance was not old enough yet and is
                * therefore re-inserted.
                */
               testItem = c_listInsert(purgeList, purgeListItem);
               assert(testItem == purgeListItem);
               OS_UNUSED_ARG(testItem);
               c_free(purgeListItem);
               purgeListItem = NULL;
            }
        }
        /* Update statistics */
        if (reader->statistics) {
            delta = count - reader->resourceSampleCount;
            if (delta) {
                reader->statistics->numberOfSamplesPurgedByNoWriters += (c_ulong) delta;
            }
        }
    }

    purgeList = entry->purgeListDisposed;
    if (c_listCount(purgeList) > 0) {
        timestamp = os_timeMSub(now, qos->lifecycle.v.autopurge_disposed_samples_delay);
        count = reader->resourceSampleCount; /* statistics */
        purgeListItem = c_removeAt(purgeList, 0);
        while (purgeListItem != NULL) {
            equality = os_timeMCompare(purgeListItem->insertionTime,timestamp);
            if ((equality == OS_LESS) || (equality == OS_EQUAL)) {
                purgeInstance = purgeListItem->instance;

                if (os_timeMCompare(purgeListItem->insertionTime,
                                    purgeInstance->purgeInsertionTime) == OS_EQUAL){
                    doInstanceAutoPurge(reader, purgeInstance, purgeListItem->genCount, -1);
                }

                c_free(purgeListItem);
                purgeListItem = c_removeAt(purgeList, 0);
            } else {
               /* the taken instance was not old enough yet and is
                * therefore re-inserted.
                */
               testItem = c_listInsert(purgeList, purgeListItem);
               assert(testItem == purgeListItem);
               c_free(purgeListItem);
               purgeListItem = NULL;
            }
        }
        /* Update statistics. */
        if (reader->statistics) {
            delta = count - reader->resourceSampleCount;
            if (delta) {
                reader->statistics->numberOfSamplesPurgedByDispose += (c_ulong) delta;
            }
        }
    }
    /* Why have statistic attributes if already available? */
    /* Update statistics. */
    if (reader->statistics) {
        reader->statistics->numberOfInstances = (c_ulong) v_dataReaderInstanceCount(reader);
        reader->statistics->numberOfSamples = (c_ulong) reader->resourceSampleCount;
    }
}

static v_writeResult
doWrite(
    v_dataReaderEntry _this,
    v_dataReaderInstance found,
    v_message message,
    v_messageContext context,
    os_boolean *a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted)
{
    v_dataReaderResult res;
    v_readerQos qos;
    v_writeResult result;
    v_dataReader reader;

    result = V_WRITE_SUCCESS;
    reader = v_dataReader(v_entryReader(_this));
    qos = v_reader(reader)->qos;

    if(found){
        v_state oldState = v_dataReaderInstanceState(found);
        c_bool wasEmpty = v_dataReaderInstanceEmpty(found);
        res = v_dataReaderInstanceInsert(found, message, context);
        v_checkMaxSamplesWarningLevel(v_objectKernel(reader),
                (c_ulong) v_dataReader(reader)->resourceSampleCount);

        switch (res) {
        case V_DATAREADER_INSERTED:
            UPDATE_READER_STATISTICS(_this,found,oldState);
            if (qos->userKey.v.enable == FALSE) {
                if (!v_dataReaderInstanceEmpty(found)) {
                    if (wasEmpty && !v_dataReaderInstanceInNotEmptyList(found)) {
                        c_tableInsert(_this->index->notEmptyList, found);
                        v_dataReaderInstanceInNotEmptyList(found) = TRUE;
                    }
                    if (v_dataReaderInstanceStateTest(found,L_DISPOSED)) {
                        if (!v_gidIsValid(message->writerGID) &&
                            qos->lifecycle.v.autopurge_dispose_all) {
                            /* This is a dispose_all and the instance needs to be
                             * purged immediately.
                             * Dispose all is indicated by the lack of a writerGID. */
                            doInstanceAutoPurge(v_dataReader(reader),
                                                found,
                                                found->disposeCount,
                                                -1);
                            if (a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted) {
                                *a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted = OS_TRUE;
                            }
                        } else if (!OS_DURATION_ISINFINITE(qos->lifecycle.v.autopurge_disposed_samples_delay)) {
                            purgeListInsert(_this->purgeListDisposed, found);
                        }
                    } else if (v_dataReaderInstanceStateTest(found,L_NOWRITERS)) {
                        if (!OS_DURATION_ISINFINITE(qos->lifecycle.v.autopurge_nowriter_samples_delay)) {
                            purgeListInsert(_this->purgeListNotEmpty, found);
                        }
                    }
                } else if (v_dataReaderInstanceStateTest(found,L_NOWRITERS) &&
                        !v_stateTest(v_nodeState(message), L_REGISTER)) {
                    v_dataReaderRemoveInstance(v_dataReader(reader),
                        v_dataReaderInstance(found));
                }
            } else {
                v_dataReaderInstanceInNotEmptyList(found) = TRUE;
            }
            result = V_WRITE_SUCCESS;
        break;
        case V_DATAREADER_INSTANCE_FULL:
            onSampleRejected(reader, S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT, v_publicGid(NULL));
            result = V_WRITE_REJECTED;
        break;
        case V_DATAREADER_FILTERED_OUT:
        case V_DATAREADER_NOT_OWNER:
        case V_DATAREADER_OUTDATED:
        case V_DATAREADER_DUPLICATE_SAMPLE:
            result = V_WRITE_SUCCESS;
        break;
        case V_DATAREADER_SAMPLE_LOST:
            v_dataReaderNotifySampleLost(reader, 1);
            result = V_WRITE_SUCCESS;
        break;
        case V_DATAREADER_OUT_OF_MEMORY:
            /* Return rejection to force a retransmit at a later moment of time. */
            result = V_WRITE_REJECTED;
        break;
        case V_DATAREADER_MAX_SAMPLES:
            onSampleRejected(v_dataReader(reader),
                             S_REJECTED_BY_SAMPLES_LIMIT,
                             v_publicGid(NULL));
            result = V_WRITE_REJECTED;
        break;
        default:
            result = V_WRITE_REJECTED;
        break;
        }
    }
    /* statistics */
    if (reader->statistics) {
        c_ulong cnt;
        cnt = (c_ulong) v_dataReaderInstanceCount(reader);
        reader->statistics->numberOfInstances = cnt;
        reader->statistics->maxNumberOfInstances.value = cnt;
        cnt = (c_ulong) reader->resourceSampleCount;
        reader->statistics->numberOfSamples = cnt;
        reader->statistics->maxNumberOfSamples.value = cnt;
    }

    if (result==V_WRITE_SUCCESS) {
        if (v_stateTest(v_nodeState(message),L_SYNCHRONOUS)) {
            v_kernel kernel = v_objectKernel(reader);
            v_gid gid = v_publicGid(v_public(reader));
            v_deliveryServiceAckMessage(kernel->deliveryService,message,gid);
        }
    }

    V_MESSAGE_STAMP(message,readerNotifyTime);

    return result;
}

v_writeResult
v_dataReaderEntryWrite(
    v_dataReaderEntry _this,
    v_message message,
    v_instance *instancePtr,
    v_messageContext context)
{
    v_writeResult result = V_WRITE_REJECTED;
    v_dataReader reader;
    v_readerQos qos;
    v_state state;
    v_dataReaderInstance instance=NULL, found;
    c_table instanceSet;
    c_ulong index = 0;
    c_ulong length;
    v_subscriber subscriber;
    c_bool complete = FALSE;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));
    assert(message != NULL);

    V_MESSAGE_STAMP(message,readerInsertTime);

    /* Only write if the message is not produced by an incompatible writer. */
    reader = v_dataReader(v_entryReader(_this));
    v_observerLock(v_observer(reader));
    subscriber = v_readerSubscriber(v_reader(reader));
    if (!subscriber) {
        /* The subscriber is detached meaning that the reader is in progress of being deleted.
         * Abort message insertion and return success as if the reader was already deleted.
         */
        v_observerUnlock(v_observer(reader));
        return V_WRITE_SUCCESS; /* reader is in progress of being deleted */
    }

    /* Purge samples */
    v_dataReaderEntryUpdatePurgeLists(_this);

    /* If the specified readerInstance has just been purged, then drop our
     * reference to it, and let the Reader implicitly create a new one for us.
     */
    if (instancePtr && *instancePtr && v_dataReaderInstanceStateTest(*instancePtr, L_REMOVED)) {
        c_free(*instancePtr);
        *instancePtr = NULL;
    }

    qos = v_reader(reader)->qos;
    state = v_nodeState(message);

    if (qos->userKey.v.enable) {
        if (!v_stateTest(state, L_WRITE)) {
            /* by user defined keys the instance state has no meaning and
             * therefore processing can be saved by filtering out all instance
             * state control messages.
             */
            /* Note: write_dispose has the side effect that the instance state
             * for subscriber defined keys is set.
             */
            v_observerUnlock(v_observer(reader));
            return V_WRITE_SUCCESS;
        }
    }
    /* Filter-out all QoS-incompatible messages. */
    if (!v_messageQos_isReaderCompatible(message->qos,v_reader(reader))) {
        v_observerUnlock(v_observer(reader));
        return V_WRITE_SUCCESS;
    }

    if (reader->statistics) {
        reader->statistics->numberOfSamplesArrived++;
    }

    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        os_boolean a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted = OS_FALSE;

        /* Drop invalid messages when the datareader instance is not known */
        if (c_getType(message) == v_kernelType(v_objectKernel(_this), K_MESSAGE)) {
            v_observerUnlock(v_observer(reader));
            return V_WRITE_SUCCESS;
        }

        instance = v_dataReaderInstanceNew(v_dataReader(reader),message);
        if (!instance) {
            OS_REPORT(OS_CRITICAL,
                      "v_dataReaderEntry::v_dataReaderEntryWrite",V_RESULT_INTERNAL_ERROR,
                      "Failed to allocate v_dataReaderInstance object.");
            /* failed to create a new instance. */
            v_observerUnlock(v_observer(reader));
            return V_WRITE_OUT_OF_RESOURCES;
        }
        if (_this->filterInstance) {
            c_bool key_filter = TRUE; /* only key fields in filter */
            c_bool pass = TRUE;
            length = c_arraySize(_this->filterInstance);
            for (index=0; index<length; index++) {
                c_bool DataMatch = TRUE;
                c_bool KeyMatch = TRUE;

                if (_this->filterData[index]) {
                    if (v_messageStateTest(message, L_WRITE)) {
                        DataMatch = v_filterEval(_this->filterData[index],message);
                    }
                    key_filter = FALSE;
                }
                if ((DataMatch == TRUE)&&(_this->filterInstance[index])) {
                    KeyMatch = v_filterEval(_this->filterInstance[index],instance);
                }
                if ((pass = (DataMatch && KeyMatch)) == TRUE) {
                    break;
                }
            }
            if (pass == FALSE) {
                c_bool is_transaction = v_message_isTransaction(message);
                /* a filter that is only operates on key fields will block all data of complying instances.
                 * so avoid creation of instance and pipeline for key values not passing a pure key filter
                 * except for transactions which require data for completeness verification.
                 */
                if (v_stateTest(state, L_WRITE) || (key_filter && !is_transaction)) {
                    if (is_transaction && _this->transactionAdmin) {
                        (void)v_transactionAdminInsertMessage(_this->transactionAdmin, message, NULL, FALSE, &complete);
                        if (complete && v__readerIsGroupCoherent(v_reader(reader))) {
                            v_subscriberTriggerGroupCoherent(subscriber, v_reader(reader));
                        }
                    }
                    v_dataReaderInstanceFree(instance);
                    v_observerUnlock(v_observer(reader));
                    return V_WRITE_SUCCESS;
                }
            }
        }
        assert(c_refCount(instance) == 2);
        if (qos->userKey.v.enable) {
            instanceSet = _this->index->notEmptyList;
        } else {
            instanceSet = _this->index->objects;
        }
        assert(c_refCount(instance) == 2);
        found = c_tableInsert(instanceSet, instance);
        if (found == instance) {
            /* Instance did not yet exist */
            assert(c_refCount(found) == 3);
#if 0
    /* The max_instance limit is disabled because it rejects the register message from the group
     * meaning that the pipeline will not be build which results in a permanent disconnect for this instance
     * even if later on room for new instances becomes available.
     * see OSPL-22
     */
            if (qos->resource.v.max_instances != V_LENGTH_UNLIMITED) {
                reader->maxInstances = (qos->resource.v.max_instances < (c_long)c_tableCount(instanceSet));
            }
#endif
            if (reader->maxInstances == TRUE) {
                /* The maximum number of instances was already reached.
                 * Therefore the instance was inserted unnecessarily and must
                 * be removed. */
                found = c_remove(instanceSet, instance,NULL,NULL);
                assert(found == instance);
                c_free(instance);
                v_dataReaderInstanceFree(instance);
                found = NULL;
                onSampleRejected(reader, S_REJECTED_BY_INSTANCES_LIMIT, v_publicGid(NULL));
                result = V_WRITE_REJECTED;
                if (v_message_isTransaction(message) || v_messageStateTest(message, L_ENDOFTRANSACTION)) {
                    if (_this->transactionAdmin) {
                        (void)v_transactionAdminInsertMessage(_this->transactionAdmin, message, NULL, TRUE, &complete);
                        if (complete && v__readerIsGroupCoherent(v_reader(reader))) {
                            v_subscriberTriggerGroupCoherent(subscriber, v_reader(reader));
                        }
                    }
                }
            } else if (v_messageStateTest(message,L_UNREGISTER)) {
                /* There is no use case to support implicit unregister.
                 * Therefore the instance was inserted unnecessarily and can
                 * be removed. */
                found = c_remove(instanceSet, instance,NULL,NULL);
                assert(found == instance);
                c_free(instance);
                v_dataReaderInstanceFree(instance);
                found = NULL;
                result = V_WRITE_SUCCESS;
            } else {
                assert(c_refCount(found) == 3);
                assert(c_refCount(instance) == 3);
                /* The reader statistics are updated for the newly inserted
                 * instance (with its initial values). The previous state
                 * was nothing, so 0 is passed as the oldState. Officially,
                 * state 0 is ALIVE, but instances are created with flag
                 * L_NOWRITERS set, this change triggers an unwanted de-
                 * crement of the Alive-counter. This special case has to be
                 * handled in the statistics updating. */
                UPDATE_READER_STATISTICS(_this,found,0);
                V_MESSAGE_STAMP(message,readerLookupTime);
                result = doWrite(_this, found, message, context,
                                 &a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted);
            }
        } else {
            /* c_tableInsert returned an existing instance.
             * The instance is not yet kept by c_tableInsert so keep it
             * to fulfill the following invariant.
             */
            v_dataReaderInstanceFree(instance);
            V_MESSAGE_STAMP(message,readerLookupTime);
            result = doWrite(_this, found, message, context,
                             &a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted);
        }
        if (instancePtr && !a_work_around_for_dispose_all_to_indicate_the_instance_is_deleted) {
            *instancePtr = c_keep(found);
        }
    } else {
        if (_this->filterInstance) {
            c_bool key_filter = TRUE; /* only key fields in filter */
            c_bool pass = TRUE;
            length = c_arraySize(_this->filterInstance);
            for (index=0; index<length; index++) {
                c_bool DataMatch = TRUE;
                c_bool KeyMatch = TRUE;

                if (_this->filterData[index]) {
                    if (v_messageStateTest(message, L_WRITE)) {
                        DataMatch = v_filterEval(_this->filterData[index],message);
                    }
                    key_filter = FALSE;
                }
                if ((DataMatch == TRUE)&&(_this->filterInstance[index])) {
                    KeyMatch = v_filterEval(_this->filterInstance[index],v_dataReaderInstance(*instancePtr));
                }
                if ((pass = (DataMatch && KeyMatch)) == TRUE) {
                    break;
                }
            }
            if (pass == FALSE) {
                c_bool is_transaction = v_message_isTransaction(message);
                /* a filter that is only operates on key fields will block all data of complying instances.
                 * so avoid creation of instance and pipeline for key values not passing a pure key filter
                 * except for transactions which require data for completeness verification.
                 */
                if (v_stateTest(state, L_WRITE) || (key_filter && !is_transaction)) {
                    if (is_transaction && _this->transactionAdmin) {
                        (void)v_transactionAdminInsertMessage(_this->transactionAdmin, message, NULL, FALSE, &complete);
                        if (complete && v__readerIsGroupCoherent(v_reader(reader))) {
                            v_subscriberTriggerGroupCoherent(subscriber, v_reader(reader));
                        }
                    }
                    v_observerUnlock(v_observer(reader));
                    return V_WRITE_SUCCESS;
                }
            }
        }
        V_MESSAGE_STAMP(message,readerLookupTime);
        result = doWrite(_this, v_dataReaderInstance(*instancePtr), message, context, NULL);
    }
    v_observerUnlock(v_observer(reader));

    return result;
}

v_writeResult
v_dataReaderEntryWriteEOT(
    v_dataReaderEntry _this,
    v_message message)
{
    v_reader reader = v_entryReader(_this);
    v_subscriber subscriber = NULL;
    c_bool complete = FALSE;

    v_observerLock(v_observer(reader));
    subscriber = v_readerSubscriber(reader);
    /* Filter-out all QoS-incompatible messages. */
    if (subscriber && _this->transactionAdmin) {
        if (v_messageQos_isReaderCompatible(message->qos,reader)) {
            (void)v_transactionAdminInsertMessage(_this->transactionAdmin, message, NULL, FALSE, &complete);
            if (complete && v__readerIsGroupCoherent(reader)) {
                v_subscriberTriggerGroupCoherent(subscriber, reader);
            }
        }
    }
    v_observerUnlock(v_observer(reader));

    return V_WRITE_SUCCESS;
}

void
v_dataReaderEntryFlushTransactionNoLock(
    v_instance instance,
    v_message message,
    c_voidp arg)
{
    v_dataReaderInstance rinst;
    v_dataReaderEntry entry;
    v_writeResult result;
    OS_UNUSED_ARG(arg);

    /* Only if the subscriber is still attached, otherwise the reader is in progress of being deleted.  */
    if (instance && v_readerSubscriber(v_reader(v_dataReaderInstanceReader(instance)))) {
        /* Push the message from the transaction into its corresponding instance.
         * Since the sample already consumed resource limits in the transactional
         * administration, no additional resource claim is needed.
         */
        rinst = v_dataReaderInstance(instance);
        entry = v_dataReaderEntry(v_index(rinst->index)->entry);
        result = doWrite(entry, rinst, message, V_CONTEXT_TRANSACTIONFLUSH, NULL);
    }
    OS_UNUSED_ARG(result);
}

struct collectArg {
    c_bool (*condition)(c_object instance, c_voidp arg);
    c_voidp conditionArg;
    c_iter instances;
};

static c_bool
collectMatchingInstances(
    c_object o,
    c_voidp arg)
{
    struct collectArg *a = (struct collectArg *)arg;

    if (a->condition == NULL || a->condition(o, a->conditionArg)) {
        a->instances = c_iterInsert(a->instances, o);
    }
    return TRUE;
}

v_writeResult
v_dataReaderEntryDisposeAll (
    v_dataReaderEntry _this,
    v_message disposeMsg,
    c_bool (*condition)(c_object instance, c_voidp arg),
    c_voidp arg)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_reader reader;
    c_table set;
    v_dataReaderInstance instance;
    struct collectArg ca;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));

    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

    /* Only if the subscriber is still attached, otherwise the reader is in progress of being deleted.  */
    if (v_readerSubscriber(v_reader(reader))) {

        /* Purge samples */
        v_dataReaderEntryUpdatePurgeLists(_this);

        if (reader->qos->userKey.v.enable) {
            set = _this->index->notEmptyList;
        } else {
            set = _this->index->objects;
        }
        /* Following first collects all instances before performing
         * the doWrite on each instance.
         * A walk over the set of instances and inserting the dispose message
         * would be more elegant because it is less heavy weight than the doWrite.
         * The doWrite operation potentially also writes the message to the network
         * which is not required and the doWrite potentially modifies
         * the set which is not allowed during a walk.
         */
        ca.condition = condition;
        ca.conditionArg = arg;
        ca.instances = NULL;

        (void)c_walk(set, collectMatchingInstances, &ca);
        while ((instance = c_iterTakeFirst(ca.instances)) != NULL) {
            result = doWrite(_this, instance, disposeMsg, V_CONTEXT_GROUPWRITE, NULL);
        }
        c_iterFree(ca.instances);
    }
    v_observerUnlock(v_observer(reader));

    return result;
}

static c_bool
unmarkInstance (
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstance instance = v_dataReaderInstance(o);
    c_ulong flags = *((c_ulong *)arg);

    v_dataReaderInstanceStateClear(instance, flags);

    return TRUE;
}

void
v_dataReaderEntryUnmarkInstanceStates (
    v_dataReaderEntry _this,
    c_ulong flags)
{
    v_reader reader;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));

    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

    c_tableWalk(_this->index->objects, unmarkInstance, &flags);

    v_observerUnlock(v_observer(reader));
}

v_typeRepresentation
v__dataReaderEntryGetTypeRepresentation (
    v_dataReaderEntry _this)
{
    c_char *typeName;
    v_typeRepresentation found;

    typeName = c_metaScopedName(c_metaObject(v_topicDataType(v_dataReaderEntryTopic(_this))));
    found = v_participantLookupTypeRepresentation(v_subscriberParticipant(v_readerSubscriber(v_reader(_this))), typeName);
    os_free(typeName);

    return found;
}

