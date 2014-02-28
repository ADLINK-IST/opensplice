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
#include "d_persistentDataListener.h"
#include "d__persistentDataListener.h"
#include "d_admin.h"
#include "d_eventListener.h"
#include "d_durability.h"
#include "d_configuration.h"
#include "d_listener.h"
#include "d_actionQueue.h"
#include "d_nameSpace.h"
#include "d_misc.h"
#include "d_store.h"
#include "d_group.h"
#include "d__group.h"
#include "d_qos.h"
#include "d_waitset.h"
#include "d_table.h"
#include "u_groupQueue.h"
#include "u_entity.h"
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

static d_storeResult
d_storeGroupAction(
    v_groupAction msg,
    d_persistentDataListener listener,
    d_store persistentStore,
    os_time waitTime)
{
    d_storeResult result;

    switch(msg->kind){
        case V_GROUP_ACTION_WRITE:
            do {
                result = d_storeMessageStore(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.samplesStored));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_DISPOSE:
            do {
                result = d_storeInstanceDispose(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.instancesDisposed));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);

            break;
        case V_GROUP_ACTION_LIFESPAN_EXPIRE:
            do {
                result = d_storeMessageExpunge(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.samplesLifespanExpired));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE:
            do {
                result = d_storeInstanceExpunge(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.instancesCleanupDelayExpired));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_DELETE_DATA:
            do {
                result = d_storeDeleteHistoricalData(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.eventsDeleteHistoricalData));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_REGISTER:
            do {
                result = d_storeInstanceRegister(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.instancesRegistered));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        case V_GROUP_ACTION_UNREGISTER:
            do {
                result = d_storeInstanceUnregister(persistentStore, msg);

                if(result == D_STORE_RESULT_OK){
                    pa_increment(&(listener->pstats.instancesUnregistered));
                } else if(result == D_STORE_RESULT_PRECONDITION_NOT_MET){
                    os_nanoSleep(waitTime);
                }
            } while(result == D_STORE_RESULT_PRECONDITION_NOT_MET);
            break;
        default:
            OS_REPORT_1(OS_ERROR, D_CONTEXT, 0,
                    "storeGroupAction: Unknown or unsupported group action received (%d)",
                    msg->kind);
            assert(FALSE);
            result = D_STORE_RESULT_ERROR;
            break;
    }
    if(result != D_STORE_RESULT_OK){
        if(listener->lastResult != result){
            OS_REPORT_2(OS_ERROR, D_CONTEXT, 0,
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
    os_time totalTime)
{
    c_ulong samplesStored, samplesLifespanExpired;
    c_ulong instancesDisposed, instancesCleanupDelayExpired;
    c_ulong instancesRegistered, instancesUnregistered;
    c_ulong eventsDeleteHistoricalData, eventsDisposeAll;
    c_ulong total;

    samplesStored = listener->pstats.samplesStored - oldStats->samplesStored;
    samplesLifespanExpired = listener->pstats.samplesLifespanExpired - oldStats->samplesLifespanExpired;
    instancesDisposed = listener->pstats.instancesDisposed - oldStats->instancesDisposed;
    instancesCleanupDelayExpired = listener->pstats.instancesCleanupDelayExpired - oldStats->instancesCleanupDelayExpired;
    instancesRegistered = listener->pstats.instancesRegistered - oldStats->instancesRegistered;
    instancesUnregistered = listener->pstats.instancesUnregistered - oldStats->instancesUnregistered;
    eventsDeleteHistoricalData = listener->pstats.eventsDeleteHistoricalData - oldStats->eventsDeleteHistoricalData;
    eventsDisposeAll = listener->pstats.eventsDisposeAll - oldStats->eventsDisposeAll;
    total = samplesStored + samplesLifespanExpired + instancesDisposed + instancesCleanupDelayExpired +
            instancesRegistered + instancesUnregistered + eventsDeleteHistoricalData + eventsDisposeAll;

    if(total != 0){
        listener->totalActions += total;
        listener->totalTime = os_timeAdd(listener->totalTime, totalTime);

        if (listener->logStatistics) {

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                D_THREAD_PERISTENT_DATA_LISTENER,
                "Current:\n" \
                "samplesStored=%d, " \
                "samplesLifespanExpired=%d, " \
                "instancesDisposed=%d, " \
                "instancesCleanupDelayExpired=%d, " \
                "instancesRegistered=%d, " \
                "instancesUnregistered=%d, " \
                "eventsDeleteHistoricalData=%d, " \
                "eventsDisposeAll=%d\n",
                samplesStored,
                samplesLifespanExpired,
                instancesDisposed,
                instancesCleanupDelayExpired,
                instancesRegistered,
                instancesUnregistered,
                eventsDeleteHistoricalData,
                eventsDisposeAll);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                D_THREAD_PERISTENT_DATA_LISTENER,
                "Current: performed %d persistent store actions in %.6f seconds (%.2f actions/sec)\n",
                total, os_timeToReal(totalTime), ((double)total)/os_timeToReal(totalTime));

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                D_THREAD_PERISTENT_DATA_LISTENER,
                "Total: performed %d persistent store actions in %.6f seconds (%.2f actions/sec)\n",
                listener->totalActions, os_timeToReal(listener->totalTime),
                ((double)listener->totalActions)/os_timeToReal(listener->totalTime));
        }

        /*
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            D_THREAD_PERISTENT_DATA_LISTENER,
            "Total:\n" \
            "-samplesStored=%d\n" \
            "-samplesLifespanExpired=%d\n" \
            "-instancesDisposed=%d\n" \
            "-instancesCleanupDelayExpired=%d\n" \
            "-instancesRegistered=%d\n" \
            "-instancesUnregistered=%d\n" \
            "-eventsDeleteHistoricalData=%d\n" \
            "-eventsDisposeAll=%d\n",
            listener->pstats.samplesStored,
            listener->pstats.samplesLifespanExpired,
            listener->pstats.instancesDisposed,
            listener->pstats.instancesCleanupDelayExpired,
            listener->pstats.instancesRegistered,
            listener->pstats.instancesUnregistered,
            listener->pstats.eventsDeleteHistoricalData,
            listener->pstats.eventsDisposeAll);
        */
    }

}

/* Start persistent action */
static void
d_persistentDataListenerSMPTake(
    v_entity entity,
    c_voidp args)
{
    struct takeData* data;
    d_persistentDataListener listener;
    d_admin admin;
    d_subscriber subscriber;
    d_durability durability;
    d_configuration config;
    d_store store;
    v_groupQueue queue;
    c_ulong size;
    os_time waitTime, startTime, totalTime, sessionEndTime, sleepTime;
    struct persistentStatistics stats;
    os_result waitResult;

    data  = (struct takeData*)args;
    listener = data->listener;
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    subscriber = d_adminGetSubscriber(admin);
    store = d_subscriberGetPersistentStore(subscriber);
    queue = v_groupQueue(entity);
    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 100000000; /*100ms*/
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000; /* 10ms */

    /* Only start persistent action when samples are available */
    size = v_groupQueueSize(queue);

    if(!d_durabilityMustTerminate(data->durability) && (size > 0)){
        os_mutexLock(&(listener->pmutex));

        /* Listener threads cannot be active outside persistent action */
        if(listener->runCount != 0){
            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                    D_THREAD_PERISTENT_DATA_LISTENER,
                    "Durability administration corrupt!! %d persistent threads active outside persistent action.\n",
                    listener->runCount);
        }

        /* Make copy of old statistics (so delta's can be evaluated at end of action) */
        memcpy(&stats, &(listener->pstats), sizeof(struct persistentStatistics));
        startTime = os_timeGet();

        /* Start persistent action */
        d_storeActionStart(store);

        /* Calculate sessionEndTime - current + the amount of time between d_storeActionStart and d_storeActionStop */
        sessionEndTime = os_timeAdd(os_timeGet(), config->persistentStoreSessionTime);

        while(!d_durabilityMustTerminate(data->durability)) {

            /* Threads will read until mark, so they will not starve because of fluctuating data throughput (there is always data available until mark is reached). */
            v_groupQueueSetMarker(queue);

            listener->runCount = c_iterLength(listener->persistentThreads);
            os_condBroadcast(&(listener->pcond));
            os_mutexUnlock(&(listener->pmutex));

            /* Wait until threads finished processing */
            os_mutexLock(&(listener->pauseMutex));
            while(listener->runCount > 0 && !d_durabilityMustTerminate(data->durability)){
                waitResult = os_condTimedWait(&(listener->pauseCond), &(listener->pauseMutex), &waitTime);
                if (waitResult == os_resultFail)
                {
                    OS_REPORT(OS_CRITICAL, "d_persistentDataListenerSMPTake", 0,
                              "os_condTimedWait failed - thread will terminate");
                    break;
                }
            }
            os_mutexUnlock(&(listener->pauseMutex));

            os_mutexLock(&(listener->pmutex));
            if((listener->runCount != 0) && (!d_durabilityMustTerminate(data->durability))){
                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                        D_THREAD_PERISTENT_DATA_LISTENER,
                        "Durability administration corrupt!! %d persistent threads active outside persistent action.\n",
                        listener->runCount);
            }

            /* If StoreSessionTime has elapsed, break from loop */
            if(os_timeCompare(sessionEndTime, os_timeGet()) == OS_LESS) {
                break;
            }else {
                /* Wait a short amount of time to prevent the thread from spinning */
                if(!d_durabilityMustTerminate(data->durability)) {
                    os_nanoSleep(sleepTime);
                }
            }
        }

        /* Stop action */
        d_storeActionStop(store);

        /* Process statistics */
        totalTime = os_timeSub(os_timeGet(), startTime);

        /* Output stats to tracing */
        d_persistentDataListenerOutputStats(
                durability,
                listener,
                &stats,
                totalTime);

        os_mutexUnlock(&(listener->pmutex));
    }
    return;
}

static void
d_getKernelEntity(
    v_entity entity,
    c_voidp args)
{
    v_entity* e;

    e = (v_entity*)args;
    *e = entity;
}

/* Persistent thread implementation */
static void*
d_smpPersist(
    void* data)
{
    d_persistentDataListener listener;
    d_admin admin;
    d_subscriber subscriber;
    d_store store;
    d_durability durability;
    os_time waitTime;
    v_groupQueue queue;
    c_ulong runCount;
    c_bool terminate;
    v_groupAction msg;
    unsigned int count;
    os_result waitResult;

    count = 0;
    listener = d_persistentDataListener(data);
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    subscriber = d_adminGetSubscriber(admin);
    store = d_subscriberGetPersistentStore(subscriber);
    u_entityAction(u_entity(listener->queue), d_getKernelEntity, &queue);

    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 100000000; /*100ms*/

    terminate = d_durabilityMustTerminate(durability);

    /* Lock condition variable */
    os_mutexLock(&(listener->pmutex));

    while(!terminate){

        terminate = d_durabilityMustTerminate(durability);

        /* Wait for persistent action */
        if(!terminate){
            do {
                waitResult = os_condWait(&(listener->pcond), &(listener->pmutex));
                if (waitResult == os_resultFail)
                {
                   OS_REPORT(OS_CRITICAL, "d_smpPersist", 0,
                             "os_condWait failed - in persistent worker thread");
                   break;
                }
                terminate = d_durabilityMustTerminate(durability);
            } while((listener->runCount == 0) && (!terminate));
        }
        os_mutexUnlock(&(listener->pmutex));

        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                D_THREAD_PERISTENT_DATA_LISTENER,
                "Worker thread awake! Now taking data...\n");

        /* Take samples */
        msg = v_groupQueueTake(queue);

        /* Store all messages */
        while(!terminate && msg){
            count++;
            (void)d_storeGroupAction(msg, listener, store, waitTime);
            c_free(msg);
            msg = v_groupQueueTake(queue);
            terminate = d_durabilityMustTerminate(durability);
        }

        /* Lock condition variable */
        os_mutexLock(&(listener->pmutex));

        if (msg == NULL) {
            /* If no samples are found, decrease runcount */
            runCount = pa_decrement(&listener->runCount);

            /* Wake up main listener thread */
            if(runCount == 0){
                os_mutexLock(&(listener->pauseMutex));
                os_condSignal(&(listener->pauseCond));
                os_mutexUnlock(&(listener->pauseMutex));
            }
        }
    }

    /* Unlock condition variable */
    os_mutexUnlock(&(listener->pmutex));

    return NULL;
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
    os_time duration;
    c_bool terminate;
    c_bool result = TRUE;

    og = (struct optimizeGroups *)args;


    if(group->count >= og->optimizePoint){
        localGroup = d_adminGetLocalGroup(
                            og->admin,
                            v_partitionName(v_groupPartition(group->group)),
                            v_topicName(v_groupTopic(group->group)),
                            D_DURABILITY_PERSISTENT);
        if(localGroup){
            durability = d_adminGetDurability(og->admin);
            duration = os_timeGet();
            d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_PERISTENT_DATA_LISTENER,
                                "Start optimization of group '%s.%s' after %d writes.\n",
                                v_partitionName(v_groupPartition(group->group)),
                                v_topicName(v_groupTopic(group->group)),
                                group->count);

            d_storeOptimizeGroup(og->store, localGroup);
            duration = os_timeSub(os_timeGet(), duration);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                D_THREAD_PERISTENT_DATA_LISTENER,
                                "Optimization of group '%s.%s' took %f seconds.\n",
                                v_partitionName(v_groupPartition(group->group)),
                                v_topicName(v_groupTopic(group->group)),
                                os_timeToReal(duration));

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
    v_entity entity,
    c_voidp args)
{
    v_groupQueue queue;
    v_groupAction msg;
    struct takeData* data;
    struct optimizeGroups og;
    d_storeResult result;
    d_persistentGroup pg, pgOld;
    c_bool doOptimize = FALSE;
    c_bool allDone = FALSE;
    c_bool terminate;
    int i, max;
    os_time sleepTime, sessionEndTime, waitTime, totalTime, startTime;
    d_configuration config;
    struct persistentStatistics stats;

    assert(C_TYPECHECK(entity, v_groupQueue));

    data  = (struct takeData*)args;
    queue = v_groupQueue(entity);
    config = d_durabilityGetConfiguration(data->durability);

    if(config->persistentStoreSleepTime.tv_sec <= 4){
        max = 10;
        sleepTime = os_timeMulReal(config->persistentStoreSleepTime, 0.1e0);
    } else {
        max = 20;
        sleepTime = os_timeMulReal(config->persistentStoreSleepTime, 0.05e0);
    }
    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 100000000; /*100ms*/

    /* Make copy of old statistics (so delta's can be evaluated at end of action) */
    memcpy(&stats, &(data->listener->pstats), sizeof(struct persistentStatistics));

    while(allDone == FALSE){
        msg   = v_groupQueueTake(queue);
        startTime = os_timeGet();
        sessionEndTime = os_timeAdd(startTime, config->persistentStoreSessionTime);
        d_storeActionStart(data->persistentStore);
        terminate = d_durabilityMustTerminate(data->durability);

        while((msg != NULL) && (terminate == FALSE)){
            /* Store message */
            result = d_storeGroupAction(msg,
                data->listener,
                data->persistentStore,
                waitTime);

            if((result == D_STORE_RESULT_OK) && (data->listener->optimizeUpdateInterval != 0)) {
                data->listener->lastResult = result;
                pg = d_persistentGroupNew(msg->group);
                pgOld = d_tableInsert(data->listener->groups, pg);

                if(pgOld){
                    d_persistentGroupFree(pg);
                    pg = pgOld;
                }
                pg->count++;

                if(pg->count >= data->listener->optimizeUpdateInterval){
                    doOptimize = TRUE;
                }
            } else {
                data->listener->lastResult = result;
            }
            if(msg){
                c_free(msg);
            }
            msg = v_groupQueueTake(queue);

            terminate = d_durabilityMustTerminate(data->durability);

            for(i=0; (i<max) && (msg == NULL) && (terminate == FALSE); i++){
                os_nanoSleep(sleepTime);

                if(os_timeCompare(sessionEndTime, os_timeGet()) == OS_LESS){
                    i = max;
                } else {
                    msg = v_groupQueueTake(queue);
                }
                terminate = d_durabilityMustTerminate(data->durability);
            }
        }
        d_storeActionStop(data->persistentStore);

        totalTime = os_timeSub(os_timeGet(), startTime);

        /* Output statistics to tracing */
        d_persistentDataListenerOutputStats(
           data->durability,
           data->listener,
           &stats,
           totalTime);

        terminate = d_durabilityMustTerminate(data->durability);

        if((doOptimize == TRUE) && (terminate == FALSE)){
            og.admin = d_listenerGetAdmin(d_listener(data->listener));
            og.store = data->persistentStore;
            og.optimizePoint = data->listener->optimizeUpdateInterval;
            allDone = d_tableWalk(data->listener->groups, optimize, &og);
            terminate = d_durabilityMustTerminate(data->durability);
        } else {
            allDone = TRUE;
        }
        if(terminate){
            allDone = TRUE;
        }
    }
    terminate = d_durabilityMustTerminate(data->durability);

    if(terminate == TRUE){
        if(queue->size > 0){
            d_printTimedEvent(data->durability, D_LEVEL_FINEST,
                            D_THREAD_PERISTENT_DATA_LISTENER,
                            "d_persistentDataListenerTake: terminating but '%u' messages are still in the queue.\n",
                            queue->size);
        }
    }

    return;
}

d_persistentDataListener
d_persistentDataListenerNew(
    d_subscriber subscriber)
{
    d_persistentDataListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_persistentDataListener(os_malloc(C_SIZEOF(d_persistentDataListener)));
        d_listener(listener)->kind = D_PERSISTENT_DATA_LISTENER;
        d_persistentDataListenerInit(listener, subscriber);
    }
    return listener;
}

c_bool
addElementExpr (
    d_element element,
    c_voidp userData) {
    c_iter exprList;

    exprList = userData;

    assert(exprList); /* exprList will always exist. */

    c_iterInsert(exprList, d_elementGetExpression(element));

    return TRUE;
}

c_iter
d_persistentDataListenerGetGroupExpr(
    d_persistentDataListener listener)
{
    d_admin admin;
    d_nameSpace ns;
    d_durabilityKind dkind;
    c_iter exprList;
    c_long i;
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

void
freeExpression(
    void* o,
    void* udata)
{
    OS_UNUSED_ARG(udata);
    d_free(o);
}

void
d_persistentDataListenerFreeGroupExpr(
    c_iter expr)
{
    c_iterWalk(expr, freeExpression, NULL);
    c_iterFree(expr);
}

void
d_persistentDataListenerInit(
    d_persistentDataListener listener,
    d_subscriber subscriber)
{
    u_subscriber usubscriber;
    d_configuration config;
    d_durability durability;
    d_admin admin;
    v_readerQos qos;
    os_mutexAttr mattr;
    os_condAttr cattr;
    c_iter expr;

    d_listenerInit(d_listener(listener), subscriber, NULL, d_persistentDataListenerDeinit);
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);

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

    listener->persistentThreads = c_iterNew(NULL);
    listener->pstats.samplesStored = 0;
    listener->pstats.samplesLifespanExpired = 0;
    listener->pstats.instancesDisposed = 0;
    listener->pstats.instancesCleanupDelayExpired = 0;
    listener->pstats.instancesRegistered = 0;
    listener->pstats.instancesUnregistered = 0;
    listener->pstats.eventsDeleteHistoricalData = 0;
    listener->pstats.eventsDisposeAll = 0;
    listener->logStatistics = FALSE;

    {
        char *p;
        if ((p = os_getenv ("OSPL_DURABILITY_LOG_STATISTICS")) != NULL && atoi (p) != 0) {
            listener->logStatistics = TRUE;
        }
    }

    os_mutexAttrInit(&mattr);
    mattr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit(&(listener->pmutex), &mattr);
    os_mutexInit(&(listener->pauseMutex), &mattr);

    os_condAttrInit(&cattr);
    cattr.scopeAttr = OS_SCOPE_PRIVATE;
    os_condInit(&(listener->pcond), &(listener->pmutex), &cattr);
    os_condInit(&(listener->pauseCond), &(listener->pauseMutex), &cattr);

    listener->totalTime.tv_sec = 0;
    listener->totalTime.tv_nsec = 0;
    listener->totalActions = 0;
}

void
d_persistentDataListenerDeinit(
    d_object object)
{
    d_persistentDataListener listener;

    assert(d_objectIsValid(object, D_LISTENER));

    if(object){
        listener = d_persistentDataListener(object);
        assert(d_listenerIsValid(d_listener(listener), D_PERSISTENT_DATA_LISTENER));
        d_persistentDataListenerStop(listener);

        if(listener->queue){
            u_groupQueueFree(listener->queue);
            listener->queue = NULL;
        }
        d_tableFree(listener->groups);

        if(listener->persistentThreads){
            c_iterFree(listener->persistentThreads);
            listener->persistentThreads = NULL;
        }
        os_condDestroy(&(listener->pcond));
        os_condDestroy(&(listener->pauseCond));
        os_mutexDestroy(&(listener->pmutex));
        os_mutexDestroy(&(listener->pauseMutex));
    }
}

void
d_persistentDataListenerFree(
    d_persistentDataListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_PERSISTENT_DATA_LISTENER));

    if(listener){
        d_listenerFree(d_listener(listener));
    }
}

c_bool
d_persistentDataListenerStart(
    d_persistentDataListener listener)
{
    c_bool result;
    u_dispatcher dispatcher;
    u_result ur;

    d_admin admin;
    d_subscriber subscriber;
    c_bool wsResult;
    d_waitset waitset;
    d_waitsetAction action;
    d_durability durability;
    d_configuration configuration;
    c_ulong i;
    os_threadAttr tattr;
    os_threadId tid, *toStore;

    assert(listener);
    result = FALSE;
    assert(d_objectIsValid(d_object(listener), D_LISTENER));

    if(listener){
        d_listenerLock(d_listener(listener));
        dispatcher    = u_dispatcher(listener->queue);
        action        = d_persistentDataListenerAction;
        admin         = d_listenerGetAdmin(d_listener(listener));
        durability    = d_adminGetDurability(admin);
        configuration = d_durabilityGetConfiguration(durability);
        subscriber    = d_adminGetSubscriber(admin);

        if(d_listener(listener)->attached == FALSE){
            listener->totalTime.tv_sec = 0;
            listener->totalTime.tv_nsec = 0;
            listener->totalActions = 0;

            /* Initialise smpthreads when storemode is MMF and more than one thread is configured */
            if(configuration->persistentStoreMode == D_STORE_TYPE_MEM_MAPPED_FILE){
                os_threadAttrInit(&tattr);

                if(configuration->persistentThreadCount > 1){
                    listener->runCount = 0;
                    for(i=0; i<configuration->persistentThreadCount; i++){
                        os_threadCreate(&tid, "smpPersist", &tattr, d_smpPersist, listener);

                        toStore = (os_threadId*)(os_malloc(sizeof(os_threadId)));
                        *toStore = tid;
                        c_iterAppend(listener->persistentThreads, toStore);
                    }
                }
            }

            ur = u_dispatcherSetEventMask(dispatcher, V_EVENT_DATA_AVAILABLE);

            if(ur == U_RESULT_OK){
                waitset = d_subscriberGetWaitset(subscriber);
                listener->waitsetData = d_waitsetEntityNew(
                            "persistentDataListener",
                            dispatcher, action,
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
                    u_dispatcherNotify(dispatcher);
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
    os_threadId* tid;

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

                tid = (os_threadId*)(c_iterTakeFirst(listener->persistentThreads));

                while(tid){
                    os_threadWaitExit(*tid, NULL);
                    os_free(tid);
                    tid = (os_threadId*)(c_iterTakeFirst(listener->persistentThreads));
                }
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

c_ulong
d_persistentDataListenerAction(
    u_dispatcher o,
    u_waitsetEvent event,
    c_voidp usrData)
{
    d_persistentDataListener listener;
    d_admin admin;
    d_subscriber subscriber;
    d_durability durability;
    struct takeData data;
    c_bool terminate;
    d_serviceState state;
    os_time sleepTime;

    listener = d_persistentDataListener(usrData);

    if((event->events & V_EVENT_DATA_AVAILABLE) ==  V_EVENT_DATA_AVAILABLE){
        admin = d_listenerGetAdmin(d_listener(listener));
        subscriber = d_adminGetSubscriber(admin);

        data.listener        = listener;
        data.persistentStore = d_subscriberGetPersistentStore(subscriber);
        data.durability      = d_adminGetDurability(admin);

        terminate            = d_durabilityMustTerminate(data.durability);
        state                = d_durabilityGetState(data.durability);
        /*100 ms*/
        sleepTime.tv_sec     = 0;
        sleepTime.tv_nsec    = 100000000;

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
            os_nanoSleep(sleepTime);
            terminate = d_durabilityMustTerminate(data.durability);
            state     = d_durabilityGetState(data.durability);

        }

        if(c_iterLength(listener->persistentThreads) > 0){

            durability = d_adminGetDurability(admin);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                D_THREAD_PERISTENT_DATA_LISTENER,
                                "Using SMP for persistency.\n");

            u_entityAction(u_entity(o), d_persistentDataListenerSMPTake, &data);
        } else {
            u_entityAction(u_entity(o), d_persistentDataListenerTake, &data);
        }
    }
    return event->events;
}
