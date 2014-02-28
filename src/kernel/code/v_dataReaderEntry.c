/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "v_kernel.h"
#include "v__entry.h"
#include "v_dataReaderEntry.h"
#include "v__dataReader.h"
#include "v_reader.h"
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
#include "v_time.h"
#include "v_instance.h"
#include "v__deadLineInstanceList.h"
#include "v__lifespanAdmin.h"
#include "v_participant.h"
#include "v_subscriber.h"
#include "v_builtin.h"
#include "v_topic.h"
#include "v__statisticsInterface.h"
#include "v_message.h"
#include "v__messageQos.h"
#include "v__deliveryService.h"
#include "v__kernel.h"

#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/
typedef struct v_lifespanArg_s {
   v_readerSampleAction action;
   c_voidp arg;
   v_index index;
   c_time time;
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
       result = lifespanArg->action(v_readerSample(sample), lifespanArg->arg);
    } else {
       result = TRUE;
    }
    if (result) {
        instance = v_dataReaderInstance(v_readerSample(sample)->instance);
        v_dataReaderInstanceSampleRemove(instance, v_dataReaderSample(sample));

        assert(v_dataReader(lifespanArg->index->reader)->sampleCount >= 0);
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
    v_topic topic,
    c_array filterInstance,
    c_array filterData)
{
    v_kernel kernel;
    v_dataReaderEntry e;

    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(dataReader);
    e = v_dataReaderEntry(v_objectNew(kernel,K_DATAREADERENTRY));
    v_entryInit(v_entry(e), v_reader(dataReader));
    e->topic = c_keep(topic);

    e->filterInstance = c_keep(filterInstance);
    e->filterData = c_keep(filterData);

    /* Aministration for lifespan of messages */
    e->lifespanAdmin = v_lifespanAdminNew(kernel);
    /* The time-ordered lists for autopurging and garbagecollection */
    e->purgeListNotEmpty = c_listNew(v_kernelType(kernel, K_PURGELISTITEM));
    e->purgeListDisposed = c_listNew(v_kernelType(kernel, K_PURGELISTITEM));

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
    if (v_statisticsValid(dataReader)) {
        switch (kind) {
        case S_REJECTED_BY_INSTANCES_LIMIT:
            v_statisticsULongValueInc(v_reader,
                                      numberOfSamplesRejectedByInstancesLimit,
                                      dataReader);
        break;
        case S_REJECTED_BY_SAMPLES_LIMIT:
            v_statisticsULongValueInc(v_reader,
                                      numberOfSamplesRejectedBySamplesLimit,
                                      dataReader);
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

C_STRUCT(updateArg) {
    v_message msg;
    c_bool found;
    v_transaction remove;
    v_dataReaderEntry entry;
};

static c_bool
updateTransaction(
    c_object o,
    c_voidp arg)
{
    v_transaction t = (v_transaction)o;
    C_STRUCT(updateArg) *a = (C_STRUCT(updateArg) *)arg;
    c_bool result = TRUE;
    if (t->transactionId == V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(a->msg->transactionId)) {
        if (v_gidEqual(t->writerGID,a->msg->writerGID)) {
            if (v_stateTest(v_nodeState(a->msg),L_TRANSACTION)) {
                t->count -= V_MESSAGE_GET_TRANSACTION_COUNT(a->msg->transactionId);
            } else {
                t->count++;
            }
            a->found = TRUE;
            if (t->count == 0) {
                a->remove = c_keep(t);
            }
        }
    }
    return result;
}

static c_bool
makeSamplesAvailable (
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstanceFlushTransaction(v_dataReaderInstance(o),V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(*(c_ulong *)arg));
    return TRUE;
}

static void
transactionListUpdate(
    v_dataReaderEntry entry,
    v_message msg)
{
    v_transaction item = NULL;
    v_dataReader reader;

    C_STRUCT(updateArg) update;

    if ((msg->transactionId != 0) &&
        (v_reader(v_entry(entry)->reader)->subQos->presentation.coherent_access) &&
        (!v_messageStateTest(msg,L_REGISTER)))
    {
        /* The given message belongs to a transaction and this DataReader
         * is interested in Coherent Updates so update the transaction adminitration.
         */
        update.msg = msg;
        update.found = FALSE;
        update.remove = NULL;
        update.entry = entry;
        /* Lookup and update transaction info record for the message.
         */
        c_walk(entry->transactionList, updateTransaction, &update);

        if (!update.found) {
            /* No existing transaction info is found,
             * So this is the first message for this transaction.
             * Create and insert a new transaction info record for this transaction.
             */
            item = c_new(v_kernelType(v_objectKernel(entry),K_TRANSACTION));
            if (item) {
                item->writerGID = msg->writerGID;
                item->count = 1;
                item->transactionId = V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(msg->transactionId);
                if (v_stateTest(v_nodeState(msg),L_TRANSACTION)) {
                    item->count -= V_MESSAGE_GET_TRANSACTION_COUNT(msg->transactionId);
                }
                if (entry->transactionList == NULL) {
                    entry->transactionList =
                        c_listNew(v_kernelType(v_objectKernel(entry),
                                               K_TRANSACTION));
                }
                c_insert(entry->transactionList,item);
                c_free(item);
            } else {
                OS_REPORT(OS_ERROR,
                          "v_dataReaderEntry::transactionListUpdate",0,
                          "Failed to allocate v_transaction object");
                assert(FALSE);
            }
        } else if (update.remove) {
            /* An existing transaction has become complete and can therefore
             * all data belonging to the transaction can be made available and
             * the transaction info record can be removed from the administration.
             */
            if (v_entryReaderQos(entry)->userKey.enable) {
                c_walk(entry->index->notEmptyList,
                       makeSamplesAvailable,
                       &msg->transactionId);
            } else {
                c_walk(entry->index->objects,
                       makeSamplesAvailable,
                       &msg->transactionId);
            }

            /* At this point we have made the data belonging to the transaction
             * available, but we still need to wakeup threads that are blocked
             * on waitsets for the data.
             * This is what we do here by calling v_dataReaderNotifyDataAvailable
             * with a NULL sample, since there is not a specific sample associated
             * with a transaction */

            reader = v_dataReader(v_entry(entry)->reader);
            v_dataReaderNotifyDataAvailable (reader, NULL);

            item = c_remove(entry->transactionList, update.remove, NULL, NULL);
            c_free(item);
            c_free(update.remove);
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
    if (item) {
        item->instance = c_keep(instance);
        item->insertionTime = v_timeGet();
        if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
            item->genCount = instance->disposeCount;
        } else {
            item->genCount = instance->noWritersCount;
        }
        c_append(purgeList,item);
        instance->purgeInsertionTime = item->insertionTime;
        c_free(item);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_dataReaderEntry::purgeListInsert",0,
                  "Failed to allocate v_purgeListItem object");
        assert(FALSE);
    }
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
        reader->sampleCount -= sampleCount;
        assert(reader->sampleCount >= 0);
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
    c_time now, timestamp;
    c_list purgeList;
    v_purgeListItem purgeListItem, testItem;
    v_dataReaderInstance purgeInstance;
    v_readerQos qos;
    v_dataReader reader;
    c_ulong count, delta; /* statistics */

    reader = v_dataReader(v_entry(entry)->reader);

    /* Purge all instances that are not alive and expired */
    if ((c_listCount(entry->purgeListNotEmpty) > 0) ||
        (c_listCount(entry->purgeListDisposed) > 0) ||
        (v_lifespanAdminSampleCount(entry->lifespanAdmin) > 0)) {
        now = v_timeGet();
    } else {
        return;
    }

    if (v_lifespanAdminSampleCount(entry->lifespanAdmin) > 0) {
        /* Walk over samples in lifespan administration in order to determine if
         * they have to be discarded */
        lifespanArg.action = onSampleDumpedAction;
        lifespanArg.arg = reader;
        lifespanArg.index = entry->index;
        lifespanArg.time = now;

        /* get current sample count for statistics */
        count = reader->sampleCount;

        v_lifespanAdminTakeExpired(entry->lifespanAdmin,
                                   lifespanTakeAction,
                                   &lifespanArg);

        /* update statistics */
        if (v_statisticsValid(reader)) {
            delta = count - reader->sampleCount;
            if (delta) {
                assert(delta > 0);
                *(v_statisticsGetRef(v_reader,
                                     numberOfSamplesExpired,
                                     reader)) += delta;
            }
        }
    }

    /* This routine walks over the purgeLists and checks if any actions
     * have to be done with the instances in the lists. */
    qos = v_reader(reader)->qos;
    purgeList = entry->purgeListNotEmpty;
    if (c_listCount(purgeList) > 0) {
        timestamp = c_timeSub(now, qos->lifecycle.autopurge_nowriter_samples_delay);
        count = reader->sampleCount; /* statistics */
        purgeListItem = c_removeAt(purgeList, 0);
        while (purgeListItem != NULL) {
            if (v_timeCompare(purgeListItem->insertionTime,timestamp) == C_LT) {
                purgeInstance = purgeListItem->instance;

                if (v_timeCompare(purgeListItem->insertionTime,
                                  purgeInstance->purgeInsertionTime) == C_EQ){
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
               c_free(purgeListItem);
               purgeListItem = NULL;
            }
        }
        /* Update statistics */
        if (v_statisticsValid(reader)) {
        delta = count - reader->sampleCount;
            if (delta) {
                assert(delta > 0);
                *(v_statisticsGetRef(v_reader,
                                     numberOfSamplesPurgedByNoWriters,
                                     reader)) += delta;
            }
        }
    }

    purgeList = entry->purgeListDisposed;
    if (c_listCount(purgeList) > 0) {
        timestamp = c_timeSub(now, qos->lifecycle.autopurge_disposed_samples_delay);
        count = reader->sampleCount; /* statistics */
        purgeListItem = c_removeAt(purgeList, 0);
        while (purgeListItem != NULL) {
            if (v_timeCompare(purgeListItem->insertionTime,timestamp) == C_LT) {
                purgeInstance = purgeListItem->instance;

                if (v_timeCompare(purgeListItem->insertionTime,
                                  purgeInstance->purgeInsertionTime) == C_EQ){
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
        if (v_statisticsValid(reader)) {
            delta = count - reader->sampleCount;
            if (delta) {
                assert(delta > 0);
                v_statisticsULongValueAdd(v_reader,
                                          numberOfSamplesPurgedByDispose,
                                          reader,
                                          delta);
            }
        }
    }
    /* Why have statistic attributes if already available? */
    /* Update statistics. */
    v_statisticsULongSetValue(v_reader,
                              numberOfInstances,
                              reader,
                              v_dataReaderInstanceCount(reader));
    v_statisticsULongSetValue(v_reader,
                              numberOfSamples,
                              reader,
                              reader->sampleCount);
}

#ifndef NDEBUG
static c_bool
alwaysFalse(
    c_object found,
    c_object requested,
    c_voidp arg)
{
    c_object *o = (c_object *)arg;
    OS_UNUSED_ARG(requested);

    assert(o != NULL);
    assert(*o == NULL); /* out param */

    *o = c_keep(found);

    return FALSE;
}
#endif

static v_writeResult
doWrite(
    v_dataReaderEntry _this,
    v_dataReaderInstance found,
    v_message message)
{
    v_dataReaderResult res;
    v_readerQos qos;
    v_writeResult result;
    v_reader reader;

    result = V_WRITE_SUCCESS;
    reader = v_entryReader(_this);
    qos = reader->qos;

    if(found){
        v_state oldState = v_dataReaderInstanceState(found);
        c_bool wasEmpty = v_dataReaderInstanceEmpty(found);
        v_dataReader(reader)->sampleCount -=
                 v_dataReaderInstanceSampleCount(found);
        res = v_dataReaderInstanceInsert(found, message);
        v_dataReader(reader)->sampleCount +=
                 v_dataReaderInstanceSampleCount(found);
        v_checkMaxSamplesWarningLevel(v_objectKernel(reader),
                v_dataReader(reader)->sampleCount);


        switch (res) {
        case V_DATAREADER_INSERTED:
            UPDATE_READER_STATISTICS(_this->index,found,oldState);
            if (qos->userKey.enable == FALSE) {
                if (!v_dataReaderInstanceEmpty(found)) {
                    if (wasEmpty && !v_dataReaderInstanceInNotEmptyList(found)) {
                        c_tableInsert(_this->index->notEmptyList, found);
                        v_dataReaderInstanceInNotEmptyList(found) = TRUE;
                    }
                    if (v_dataReaderInstanceStateTest(found,L_DISPOSED)) {
                        if (!c_timeIsInfinite(qos->lifecycle.autopurge_disposed_samples_delay)) {
                            purgeListInsert(_this->purgeListDisposed, found);
                        }
                    } else if (v_dataReaderInstanceStateTest(found,L_NOWRITERS)) {
                        if (!c_timeIsInfinite(qos->lifecycle.autopurge_nowriter_samples_delay)) {
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
            onSampleRejected(v_dataReader(reader),
                             S_REJECTED_BY_INSTANCES_LIMIT,
                             v_publicGid(NULL));
            result = V_WRITE_REJECTED;
        break;
        case V_DATAREADER_FILTERED_OUT:
        case V_DATAREADER_NOT_OWNER:
        case V_DATAREADER_OUTDATED:
        case V_DATAREADER_DUPLICATE_SAMPLE:
            result = V_WRITE_SUCCESS;
        break;
        case V_DATAREADER_SAMPLE_LOST:
            v_dataReaderUpdateSampleLost(reader, 1);
            result = V_WRITE_SUCCESS;
        break;
        case V_DATAREADER_OUT_OF_MEMORY:
            /* Return rejection to force a retransmit at a later moment of time. */
            result = V_WRITE_REJECTED;
        break;
        default:
            result = V_WRITE_REJECTED;
        break;
        }
    }
    /* statistics */
    {
        c_long cnt;
        cnt = v_dataReaderInstanceCount(v_dataReader(reader));
        v_statisticsULongSetValue(v_reader,numberOfInstances,reader,cnt);
        v_statisticsMaxValueSetValue(v_reader,maxNumberOfInstances,reader,cnt);
        cnt = v_dataReader(reader)->sampleCount;
        v_statisticsULongSetValue(v_reader,numberOfSamples,reader,cnt);
        v_statisticsMaxValueSetValue(v_reader,maxNumberOfSamples,reader,cnt);
    }

    if (result==V_WRITE_SUCCESS) {
        if (v_stateTest(v_nodeState(message),L_SYNCHRONOUS)) {
            v_kernel kernel = v_objectKernel(reader);
            v_gid gid = v_publicGid(v_public(reader));
            v_deliveryServiceAckMessage(kernel->deliveryService,message,gid);
        }
        transactionListUpdate(_this,message);
    }

    V_MESSAGE_STAMP(message,readerNotifyTime);

    return result;
}

v_writeResult
v_dataReaderEntryWrite(
    v_dataReaderEntry _this,
    v_message message,
    v_instance *instancePtr)
{
    v_writeResult result = V_WRITE_REJECTED;
    v_reader reader;
    v_readerQos qos;
    c_long count;
    c_long maxSamples;
    v_state state;
    v_dataReaderInstance instance=NULL, found;
    c_table instanceSet;
    c_bool filter = FALSE;
    int index = 0;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));
    assert(message != NULL);

    V_MESSAGE_STAMP(message,readerInsertTime);

    /* Only write if the message is not produced by an incompatible writer. */
    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

    /* Purge samples */
    v_dataReaderEntryUpdatePurgeLists(_this);

    /* If the specified readerInstance has just been purged, then drop our
     * reference to it, and let the Reader implicitly create a new one for us.
     */
    if (instancePtr && *instancePtr && v_dataReaderInstanceStateTest(*instancePtr, L_REMOVED)) {
        c_free(*instancePtr);
        *instancePtr = NULL;
    }

    qos = reader->qos;
    state = v_nodeState(message);

    if (qos->userKey.enable) {
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
    if (!v_messageQos_isReaderCompatible(message->qos,reader)) {
        v_observerUnlock(v_observer(reader));
        return V_WRITE_SUCCESS;
    }

    v_statisticsULongValueInc(v_reader, numberOfSamplesArrived, reader);

    if (v_messageStateTest(message,L_TRANSACTION)) {
        /* The message is a transaction end message.
         * Store the transaction size in the transaction admin.
         */
        transactionListUpdate(_this,message);
        /*
         * This message is meaningless for further processing so exit here.
         */
        v_observerUnlock(v_observer(reader));
        return V_WRITE_SUCCESS;
    }

    if ((instancePtr == NULL) || (*instancePtr == NULL))
    {
        instance = v_dataReaderInstanceNew(v_dataReader(reader),message);
        if (!instance)
        {
            OS_REPORT(OS_ERROR,
                      "v_dataReaderEntry::v_dataReaderEntryWrite",0,
                      "Failed to allocate v_dataReaderInstance object.");
            /* failed to create a new instance. */
            v_observerUnlock(v_observer(reader));
            return V_WRITE_OUT_OF_RESOURCES;
        }
    }

    for (;index < c_arraySize(_this->filterInstance); index++)
    {
        c_bool DataMatch = TRUE;
        c_bool KeyMatch = TRUE;

        if ((v_stateTest(state, L_WRITE))&&(_this->filterData)&&(_this->filterData[index]))
        {
            DataMatch = v_filterEval(_this->filterData[index],message);
        }

        if ((DataMatch == TRUE) &&(_this->filterInstance[index]))
        {
            KeyMatch = v_filterEval(_this->filterInstance[index],instance==NULL?v_dataReaderInstance(*instancePtr):instance);
        }
        filter = (DataMatch && KeyMatch);
        if (filter == TRUE)
        {
            break;
        }
    }
    if (filter != TRUE)
    {
        transactionListUpdate(_this,message);
        v_observerUnlock(v_observer(reader));
        c_free(instance);
        return V_WRITE_SUCCESS;
    }

    /* Check the max samples limit. If no resources then report message rejected. */
    maxSamples = qos->resource.max_samples;
    count = v_dataReader(reader)->sampleCount;
    assert(count >= 0);

    /* Reject data if max sample limit is reached. */
    if ((maxSamples != V_LENGTH_UNLIMITED) &&
        (count >= maxSamples) &&
        (v_stateTest(state, L_WRITE)))
    {
        onSampleRejected(v_dataReader(reader),
                         S_REJECTED_BY_SAMPLES_LIMIT,
                         v_publicGid(NULL));
        v_observerUnlock(v_observer(reader));
        c_free(instance);
        return V_WRITE_REJECTED;
    }


    result = V_WRITE_SUCCESS;
    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        assert(c_refCount(instance) == 1);
        if (qos->userKey.enable) {
            instanceSet = _this->index->notEmptyList;
        } else {
            instanceSet = _this->index->objects;
        }
        assert(c_refCount(instance) == 1);
        found = c_tableInsert(instanceSet, instance);
        if (found == instance) {
            /* Instance did not yet exist */
            assert(c_refCount(found) == 2);
            if (v_dataReader(reader)->maxInstances == TRUE) {
                /* The maximum number of instances was already reached.
                 * Therefore the instance was inserted unnecessarily and must
                 * be removed. */
                found = c_remove(instanceSet, instance,NULL,NULL);
                assert(found == instance);
                c_free(found);
                assert(c_refCount(found) == 1);
                found = NULL;
                onSampleRejected(v_dataReader(reader),
                                 S_REJECTED_BY_SAMPLES_LIMIT,
                                 v_publicGid(NULL));
                result = V_WRITE_REJECTED;

                assert(c_refCount(instance) == 1);
            } else if (v_messageStateTest(message,L_UNREGISTER)) {
                /* There is no use case to support implicit unregister.
                 * Therefore the instance was inserted unnecessarily and can
                 * be removed. */
                found = c_remove(instanceSet, instance,NULL,NULL);
                assert(found == instance);
                c_free(found);
                assert(c_refCount(found) == 1);
                found = NULL;
                result = V_WRITE_SUCCESS;
                assert(c_refCount(instance) == 1);
            } else {
                v_publicInit(v_public(found));
                c_keep(found);
                assert(c_refCount(found) == 4);
                assert(c_refCount(instance) == 4);
                /* The reader statistics are updated for the newly inserted
                 * instance (with its initial values). The previous state
                 * was nothing, so 0 is passed as the oldState. Officially,
                 * state 0 is ALIVE, but instances are created with flag
                 * L_NOWRITERS set, this change triggers an unwanted de-
                 * crement of the Alive-counter. This special case has to be
                 * handled in the statistics updating. */
                UPDATE_READER_STATISTICS(_this->index,found,0);
            }
        } else {
            /* c_tableInsert returned an existing instance.
             * The instance is not yet kept by c_tableInsert so keep it
             * to fullfil the following invariant.
             */
            c_keep(found);
        }
        c_free(instance);
    } else {
        /* Use the provided instance.
         * The instance need to be kept because it will be freed at the end of
         * this method.
         */
        c_free(instance);
#ifndef NDEBUG
        if (qos->userKey.enable) {
            instanceSet = _this->index->notEmptyList;
        } else {
            instanceSet = _this->index->objects;
        }
        found = NULL;
        c_tableRemove(instanceSet, *instancePtr, alwaysFalse, &found);
        assert(found == v_dataReaderInstance(*instancePtr));
#else
        found = c_keep(*instancePtr);
#endif
    }

    V_MESSAGE_STAMP(message,readerLookupTime);

    /* Invariant: found == NULL or is locally kept. */
    if ((result == V_WRITE_SUCCESS) && (found)) {
        result = doWrite(_this, found, message);

        if ((instancePtr) && (*instancePtr == NULL)) {
            *instancePtr = v_instance(found); /* transfer keep */
        } else {
            c_free(found);
        }
    }
    v_observerUnlock(v_observer(reader));

    return result;
}

C_CLASS(disposeAllArg);
C_STRUCT(disposeAllArg) {
    v_writeResult result;
    v_message disposeMsg;
    v_dataReaderEntry entry;
};

static c_bool
disposeAll (
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstance instance = v_dataReaderInstance(o);
    disposeAllArg a = (disposeAllArg)arg;

    a->result = doWrite(a->entry, instance, a->disposeMsg);

    return (a->result != V_WRITE_REJECTED);
}

v_writeResult
v_dataReaderEntryDisposeAll (
    v_dataReaderEntry _this,
    v_message disposeMsg)
{
    C_STRUCT(disposeAllArg) disposeArg;
    v_reader reader;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));

    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

    /* Purge samples */
    v_dataReaderEntryUpdatePurgeLists(_this);

    disposeArg.result = V_WRITE_SUCCESS;
    disposeArg.disposeMsg = disposeMsg;
    disposeArg.entry = _this;

    if (reader->qos->userKey.enable) {
        c_tableWalk(_this->index->notEmptyList, disposeAll, &disposeArg);
    } else {
        c_tableWalk(_this->index->objects, disposeAll, &disposeArg);
    }
    v_observerUnlock(v_observer(reader));

    return disposeArg.result;
}

static c_bool
markInstance (
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstance instance = v_dataReaderInstance(o);
    c_ulong flags = *((c_ulong *)arg);

    v_dataReaderInstanceStateSet(instance, flags);

    return TRUE;
}

void
v_dataReaderEntryMarkInstanceStates (
    v_dataReaderEntry _this,
    c_ulong flags)
{
    v_reader reader;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));

    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

    c_tableWalk(_this->index->objects, markInstance, &flags);

    v_observerUnlock(v_observer(reader));
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

v_dataReaderResult
v_dataReaderEntryApplyUnregisterMessageToInstanceList (
    v_dataReaderEntry _this,
    v_message unregisterMsg,
    c_iter instanceList)
{
    v_reader reader;
    v_dataReaderInstance drInst;
    v_dataReaderResult result =  V_DATAREADER_INSERTED;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));

    reader = v_entryReader(_this);

    /* Walk over all instances, and unregister each of them. */
    v_observerLock(v_observer(reader));
    drInst = v_dataReaderInstance(c_iterTakeFirst(instanceList));
    while (drInst != NULL &&
            result != V_DATAREADER_OUT_OF_MEMORY &&
            result != V_DATAREADER_INTERNAL_ERROR)
    {
        result = v_dataReaderInstanceInsert(drInst, unregisterMsg);
        drInst = v_dataReaderInstance(c_iterTakeFirst(instanceList));
    }
    v_observerUnlock(v_observer(reader));

    return result;
}

/**************************************************************
 * Protected functions
 **************************************************************/
static c_bool
removeSamples (
    c_object o,
    c_voidp arg)
{
    v_dataReaderInstanceAbortTransaction(v_dataReaderInstance(o),*(c_ulong *)arg);
    return TRUE;
}

static void
abortTransaction (
    v_dataReaderEntry entry,
    c_ulong transactionId)
{
    c_table instanceSet;
    v_readerQos qos;
    v_dataReader reader;

    reader = v_dataReader(v_entry(entry)->reader);
    qos = v_reader(reader)->qos;
    if (qos->userKey.enable) {
        instanceSet = entry->index->notEmptyList;
    } else {
        instanceSet = entry->index->objects;
    }
    c_walk(instanceSet,removeSamples,&transactionId);
}

C_STRUCT(findTransactionIdArg) {
    v_gid writerGID;
    v_dataReaderEntry entry;
};
C_CLASS(findTransactionIdArg);

static c_bool
findTransactionId(
    c_object o,
    c_voidp arg)
{
    v_transaction t = (v_transaction)o;
    findTransactionIdArg a = (findTransactionIdArg)arg;
    c_bool result = TRUE;

    if (v_gidEqual(a->writerGID,t->writerGID)) {
        abortTransaction(a->entry,t->transactionId);
    }
    return result;
}

void
v_dataReaderEntryAbortTransaction(
    v_dataReaderEntry _this,
    v_gid writerGID)
{
    C_STRUCT(findTransactionIdArg) arg;

    arg.writerGID = writerGID;
    arg.entry = _this;
    c_walk(_this->transactionList, findTransactionId, &arg);
}

/**************************************************************
 * Public functions
 **************************************************************/
