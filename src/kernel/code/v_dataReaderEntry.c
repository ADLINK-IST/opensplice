/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
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

#include "os_report.h"

static c_type v_publicationHandle_t = NULL;

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
    c_long sampleCount;

    /* Data is expired, remove the sample from its instance
     * NOTE: the sampleCount of index is decreased by this function */
    if (lifespanArg->action != NULL) {
       result = lifespanArg->action(v_readerSample(sample), lifespanArg->arg);
    } else {
       result = TRUE;
    }
    if (result) {
        instance = v_dataReaderInstance(v_readerSample(sample)->instance);
        sampleCount = instance->sampleCount;
        v_dataReaderSampleTake(v_dataReaderSample(sample),NULL,NULL);
        sampleCount -= instance->sampleCount;
        assert(sampleCount >= 0);
        v_dataReader(lifespanArg->index->reader)->sampleCount -= sampleCount;
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
    v_filter filter)
{
    v_kernel kernel;
    v_dataReaderEntry e;

    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(dataReader);
    e = v_dataReaderEntry(v_objectNew(kernel,K_DATAREADERENTRY));
    v_entryInit(v_entry(e), v_reader(dataReader));
    e->topic = c_keep(topic);

    e->filter = c_keep(filter);

    /* Aministration for lifespan of messages */
    e->lifespanAdmin = v_lifespanAdminNew(kernel);
    /* The time-ordered lists for autopurging and garbagecollection */
    e->purgeListNotEmpty = c_listNew(v_kernelType(kernel, K_PURGELISTITEM));
    e->purgeListDisposed = c_listNew(v_kernelType(kernel, K_PURGELISTITEM));

    return e;
}

/* Callback functions for the index write action */

static c_bool
onSampleDumpedAction(
    v_readerSample sample,
    c_voidp arg)
{
    v_dataReaderSampleWipeViews(v_dataReaderSample(sample));
    return TRUE;
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

static void
purgeListInsert(
    c_collection purgeList,
    v_dataReaderInstance instance)
{
    v_purgeListItem item;

    item = c_new(v_kernelType(v_objectKernel(instance),K_PURGELISTITEM));
    item->instance = c_keep(instance);
    item->insertionTime = v_timeGet();
    c_append(purgeList,item);
    instance->purgeInsertionTime = item->insertionTime;
    c_free(item);
}

static void
doInstanceAutoPurge(
    v_dataReader reader,
    v_dataReaderInstance instance,
    c_time purgeTime)
{
    c_long sampleCount;

    if (!v_dataReaderInstanceEmpty(instance)) {
        /* Remove all samples from the instance */
        sampleCount = v_dataReaderInstanceSampleCount(instance);
        v_dataReaderInstancePurge(instance);
        sampleCount -= v_dataReaderInstanceSampleCount(instance);
        assert(sampleCount >= 0);
        assert(v_dataReaderInstanceEmpty(instance));
        reader->sampleCount -= sampleCount;
        assert(reader->sampleCount >= 0);
    }
    v_dataReaderRemoveInstance(reader,instance);
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
    c_equality eq;
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
        eq = c_timeCompare(now, qos->lifecycle.autopurge_nowriter_samples_delay);
        if (eq == C_GT) {
            timestamp = c_timeSub(now, qos->lifecycle.autopurge_nowriter_samples_delay);
            count = reader->sampleCount; /* statistics */
            purgeListItem = c_removeAt(purgeList, 0);
            while (purgeListItem != NULL) {
                if (v_timeCompare(purgeListItem->insertionTime,timestamp) == C_LT) {
                    purgeInstance = purgeListItem->instance;
                    if (v_timeCompare(purgeListItem->insertionTime,
                                      purgeInstance->purgeInsertionTime) == C_EQ){
                        doInstanceAutoPurge(reader, purgeInstance, now);
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
    }
    purgeList = entry->purgeListDisposed;
    if (c_listCount(purgeList) > 0) {
        eq = c_timeCompare(now, qos->lifecycle.autopurge_disposed_samples_delay);
        if (eq == C_GT) {
            timestamp = c_timeSub(now, qos->lifecycle.autopurge_disposed_samples_delay);
            count = reader->sampleCount; /* statistics */
            purgeListItem = c_removeAt(purgeList, 0);
            while (purgeListItem != NULL) {
                if (v_timeCompare(purgeListItem->insertionTime,timestamp) == C_LT) {
                    purgeInstance = purgeListItem->instance;
                    if (v_timeCompare(purgeListItem->insertionTime,
                                      purgeInstance->purgeInsertionTime) == C_EQ){
                        doInstanceAutoPurge(reader, purgeInstance, now);
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

    assert(o != NULL);
    assert(*o == NULL); /* out param */

    *o = c_keep(found);

    return FALSE;
}
#endif

v_writeResult
v_dataReaderEntryWrite(
    v_dataReaderEntry _this,
    v_message message,
    v_instance *instancePtr)
{
    v_writeResult result = V_WRITE_REJECTED;
    v_dataReaderResult res;
    v_reader reader;
    v_readerQos qos;
    c_long count;
    c_long maxSamples;
    v_state state;
    v_dataReaderInstance instance, found;
    c_table instanceSet;

    assert(C_TYPECHECK(_this,v_dataReaderEntry));
    assert(message != NULL);

    V_MESSAGE_STAMP(message,readerInsertTime);

    /* Only write if the message is not produced by an incompatible writer. */
    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

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
        /* For Subscriber defined keys always filter-out all QoS-incompatible
         * messages.
         */
        if (!v_messageQos_isReaderCompatible(message->qos,reader)) {
            v_observerUnlock(v_observer(reader));
            return V_WRITE_SUCCESS;
        }
    } else {
        /* Filter-out all QoS-incompatible messages. */
        if (!v_messageQos_isReaderCompatible(message->qos,reader)) {
            v_observerUnlock(v_observer(reader));
            return V_WRITE_SUCCESS;
        }
    }

    v_statisticsULongValueInc(v_reader, numberOfSamplesArrived, reader);

    /* Execute content based filter (if set).
     * For now do not filter on register/unregister messages. The filter must
     * be split into a key part and a non-key part. For register/unregister
     * messages only the key part of the filter needs to be evaluated.
     * Functionally it is not a problem by always allowing instances
     * regardless of the filter, it will only consume resources (memory),
     * which was not needed.
     */
    if (_this->filter != NULL) {
        if (((v_stateTest(state, L_WRITE)) ||
             (v_stateTest(state, L_DISPOSED))) &&
            (!v_filterEval(_this->filter,message))) {
            v_observerUnlock(v_observer(reader));
            return V_WRITE_SUCCESS;
        }
    }

    /*** Try to free resources and check the max samples limit.
         If no resources then report message rejected. ***/

    v_dataReaderEntryUpdatePurgeLists(_this);

    maxSamples = qos->resource.max_samples;
    count = v_dataReader(reader)->sampleCount;
    assert(count >= 0);

    /* Reject data if max sample limit is reached. */
    if ((maxSamples != V_LENGTH_UNLIMITED) &&
        (count >= maxSamples) &&
        v_stateTest(state, L_WRITE))
    {
        onSampleRejected(v_dataReader(reader),
                         S_REJECTED_BY_SAMPLES_LIMIT,
                         v_publicGid(NULL));
        v_observerUnlock(v_observer(reader));
        return V_WRITE_REJECTED;
    }

    result = V_WRITE_SUCCESS;
    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        instance = v_dataReaderInstanceNew(v_dataReader(reader),message);
        if (instance) {
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
                     * Therefore the instance was inserted undeserved and must
                     * be removed. */
                    found = c_remove(instanceSet, instance,NULL,NULL);
                    assert(found == instance);
                    v_dataReaderInstanceFree(found);
                    assert(c_refCount(found) == 1);
                    found = NULL;
                    onSampleRejected(v_dataReader(reader),
                                     S_REJECTED_BY_SAMPLES_LIMIT,
                                     v_publicGid(NULL));
                    result = V_WRITE_REJECTED;

                    assert(c_refCount(instance) == 1);
                } else if (v_messageStateTest(message,L_UNREGISTER)) {
                    /* There is no use case to support implicit unregister.
                     * Therefore the instance was inserted unnecessary and can
                     * be removed. */
                    found = c_remove(instanceSet, instance,NULL,NULL);
                    assert(found == instance);
                    v_dataReaderInstanceFree(found);
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
            v_dataReaderInstanceFree(instance);
        } else {
            /* failed to create a new instance. */
            found = NULL;
            result = V_WRITE_REJECTED;
        }
    } else {
        /* Use the provided instance.
         * The instance need to be kept because it will be freed at the end of
         * this method.
         */
#ifndef NDEBUG
        if (qos->userKey.enable) {
            instanceSet = _this->index->notEmptyList;
        } else {
            instanceSet = _this->index->objects;
        }
        found = NULL;
        c_tableRemove(instanceSet, *instancePtr, alwaysFalse, &found);
        assert(found == *instancePtr);
#else
        found = c_keep(*instancePtr);
#endif
    }

    V_MESSAGE_STAMP(message,readerLookupTime);

    /* Invariant: found == NULL or is locally kept. */
    if ((result == V_WRITE_SUCCESS) && (found)) {
        /* An instance is found so the message can be inserted. */
        v_state oldState = v_dataReaderInstanceState(found);
        c_bool wasEmpty = v_dataReaderInstanceEmpty(found);
        v_dataReader(reader)->sampleCount -=
                 v_dataReaderInstanceSampleCount(found);
        res = v_dataReaderInstanceInsert(found,message);
        v_dataReader(reader)->sampleCount +=
                 v_dataReaderInstanceSampleCount(found);
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
                        if (!c_timeIsInfinite(qos->lifecycle.autopurge_disposed_samples_delay))
                        {
                            purgeListInsert(_this->purgeListDisposed, found);
                        }
                    } else if (v_dataReaderInstanceStateTest(found,L_NOWRITERS)) {
                        if (!c_timeIsInfinite(qos->lifecycle.autopurge_nowriter_samples_delay))
                        {
                            purgeListInsert(_this->purgeListNotEmpty, found);
                        }
                    }
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
        case V_DATAREADER_NOT_OWNER:
        case V_DATAREADER_OUTDATED:
            result = V_WRITE_SUCCESS;
        break;
        default:
            result = V_WRITE_REJECTED;
        break;
        }
        if ((instancePtr) && (*instancePtr == NULL)) {
            *instancePtr = v_instance(found); /* transfer keep */
        } else {
            v_dataReaderInstanceFree(found);
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

    V_MESSAGE_STAMP(message,readerNotifyTime);
    v_observerUnlock(v_observer(reader));

    return result;
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
