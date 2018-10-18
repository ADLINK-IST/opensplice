/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__persistentDataListener.h"
#include "d__admin.h"
#include "d__configuration.h"
#include "d__nameSpace.h"
#include "d__listener.h"
#include "d__durability.h"
#include "d__waitset.h"
#include "d__misc.h"
#include "d__table.h"
#include "d__thread.h"
#include "d__group.h"
#include "d__subscriber.h"
#include "d__eventListener.h"
#include "d__element.h"
#include "d_store.h"
#include "d_qos.h"
#include "u_groupQueue.h"
#include "u_entity.h"
#include "u_observable.h"
#include "v_observer.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_partition.h"
#include "v_groupQueue.h"
#include "v_event.h"
#include "v_state.h"
#include "v_time.h"
#include "c_laptime.h"
#include "os_report.h"
#include "os_thread.h"
#include "os_abstract.h"
#include "os_atomics.h"

/**
 * Macro that checks the d_persistentDataListener validity.
 * Because d_persistentDataListener is a concrete class typechecking is required.
 */
#define             d_persistentDataListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_PERSISTENT_DATA_LISTENER)

/**
 * \brief The d_persistentDataListener cast macro.
 *
 * This macro casts an object to a d_persistentDataListener object.
 */
#define d_persistentDataListener(_this) ((d_persistentDataListener)(_this))

/**
 * \brief The d_persistentGroup cast macro.
 *
 * This macro casts an object to a d_persistentGroup object.
 */
#define d_persistentGroup(_this) ((d_persistentGroup)(_this))
C_CLASS(d_persistentGroup);

C_STRUCT(d_persistentGroup){
    v_group group;
    c_ulong count;
};

struct persistentStatistics {
    pa_uint32_t samplesStored;
    pa_uint32_t samplesLifespanExpired;
    pa_uint32_t instancesDisposed;
    pa_uint32_t instancesCleanupDelayExpired;
    pa_uint32_t instancesRegistered;
    pa_uint32_t instancesUnregistered;
    pa_uint32_t eventsDeleteHistoricalData;
    pa_uint32_t eventsDisposeAll;
    pa_uint32_t eventsTransactionComplete;
    os_uint32 storeActions;
    os_duration maxStoreTime;
    os_duration minStoreTime;
    os_duration maxTotalTime;
    os_duration minTotalTime;
    os_duration avgTotalTime;
    os_duration avgStoreTime;
};

struct takeData {
    d_persistentDataListener listener;
    d_store persistentStore;
    d_durability durability;
};

C_STRUCT(d_persistentDataListener){
    C_EXTENDS(d_listener);
    u_groupQueue queue;
    struct persistentStatistics pstats;
    os_duration totalTime;
    os_uint32 totalActions;
    pa_uint32_t runCount;
    os_mutex pmutex;
    os_cond pcond;
    os_mutex pauseMutex;
    os_cond pauseCond;
    d_waitsetEntity waitsetData;
    d_table groups;
    c_ulong optimizeUpdateInterval;
    d_storeResult lastResult;
    c_bool logStatistics;
    c_bool sessionExpired;    /* indicates whether the StoreSessionTime has expired or not; used in SMP */
};

static d_storeResult
d_storeGroupAction(
    v_groupAction msg,
    d_persistentDataListener listener,
    d_store persistentStore,
    os_duration waitTime)
{
    d_storeResult result;
    os_timeE startTime = os_timeEGet();
    d_thread self = d_threadLookupSelf ();
    os_uint64 attempt = 0;
    d_durability durability = d_threadsDurability();

    switch(msg->kind){
        case V_GROUP_ACTION_WRITE:
            do {
                result = d_storeMessageStore(persistentStore, msg);

                if (result == D_STORE_RESULT_OK) {
                    pa_inc32(&(listener->pstats.samplesStored));
                } else if (result == D_STORE_RESULT_PRECONDITION_NOT_MET) {
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeMessageStore for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_DISPOSE:
            do {
                result = d_storeInstanceDispose(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.instancesDisposed));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeInstanceDispose for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);

            break;
        case V_GROUP_ACTION_LIFESPAN_EXPIRE:
            do {
                result = d_storeMessageExpunge(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.samplesLifespanExpired));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeMessageExpunge for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE:
            do {
                result = d_storeInstanceExpunge(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.instancesCleanupDelayExpired));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeInstanceExpunge for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_DELETE_DATA:
            do {
                result = d_storeDeleteHistoricalData(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.eventsDeleteHistoricalData));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeDeleteHistoricalData for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_REGISTER:
            do {
                result = d_storeInstanceRegister(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.instancesRegistered));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeInstanceRegister for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_UNREGISTER:
            do {
                result = d_storeInstanceUnregister(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.instancesUnregistered));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeInstanceUnregister for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_TRANSACTION_COMPLETE :
            do {
                result = d_storeTransactionComplete(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_inc32(&(listener->pstats.eventsTransactionComplete));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    attempt++;
                    d_sleep(self, waitTime);
                }
                /* Print log message in case PRECONDITION_NOT_MET has occurred, but only first and last time */
                if (((attempt == 1) && (result == D_STORE_RESULT_PRECONDITION_NOT_MET)) ||
                    ((attempt != 0) && (result == D_STORE_RESULT_OK))) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "d_storeTransactionComplete for group %s %s (attempt: %"PA_PRIu64")\n", v_groupName(msg->group), (result == D_STORE_RESULT_OK) ? "succeeded" : "failed", attempt);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        default:
            OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                "storeGroupAction: Unknown or unsupported group action received (%d)",
                msg->kind);
            assert(FALSE);
            result = D_STORE_RESULT_ERROR;
            break;
    }
    if(listener->logStatistics == TRUE){
        os_timeE curTime;
        os_duration storeTime, totalTime;
        os_duration delta;

        curTime = os_timeEGet();
        storeTime = os_timeEDiff(curTime, startTime);
        totalTime = os_timeEDiff(curTime, msg->actionTime);

        d_listenerLock(d_listener(listener));

        listener->pstats.storeActions++;

        if(listener->pstats.storeActions == 1){
            listener->pstats.avgStoreTime = storeTime;
        } else {
            delta = os_durationSub(storeTime, listener->pstats.avgStoreTime) / listener->pstats.storeActions;
            listener->pstats.avgStoreTime += delta;
        }

        if(storeTime > listener->pstats.maxStoreTime){
            listener->pstats.maxStoreTime = storeTime;
        }
        if(storeTime < listener->pstats.minStoreTime){
            listener->pstats.minStoreTime = storeTime;
        }

        if(listener->pstats.storeActions == 1){
            listener->pstats.avgTotalTime = totalTime;
        } else {
            delta = os_durationSub(totalTime, listener->pstats.avgTotalTime) / listener->pstats.storeActions;
            listener->pstats.avgTotalTime += delta;
        }

        if(totalTime > listener->pstats.maxTotalTime){
            listener->pstats.maxTotalTime = totalTime;
        }
        if(totalTime < listener->pstats.minTotalTime){
            listener->pstats.minTotalTime = totalTime;
        }
        d_listenerUnlock(d_listener(listener));
    }
    if(result != D_STORE_RESULT_OK){
        if(listener->lastResult != result){
            OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                "storeGroupAction: Error in handling action on persistent storage. Action: '%d' and Reason: '%d'\n",
                msg->kind,
                result);
            listener->lastResult = result;
        }
    }
    return result;
}

/* Output statistics to tracing. */
static void d_persistentDataListenerOutputStats(
    d_durability durability,
    d_persistentDataListener listener,
    struct persistentStatistics* oldStats,
    os_duration totalTime)
{
    c_ulong samplesStored, samplesLifespanExpired;
    c_ulong instancesDisposed, instancesCleanupDelayExpired;
    c_ulong instancesRegistered, instancesUnregistered;
    c_ulong eventsDeleteHistoricalData, eventsDisposeAll;
    c_ulong eventsTransactionComplete;
    c_ulong total;

    samplesStored = pa_ld32(&listener->pstats.samplesStored) - pa_ld32(&oldStats->samplesStored);
    samplesLifespanExpired = pa_ld32(&listener->pstats.samplesLifespanExpired) - pa_ld32(&oldStats->samplesLifespanExpired);
    instancesDisposed = pa_ld32(&listener->pstats.instancesDisposed) - pa_ld32(&oldStats->instancesDisposed);
    instancesCleanupDelayExpired = pa_ld32(&listener->pstats.instancesCleanupDelayExpired) - pa_ld32(&oldStats->instancesCleanupDelayExpired);
    instancesRegistered = pa_ld32(&listener->pstats.instancesRegistered) - pa_ld32(&oldStats->instancesRegistered);
    instancesUnregistered = pa_ld32(&listener->pstats.instancesUnregistered) - pa_ld32(&oldStats->instancesUnregistered);
    eventsDeleteHistoricalData = pa_ld32(&listener->pstats.eventsDeleteHistoricalData) - pa_ld32(&oldStats->eventsDeleteHistoricalData);
    eventsDisposeAll = pa_ld32(&listener->pstats.eventsDisposeAll) - pa_ld32(&oldStats->eventsDisposeAll);
    eventsTransactionComplete = pa_ld32(&listener->pstats.eventsTransactionComplete) - pa_ld32(&oldStats->eventsTransactionComplete);
    total = samplesStored + samplesLifespanExpired + instancesDisposed + instancesCleanupDelayExpired +
            instancesRegistered + instancesUnregistered + eventsDeleteHistoricalData + eventsDisposeAll;

    if(total != 0){
        listener->totalActions += total;
        listener->totalTime = os_durationAdd(listener->totalTime, totalTime);

        if (listener->logStatistics) {

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Current:\n" \
                "samplesStored=%u, " \
                "samplesLifespanExpired=%u, " \
                "instancesDisposed=%u, " \
                "instancesCleanupDelayExpired=%u, " \
                "instancesRegistered=%u, " \
                "instancesUnregistered=%u, " \
                "eventsDeleteHistoricalData=%u, " \
                "eventsDisposeAll=%u, " \
                "eventsTranactionComplete=%u\n", \
                samplesStored,
                samplesLifespanExpired,
                instancesDisposed,
                instancesCleanupDelayExpired,
                instancesRegistered,
                instancesUnregistered,
                eventsDeleteHistoricalData,
                eventsDisposeAll,
                eventsTransactionComplete);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Current: performed %d persistent store actions in %.6f seconds (%.2f actions/sec)\n",
                total, os_durationToReal(totalTime), ((double)total)/os_durationToReal(totalTime));

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Total: performed %d persistent store actions in %.6f seconds (%.2f actions/sec)\n",
                listener->totalActions, os_durationToReal(listener->totalTime),
                ((double)listener->totalActions)/os_durationToReal(listener->totalTime));

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Time to get sample persisted after writing: "
                "avg: %.3f ms, min: %.3f ms, max:  %.3f ms\n",
                os_durationToReal(listener->pstats.avgTotalTime) * 1000.000000,
                os_durationToReal(listener->pstats.minTotalTime) * 1000.000000,
                os_durationToReal(listener->pstats.maxTotalTime) * 1000.000000);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Time to get sample persisted after receiving: "
                "avg: %.3f ms, min: %.3f ms, max:  %.3f ms\n",
                os_durationToReal(listener->pstats.avgStoreTime) * 1000.000000,
                os_durationToReal(listener->pstats.minStoreTime) * 1000.000000,
                os_durationToReal(listener->pstats.maxStoreTime)* 1000.000000);
        }
    }

}

static void
d_getKernelEntity(
    v_public entity,
    c_voidp args)
{
    v_entity* e;

    e = (v_entity*)args;
    *e = v_entity(entity);
}

static d_persistentGroup
d_persistentGroupNew(
    const v_group group)
{
    d_persistentGroup g = NULL;

    g = d_persistentGroup(os_malloc(C_SIZEOF(d_persistentGroup)));

    if(g){
        g->count     = 0;

        if(group){
            g->group = c_keep(group);
        } else {
            g->group = NULL;
        }
    }
    return g;
}

static void
d_persistentGroupFree(
    d_persistentGroup g)
{
    assert(g);
    assert(g->group);

    if(g->group){
        c_free (g->group);
    }
    os_free(g);
}

static int
d_persistentGroupCompare(
    d_persistentGroup g1,
    d_persistentGroup g2)
{
    int result;

    assert(g1);
    assert(g2);

    result = strcmp(v_partitionName(v_groupPartition(g1->group)),
                    v_partitionName(v_groupPartition(g2->group)));

    if(result == 0){
        result = strcmp(v_topicName(v_groupTopic(g1->group)),
                        v_topicName(v_groupTopic(g2->group)));
    }
    return result;
}

struct optimizeGroups {
    d_store store;
    d_admin admin;
    c_ulong optimizePoint;
};

static c_bool
optimize(
    d_persistentGroup group,
    c_voidp args)
{
    d_group localGroup;
    struct optimizeGroups *og;
    d_durability durability;
    os_duration duration;
    c_bool terminate;
    c_bool result = TRUE;

    og = (struct optimizeGroups *)args;


    if (group->count >= og->optimizePoint) {
        localGroup = d_adminGetLocalGroup(
                            og->admin,
                            v_partitionName(v_groupPartition(group->group)),
                            v_topicName(v_groupTopic(group->group)),
                            D_DURABILITY_PERSISTENT);
        if(localGroup){
            os_timeM before = os_timeMGet();
            durability = d_adminGetDurability(og->admin);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Start optimization of group '%s.%s' after %d writes.\n",
                                v_partitionName(v_groupPartition(group->group)),
                                v_topicName(v_groupTopic(group->group)),
                                group->count);

            d_storeOptimizeGroup(og->store, localGroup);
            duration = os_timeMDiff(os_timeMGet(), before);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Optimization of group '%s.%s' took %f seconds.\n",
                                v_partitionName(v_groupPartition(group->group)),
                                v_topicName(v_groupTopic(group->group)),
                                os_durationToReal(duration));

            group->count = 0;
            terminate = d_durabilityMustTerminate(durability);

            if(terminate){
                result = FALSE;
            }
        }
    }
    return result;
}


static void
d_persistentDataListenerTake(
    v_public entity,
    c_voidp args)
{
    d_thread self = d_threadLookupSelf ();
    v_groupQueue queue;
    v_groupAction msg;
    struct takeData* data;
    struct optimizeGroups og;
    d_storeResult result;
    d_persistentGroup pg, pgOld;
    c_bool doOptimize = FALSE;
    c_bool terminate;
    c_bool sessionExpired;
    os_duration waitInSessionTime = OS_DURATION_INIT(0, 10000000);  /* 10 ms */
    os_duration waitInSleepTime = OS_DURATION_INIT(0, 10000000);  /* 10 ms */
    os_duration waitTime = OS_DURATION_INIT(0, 100000000);  /* 10 ms */
    os_duration totalTime;
    os_timeM startTime, sessionEndTime;
    d_configuration config;
    struct persistentStatistics stats;

    assert(C_TYPECHECK(entity, v_groupQueue));

    data  = (struct takeData*)args;
    queue = v_groupQueue(entity);
    config = d_durabilityGetConfiguration(data->durability);
    /* Set the waitInSessionTime to min(10ms, 0.1 * StoreSessionTime)).
     * This is the time used to check to for expiration of the StoreSessionTime
     * while waiting for data during a session */
    if (os_durationCompare(os_durationMul(config->persistentStoreSessionTime, (double)0.1), waitInSessionTime) == OS_LESS) {
        waitInSessionTime = os_durationMul(config->persistentStoreSessionTime, (double)0.1);
    }
    /* Set the waitInSleepTime to min(10ms, 0.1 * StoreSleepTime)).
     * This is the time used to check for expiration of the StoreSleepTime
     * while in a sleep */
    if (os_durationCompare(os_durationMul(config->persistentStoreSleepTime, (double)0.1), waitInSleepTime) == OS_LESS) {
        waitInSleepTime = os_durationMul(config->persistentStoreSleepTime, (double)0.1);
    }

    terminate = d_durabilityMustTerminate(data->durability);
    while (!terminate && v_groupQueueSize(queue) > 0) {
        memcpy(&stats, &(data->listener->pstats), sizeof(struct persistentStatistics));
        startTime = os_timeMGet();
        sessionEndTime = os_timeMAdd(startTime, config->persistentStoreSessionTime);
        d_storeActionStart(data->persistentStore);

        /* StoreSessionTime loop */
        do {
            d_threadAwake (self);
            msg = v_groupQueueTake(queue);
            if (msg) {
                /* Store message */
                result = d_storeGroupAction(msg,
                    data->listener,
                    data->persistentStore,
                    waitTime);
                /* Set the last result */
                data->listener->lastResult = result;
                if ((result == D_STORE_RESULT_OK) && (data->listener->optimizeUpdateInterval != 0)) {
                    /* create a d_persistentGroupNew() (if not yet exists) */
                    pg = d_persistentGroupNew(msg->group);
                    pgOld = d_tableInsert(data->listener->groups, pg);
                    if (pgOld) {
                        /* persistent group already existed */
                        d_persistentGroupFree(pg);
                        pg = pgOld;
                    }
                    /* update the number of persistent groups */
                    pg->count++;
                    if (pg->count >= data->listener->optimizeUpdateInterval) {
                        doOptimize = TRUE;
                    }
                }
                c_free(msg);
            } else {
                /* The queue was empty, wait a small amount of time
                 * before checking again */
                d_sleep(self, waitInSessionTime);
            }
            sessionExpired = (os_timeMCompare(sessionEndTime, os_timeMGet()) == OS_LESS);
            terminate = d_durabilityMustTerminate(data->durability);
        } while ((!sessionExpired) && (!terminate));

        d_storeActionStop(data->persistentStore);  /* This can take long when */

        /* Output statistics to tracing */
        totalTime = os_timeMDiff(os_timeMGet(), startTime);
        d_persistentDataListenerOutputStats(
           data->durability,
           data->listener,
           &stats,
           totalTime);

        /* If terminate then return.
         * Because the d_storeActionStop can take long we are going
         * to recheck whether to terminate or not. */
        if ((terminate = d_durabilityMustTerminate(data->durability)) == TRUE) {
            return;
        }

        /* Optimize when needed */
        if (doOptimize == TRUE) {
            og.admin = d_listenerGetAdmin(d_listener(data->listener));
            og.store = data->persistentStore;
            og.optimizePoint = data->listener->optimizeUpdateInterval;
            if (!d_tableWalk(data->listener->groups, optimize, &og))
            {
                OS_REPORT(OS_CRITICAL, "d_persistentDataListenerTake", 0,
                          "d_tableWalk failed : inconsistent administration");
                d_durabilityTerminate(data->durability, TRUE);
            }
        }

        /* If terminate then return.
         * Because the optimize can take long we are going
         * to recheck whether to terminate or not. */
        if ((terminate = d_durabilityMustTerminate(data->durability)) == TRUE) {
            return;
        }

        if (os_durationCompare(config->persistentStoreSleepTime, OS_DURATION_ZERO) == OS_MORE) {
            /* StoreSleepTime loop */
            sessionEndTime = os_timeMAdd(os_timeMGet(), config->persistentStoreSleepTime);
            do {
                d_sleep(self, waitInSleepTime);
                sessionExpired = (os_timeMCompare(sessionEndTime, os_timeMGet()) == OS_LESS);
                terminate = d_durabilityMustTerminate(data->durability);
            } while ((!sessionExpired) && (!terminate) && (v_groupQueueSize(queue) == 0));
        }
    } /* while */

    terminate = d_durabilityMustTerminate(data->durability);
    if (terminate) {
         if (queue->size > 0) {
            d_printTimedEvent(data->durability, D_LEVEL_FINEST,
                "d_persistentDataListenerTake: terminating but '%u' messages are still in the queue.\n",
                queue->size);
        }
    }
}


static void
flushRemainingPersistentData(
    d_persistentDataListener listener)
{
    os_timeM now, endTime, commitEndTime;
    os_duration persistDuration;
    os_duration commitDuration = OS_DURATION_INIT(1,0);
    os_duration waitTime = OS_DURATION_INIT(0, 100000000);  /* 100 ms */
    d_durability durability;
    d_admin admin;
    d_configuration config;
    d_thread self = d_threadLookupSelf ();
    c_ulong count = 0;
    v_groupAction msg;
    v_groupQueue queue;
    d_subscriber subscriber;
    d_store store;
    u_result ures;
    c_ulong size;

    admin      = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);
    subscriber = d_adminGetSubscriber(admin);
    store      = d_subscriberGetPersistentStore(subscriber);

    ures = u_observableAction(u_observable(listener->queue), d_getKernelEntity, &queue);
    if (ures != U_RESULT_OK) {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
                          "Out of memory: any remaining persisting data will NOT be saved.\n");
        goto err_get_queue;
    }

    if (os_durationCompare(config->serviceTerminatePeriod, OS_DURATION_ZERO) == OS_EQUAL) {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
                          "The service termination period is 0.0s, any remaining persisting data will NOT be saved.\n");
    } else {
        now = os_timeMGet();
        /* If there is any persistent data in the queue we try
         * to persist the data for a period of 70% of the service
         * termination period. While saving the data we will NOT
         * respect the StoreSessionTime and StoreSleepTime in an
         * attempt to save as much data as possible. To reduce
         * the risk of losing data the commit interval is set to
         * the min(1s, 10% of the time that is available to commit).
         * When persisting the data we do NOT optimize, even if
         * config->optimizeUpdateInterval != 0. This is because
         * we have limited time and want to use all the available
         * time to store data. */
        persistDuration = os_durationMul(config->serviceTerminatePeriod, (double)0.7);
        endTime = os_timeMAdd(now, persistDuration);
        if (os_durationCompare(os_durationMul(persistDuration, (double)0.1), commitDuration) == OS_LESS) {
            commitDuration = os_durationMul(persistDuration, (double)0.1);
        }
        d_printTimedEvent(durability, D_LEVEL_FINER,
                          "Trying to persist any available persistent data for %"PA_PRIduration" seconds\n",
                          OS_DURATION_PRINT(persistDuration));
       /* Set the end marker. Any data AFTER this marker will NOT be persisted. */
       v_groupQueueSetMarker(queue);
       size = v_groupQueueSize(queue);
       msg = v_groupQueueTake(queue);
       now = os_timeMGet();
       while ( (msg != NULL) &&
               (os_timeMCompare(now, endTime) == OS_LESS) ) {
            commitEndTime = os_timeMAdd(now, commitDuration);
            if (os_timeMCompare(endTime, commitEndTime) == OS_LESS) {
                commitEndTime = endTime;
            }
            d_storeActionStart(store);
            while ((msg != NULL) &&
                   (os_timeMCompare(now, commitEndTime) == OS_LESS)) {
                d_threadAwake(self);
                /* store message */
                (void)d_storeGroupAction(msg,
                    listener,
                    store,
                    waitTime);
                count++;
                c_free(msg);
                now = os_timeMGet();
                msg = v_groupQueueTake(queue);
            } /* while */
            d_storeActionStop(store);
            now = os_timeMGet();
        } /* while */
        if (msg == NULL) {
            /* All messages are saved within the service termination period */
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "All (%u of %u) persistent messages stored before the service termination period expired.\n",
                      count, size);
        } else {
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                      "Some (%u of %u) persistent messages stored before the service termination period expired.\n",
                      count, size);
        }
    }

err_get_queue:
    return;
}

static c_bool
addElementExpr (
    d_element element,
    c_voidp userData) {
    c_iter exprList;

    exprList = userData;

    assert(exprList); /* exprList will always exist. */

    c_iterInsert(exprList, d_elementGetExpression(element));

    return TRUE;
}

static c_iter
d_persistentDataListenerGetGroupExpr(
    d_persistentDataListener listener)
{
    d_admin admin;
    d_nameSpace ns;
    d_durabilityKind dkind;
    c_iter exprList;
    c_ulong i;
    c_iter nameSpaces;

    admin = d_listenerGetAdmin(d_listener(listener));
    nameSpaces = NULL;
    exprList = c_iterNew(NULL);

    assert (admin);

    /* Collect namespaces */
    nameSpaces = d_adminNameSpaceCollect(admin);

    for(i=0; i<c_iterLength(nameSpaces); i++){
        ns    = d_nameSpace(c_iterObject(nameSpaces, i));
        dkind = d_nameSpaceGetDurabilityKind(ns);

        /* Only take into account persisting namespaces */
        if((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)){

            /* Walk elements of namespace, for each element add an partition.topic expression to the list. */
            d_nameSpaceElementWalk(ns, addElementExpr, exprList);
        }
    }

    d_adminNameSpaceCollectFree(admin, nameSpaces);

    return exprList;
}

static void
d_persistentDataListenerDeinit(
    d_persistentDataListener listener)
{
    assert(d_persistentDataListener(listener));

    /* Stop the listener */
    d_persistentDataListenerStop(listener);
    /* Flush remaining data */
    flushRemainingPersistentData(listener);
    /* Deallocate */
    if (listener->queue) {
        u_objectFree(u_object(listener->queue));
        listener->queue = NULL;
    }
    d_tableFree(listener->groups);

    os_condDestroy(&(listener->pcond));
    os_condDestroy(&(listener->pauseCond));
    os_mutexDestroy(&(listener->pmutex));
    os_mutexDestroy(&(listener->pauseMutex));
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));
}

static void
freeExpression(
    void* o,
    void* udata)
{
    OS_UNUSED_ARG(udata);
    d_free(o);
}

static void
d_persistentDataListenerFreeGroupExpr(
    c_iter expr)
{
    c_iterWalk(expr, freeExpression, NULL);
    c_iterFree(expr);
}


static void
d_persistentDataListenerInit(
    d_persistentDataListener listener,
    d_subscriber subscriber)
{
    u_subscriber usubscriber;
    d_configuration config;
    d_durability durability;
    d_admin admin;
    v_readerQos qos;
    c_iter expr;

    /* Do not assert the persistentDataListener because
     * the initialization of the listener has not yet completed */

    assert(d_subscriberIsValid(subscriber));
    assert(listener);

    /* Call super-init */
    d_listenerInit(d_listener(listener), D_PERSISTENT_DATA_LISTENER, subscriber, NULL,
                   (d_objectDeinitFunc)d_persistentDataListenerDeinit);
    /* Initialize the persistentDataListener */
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    listener->groups = d_tableNew(d_persistentGroupCompare, d_persistentGroupFree);
    usubscriber = d_subscriberGetPersistentSubscriber(subscriber);
    qos = d_readerQosNew(V_DURABILITY_PERSISTENT, V_RELIABILITY_RELIABLE);
    /* Collect expressions from matching namespaces */
    expr = d_persistentDataListenerGetGroupExpr(listener);
    listener->queue = u_groupQueueNew(usubscriber, "persistentQueue",
                                      config->persistentQueueSize, qos, expr);
    /* Free expressionlist */
    d_persistentDataListenerFreeGroupExpr(expr);

    listener->optimizeUpdateInterval = config->persistentUpdateInterval;
    listener->lastResult             = D_STORE_RESULT_OK;
    d_readerQosFree(qos);
    pa_st32(&listener->pstats.samplesStored, 0);
    pa_st32(&listener->pstats.samplesLifespanExpired, 0);
    pa_st32(&listener->pstats.instancesDisposed, 0);
    pa_st32(&listener->pstats.instancesCleanupDelayExpired, 0);
    pa_st32(&listener->pstats.instancesRegistered, 0);
    pa_st32(&listener->pstats.instancesUnregistered, 0);
    pa_st32(&listener->pstats.eventsDeleteHistoricalData, 0);
    pa_st32(&listener->pstats.eventsDisposeAll, 0);
    pa_st32(&listener->pstats.eventsTransactionComplete, 0);
    listener->pstats.storeActions = 0;
    listener->pstats.avgStoreTime = 0.0;
    listener->pstats.maxStoreTime = 0.0;
    listener->pstats.minStoreTime = 123456789.0;
    listener->pstats.avgTotalTime = 0.0;
    listener->pstats.maxTotalTime = 0.0;
    listener->pstats.minTotalTime = 123456789.0;
    listener->logStatistics = FALSE;

    {
        char *p;
        if ((p = os_getenv ("OSPL_DURABILITY_LOG_STATISTICS")) != NULL && atoi (p) != 0) {
            listener->logStatistics = TRUE;
        }
    }

    os_mutexInit(&(listener->pmutex), NULL);
    os_mutexInit(&(listener->pauseMutex), NULL);

    os_condInit(&(listener->pcond), &(listener->pmutex), NULL);
    os_condInit(&(listener->pauseCond), &(listener->pauseMutex), NULL);

    listener->totalTime = OS_DURATION_ZERO;
    listener->totalActions = 0;
    u_entityEnable(u_entity(listener->queue));
}

d_persistentDataListener
d_persistentDataListenerNew(
    d_subscriber subscriber)
{
    d_persistentDataListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate persistentDataListener object */
    listener = d_persistentDataListener(os_malloc(C_SIZEOF(d_persistentDataListener)));
    if (listener) {
        /* Initialize the persistentDataListener */
        d_persistentDataListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_persistentDataListenerFree(
    d_persistentDataListener listener)
{
    assert(d_persistentDataListener(listener));

    d_objectFree(d_object(listener));
}

static void
setMask (
    v_public p, c_voidp arg)
{
    c_ulong mask = *((c_ulong *)arg);

    v_observerSetEventMask(v_observer(p), mask);
}

c_ulong
d_persistentDataListenerAction(
    u_object o,
    v_waitsetEvent event,
    c_voidp usrData)
{
    d_thread self = d_threadLookupSelf ();
    d_persistentDataListener listener;
    d_admin admin;
    d_subscriber subscriber;
    struct takeData data;
    c_bool terminate;
    d_serviceState state;
    os_duration sleepTime = OS_DURATION_INIT(0, 100000000);  /* 100 ms */

    listener = d_persistentDataListener(usrData);

    if ((event->kind & V_EVENT_DATA_AVAILABLE) ==  V_EVENT_DATA_AVAILABLE) {
        admin = d_listenerGetAdmin(d_listener(listener));
        subscriber = d_adminGetSubscriber(admin);

        data.listener        = listener;
        data.persistentStore = d_subscriberGetPersistentStore(subscriber);
        data.durability      = d_adminGetDurability(admin);

        terminate            = d_durabilityMustTerminate(data.durability);
        state                = d_durabilityGetState(data.durability);

        /*Do not store persistent data before it has been injected*/
        while((terminate == FALSE) &&
              (
                (state == D_STATE_INJECT_PERSISTENT) ||
                (state == D_STATE_INIT) ||
                (state == D_STATE_DISCOVER_PERSISTENT_SOURCE) ||
                (state == D_STATE_DISCOVER_FELLOWS_GROUPS)
              )
             )
        {
            d_sleep(self, sleepTime);
            terminate = d_durabilityMustTerminate(data.durability);
            state     = d_durabilityGetState(data.durability);
        }


        (void)u_observableAction(u_observable(o), d_persistentDataListenerTake, &data);
    }
    return event->kind;
}

c_bool
d_persistentDataListenerStart(
    d_persistentDataListener listener)
{
    c_bool result;
    u_object object;
    u_result ur;

    d_admin admin;
    d_subscriber subscriber;
    c_bool wsResult;
    d_waitset waitset;
    d_waitsetAction action;
    d_durability durability;
    d_configuration configuration;
    c_ulong mask;

    assert(listener);
    result = FALSE;
    assert(d_objectIsValid(d_object(listener), D_LISTENER));

    if(listener){
        d_listenerLock(d_listener(listener));
        if ((object = u_object(listener->queue)) == NULL) {
            d_listenerUnlock(d_listener(listener));
            return FALSE;
        }
        action        = d_persistentDataListenerAction;
        admin         = d_listenerGetAdmin(d_listener(listener));
        durability    = d_adminGetDurability(admin);
        configuration = d_durabilityGetConfiguration(durability);
        subscriber    = d_adminGetSubscriber(admin);

        if(d_listener(listener)->attached == FALSE){
            listener->totalTime = OS_DURATION_ZERO;
            listener->totalActions = 0;

            mask = V_EVENT_DATA_AVAILABLE;
            ur = u_observableAction(u_observable(object), setMask, &mask);

            if(ur == U_RESULT_OK){
                waitset = d_subscriberGetWaitset(subscriber);
                listener->waitsetData = d_waitsetEntityNew(
                            "persistentDataListener",
                            object, action,
                            V_EVENT_DATA_AVAILABLE,
                            configuration->persistentScheduling, listener);
                wsResult = d_waitsetAttach(waitset, listener->waitsetData);

                if(wsResult == TRUE) {
                    ur = U_RESULT_OK;
                } else {
                    ur = U_RESULT_ILL_PARAM;
                }
                if(ur == U_RESULT_OK){
                        d_listener(listener)->attached = TRUE;
                    result = TRUE;
                    d_listenerUnlock(d_listener(listener));
                    (void) u_observableNotify(u_observable(object));
                } else {
                    d_listenerUnlock(d_listener(listener));
                }
            } else {
                d_listenerUnlock(d_listener(listener));
            }
        } else {
            d_listenerUnlock(d_listener(listener));
            result = TRUE;
        }
    }
    return result;
}

c_bool
d_persistentDataListenerStop(
    d_persistentDataListener listener)
{
    c_bool result;
    u_result ur;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_objectIsValid(d_object(listener), D_LISTENER));
    result = FALSE;

    if(listener){
        d_listenerLock(d_listener(listener));

        if(d_listener(listener)->attached == TRUE){
            admin      = d_listenerGetAdmin(d_listener(listener));
            subscriber = d_adminGetSubscriber(admin);
            waitset    = d_subscriberGetWaitset(subscriber);
            result     = d_waitsetDetach(waitset, listener->waitsetData);

            if(result == TRUE) {
                d_waitsetEntityFree(listener->waitsetData);

                os_mutexLock(&(listener->pmutex));
                os_condBroadcast(&(listener->pcond));
                os_mutexUnlock(&(listener->pmutex));

                ur = U_RESULT_OK;
            } else {
                ur = U_RESULT_ILL_PARAM;
            }

            if(ur == U_RESULT_OK){
                d_listener(listener)->attached = FALSE;
                result = TRUE;
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}
