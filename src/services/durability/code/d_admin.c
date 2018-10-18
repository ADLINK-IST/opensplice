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
#include "d__lock.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__actionQueue.h"
#include "d__fellow.h"
#include "d__nameSpace.h"
#include "d__configuration.h"
#include "d__group.h"
#include "d__table.h"
#include "d__listener.h"
#include "d__publisher.h"
#include "d__subscriber.h"
#include "d__nameSpacesRequestListener.h"
#include "d__remoteReaderListener.h"
#include "d__misc.h"
#include "d__readerRequest.h"
#include "d__eventListener.h"
#include "d__conflictMonitor.h"
#include "d__conflictResolver.h"
#include "d__filter.h"
#include "d__element.h"
#include "vortex_os.h"
#include "d_newGroup.h"
#include "d__mergeState.h"
#include "d_nameSpaces.h"
#include "d_groupsRequest.h"
#include "d_nameSpacesRequest.h"
#include "d_message.h"
#include "d_qos.h"
#include "d_store.h"
#include "d_networkAddress.h"
#include "d__statistics.h"
#include "d__thread.h"
#include "v_kernel.h"
#include "v_topic.h"
#include "v_event.h"
#include "v_reader.h"
#include "v_service.h"
#include "v_group.h"
#include "v_public.h"
#include "v_builtin.h"
#include "v_time.h"
#include "u_observable.h"
#include "u_entity.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_defs.h"
#include "os_report.h"
#include "os_time.h"
#include "c_base.h"
#include <stddef.h>

/*
 * TODO: Determine the compatibility of the namespaces of two fellows before
 * allowing communication between them.
 */

/* Prototype function used in checkAlignerForRole */
static d_nameSpace d_adminGetNameSpaceNoLock(d_admin admin, c_char *name);

void
d_adminUpdateStatistics(
    d_admin admin,
    d_adminStatisticsInfo statistics)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        d_lockLock(d_lock(admin));
        d_durabilityUpdateStatistics(admin->durability, d_statisticsUpdateAdmin, statistics);
        d_lockUnlock(d_lock(admin));
    }
    return;
}

d_admin
d_adminNew(
    d_durability durability)
{
    d_admin admin;
    os_threadAttr ta;
    os_result osr;
    os_duration sleepTime;
    d_configuration config;
    u_result uResult;
    int fail = FALSE;

    assert(d_durabilityIsValid(durability));
    config = d_durabilityGetConfiguration(durability);
    assert(d_configurationIsValid(config));
    /* Allocate admin object */
    admin = d_admin(os_malloc(C_SIZEOF(d_admin)));
    if (admin) {
        /* zero fill structure so d_adminDeinit can safely be invoked */
        memset(admin, 0, sizeof(C_STRUCT(d_admin)));
        /* Call super-init */
        d_lockInit(d_lock(admin), D_ADMIN,
                   (d_objectDeinitFunc)d_adminDeinit);
        /* Initialize admin */
        admin->durability = durability;
        admin->seqnum = config->seqnum_initval;
        uResult = u_observableAction(
            u_observable(d_durabilityGetService(durability)),
            d_adminInitAddress,
            admin);
        assert(admin->myAddress != NULL);
        if (uResult != U_RESULT_OK) {
            fail = TRUE;
        }
        admin->initMask = D__INIT_FLAG_NONE;
        admin->conflictResolver = NULL;
        /* Various mutexes and condition variables */
        if (!fail) {
            d_printTimedEvent(durability, D_LEVEL_FINER, "Initializing administration...\n");
            if (os_mutexInit(&admin->eventMutex, NULL) == os_resultSuccess) {
                admin->initMask |= D__INIT_FLAG_EVENT_MUTEX;
            } else {
               fail = TRUE;
            }
        }
        if (!fail) {
            if (os_mutexInit(&admin->conflictQueueMutex, NULL) == os_resultSuccess) {
                admin->initMask |= D__INIT_FLAG_CONFLICTQUEUE_MUTEX;
            } else {
                fail = TRUE;
            }
        }
        if (!fail) {
            if (os_condInit(&admin->eventCondition, &admin->eventMutex, NULL) == os_resultSuccess) {
                admin->initMask |= D__INIT_FLAG_EVENT_COND;
            } else {
                fail = TRUE;
            }
        }
        if (!fail) {
            if (os_mutexInit(&admin->seqnumMutex, NULL) == os_resultSuccess) {
                admin->initMask |= D__INIT_FLAG_SEQNUM_MUTEX;
            } else {
                fail = TRUE;
            }
        }
        if (!fail) {
            admin->cachedFellow = d_fellowNew(admin->myAddress, D_STATE_INIT, TRUE);
            /* Various tables and iters */
            admin->unconfirmedFellows   = d_tableNew(d_fellowCompare, d_fellowFree);
            admin->fellows              = d_tableNew(d_fellowCompare, d_fellowFree);
            admin->initial_fellows      = NULL;  /* Lazy initialized when resolving an initial conflict */
            admin->clients              = (config->clientDurabilityEnabled) ? d_tableNew(d_clientCompareByAddress, d_clientFree) : NULL;
            admin->readerRequests       = d_tableNew(d_readerRequestCompare, d_readerRequestFree);
            admin->terminateFellows     = d_tableNew(d_fellowCompare, d_fellowFree);
            admin->eventListeners       = c_iterNew(NULL);
            admin->nameSpaces           = c_iterNew(NULL);
            admin->eventQueue           = c_iterNew(NULL);
            admin->conflictQueue        = c_iterNew(NULL);
            admin->groups               = d_tableNew(d_groupCompare, d_groupFree);
            admin->alignerGroupCount    = 0;
            admin->eventThreadTerminate = FALSE;
            /* Check for failures */
            fail = (admin->cachedFellow == NULL);
            fail |= (admin->unconfirmedFellows == NULL);
            fail |= (admin->fellows == NULL);
            fail |= (admin->readerRequests == NULL);
            fail |= (admin->terminateFellows == NULL);
            fail |= (admin->eventListeners == NULL);
            fail |= (admin->nameSpaces == NULL);
            fail |= (admin->eventQueue == NULL);
            fail |= (admin->conflictQueue == NULL);
            fail |= (admin->groups == NULL);
        }
        if (!fail) {
            /* Set the conflictMonitor */
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Initializing conflict monitor...\n");
            admin->conflictMonitor = d_conflictMonitorNew(admin);
            fail = (admin->conflictMonitor == NULL);
        }
        if (!fail) {
             /* Initialize topics */
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Initializing protocol topics...\n");
            admin->groupsRequestTopic       = d_adminInitTopic(
                                                admin, D_GROUPS_REQ_TOPIC_NAME,
                                                D_GROUPS_REQ_TYPE_NAME,
                                                D_GROUPS_REQ_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            admin->sampleRequestTopic       = d_adminInitTopic(
                                                admin, D_SAMPLE_REQ_TOPIC_NAME,
                                                D_SAMPLE_REQ_TYPE_NAME,
                                                D_SAMPLE_REQ_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            admin->newGroupTopic            = d_adminInitTopic(
                                                admin, D_NEWGROUP_TOPIC_NAME,
                                                D_NEWGROUP_TYPE_NAME,
                                                D_NEWGROUP_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            admin->statusTopic              = d_adminInitTopic(
                                                admin, D_STATUS_TOPIC_NAME,
                                                D_STATUS_TYPE_NAME,
                                                D_STATUS_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPLAST,
                                                V_ORDERBY_RECEPTIONTIME,
                                                1);

            admin->sampleChainTopic         = d_adminInitTopic(
                                                admin, D_SAMPLE_CHAIN_TOPIC_NAME,
                                                D_SAMPLE_CHAIN_TYPE_NAME,
                                                D_SAMPLE_CHAIN_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            admin->nameSpacesTopic          = d_adminInitTopic(
                                                admin, D_NAMESPACES_TOPIC_NAME,
                                                D_NAMESPACES_TYPE_NAME,
                                                D_NAMESPACES_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            admin->nameSpacesRequestTopic   = d_adminInitTopic(
                                                admin, D_NAMESPACES_REQ_TOPIC_NAME,
                                                D_NAMESPACES_REQ_TYPE_NAME,
                                                D_NAMESPACES_REQ_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            admin->deleteDataTopic          = d_adminInitTopic(
                                                admin, D_DELETE_DATA_TOPIC_NAME,
                                                D_DELETE_DATA_TYPE_NAME,
                                                D_DELETE_DATA_KEY_LIST,
                                                V_RELIABILITY_RELIABLE,
                                                V_HISTORY_KEEPALL,
                                                V_ORDERBY_RECEPTIONTIME,
                                                V_LENGTH_UNLIMITED);

            if (config->capabilitySupport) {
                admin->capabilityTopic          = d_adminInitTopic(
                                                    admin, D_CAPABILITY_TOPIC_NAME,
                                                    D_CAPABILITY_TYPE_NAME,
                                                    D_CAPABILITY_KEY_LIST,
                                                    V_RELIABILITY_RELIABLE,
                                                    V_HISTORY_KEEPLAST,
                                                    V_ORDERBY_RECEPTIONTIME,
                                                    1);
            } else {
                admin->capabilityTopic = NULL;
            }

            if (config->clientDurabilityEnabled) {
                admin->durabilityStateRequestTopic = d_adminInitTopic(
                                                    admin, D_DURABILITY_STATE_REQUEST_TOPIC_NAME,
                                                    D_DURABILITY_STATE_REQUEST_TYPE_NAME,
                                                    D_DURABILITY_STATE_REQUEST_KEY_LIST,
                                                    V_RELIABILITY_RELIABLE,
                                                    V_HISTORY_KEEPALL,
                                                    V_ORDERBY_SOURCETIME,
                                                    V_LENGTH_UNLIMITED);

                admin->durabilityStateTopic       = d_adminInitTopic(
                                                    admin, D_DURABILITY_STATE_TOPIC_NAME,
                                                    D_DURABILITY_STATE_TYPE_NAME,
                                                    D_DURABILITY_STATE_KEY_LIST,
                                                    V_RELIABILITY_RELIABLE,
                                                    V_HISTORY_KEEPALL,
                                                    V_ORDERBY_SOURCETIME,
                                                    V_LENGTH_UNLIMITED);

                admin->historicalDataRequestTopic = d_adminInitTopic(
                                                    admin, D_HISTORICAL_DATA_REQUEST_TOPIC_NAME,
                                                    D_HISTORICAL_DATA_REQUEST_TYPE_NAME,
                                                    D_HISTORICAL_DATA_REQUEST_KEY_LIST,
                                                    V_RELIABILITY_RELIABLE,
                                                    V_HISTORY_KEEPALL,
                                                    V_ORDERBY_SOURCETIME,
                                                    V_LENGTH_UNLIMITED);

                admin->historicalDataTopic        = d_adminInitTopic(
                                                    admin, D_HISTORICAL_DATA_TOPIC_NAME,
                                                    D_HISTORICAL_DATA_TYPE_NAME,
                                                    D_HISTORICAL_DATA_KEY_LIST,
                                                    V_RELIABILITY_RELIABLE,
                                                    V_HISTORY_KEEPALL,
                                                    V_ORDERBY_SOURCETIME,
                                                    V_LENGTH_UNLIMITED);
            } else {
                admin->durabilityStateRequestTopic = NULL;
                admin->durabilityStateTopic = NULL;
                admin->historicalDataRequestTopic = NULL;
                admin->historicalDataTopic = NULL;
            }

            fail = (admin->groupsRequestTopic == NULL);
            fail |= (admin->sampleRequestTopic == NULL);
            fail |= (admin->newGroupTopic == NULL);
            fail |= (admin->statusTopic == NULL);
            fail |= (admin->sampleChainTopic == NULL);
            fail |= (admin->nameSpacesTopic == NULL);
            fail |= (admin->nameSpacesRequestTopic == NULL);
            fail |= (admin->deleteDataTopic == NULL);
            fail |= (config->capabilitySupport && admin->capabilityTopic == NULL);
            fail |= (config->clientDurabilityEnabled && admin->durabilityStateRequestTopic == NULL);
            fail |= (config->clientDurabilityEnabled && admin->durabilityStateTopic == NULL);
            fail |= (config->clientDurabilityEnabled && admin->historicalDataRequestTopic == NULL);
            fail |= (config->clientDurabilityEnabled && admin->historicalDataTopic == NULL);

       }
       if (!fail) {
            /* Create the action queue.
             * Actions in this queue are scheduled every 100ms
             */
            sleepTime  = OS_DURATION_INIT(0, 100000000);
            admin->actionQueue = d_actionQueueNew("d_adminActionQueue", sleepTime,
                    config->heartbeatScheduling);
            fail = (admin->actionQueue == NULL);
        }

        if (!fail) {
            /* start adminEventThread */
            os_threadAttrInit(&ta);
            osr = d_threadCreate(&(admin->eventThread),
                                 "AdminEventDispatcher",
                                 &ta,
                                 (void*(*)(void*))d_adminEventThreadStart,
                                 (void*)admin);
            fail = (osr != os_resultSuccess);
            if (!fail) {
                admin->initMask |= D__INIT_FLAG_EVENT_THREAD;
            }
        }

        if (!fail) {
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Initializing protocol publisher and writers...\n");
            admin->publisher = d_publisherNew(admin);
            fail = (admin->publisher == NULL);
        }
    }
    if (fail) {
        d_printTimedEvent(durability, D_LEVEL_SEVERE, "Failed to initialize the administration, trying to cleanup admin\n");
        if (admin) {
            d_adminFree(admin);
            admin = NULL;
        }
    }
    return admin;
}


void
d_adminInitSubscriber(
    d_admin admin)
{
    d_durability durability;

    durability = d_adminGetDurability(admin);

    /* Initialize protocol subscriber. Needs to be seperated from d_adminNew because
     * of dependency between store (which is created by subscriber) and nameSpaces
     * list in admin.
     */
    d_printTimedEvent(durability, D_LEVEL_FINER, "Initializing protocol subscriber...\n");
    admin->subscriber = d_subscriberNew(admin);
    assert(admin->subscriber);
}

c_bool
d_adminAddLocalGroup(
    d_admin admin,
    d_group group)
{
    d_group duplicate;
    c_bool result;
    d_adminStatisticsInfo info;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    result = FALSE;

    if(group && admin){
        d_lockLock(d_lock(admin));
        duplicate = d_tableInsert(admin->groups, group);
        /* In some cases a group might be registered multiple times. Of
         * course, it is not added and notified anymore.
         */
        if(duplicate == NULL){
            info = d_adminStatisticsInfoNew();
            info->kind = D_ADMIN_STATISTICS_GROUP;

            switch(d_groupGetCompleteness(group)){
            case D_GROUP_COMPLETE:
                switch(d_groupGetKind(group)){
                case D_DURABILITY_VOLATILE:
                    info->groupsKnownVolatileDif += 1;
                    info->groupsCompleteVolatileDif += 1;
                    break;
                case D_DURABILITY_TRANSIENT:
                case D_DURABILITY_TRANSIENT_LOCAL:
                    info->groupsKnownTransientDif += 1;
                    info->groupsCompleteTransientDif += 1;
                    break;
                case D_DURABILITY_PERSISTENT:
                    info->groupsKnownPersistentDif += 1;
                    info->groupsCompletePersistentDif += 1;
                    break;
                case D_DURABILITY_ALL:
                    break;
                }
                break;
            case D_GROUP_INCOMPLETE:
                switch(d_groupGetKind(group)){
                case D_DURABILITY_VOLATILE:
                    info->groupsKnownVolatileDif += 1;
                    info->groupsIncompleteVolatileDif += 1;
                    break;
                case D_DURABILITY_TRANSIENT:
                case D_DURABILITY_TRANSIENT_LOCAL:
                    info->groupsKnownTransientDif += 1;
                    info->groupsIncompleteTransientDif += 1;
                    break;
                case D_DURABILITY_PERSISTENT:
                    info->groupsKnownPersistentDif += 1;
                    info->groupsCompletePersistentDif += 1;
                    break;
                case D_DURABILITY_ALL:
                    break;
                }
                break;
            case D_GROUP_KNOWLEDGE_UNDEFINED:
            case D_GROUP_UNKNOWN:
                break;
            }

            d_durabilityUpdateStatistics(admin->durability, d_statisticsUpdateAdmin, info);
            d_adminStatisticsInfoFree(info);

            admin->alignerGroupCount++;
            d_adminNotifyListeners(admin, D_GROUP_LOCAL_NEW, NULL, NULL, group, NULL);
            result = TRUE;
        }
        d_lockUnlock(d_lock(admin));
    }
    return result;
}

c_ulong
d_adminGetAlignerGroupCount(
    d_admin admin)
{
    c_ulong count = 0;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        count = admin->alignerGroupCount;
    }
    return count;
}


d_group
d_adminGetLocalGroupNoLock(
    d_admin admin,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    d_group dummy;
    d_group found;

    assert(d_adminIsValid(admin));

    dummy = d_groupNew(partition, topic, kind, D_GROUP_KNOWLEDGE_UNDEFINED, D_QUALITY_ZERO);
    found = d_tableFind(admin->groups, dummy);
    d_groupFree(dummy);
    return found;
}


d_group
d_adminGetLocalGroup(
    d_admin admin,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    d_group found;

    assert(d_adminIsValid(admin));

    d_lockLock(d_lock(admin));
    found = d_adminGetLocalGroupNoLock(admin, partition, topic, kind);
    d_lockUnlock(d_lock(admin));
    return found;
}


void
d_adminDeinit(
    d_admin admin)
{
    d_durability durability;
    d_nameSpace nameSpace;
    d_adminEvent event;
    d_conflict conflict;

    assert(d_adminIsValid(admin));

    durability = admin->durability;
    assert(d_durabilityIsValid(durability));

    /* First stop the threads associated with the admin and
     * destroy associated mutexes and conditions.
     *
     * Deinitialization can be invoked either by failure of
     * d_adminNew(), or by stopping the durability service.
     * In latter case threads may have already been stopped.
     * To test for this* flags are used. These flags are set
     * when the objects are created, and reset when destroyed.
     */
    if (admin->initMask & D__INIT_FLAG_EVENT_MUTEX) {
        if (admin->initMask & D__INIT_FLAG_EVENT_COND) {
            os_mutexLock(&admin->eventMutex);
            admin->eventThreadTerminate = TRUE;
            os_condSignal(&admin->eventCondition);
            os_mutexUnlock(&admin->eventMutex);
            /* Stop it if it is still running. */
            if (admin->initMask & D__INIT_FLAG_EVENT_THREAD) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Waiting for admin event dispatcher thread to terminate...\n");
                d_threadWaitExit(admin->eventThread, NULL);
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Thread destroyed.\n");
                /* Reset the flag to indicate that the event thread has been cleared */
                admin->initMask &= ~(D__INIT_FLAG_EVENT_THREAD);
            }
            os_condDestroy(&admin->eventCondition);
            /* Reset the flag to indicate that the eventcondition has been destroyed */
            admin->initMask &= ~(D__INIT_FLAG_EVENT_COND);
        }
        /* Removing eventListeners */
        if (admin->eventListeners) {
            os_mutexLock(&admin->eventMutex);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                            "Removing event listeners.\n");
            c_iterFree(admin->eventListeners);
            admin->eventListeners = NULL;
            os_mutexUnlock(&admin->eventMutex);
        }
        /* Clearing the event queue */
        if (admin->eventQueue) {
            os_mutexLock(&admin->eventMutex);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                                "Clearing event queue.\n");
            event = d_adminEvent(c_iterTakeFirst(admin->eventQueue));
            while(event){
                d_adminEventFree(event);
                event = d_adminEvent(c_iterTakeFirst(admin->eventQueue));
            }
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                                "Destroying event queue.\n");
            c_iterFree(admin->eventQueue);
            os_mutexUnlock(&admin->eventMutex);
        }
        /* Destroy eventMutex */
        os_mutexDestroy(&admin->eventMutex);
        admin->initMask &= ~(D__INIT_FLAG_EVENT_MUTEX);
     }

    /* Destroy the conflictMonitor */
    if (admin->conflictMonitor) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
                          "Destroying conflict monitor...\n");
        d_conflictMonitorFree(admin->conflictMonitor);
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                          "Conflict monitor destroyed\n");
    }

    /* Destroy the conflictResolver */
    if (admin->conflictResolver) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
                          "Destroying conflict resolver...\n");
        d_conflictResolverFree(admin->conflictResolver);
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Conflict resolver destroyed\n");
        admin->conflictResolver = NULL;
    }


    /* Destroy the conflictQueue */
    if (admin->conflictQueue) {
        while ((conflict = d_conflict(c_iterTakeFirst(admin->conflictQueue))) != NULL) {
            d_conflictFree(conflict);
        }
        c_iterFree(admin->conflictQueue);
    }
    if (admin->initMask & D__INIT_FLAG_CONFLICTQUEUE_MUTEX) {
        os_mutexDestroy(&admin->conflictQueueMutex);
        /* Reset the flag to indicate that the eventcondition has been destroyed */
        admin->initMask &= ~(D__INIT_FLAG_CONFLICTQUEUE_MUTEX);
    }
    /* Destroy the admin->actionQueue. */
    if (admin->actionQueue) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying action queue %s .\n", admin->actionQueue->name);
        d_actionQueueFree(admin->actionQueue);
    }
    /* Destroying subscriber */
    if (admin->subscriber) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying subscriber...\n");
        d_subscriberFree(admin->subscriber);

        d_printTimedEvent(durability, D_LEVEL_FINER, "Subscriber destroyed\n");
        admin->subscriber = NULL;
    }
    /* Destroying publisher */
    if (admin->publisher) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying publisher...\n");
        d_publisherFree(admin->publisher);
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Publisher destroyed\n");
        admin->publisher = NULL;
    }
    /* Destroy pending reader requests */
    if (admin->readerRequests) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying readerRequests...\n");
        d_tableFree(admin->readerRequests);
        admin->readerRequests = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "ReaderRequests destroyed\n");
    }
    /* Destroy list of fellows that have recently terminated */
    if (admin->terminateFellows) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying terminateFellow admin...\n");
        d_tableFree(admin->terminateFellows);
        admin->terminateFellows = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "TerminateFellows destroyed\n");
    }
    /* Destroy fellow administration */
    if (admin->fellows) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Destroying fellow admin...\n");
        d_tableFree(admin->fellows);
        admin->fellows = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Fellows destroyed\n");
    }
    /* Destroy unconfirmed fellow administration */
    if (admin->unconfirmedFellows) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying unconfirmed fellow admin...\n");
        d_tableFree(admin->unconfirmedFellows);
        admin->unconfirmedFellows = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Unconfirmed fellows destroyed\n");
    }
    /* Destroy client administration */
    if (admin->clients) {
        d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying clients admin...\n");
        d_tableFree(admin->clients);
        admin->clients = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Clients destroyed\n");
    }
    /* Destroy group administration */
    if (admin->groups) {
        d_tableFree(admin->groups);
        d_printTimedEvent(durability, D_LEVEL_FINER, "My groups destroyed\n");
    }
    if (admin->cachedFellow) {
        d_fellowFree(admin->cachedFellow);
        admin->cachedFellow = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Cached fellow destroyed\n");
    }
    /* Destroy topics */
    d_printTimedEvent(durability, D_LEVEL_FINER, "Destroying topics...\n");
    if (admin->statusTopic) {
        u_objectFree(u_object(admin->statusTopic));
        admin->statusTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_STATUS_TOPIC_NAME);
    }
    if (admin->newGroupTopic) {
        u_objectFree(u_object(admin->newGroupTopic));
        admin->newGroupTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_NEWGROUP_TOPIC_NAME);
    }
    if (admin->groupsRequestTopic) {
        u_objectFree(u_object(admin->groupsRequestTopic));
        admin->groupsRequestTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_GROUPS_REQ_TOPIC_NAME);
    }
    if (admin->sampleRequestTopic) {
        u_objectFree(u_object(admin->sampleRequestTopic));
        admin->sampleRequestTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_SAMPLE_REQ_TOPIC_NAME);
    }
    if(admin->sampleChainTopic){
        u_objectFree(u_object(admin->sampleChainTopic));
        admin->sampleChainTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_SAMPLE_CHAIN_TOPIC_NAME);
    }
    if (admin->nameSpacesTopic) {
        u_objectFree(u_object(admin->nameSpacesTopic));
        admin->nameSpacesTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_NAMESPACES_TOPIC_NAME);
    }
    if (admin->nameSpacesRequestTopic) {
        u_objectFree(u_object(admin->nameSpacesRequestTopic));
        admin->nameSpacesRequestTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_NAMESPACES_REQ_TOPIC_NAME);
    }
    if (admin->deleteDataTopic) {
        u_objectFree(u_object(admin->deleteDataTopic));
        admin->deleteDataTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_DELETE_DATA_TOPIC_NAME);
    }
    if (admin->capabilityTopic) {
        u_objectFree(u_object(admin->capabilityTopic));
        admin->capabilityTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_CAPABILITY_TOPIC_NAME);
    }
    if (admin->durabilityStateRequestTopic) {
        u_objectFree(u_object(admin->durabilityStateRequestTopic));
        admin->durabilityStateRequestTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_DURABILITY_STATE_REQUEST_TOPIC_NAME);
    }
    if (admin->durabilityStateTopic) {
        u_objectFree(u_object(admin->durabilityStateTopic));
        admin->durabilityStateTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_DURABILITY_STATE_TOPIC_NAME);
    }
    if (admin->historicalDataRequestTopic) {
        u_objectFree(u_object(admin->historicalDataRequestTopic));
        admin->historicalDataRequestTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_HISTORICAL_DATA_REQUEST_TOPIC_NAME);
    }
    if (admin->historicalDataTopic) {
        u_objectFree(u_object(admin->historicalDataTopic));
        admin->historicalDataTopic = NULL;
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s topic destroyed\n", D_HISTORICAL_DATA_TOPIC_NAME);
    }
    d_printTimedEvent(durability, D_LEVEL_FINEST, "Topics destroyed\n");
    if (admin->myAddress) {
        d_networkAddressFree(admin->myAddress);
        admin->myAddress = NULL;
    }
    /* Destroy namespaces administration */
    if (admin->nameSpaces) {
        nameSpace = d_nameSpace(c_iterTakeFirst(admin->nameSpaces));

        while(nameSpace){
            d_nameSpaceFree(nameSpace);
            nameSpace = d_nameSpace(c_iterTakeFirst(admin->nameSpaces));
        }
        c_iterFree (admin->nameSpaces);
        admin->nameSpaces = NULL;
    }
    /* Destroy seqnumMutex */
    if (admin->initMask & D__INIT_FLAG_SEQNUM_MUTEX) {
        os_mutexDestroy(&admin->seqnumMutex);
        admin->initMask &= ~(D__INIT_FLAG_SEQNUM_MUTEX);
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(admin));
    d_printTimedEvent(durability, D_LEVEL_FINER, "Admin destroyed\n");
}

void
d_adminFree(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    d_objectFree(d_object(admin));
}


/**
 * \brief Retrieve the confirmed fellow that matches the address
 *
 * If found, a reference to the fellow is returned.
 * If no confirmed fellow could be found, NULL is returned.
 */
d_fellow
d_adminGetFellow(
    d_admin admin,
    d_networkAddress address)
{
    d_fellow found = NULL;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    d_lockLock(d_lock(admin));

    d_fellowSetAddress(admin->cachedFellow, address);
    found = d_tableFind(admin->fellows, admin->cachedFellow);

    if (found) {
        found = d_fellow(d_objectKeep(d_object(found)));
    }

    d_lockUnlock(d_lock(admin));

    return found;
}



/**
 * \brief Retrieve the unconfirmed fellow that matches the address
 *
 * If found, a reference to the fellow is returned.
 * If no unconfirmed fellow could be found, NULL is returned.
 */
d_fellow
d_adminGetUnconfirmedFellow(
    d_admin admin,
    d_networkAddress address)
{
    d_fellow found, dummy;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    found = NULL;
    dummy = d_fellowNew(address, D_STATE_INIT, FALSE);

    d_lockLock(d_lock(admin));
    found = d_tableFind(admin->unconfirmedFellows, dummy);
    if(found){
        found = d_fellow(d_objectKeep(d_object(found)));
    }
    d_lockUnlock(d_lock(admin));
    d_fellowFree(dummy);

    return found;
}


/**
 * \brief Add a reference to the fellow to the durability administration.
 *
 * If the fellow was recently terminated it is not added and NULL is returned.
 *
 * If the fellow is confirmed but there already exists and unconfirmed fellow
 * with the same address, then this fellow is promoted to become confirmed.
 * If there already exists a confirmed fellow with the same address then this
 * fellow returned.
 *
 * The fellow is refCounted, so it must be freed by the caller.
 *
 * @return
 *  NULL, if the fellow has recently terminated.
 *  the fellow that is added, otherwise.
 */
d_fellow
d_adminAddFellow(
    d_admin admin,
    d_fellow fellow)
{
    d_networkAddress fellowAddr;
    d_fellow found, duplicate, result = NULL;
    d_adminStatisticsInfo info;
    c_bool isConfirmed;
    c_bool justConfirmed = FALSE;
    d_configuration config = d_durabilityGetConfiguration(d_adminGetDurability(admin));
    d_serviceState fellowState;

    assert(d_adminIsValid(admin));
    assert(d_fellowIsValid(fellow));

    /* The admin maintains two lists of fellows
     *   admin->fellows contains the list of confirmed fellows
     *   admin->unconfirmedFellows contains the list of unconfirmed fellows
     * Adding a confirmed fellow may result in moving the fellow from 
     * admin->unconfirmedFellows to admin->fellows.
     * Trying to add an unconfiremd fellow while it has been confirmed
     * previously is not possible, in that case the confirmed fellow will
     * be returned.
     */
    result = NULL;
    isConfirmed = d_fellowIsConfirmed(fellow);
    fellowAddr = d_fellowGetAddress(fellow);
    fellowState = d_fellowGetState(fellow);

    d_lockLock(d_lock(admin));

    /* If the fellow has been terminated recently then do not add it
     * for a while to prevent stuttering */
    found = d_tableFind(admin->terminateFellows, fellow);
    if (found) {
        if (!d_fellowHasRecentlyTerminated(found)) {
            d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                    "Fellow %u has recently terminated, therefore NOT added to admin until this fellow is cleaned up.\n",
                    fellowAddr->systemId);
        }
        goto ret;
    }

    if (!isConfirmed) {
        /* Trying to add an unconfirmed fellow while it has been confirmed
         * earlier is not possible */
        if ((found = d_tableFind(admin->fellows, fellow)) != NULL) {
            d_printTimedEvent(admin->durability, D_LEVEL_FINEST,
                    "Fellow %u is already confirmed, no need to make it unconfirmed again.\n",
                    fellowAddr->systemId);
            result = d_fellow(d_objectKeep(d_object(found)));
            goto ret;
        }

        /* Do not add an unconfirmed fellow if there already exists one */
        if ((duplicate = d_tableInsert(admin->unconfirmedFellows, fellow)) != NULL) {
            d_printTimedEvent(admin->durability, D_LEVEL_FINEST,
                    "Fellow %u is already unconfirmed, no need to add it again.\n",
                    fellowAddr->systemId);
            result = d_fellow(d_objectKeep(d_object(duplicate)));
        } else {
            /* A new unconfirmed fellow was added. */
            d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                    "Unconfirmed fellow %u added to admin [state %s].\n",
                    fellowAddr->systemId, d_fellowStateText(fellowState));
            result = d_fellow(d_objectKeep(d_object(fellow)));
        }
        goto ret;

    } else {
        /* An existing unconfirmed fellow is about to become
         * confirmed. Update the state and last status. */
        if ((found = d_tableRemove(admin->unconfirmedFellows, fellow)) != NULL) {
            d_fellowUpdateStatus(found, d_fellowGetState(fellow), d_fellowGetLastSeqNum(fellow));
            d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                    "Confirming fellow %u\n",
                    fellowAddr->systemId);
            justConfirmed = TRUE;
            d_fellowSetConfirmed(found, TRUE);
            fellow = found;
        }
        duplicate = (d_fellow)d_tableInsert(admin->fellows, fellow);

        /* Duplicate confirmed fellow found. Update the last status. */
        if (duplicate) {
            d_fellowUpdateStatus(duplicate, d_fellowGetState(duplicate), d_fellowGetLastSeqNum(fellow));
            result = d_fellow(d_objectKeep(d_object(duplicate)));

        /* The fellow has become confirmed for the first time. */
        } else {
            d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                    "Confirmed fellow %u added to admin [state %s].\n",
                    fellowAddr->systemId, d_fellowStateText(fellowState));
            justConfirmed = TRUE;
            d_adminNotifyListeners(admin, D_FELLOW_NEW, fellow, NULL, NULL, NULL);
            info = d_adminStatisticsInfoNew();
            info->fellowsKnownDif = 1;
            d_durabilityUpdateStatistics(admin->durability, d_statisticsUpdateAdmin, info);
            d_adminStatisticsInfoFree(info);
            result = d_fellow(d_objectKeep(d_object(fellow)));
        }
    }

ret:
    d_lockUnlock(d_lock(admin));
    /* Rediscover readers, but only when you must wait for remote readers */
    if ((justConfirmed) && (config->waitForRemoteReaders)) {
        d_remoteReaderListenerCheckReaders(admin->subscriber->remoteReaderListener);
    }
    d_networkAddressFree(fellowAddr);
    return result;
}

static c_bool
clearMaster(
    d_fellow fellow,
    c_voidp userData)
{
    d_fellowClearMaster(fellow, d_networkAddress(userData));

    return TRUE;
}

struct fellowsExistForRoleHelper
{
    d_name role;
    c_bool found;
};


struct checkAlignerForRoleHelper {
    d_admin admin;     /* the admininistration */
    d_fellow fellow;   /* the fellow */
    c_char *role;      /* the fellow role */
};


/**
 * \brief Clears the mergestate of the namespace if no alternative aligner
 *        in the same role as myself could be found for this namespace.
 *
 * @return TRUE
 */
c_bool
checkAlignerForRole (d_nameSpace nameSpace, void *userData) {

    struct checkAlignerForRoleHelper *roleHelper;
    d_nameSpace myNameSpace;

    roleHelper = (struct checkAlignerForRoleHelper *)userData;

    assert(roleHelper->admin);
    assert(roleHelper->fellow);

    /* Retrieve the local namespace corresponding to the fellow's namespace
     * from my own administration.
     */
    myNameSpace = d_adminGetNameSpaceNoLock(roleHelper->admin, d_nameSpaceGetName(nameSpace));
    if (myNameSpace != NULL) {
        d_durability durability = d_adminGetDurability(roleHelper->admin);
        d_configuration config = d_durabilityGetConfiguration(durability);
        d_networkAddress masterAddr = d_nameSpaceGetMaster(myNameSpace);
        d_networkAddress fellowAddr = d_fellowGetAddress(roleHelper->fellow);

        if ( (strcmp(config->role, roleHelper->role) == 0) && /* same role */
             (d_networkAddressEquals(masterAddr, fellowAddr)) && /* removed fellow was master */
             ((!d_nameSpaceIsAligner(myNameSpace)) || (d_nameSpaceGetMasterPriority(myNameSpace) == D_MINIMUM_MASTER_PRIORITY))) {

            d_networkAddress unAddressed = d_networkAddressUnaddressed();

            /* No alternative aligner has been found (not even myself).
             * Reset the master and the current state of the namespace for this role
             */
            d_nameSpaceSetMaster(myNameSpace, unAddressed);
            d_printTimedEvent(roleHelper->admin->durability, D_LEVEL_INFO,
                   "Confirming master: Fellow '%u' is the master for nameSpace '%s'.\n",
                   unAddressed->systemId, d_nameSpaceGetName(myNameSpace));
            d_nameSpaceMasterConfirmed(myNameSpace);

            d_nameSpaceClearMergeState (myNameSpace, roleHelper->role);
            /* Log the clearing of the state to indicate that no aligner is available for the namespace. */
            d_printTimedEvent(roleHelper->admin->durability, D_LEVEL_FINER,
                              "State and master of namespace '%s' for role '%s' cleared\n",
                              d_nameSpaceGetName(nameSpace),
                              roleHelper->role);
            d_networkAddressFree(unAddressed);
        }
        d_networkAddressFree(masterAddr);
        d_networkAddressFree(fellowAddr);
        d_nameSpaceFree(myNameSpace);
    }
    return TRUE;
}


/**
 * \brief Remove a fellow from the durability administration.
 *
 * If the fellow was a master for one of my namespaces in my role then check
 * if an alternative master can be found. If no alternative aligner can be
 * found then the mergestate for this namespace is cleared.
 *
 * @return a reference to the fellow that is removed, or NULL otherwise.
 */
d_fellow
d_adminRemoveFellow(
    d_admin admin,
    d_fellow fellow,
    c_bool trackFellow)
{
    d_fellow result;
    d_networkAddress fellowAddr;
    d_adminStatisticsInfo info;
    struct checkAlignerForRoleHelper helper;
    d_durability durability;
    d_configuration config;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    result = NULL;
    if (admin && fellow) {
        d_lockLock(d_lock(admin));
        durability = d_adminGetDurability(admin);
        assert(d_durabilityIsValid(durability));
        config = d_durabilityGetConfiguration(durability);
        assert(d_configurationIsValid(config));
        /* Remove the fellow from the admin */
        result = d_tableRemove(admin->fellows, fellow);
        if (result) {
            /* Check if there exists an alternative aligner for
             * the namespaces for which the fellow was an aligner.
             * If not found then there is nobody who can align
             * the namespace. In this case the merge state for
             * this namespace and role is cleared.
             */
            helper.fellow = result;
            helper.admin = admin;
            helper.role = d_fellowGetRole(result);
            d_fellowNameSpaceWalk(result, checkAlignerForRole, &helper);

            info = d_adminStatisticsInfoNew();
            info->fellowsKnownDif = -1;

            switch(d_fellowGetCommunicationState(fellow)){
                case D_COMMUNICATION_STATE_APPROVED:
                    info->fellowsApprovedDif -= 1;
                    break;
                case D_COMMUNICATION_STATE_INCOMPATIBLE_STATE:
                    info->fellowsIncompatibleStateDif -= 1;
                    break;
                case D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL:
                    info->fellowsIncompatibleDataModelDif -= 1;
                    break;
                case D_COMMUNICATION_STATE_TERMINATED:
                case D_COMMUNICATION_STATE_UNKNOWN:
                    break;
            }
            /* Make sure that a termination message exceeds the maximum valid sequence number.
             * This prevents resurrection of the fellow in case another threads indicates that it
             * is still alive.
             */
            d_fellowUpdateStatus(result, D_STATE_TERMINATED, D_SEQNUM_INFINITE);
            fellowAddr = d_fellowGetAddress(result);
            d_tableWalk(admin->fellows, clearMaster, fellowAddr);

            d_durabilityUpdateStatistics(admin->durability, d_statisticsUpdateAdmin, info);
            d_adminStatisticsInfoFree(info);
            if ((os_durationCompare(config->terminateRemovalPeriod, OS_DURATION_ZERO) != OS_EQUAL) && (trackFellow == TRUE)) {
                /* The fellow has a non-zero terminateRemovalTime.
                 * To prevent that pending messages from the fellow cause the
                 * fellow to be added again to the admin, we add the fellow
                 * to the list admin->terminateFellow. This list is cleaned up
                 * after awhile in d_durabilityUpdateLeaseAndNotifyStatus.
                 */
                result->removalTime = os_timeMGet();
                (void)d_tableInsert(admin->terminateFellows, d_objectKeep(d_object(result)));
                d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                        "Removing confirmed fellow %u, ignoring messages from this fellow for %"PA_PRIduration"\n",
                        fellowAddr->systemId, OS_DURATION_PRINT(config->terminateRemovalPeriod));
            } else {
                d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                        "Removing confirmed fellow %u.\n",
                        fellowAddr->systemId);
            }
            /* Notify that the fellow is removed */
            d_adminNotifyListeners(admin, D_FELLOW_REMOVED, result, NULL, NULL, NULL);
            d_lockUnlock(d_lock(admin));

            d_publisherUnregisterInstances(admin->publisher, fellowAddr);

            d_networkAddressFree(fellowAddr);
        } else {
            d_lockUnlock(d_lock(admin));
        }
    }
    return result;
}

void
d_adminAsymRemoveFellow(
    d_admin admin,
    d_fellow fellow,
    c_bool trackFellow)
{
    /* The fellow was asymmetrically disconnected
     * Even though the fellow was not actually removed,
     * I must do as if it was removed (even if it was
     * never actually removed from admin->fellows)
     */

    OS_UNUSED_ARG(trackFellow);

    if (admin && fellow) {
        d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                "Reestablish connection with asymmetrically disconnected fellow %u.\n",
                fellow->address->systemId);

        d_lockLock(d_lock(admin));
        d_adminNotifyListeners(admin, D_FELLOW_REMOVED, fellow, NULL, NULL, NULL);
        d_lockUnlock(d_lock(admin));
    }
}

struct findNsWalkData
{
    const char* name;
    c_bool found;
};

static void
findNsWalk (
    void* o, void* userData)
{
    struct findNsWalkData* walkData = (struct findNsWalkData*)userData;
    if (!walkData->found && (strcmp (d_nameSpaceGetName(d_nameSpace(o)), walkData->name) == 0))
    {
        walkData->found = TRUE;
    }
}

/**
 * \brief Add a namespace received by the nameSpacesListener to the administration.
 *
 * This implements dynamic namespace functionality.
 */
void
d_adminAddNameSpace(
    d_admin admin,
    d_nameSpace nameSpace)
{
    d_durability durability;
    struct findNsWalkData walkData;

    /* For convenient logging output */
    const char* akindStr[3] =
        {"INITIAL", "LAZY", "ON_REQUEST"};
    const char* dkindStr[5] =
        {"VOLATILE", "TRANSIENT_LOCAL", "TRANSIENT", "PERSISTENT", "ALL"};

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE) == TRUE);

    durability = d_adminGetDurability(admin);

    if (admin && nameSpace)
    {
        d_lockLock(d_lock(admin));

        /* Check if namespace with that name is already in list */
        walkData.name = d_nameSpaceGetName(nameSpace);

        if (walkData.name)
        {
            walkData.found = FALSE;

            c_iterWalk (admin->nameSpaces, findNsWalk, &walkData);

            if (!walkData.found)
            {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Add namespace '%s' to administration with policy {aligner=%d, alignee=%s, durability=%s}\n",
                        d_nameSpaceGetName(nameSpace),
                        d_nameSpaceIsAligner(nameSpace),
                        akindStr[d_nameSpaceGetAlignmentKind(nameSpace)],
                        dkindStr[d_nameSpaceGetDurabilityKind(nameSpace)]);

                /* Add namespace to admin */
                admin->nameSpaces = c_iterAppend(
                    admin->nameSpaces, d_objectKeep(d_object(nameSpace)));

                d_printTimedEvent(durability, D_LEVEL_FINER, "Namespace '%s' added to administration, notifying listeners...\n", d_nameSpaceGetName(nameSpace));

                /* New namespace event */
                d_adminNotifyListeners(admin, D_NAMESPACE_NEW, NULL, nameSpace, NULL, NULL);
            }
        }

        d_lockUnlock(d_lock(admin));
    }
}

/* This function may only be called when its context has lock admin */
d_nameSpace
d_adminGetNameSpaceForGroupNoLock(
        d_admin admin,
        d_partition partition,
        d_topic topic)
{
    d_durability d;
    d_configuration c;
    d_nameSpace nameSpace;
    c_ulong i;

    assert(d_adminIsValid(admin));

    if (d_isBuiltinGroup(partition, topic)) {
        d = d_adminGetDurability(admin);
        c = d_durabilityGetConfiguration(d);
        if (!c->mustAlignBuiltinTopics) {
            return NULL;
        }
    }

    nameSpace = NULL;
    for(i=0; (i<c_iterLength(admin->nameSpaces)) && (nameSpace == NULL); i++) {
        nameSpace = d_nameSpace(c_iterObject(admin->nameSpaces, i));
        if (d_adminInNameSpace(nameSpace, partition, topic, FALSE) == TRUE) {
         /* do nothing */
        } else {
            nameSpace = NULL;
        }
    }
    return nameSpace;
}

d_nameSpace
d_adminGetNameSpaceForGroup(
        d_admin admin,
        d_partition partition,
        d_topic topic)
{
    d_nameSpace nameSpace;

    assert(d_adminIsValid(admin));

    d_lockLock (d_lock(admin));
    nameSpace = d_adminGetNameSpaceForGroupNoLock(admin, partition, topic);
    d_lockUnlock (d_lock(admin));

    return nameSpace;
}

c_bool
d_adminInNameSpace(
        d_nameSpace ns,
        d_partition partition,
        d_topic topic,
        c_bool aligner)
{
    d_group group;
    c_bool result;
    d_durabilityKind durabilityKind = D_DURABILITY_VOLATILE;
    result = FALSE;

    /* DCPSHeartbeat has become transient, but must not ever be
     * considered by durability
     */
    if (strcmp (partition, V_BUILTIN_PARTITION) == 0 && strcmp (topic, V_HEARTBEATINFO_NAME) == 0) {
        return FALSE;
    }

    if ((group = d_adminGetLocalGroupNoLock(d_threadsDurability()->admin, partition, topic, 0/*kind is don't care in d_adminGetLocalGroup*/)) != NULL) {
        durabilityKind = d_groupGetKind(group);
    }
    /* If DDSI will take care of builtin topic alignment
     * (indicated by ns->mustAlignBuiltinTopics == FALSE)
     * then durability must exclude builtin topics from
     * alignment by durability.
     *
     * If DDSI does NOT take care of builtin topic alignment
     * (indicated by ns->mustAlignBuiltinTopics == TRUE)
     * then durability must ALWAYS align these. For this case, a
     * namespace is automatically added by durability with the
     * proper merge-policy. This is to ensure that builtin
     * topics are always aligned even when customers have not
     * explicitly configured namespaces for builtin topics.
     */
    if (d_isBuiltinGroup(partition, topic)) {
        if(!ns->mustAlignBuiltinTopics){
            return FALSE;
        }
    }

    /* For all non-builtin topics check whether it matches the namespace. */
    if (d_nameSpaceIsIn(ns, partition, topic) == TRUE) {
        if(aligner == TRUE) {
           if (d_nameSpaceIsAligner(ns) == TRUE) {
                result = TRUE;
           }
        } else {
            result = TRUE;
        }
    }
    if (durabilityKind == D_DURABILITY_TRANSIENT_LOCAL) {
        result &= ns->mustAlignBuiltinTopics;
    }

    return result;
}

c_bool
d_adminGroupInAligneeNS(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    inNameSpace = FALSE;

    d_lockLock (d_lock(admin));
    count       = c_iterLength(admin->nameSpaces);
    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(admin->nameSpaces, i));
        inNameSpace = d_adminInNameSpace(ns, partition, topic, FALSE);
    }
    d_lockUnlock(d_lock(admin));

    return inNameSpace;
}

c_bool
d_adminGroupInActiveAligneeNS(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    inNameSpace = FALSE;

    d_lockLock (d_lock(admin));
    count       = c_iterLength(admin->nameSpaces);
    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(admin->nameSpaces, i));
        inNameSpace = d_adminInNameSpace(ns, partition, topic, FALSE);
        if (inNameSpace) {
            /* Defer alignment of non-builtin topics in case
             * alignment policy is ON_REQUEST.
             */
            if (!d_isBuiltinGroup(partition, topic)) {
                if (d_nameSpaceGetAlignmentKind(ns) == D_ALIGNEE_ON_REQUEST) {
                    inNameSpace = FALSE;
                }
            }
        }
    }
    d_lockUnlock (d_lock(admin));

    return inNameSpace;
}


c_bool
d_adminGroupInAlignerNSNoLock(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    inNameSpace = FALSE;
    count       = c_iterLength(admin->nameSpaces);
    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(admin->nameSpaces, i));
        inNameSpace = d_adminInNameSpace(ns, partition, topic, TRUE);
    }

    return inNameSpace;
}

c_bool
d_adminGroupInAlignerNS(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    c_bool inNameSpace;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    d_lockLock(d_lock(admin));
    inNameSpace = d_adminGroupInAlignerNSNoLock(admin, partition, topic);
    d_lockUnlock(d_lock(admin));

    return inNameSpace;
}

c_bool
d_adminGroupInInitialAligneeNS(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    inNameSpace = FALSE;

    d_lockLock (d_lock(admin));

    count       = c_iterLength(admin->nameSpaces);
    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(admin->nameSpaces, i));
        inNameSpace = d_adminInNameSpace(ns, partition, topic, FALSE);
        if (inNameSpace == TRUE) {
            /* Defer alignment of non-builtin groups in case
             * alignment policy does not match INITIAL.
             */
            if (!d_isBuiltinGroup(partition, topic)) {
                switch(d_nameSpaceGetAlignmentKind(ns)) {
                    case D_ALIGNEE_INITIAL:
                        break;
                    default:
                        inNameSpace = FALSE;
                        break;
                }
            }
        }
    }
    d_lockUnlock(d_lock(admin));

    return inNameSpace;
}

d_durability
d_adminGetDurability(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    return admin->durability;
}

d_publisher
d_adminGetPublisher(
    d_admin admin)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    return admin->publisher;
}

d_subscriber
d_adminGetSubscriber(
    d_admin admin)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    return admin->subscriber;
}

d_networkAddress
d_adminGetMyAddress(
    d_admin admin)
{
    d_networkAddress address;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    address = d_networkAddressNew(
                    admin->myAddress->systemId,
                    admin->myAddress->localId,
                    admin->myAddress->lifecycleId);

    return address;
}

u_topic
d_adminGetStatusTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->statusTopic;
    }
    return topic;
}

u_topic
d_adminGetNewGroupTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->newGroupTopic;
    }
    return topic;
}

u_topic
d_adminGetGroupsRequestTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->groupsRequestTopic;
    }
    return topic;
}

u_topic
d_adminGetSampleRequestTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->sampleRequestTopic;
    }
    return topic;
}

u_topic
d_adminGetSampleChainTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->sampleChainTopic;
    }
    return topic;
}

u_topic
d_adminGetNameSpacesTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->nameSpacesTopic;
    }
    return topic;
}

u_topic
d_adminGetNameSpacesRequestTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->nameSpacesRequestTopic;
    }
    return topic;
}

u_topic
d_adminGetDeleteDataTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->deleteDataTopic;
    }
    return topic;
}


u_topic
d_adminGetDurabilityStateRequestTopic(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    return admin->durabilityStateRequestTopic;
}


u_topic
d_adminGetDurabilityStateTopic(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    return admin->durabilityStateTopic;
}


u_topic
d_adminGetHistoricalDataRequestTopic(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    return admin->historicalDataRequestTopic;
}

u_topic
d_adminGetHistoricalDataTopic(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    return admin->historicalDataTopic;
}

u_topic
d_adminGetCapabilityTopic(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    return admin->capabilityTopic;
}


void
d_adminInitAddress(
    v_public entity,
    c_voidp args)
{
    d_admin admin;
    d_durability durability;
    v_gid gid;

    admin = d_admin(args);
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    durability = d_adminGetDurability(admin);
    gid = v_kernel(v_object(entity)->kernel)->GID;
    admin->myAddress = d_networkAddressNew(
        v_gidSystemId(gid), v_gidLocalId(gid), v_gidLifecycleId(gid));
    /* Also set the durability gid that is used for client-side durability.
     * The gid is determined as follows:
     *    - gid.prefix = systemId << 32 + localId
     *    - gid.suffix = lifecycleId
     */
    durability->myServerId.prefix = admin->myAddress->systemId;
    durability->myServerId.prefix = ((durability->myServerId.prefix) << 32) + admin->myAddress->localId;
    durability->myServerId.suffix = admin->myAddress->lifecycleId;
}

u_topic
d_adminInitTopic(
    d_admin admin,
    const c_char* topicName,
    const c_char* typeName,
    const c_char* keyList,
    v_reliabilityKind reliability,
    v_historyQosKind historyKind,
    v_orderbyKind orderKind,
    c_long historyDepth)
{
    v_topicQos topicQos;
    u_topic topic;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    topic = NULL;

    topicQos = d_topicQosNew(V_DURABILITY_VOLATILE, reliability);

    if(topicQos){
        topicQos->history.v.kind = historyKind;
        topicQos->history.v.depth = historyDepth;
        topicQos->orderby.v.kind = orderKind;

        /* Unfortunately d_topicQosNew() delivers different qos settings for the
         * max_blocking_time and lease_duration than the defaults. u_topicQosNew()
         * is to blame for that. For the clientDurability topics this is compensated
         * by explicitly setting these to the defaults. For compatibility reason
         * this is not done for the other durability topics.
         */

        if ( (strcmp(topicName, D_DURABILITY_STATE_REQUEST_TOPIC_NAME) == 0) ||
             (strcmp(topicName, D_DURABILITY_STATE_TOPIC_NAME) == 0) ||
             (strcmp(topicName, D_HISTORICAL_DATA_REQUEST_TOPIC_NAME) == 0) ||
             (strcmp(topicName, D_HISTORICAL_DATA_TOPIC_NAME) == 0)) {
            topicQos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
            topicQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
        }

        /* Create the topic */
        topic = u_topicNew(
                    u_participant(d_durabilityGetService(
                                    d_adminGetDurability(admin))),
                    topicName, typeName, keyList, topicQos);
        d_topicQosFree(topicQos);
    }
    return topic;
}

struct localGroupsCompleteArg {
    c_bool complete;
    c_bool report;
    d_durability durability;
};

static c_bool
d__adminLocalGroupsCompleteAction(
    d_group group,
    c_voidp userData)
{
    c_bool result;
    struct localGroupsCompleteArg *arg = (struct localGroupsCompleteArg *)userData;
    d_completeness c;

    c = d_groupGetCompleteness(group);
    if((c != D_GROUP_COMPLETE) && (c != D_GROUP_UNKNOWN)){
        /* If true, all groups will be reported. Otherwise first non-complete group will stop the walk. */
        result = arg->report;
        arg->complete = FALSE;
        if(arg->report) {
            assert(arg->durability);
            d_printTimedEvent(arg->durability, D_LEVEL_FINEST,
                    "Waiting for local group '%s.%s' to become complete.\n", group->partition, group->topic);
        }
    } else {
        result = TRUE; /* Continue walk */
    }
    return result;
}


c_bool
d_adminAreLocalGroupsComplete(
    d_admin admin,
    c_bool report)
{
    struct localGroupsCompleteArg arg;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    d_lockLock(d_lock(admin));
    if(d_tableSize(admin->groups) >= 1){
        arg.complete = TRUE;

        if(admin->subscriber) {
            arg.complete = d_subscriberAreRemoteGroupsHandled(admin->subscriber);
        }

        if(arg.complete == TRUE) {
            arg.report = report;
            if(report){
                arg.durability = d_adminGetDurability(admin);
            }
            d_tableWalk(admin->groups, &d__adminLocalGroupsCompleteAction, &arg);
        } else if (report) {
            d_printTimedEvent(d_adminGetDurability(admin), D_LEVEL_FINEST,
                    "Completeness of local groups not yet determined. Still waiting for remote groups to be handled first.\n");
        }
    } else {
        arg.complete = FALSE;
    }
    d_lockUnlock(d_lock(admin));

    return arg.complete;
}

c_ulong
d_adminGetFellowCount(
    d_admin admin)
{
    c_ulong result;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    result = 0;

    if(admin){
        d_lockLock(d_lock(admin));
        result = d_tableSize(admin->fellows);
        d_lockUnlock(d_lock(admin));
    }
    return result;
}

c_ulong
d_adminGetInitialFellowCount(
    d_admin admin)
{
    c_ulong result = 0;

    assert(d_adminIsValid(admin));

    if (admin) {
        d_lockLock(d_lock(admin));
        result = d_tableSize(admin->initial_fellows);
        d_lockUnlock(d_lock(admin));
    }
    return result;
}

c_ulong
d_adminGetNameSpacesCount(
    d_admin admin)
{
    c_ulong result;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    result = 0;

    if(admin){
        d_lockLock(d_lock(admin));
        result = c_iterLength(admin->nameSpaces);
        d_lockUnlock(d_lock(admin));
    }
    return result;
}

static d_nameSpace
d_adminGetNameSpaceNoLock(
    d_admin admin,
    os_char* name)
{
    d_nameSpace nameSpace = NULL;
    c_ulong i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin && name){

        for(i=0; i<c_iterLength(admin->nameSpaces) && !nameSpace; i++){
            nameSpace = d_nameSpace(c_iterObject(admin->nameSpaces, i));

            if(strcmp(d_nameSpaceGetName(nameSpace), name) == 0){
                nameSpace = d_nameSpace(d_objectKeep(d_object(nameSpace)));
            } else {
                nameSpace = NULL;
            }
        }
    }
    return nameSpace;
}


d_nameSpace
d_adminGetNameSpace(
    d_admin admin,
    os_char* name)
{
    d_nameSpace nameSpace = NULL;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin && name){
        d_lockLock(d_lock(admin));
        nameSpace = d_adminGetNameSpaceNoLock(admin, name);
        d_lockUnlock(d_lock(admin));
    }
    return nameSpace;
}


c_bool
d_adminFellowWalk(
    d_admin admin,
    c_bool ( * action ) (d_fellow fellow, c_voidp userData),
    c_voidp userData)
{
    c_bool result;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    result = FALSE;

    if(admin){
        result = TRUE;
        d_lockLock(d_lock(admin));
        d_tableWalk(admin->fellows, action, userData);
        d_lockUnlock(d_lock(admin));
    }
    return result;
}

c_bool
d_adminInitialFellowWalk(
    d_admin admin,
    c_bool ( * action ) (d_fellow fellow, c_voidp userData),
    c_voidp userData)
{
    c_bool result;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    result = FALSE;

    if(admin){
        result = TRUE;
        d_lockLock(d_lock(admin));
        (void)d_tableWalk(admin->initial_fellows, action, userData);
        d_lockUnlock(d_lock(admin));
    }
    return result;
}


void
d_adminNameSpaceWalk(
    d_admin admin,
    void (*action) (d_nameSpace nameSpace, c_voidp userData),
    c_voidp userData)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        d_lockLock(d_lock(admin));
        c_iterWalk(admin->nameSpaces, (c_iterWalkAction)action, userData);
        d_lockUnlock(d_lock(admin));
    }
}

static void
collectNsWalk(
    d_nameSpace ns, void* userData)
{
    c_iter nameSpaces = (c_iter)userData;

    if (ns)
    {
        d_objectKeep(d_object(ns));
        assert(nameSpaces != NULL);
        (void)c_iterInsert (nameSpaces, ns);
    }
}

static void
deleteNsWalk(
   void* o, void* userData)
{
    OS_UNUSED_ARG(userData);
    d_nameSpaceFree(o);
}

c_iter
d_adminNameSpaceCollect(
    d_admin admin)
{
    c_iter result;
    result = c_iterNew(NULL);
    d_adminNameSpaceWalk(admin, collectNsWalk, result);
    return result;
}

void
d_adminNameSpaceCollectFree(
    d_admin admin,
    c_iter nameSpaces)
{
    OS_UNUSED_ARG(admin);
    c_iterWalk(nameSpaces, deleteNsWalk, NULL);
    c_iterFree(nameSpaces);
}


void
d_adminGroupWalk(
    d_admin admin,
    c_bool ( * action ) (d_group group, c_voidp userData),
    c_voidp args)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        d_lockLock(d_lock(admin));
        d_tableWalk(admin->groups, action, args);
        d_lockUnlock(d_lock(admin));
    }
}

void
d_adminAddListener(
    d_admin admin,
    d_eventListener listener)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(listener), D_EVENT_LISTENER) == TRUE);

    if (admin && listener) {
        if (admin->initMask & D__INIT_FLAG_EVENT_MUTEX) {
            os_mutexLock(&admin->eventMutex);
            admin->eventListeners = c_iterInsert(admin->eventListeners, listener);
            os_mutexUnlock(&admin->eventMutex);
        }
    }
}

void
d_adminRemoveListener(
    d_admin admin,
    d_eventListener listener)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(listener), D_EVENT_LISTENER) == TRUE);

    if (admin && listener) {
        if (admin->initMask & D__INIT_FLAG_EVENT_MUTEX) {
            os_mutexLock(&admin->eventMutex);
            c_iterTake(admin->eventListeners, listener);
            os_mutexUnlock(&admin->eventMutex);
        }
    }
}


/* TODO: redesign this to pass through only userdata */
void
d_adminNotifyListeners(
    d_admin admin,
    c_ulong mask,
    d_fellow fellow,
    d_nameSpace nameSpace,
    d_group group,
    c_voidp userData)
{
    d_adminEvent event;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        event = d_adminEventNew(mask, fellow, nameSpace, group, userData);
        os_mutexLock(&admin->eventMutex);
        admin->eventQueue = c_iterAppend(admin->eventQueue, event);
        os_condSignal(&admin->eventCondition);
        os_mutexUnlock(&admin->eventMutex);
    }
}


d_adminEvent
d_adminEventNew(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace nameSpace,
    d_group group,
    c_voidp userData)
{
    d_adminEvent evt = NULL;
    d_networkAddress addr;
    c_char *topic, *partition;
    v_group vgroup;

    /* Allocate adminEvent object */
    evt = d_adminEvent(os_malloc(C_SIZEOF(d_adminEvent)));
    if (evt) {
        /* Call super-init */
        d_objectInit(d_object(evt), D_ADMIN_EVENT,
                     (d_objectDeinitFunc)d_adminEventDeinit);
        /* Initialize adminEvent */
        evt->event = event;
        evt->userData = userData;
        if (fellow) {
            assert(d_fellowIsValid(fellow));
            addr = d_fellowGetAddress(fellow);
            evt->fellow = d_fellowNew(addr, d_fellowGetState(fellow), TRUE);
            d_networkAddressFree(addr);
        } else {
            evt->fellow = NULL;
        }
        if (group) {
            assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
            partition = d_groupGetPartition(group);
            topic = d_groupGetTopic(group);
            evt->group = d_groupNew (partition, topic, d_groupGetKind(group),
                                        d_groupGetCompleteness(group),
                                        d_groupGetQuality(group));
            vgroup = d_groupGetKernelGroup(group);
            if (vgroup) {
                d_groupSetKernelGroup(evt->group, vgroup);
                c_free(vgroup);
            }
            os_free(partition);
            os_free(topic);
        } else {
            evt->group = NULL;
        }
        if (nameSpace) {
            assert(d_nameSpaceIsValid(nameSpace));
            evt->nameSpace = nameSpace;
        } else {
            evt->nameSpace = NULL;
        }
    }
    return evt;
}


void
d_adminEventDeinit(
    d_adminEvent event)
{
    assert(d_adminEventIsValid(event));

    if (event->fellow) {
        d_fellowFree(event->fellow);
    }
    if (event->group) {
        d_groupFree(event->group);
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(event));
}


void
d_adminEventFree(
    d_adminEvent event)
{
    assert(d_adminEventIsValid(event));

    d_objectFree(d_object(event));
}


void*
d_adminEventThreadStart(
    void* arg)
{
    d_thread self = d_threadLookupSelf ();
    d_admin admin;
    d_adminEvent event;
    d_eventListener listener;
    c_ulong i;

    admin = d_admin(arg);

    while(admin->eventThreadTerminate == FALSE){
        os_mutexLock(&admin->eventMutex);
        event = c_iterTakeFirst(admin->eventQueue);
        os_mutexUnlock(&admin->eventMutex);

        while(event){
            for(i=0; i<c_iterLength(admin->eventListeners); i++){
                listener = d_eventListener(c_iterObject(admin->eventListeners, i));

                if((listener->interest & event->event) == event->event){
                    (void)listener->func(event->event, event->fellow, event->nameSpace,
                                         event->group, event->userData, listener->args);
                }
            }
            d_adminEventFree(event);
            os_mutexLock(&admin->eventMutex);
            event = c_iterTakeFirst(admin->eventQueue);
            os_mutexUnlock(&admin->eventMutex);
        }
        os_mutexLock(&admin->eventMutex);

        if((c_iterLength(admin->eventQueue) == 0) && (admin->eventThreadTerminate == FALSE)){
            d_condWait(self, &admin->eventCondition, &admin->eventMutex);
        }
        os_mutexUnlock(&admin->eventMutex);
    }

    /* Added assert to ensure the conflictResolver is always stopped after this thread */
    assert(admin->conflictResolver != NULL);

    return NULL;
}



d_actionQueue
d_adminGetActionQueue(
    d_admin admin)
{
    d_actionQueue queue = NULL;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin) {
        d_lockLock(d_lock(admin));
        queue = admin->actionQueue;
        d_lockUnlock(d_lock(admin));
    }
    return queue;
}


static c_bool
getIncompatibleStateCount(
    d_fellow fellow,
    c_voidp args)
{
    c_ulong* count;

    count = (c_ulong*)args;

    if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_STATE){
        (*count)++;
    }
    return TRUE;
}

c_ulong
d_adminGetIncompatibleStateCount(
    d_admin admin)
{
    c_ulong count;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    count = 0;

    if(admin) {
        d_lockLock(d_lock(admin));
        d_tableWalk(admin->fellows, getIncompatibleStateCount, &count);
        d_lockUnlock(d_lock(admin));
    }
    return count;
}

static c_bool
getIncompatibleDataModelCount(
    d_fellow fellow,
    c_voidp args)
{
    c_ulong* count;

    count = (c_ulong*)args;

    if (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL) {
        (*count)++;
    }
    return TRUE;
}

c_ulong
d_adminGetIncompatibleDataModelCount(
    d_admin admin)
{
    c_ulong count;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    count = 0;

    if(admin) {
        d_lockLock(d_lock(admin));
        d_tableWalk(admin->fellows, getIncompatibleDataModelCount, &count);
        d_lockUnlock(d_lock(admin));
    }
    return count;
}

c_bool
d_adminAddReaderRequest(
    d_admin admin,
    d_readerRequest request)
{
    d_readerRequest found;
    c_bool result;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin && request){
        d_lockLock(d_lock(admin));
        found = d_tableInsert(admin->readerRequests, request);
        d_lockUnlock(d_lock(admin));

        if(!found){
            d_objectKeep(d_object(request));
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

c_bool
d_adminRemoveReaderRequest(
    d_admin admin,
    d_networkAddress source)
{
    d_readerRequest request, found;
    c_bool result;
    v_handle handle;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin && source){
        handle.index = source->systemId;
        handle.serial = source->localId;
        handle.server = 0;
        request = d_readerRequestProxyNew(handle);

        d_lockLock(d_lock(admin));
        found = d_tableRemove(admin->readerRequests, request);
        d_lockUnlock(d_lock(admin));

        d_readerRequestFree(request);

        if(found){
            d_readerRequestFree(found);
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

d_readerRequest
d_adminGetReaderRequest(
    d_admin admin,
    d_networkAddress source)
{
    d_readerRequest request, found;
    v_handle handle;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin && source){
        handle.index  = source->systemId;
        handle.serial = source->localId;
        handle.server = 0;
        request = d_readerRequestProxyNew(handle);

        d_lockLock(d_lock(admin));
        found = d_tableFind(admin->readerRequests, request);

        if(found){
            d_objectKeep(d_object(found));
        }
        d_lockUnlock(d_lock(admin));
        d_readerRequestFree(request);
    } else {
        found = NULL;
    }
    return found;
}

c_bool
d_adminCheckReaderRequestFulfilled(
    d_admin admin,
    d_readerRequest request)
{
    c_bool result;
    d_readerRequest found;
    v_reader reader;
    v_handle handle;
    v_handleResult handleResult;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(admin && request){
        d_lockLock(d_lock(admin));
        found = d_tableFind(admin->readerRequests, request);

        if(found){
            if(!d_readerRequestHasChains(found)){
                if(d_readerRequestAreGroupsComplete(found)){
                    found = d_tableRemove(admin->readerRequests, request);
                    if (d_readerRequestGetGroupIgnored(found) == FALSE){
                        handle = d_readerRequestGetHandle(found);
                        handleResult = v_handleClaim(handle, (v_object*)&reader);

                        if(handleResult == V_HANDLE_OK){
                            v_readerNotifyStateChange(reader, TRUE);
                            v_handleRelease(handle);
                        }
                    }
                    result = TRUE;
                    d_readerRequestFree(found);
                } else {
                    result = FALSE;
                }
            } else {
                result = FALSE;
            }
        } else {
            result = FALSE;
        }
        d_lockUnlock(d_lock(admin));
    } else {
        result = FALSE;
    }
    return result;
}


c_bool
d_nameSpaceCountMastersForRoleWalk (
    d_fellow fellow,
    void* userData)
{
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowAddress, nsMaster;
    struct masterCountForRoleHelper* helper;
    d_name fellowRole;

    helper = (struct masterCountForRoleHelper*)userData;
    fellowRole = d_fellowGetRole(fellow);

    if (fellowRole) {
        if (strcmp(fellowRole, helper->role) == 0) {
            fellowNameSpace = d_fellowGetNameSpace (fellow, helper->nameSpace);

            if (fellowNameSpace) {
                fellowAddress = d_fellowGetAddress (fellow);
                nsMaster = d_nameSpaceGetMaster (fellowNameSpace);

                if (d_networkAddressEquals (fellowAddress, nsMaster)) {
                    helper->masterCount++;
                }
                d_networkAddressFree (fellowAddress);
                d_networkAddressFree (nsMaster);
            }
        }
    }

    return TRUE;
}


void
d_adminReportMaster(
    d_admin admin,
    d_fellow fellow,
    d_nameSpace nameSpace)
{
    /* Check for conflicts, and if a conflict is found, add it to the resolver queue. */
    d_conflictMonitorCheckForConflicts(admin->conflictMonitor, fellow, nameSpace);
}


void
d_adminReportDelayedInitialSet (
    d_admin admin,
    d_nameSpace nameSpace,
    d_fellow fellow)
{
    d_durability durability;
    d_nameSpace localNameSpace;
    d_quality q;
    d_subscriber subscriber;
    d_nameSpacesRequestListener nsrListener;
    d_networkAddress addr;

    subscriber = d_adminGetSubscriber(admin);
    nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);
    durability = admin->durability;
    localNameSpace = d_adminGetNameSpace(admin, d_nameSpaceGetName(nameSpace));

    /* Do not report when the namespace is not configured to allow delayed alignment. */
    if(localNameSpace && d_nameSpaceGetDelayedAlignment(localNameSpace)) {

        /* Get quality of local namespace */
        q = d_nameSpaceGetInitialQuality(localNameSpace);

        /* If own quality is non-zero, delayed alignment is not allowed. */
        if (d_qualityCompare(q, D_QUALITY_ZERO) == OS_EQUAL) {

            /* When the namespace-master is not confirmed, the service is determining masters, in which case this will resolve itself. */
            if(d_nameSpaceIsMasterConfirmed(localNameSpace)) {

                /* Set namespace state to pending - so incoming namespace messages will not re-trigger the namespacesListener. */
                d_nameSpaceMasterPending(localNameSpace);

                /* Set master to 0 */
                addr = d_networkAddressNew(0,0,0);
                d_nameSpaceSetMaster(localNameSpace, addr);
                d_networkAddressFree(addr);

                /* Let others know that I'm reconsidering my master */
                d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

                d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Delayed initial set discovered for namespace '%s'.\n",
                        d_nameSpaceGetName(nameSpace));

                /* Notify others that a delayed initial set is available */
                d_adminNotifyListeners(admin, D_NAMESPACE_DELAYED_INITIAL, fellow, localNameSpace, NULL, NULL);
            }
        } else {
            d_printTimedEvent(
                    durability, D_LEVEL_FINER,
                    "No delayed alignment for local namespace '%s', local quality is non-zero.\n",
                    d_nameSpaceGetName(nameSpace));
        }
    } else {
        d_printTimedEvent(
                durability, D_LEVEL_FINEST,
                "No delayed alignment for local namespace '%s', namespace does not exist locally, or delayed alignment is not enabled.\n",
                d_nameSpaceGetName(nameSpace));
    }
    if (localNameSpace) {
        d_nameSpaceFree(localNameSpace);
    }
}

static c_bool
cleanupTerminateFellowsAction(
    d_fellow fellow,
    c_voidp args)
{
    os_timeM timeToRemove;
    struct cleanupData *data;
    d_durability durability;
    d_configuration config;

    data = (struct cleanupData*)(args);
    durability = data->durability;
    config = d_durabilityGetConfiguration(durability);
    /* Select the fellow if its removalTime is more than
     * config->terminateRemovalPeriod in the past. This
     * is currently fixed to 5sec.
     */
    timeToRemove = os_timeMAdd(fellow->removalTime, config->terminateRemovalPeriod);
    if (os_timeMCompare(timeToRemove, os_timeMGet()) == OS_LESS) {
        /* The fellow has been removed at least terminateRemovalPeriod ago.
         * Now it is safe to remove it from the admin->terminateFellows list
         */
        data->fellows = c_iterInsert(data->fellows, fellow);
    }
    return TRUE;
}

/**
 * \brief Clean up fellows that have terminated.
 *
 * Fellows that have recently terminated are stored some while
 * in admin->terminateFellows. The durability service will be
 * immune for messages from the fellow that are received AFTER
 * the fellow has been removed for config->terminateRemovalPeriod
 * seconds. A monotonic clock is used so that any time spent in
 * hibernate state is not counted.
 */
void
d_adminCleanupTerminateFellows(
    d_admin admin)
{
    struct cleanupData data;
    d_fellow fellow;
    d_networkAddress address;
    d_durability durability;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    durability      = admin->durability;
    data.fellows    = c_iterNew(NULL);
    data.durability = durability;

    d_lockLock(d_lock(admin));
    (void)d_tableWalk(admin->terminateFellows, cleanupTerminateFellowsAction , &data);
    fellow = c_iterTakeFirst(data.fellows);
    while(fellow){
        address = d_fellowGetAddress(fellow);
        d_printTimedEvent(durability, D_LEVEL_FINER,
                          "Cleaning up terminated fellow %u\n",
                          address->systemId);
        d_networkAddressFree(address);
        (void)d_tableRemove(admin->terminateFellows, fellow);
        d_fellowFree(fellow);
        fellow = c_iterTakeFirst(data.fellows);
    }
    d_lockUnlock(d_lock(admin));
    c_iterFree(data.fellows);
}


/**
 * \brief Collect the group if it matches the specified partitionExpr
 *        and topic in the argument.
 *
 * Groups are only collected under the following conditions.
 * - The group must match a namespace for which I am aligner, and
 * - The orginating request for this group was explicitly addressed
 *   to me, or the request was addressed to everybody and I am master
 *   for the namespace of the group.
 *
 * This implies that groups for which I am not an aligner are
 * never collected.
 *
 * Every group that is collected is added to helper->matchingGroups.
 */
static c_bool
collectMatchingGroups(
    d_group group,
    c_voidp arg)
{
    c_string partitionExpr;
    d_group duplicate;
    struct collectMatchingGroupsHelper *helper = (struct collectMatchingGroupsHelper *)arg;
    c_bool isAligner;
    c_bool isResponsible = FALSE;
    c_bool masterKnown = FALSE;
    c_bool toCollect = FALSE;
    d_nameSpace nameSpace;
    d_admin admin;
    c_iterIter iter;
    d_durability durability;

    assert(d_groupIsValid(group));

    admin = helper->admin;
    assert(d_adminIsValid(admin));
    durability = d_adminGetDurability(admin);
    iter = c_iterIterGet(helper->partitions);

    d_printTimedEvent(durability, D_LEVEL_FINER,
          "  Checking group '%s.%s'\n", group->partition, group->topic);
    /* The server reacts to all requests, even requests for private groups.
     * This is to enable remote monitoring use case where a remote application
     * wants to see the group state of a remote federation. It is the client's
     * responsibility to prevent unnecessary requests, not the server's responsibility
     * to deny a request.
     */
    while (((partitionExpr = (c_string)c_iterNext(&iter)) != NULL) && (!toCollect)) {
        /* Look for a partitionExpr.topic that matches the group. */
        d_printTimedEvent(durability, D_LEVEL_FINEST,
              "    Using partition expression '%s'\n", partitionExpr);

        if ( d_patternMatch(group->partition, partitionExpr) && (strcmp(group->topic, helper->topic) == 0) ) {

            d_printTimedEvent(durability, D_LEVEL_FINER,
                  "    Match for partition expression '%s' and topic '%s'\n", partitionExpr, helper->topic);

            /* Collect group data if I am aligner for the group AND the
             * request was addressed to me, or if it the request was
             * addressed to everyone and I am master for the group
             */
            nameSpace = d_adminGetNameSpaceForGroupNoLock(admin, group->partition, group->topic);
            if (nameSpace) {
                isAligner = d_nameSpaceIsAligner(nameSpace);
                masterKnown = d_nameSpaceIsMasterConfirmed(nameSpace);
                if (masterKnown ) {
                    isResponsible = d_nameSpaceMasterIsMe(nameSpace, admin);
                }

                /* Only collect groups for which I can be aligner */
                if (isAligner) {
                    if (helper->forMe) {
                        /* Always collect groups for requests that are explicitly addressed to me. */
                        toCollect = TRUE;
                    } else if (helper->forEverybody) {
                        /* Always collect groups for DurabilityStateRequests that are addressed to everybody.
                         * Only collect groups for HistoricalDataRequests hat are addressed to everybody only if I am responsible.
                         */
                        toCollect = (helper->isHistoricalDataRequest) ? (isResponsible==TRUE) : TRUE;
                    }
                }

                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "    Matching namespace '%s' found for group '%s.%s' (isAligner=%d, isResponsible=%d, forMe=%d, forEverybody=%d, toCollect=%d)\n",
                      nameSpace->name, group->partition, group->topic, isAligner, isResponsible, helper->forMe, helper->forEverybody, toCollect);

                helper->isAligner |= isAligner;           /* I am an aligner for a matching request */
                helper->isResponsible |= isResponsible;   /* I am responsible for a matching request */

                if (toCollect) {
                    /* The group is a valid candidate to be aligned */
                    duplicate = d_tableInsert(helper->matchingGroups, d_objectKeep(d_object(group)));
                    if (!duplicate) {
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                              "    Collecting group '%s.%s' collected partition expression '%s' and topic '%s'\n",
                              group->partition, group->topic, partitionExpr, helper->topic);
                    }
                    /* Done checking this group, I am going to align it */
                }
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "    No namespace found for group '%s.%s'\n", group->partition, group->topic);
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                  "    No match for partition expression '%s', skip\n", partitionExpr);
        }
    } /* while */
    return !toCollect;
}


/**
 * \brief Check if the nameSpace matches one of the partition expressions and
 *        topic in arg. To match the namespace must be configured as aligner.
 */
static void
hasMatchingAlignerNameSpace(
    d_nameSpace nameSpace,
    void *arg)
{
    struct collectMatchingGroupsHelper *helper = (struct collectMatchingGroupsHelper *)arg;
    c_iterIter iter;
    c_string partitionExpr;
    d_admin admin;
    d_durability durability;

    assert(d_nameSpaceIsValid(nameSpace));

    admin = helper->admin;
    /* Not yet found evidence that a response must be sent */
    durability = d_adminGetDurability(admin);

    d_printTimedEvent(durability, D_LEVEL_FINEST,
          "  Checking namespace '%s'\n", nameSpace->name);
    if (d_nameSpaceIsAligner(nameSpace)) {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
              "    Namespace '%s' is aligner, check against partition expressions\n", nameSpace->name);
        iter = c_iterIterGet(helper->partitions);
        while (((partitionExpr = (c_string)c_iterNext(&iter)) != NULL) && (!helper->isAligner)) {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                  "      Check against partition expression '%s'\n", partitionExpr);
            if (d_nameSpaceIsIn(nameSpace, partitionExpr, helper->topic)) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "      Partition expression '%s' does match aligner namespace '%s'\n", partitionExpr, nameSpace->name);
                /* The nameSpace is aligner and matches partitionExpr.topic */
                helper->isAligner = TRUE;
                /* Check if I am responsible for alignment (i.e., I am able to align) */
                if (d_nameSpaceIsMasterConfirmed(nameSpace)) {
                    helper->isResponsible |= d_nameSpaceMasterIsMe(nameSpace, admin);
                    helper->masterKnown = TRUE;
                }

                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "      helper->isAligner=%d, helper->isResponsible=%d helper->masterKnown=%d\n",
                      helper->isAligner, helper->isResponsible, helper->masterKnown);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "      Partition expression '%s' does NOT match aligner namespace '%s'\n", partitionExpr, nameSpace->name);
            }
        }
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
              "    Namespace '%s' is NOT aligner, skip\n");

    }
}


/**
 * \brief Collect all groups that match the specified partitionExpr.topic
 *        in the argument, and decide if a response is required.
 */
void
d_adminCollectMatchingGroups(
    d_admin admin,
    void *arg)
{
    struct collectMatchingGroupsHelper *helper = (struct collectMatchingGroupsHelper *)arg;

    d_durability durability = d_adminGetDurability(admin);

    d_printTimedEvent(durability, D_LEVEL_FINER,
          "  Start calculating the matching groups\n");

    /* Collect all groups for which data must be retrieved. */
    d_adminGroupWalk(admin, collectMatchingGroups, helper);

    d_printTimedEvent(durability, D_LEVEL_FINER,
          "  End calculating matching groups (%d)\n", d_tableSize(helper->matchingGroups));

    helper->groupFound = (d_tableSize(helper->matchingGroups) > 0);

    /* We now have collected all groups that matched the request. */
    if (!helper->isAligner) {
        /* I have not yet found a namespace that matches one of the
         * partition/topic expressions and for which I am aligner. This
         * might be because only namespaces for groups have been checked,
         * but there may be namespaces for which there are no groups yet.
         * So let's find out if there exists a namespace that matches
         * my request and for which I am aligner.
         */

        d_printTimedEvent(durability, D_LEVEL_FINEST,
                  "  No matching aligner namespace found yet, check all namespaces now\n");

        d_adminNameSpaceWalk(admin, hasMatchingAlignerNameSpace, helper);
    }
    return;
}

/**
 * \brief Get the sql expressions for which the static filter expression matches the
 *        partition and topic
 *
 * If no match is found NULL is returned.
 */
char *
d_adminGetStaticFilterExpression(
    d_admin admin,
    char *partition,
    char *topic)
{
    c_iterIter iter, iter2;
    d_durability durability;
    d_configuration config;
    c_bool matchFound = FALSE;
    d_filter filter;
    d_element element;

    assert(d_adminIsValid(admin));
    assert(partition);
    assert(topic);

    /* Walk over the filters and return the sql expression as
     * soon as there is a match
     */
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    if (config->filters) {
        iter = c_iterIterGet(config->filters);
        while ((!matchFound) && ((filter = (d_filter)c_iterNext(&iter)) != NULL)) {
            iter2 = c_iterIterGet(filter->elements);
            while ((!matchFound) && ((element = (d_element)c_iterNext(&iter2)) != NULL)) {
                 if (d_patternMatch(partition, element->partition) &&
                     d_patternMatch(topic, element->topic)) {
                     /* Match found */
                     matchFound = TRUE;
                 }
            }
        }
    }
    if (matchFound) {
        return filter->sqlExpression;
    } else {
        return NULL;
    }
}


d_client
d_adminGetClient(
    d_admin admin,
    d_networkAddress address)
{
    d_client dummy, found;

    assert(d_adminIsValid(admin));

    d_lockLock(d_lock(admin));
    dummy = d_clientNew(address);
    found = d_tableFind(admin->clients, dummy);
    d_clientFree(dummy);
    if (found) {
        found = d_client(d_objectKeep(d_object(found)));
    }
    d_lockUnlock(d_lock(admin));
    return found;
}


d_client
d_adminAddClient(
    d_admin admin,
    d_client client)
{
    d_client result, duplicate;

    assert(d_adminIsValid(admin));
    assert(d_clientIsValid(client));
    assert(admin->clients);  /* If and only if config->clientDurabilityEnabled is TRUE */

    d_lockLock(d_lock(admin));
    duplicate = d_tableInsert(admin->clients, client);
    if (!duplicate) {
        d_networkAddress clientAddr = d_clientGetAddress(client);
        result = d_client(d_objectKeep(d_object(client)));
        d_printTimedEvent(admin->durability, D_LEVEL_INFO,
            "Adding client for federation %u.\n", clientAddr->systemId);
        d_networkAddressFree(clientAddr);
    } else {
        result = d_client(d_objectKeep(d_object(duplicate)));
    }
    d_lockUnlock(d_lock(admin));
    return result;
}


d_client
d_adminRemoveClient(
    d_admin admin,
    d_client client)
{
    d_client found = NULL;

    assert(d_adminIsValid(admin));
    assert(d_clientIsValid(client));
    assert(admin->clients);  /* If and only if config->clientDurabilityEnabled is TRUE */

    d_lockLock(d_lock(admin));
    found = d_client(d_tableRemove(admin->clients, client));
    if (found) {
        d_networkAddress clientAddr = d_clientGetAddress(found);
        d_printTimedEvent(admin->durability, D_LEVEL_INFO,
        "Removing client for federation %u.\n", clientAddr->systemId);
        d_networkAddressFree(clientAddr);
    }
    d_lockUnlock(d_lock(admin));
    return found;
}


d_client
d_adminFindClientByClientId(
    d_admin admin,
    struct _DDS_Gid_t clientId)
{
    d_tableIter tableIter;
    d_client client = NULL;
    c_bool found = FALSE;

    assert(d_adminIsValid(admin));

    /* Currently we do a linear walk over admin->clients to find the
     * client with the specified clientId. Better would be to use a
     * tree structure
     */
    d_lockLock(d_lock(admin));
    client = d_client(d_tableIterFirst(admin->clients, &tableIter));
    while ((client) && (!found)) {
        if ((found = ((client->clientId.prefix == clientId.prefix) && (client->clientId.suffix == clientId.suffix))) == FALSE) {
            client = d_client(d_tableIterNext(&tableIter));
        }
    }
    d_lockUnlock(d_lock(admin));
    return client;
}

os_uint32
d_adminGetNextSeqNum(
    d_admin admin)
{
    os_uint32 seqnum;
    c_bool wrap = FALSE;

    assert(d_adminIsValid(admin));

    /* Increment the sequence number (32-bit unsigned) atomically. */
    os_mutexLock(&admin->seqnumMutex);
    if (admin->seqnum == D_MAX_VALID_SEQNUM) {
        wrap = TRUE;
        admin->seqnum = 1;
    } else {
        admin->seqnum += 1;
    }
    seqnum = admin->seqnum;
    os_mutexUnlock(&admin->seqnumMutex);
    if (wrap) {
        d_durability durability = d_adminGetDurability(admin);
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Durability service sequence number has reached max (%"PA_PRIu32"), wrapping occurred\n", D_MAX_VALID_SEQNUM);
    }
    return seqnum;
}

c_iter
d_adminGetNameSpaces(
    d_admin admin)
{
    c_iter nameSpaces = NULL;
    c_iterIter iter;
    d_nameSpace nameSpace;

    if (c_iterLength(admin->nameSpaces) > 0) {
        nameSpaces = c_iterNew(NULL);
        iter = c_iterIterGet(admin->nameSpaces);
        while ((nameSpace = d_nameSpace(c_iterNext(&iter))) != NULL) {
            nameSpaces = c_iterAppend(nameSpaces, d_objectKeep(d_object(nameSpace)));
        }
    }
    return nameSpaces;
}

void
d_admin_sync_mergeStates(
    d_admin admin)
{
    d_durability durability;
    d_nameSpace nameSpace;
    c_iter nameSpaces;
    c_bool synced = FALSE;
    c_iterIter iter;
    d_mergeState mergeState, advertisedMergeState;
    durability = d_adminGetDurability(admin);

     d_printTimedEvent(durability, D_LEVEL_FINER,
             "No more pending conflicts, going to sync the advertised namespace states for which I am confirmed master\n");

     nameSpaces = d_adminGetNameSpaces(admin);
     iter = c_iterIterGet(nameSpaces);
     while ((nameSpace = d_nameSpace(c_iterNext(&iter))) != NULL) {
         if (d_nameSpaceMasterIsMe(nameSpace, admin) && d_nameSpaceIsMasterConfirmed(nameSpace)) {
             c_long mv, amv;

             mergeState = d_nameSpaceGetMergeState(nameSpace, NULL);
             advertisedMergeState = d_nameSpaceGetAdvertisedMergeState(nameSpace);
             /* I am the confirmed master of a namespace, so the mergeState and advertisedMergeState
              * can never be cleared
              */
             assert(mergeState == NULL || advertisedMergeState == NULL || strcmp(mergeState->role, advertisedMergeState->role) == 0);

             mv = mergeState ? (c_long)mergeState->value : -1;
             amv = advertisedMergeState ? (c_long)advertisedMergeState->value : -1;

             d_printTimedEvent(durability, D_LEVEL_FINE,
                     " - Syncing namespace state for namespace '%s' in role '%s' (%d -> %d)\n",
                     nameSpace->name, nameSpace->mergeState->role, amv, mv);

             /* Sync the advertised mergeState with the internal mergeState */
             if (mv != amv) {
                 d_nameSpaceSyncMergeState(nameSpace);
                 synced = TRUE;
             }

             d_mergeStateFree(mergeState);
             d_mergeStateFree(advertisedMergeState);

         }
     }

     /* Cleanup the nameSpaces */
     nameSpace = d_nameSpace(c_iterTakeFirst(nameSpaces));
     while(nameSpace){
         d_nameSpaceFree(nameSpace);
         nameSpace = d_nameSpace(c_iterTakeFirst(nameSpaces));
     }
     c_iterFree (nameSpaces);

    /* Advertise namespaces in case syncing took place */
    if (synced) {
        d_subscriber subscriber = d_adminGetSubscriber(admin);
        d_nameSpacesRequestListener nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);

        d_nameSpacesRequestListenerReportNameSpaces(nsrListener);
    }

}

void
d_adminReportGroup(
    d_admin admin,
    d_group group)
{
    d_newGroup newGroup;
    d_completeness completeness;
    d_networkAddress addr;
    c_char *partition, *topic;
    d_publisher publisher;
    d_durabilityKind kind;
    c_bool inNameSpace;
    d_durability durability;
    d_quality quality;

    if(!d_groupIsPrivate(group)){
        durability   = d_adminGetDurability(admin);
        publisher    = d_adminGetPublisher(admin);

        completeness = d_groupGetCompleteness(group);
        partition    = d_groupGetPartition(group);
        topic        = d_groupGetTopic(group);
        quality      = d_groupGetQuality(group);
        kind         = d_groupGetKind(group);

        inNameSpace = d_adminGroupInAlignerNS(admin, partition, topic);

        if((inNameSpace == TRUE) || (completeness != D_GROUP_COMPLETE)) {
            d_printTimedEvent(durability, D_LEVEL_FINER,
                                "Reporting group %s.%s, kind: %u, completeness: %u\n",
                                partition, topic, kind, completeness);
            newGroup = d_newGroupNew(admin, partition, topic, kind, completeness,
                                     quality);
            addr = d_networkAddressUnaddressed();
            d_publisherNewGroupWrite(publisher, newGroup, addr);
            d_networkAddressFree(addr);
            d_newGroupFree(newGroup);
        }
        os_free(partition);
        os_free(topic);
    }
    return;
}

struct groupAdmin {
    d_group group;
    c_bool isAligner;
};

struct collectNameSpaceGroupsArg {
    c_iter groups /* d_group */;
    d_admin admin;
};

static c_bool
collectNameSpaceGroups(
    d_group group,
    c_voidp arg /* struct collectNameSpaceGroupsArg * */)
{
    struct collectNameSpaceGroupsArg *a = (struct collectNameSpaceGroupsArg *)arg;
    d_nameSpace inNameSpace;

    if(!d_groupIsPrivate(group) && (d_groupGetCompleteness(group) > D_GROUP_UNKNOWN)) {
        inNameSpace = d_adminGetNameSpaceForGroupNoLock(a->admin, group->partition, group->topic);
        if (inNameSpace) {
            assert(d_adminInNameSpace(inNameSpace, group->partition, group->topic, FALSE));
            a->groups = c_iterAppend(a->groups, d_objectKeep(d_object(group)));
        }
    }

    return TRUE;
}

void
d_adminMarkNameSpaceKernelGroupsCompleteness(
    _Inout_ d_admin admin,
    _In_ c_bool complete)
{
    d_durability durability;
    u_domain domain;
    struct collectNameSpaceGroupsArg arg;
    d_group g;

    arg.groups = NULL;
    arg.admin = admin;

    assert(d_adminIsValid(admin));

    durability = d_adminGetDurability(admin);
    domain = u_participantDomain(u_participant(durability->service));
    (void)u_domainSetAlignedState(domain, (complete == FALSE) ? OS_FALSE : OS_TRUE);

    d_adminGroupWalk(admin, &collectNameSpaceGroups, &arg);
    while((g = c_iterTakeFirst(arg.groups)) != NULL){
        d_printTimedEvent(d_adminGetDurability(admin), D_LEVEL_FINEST, "Marking kernel group '%s.%s' %s\n",
            g->partition, g->topic, complete ? "COMPLETE" : "INCOMPLETE");
        d_groupSetKernelGroupCompleteness(g, complete);

        d_groupFree(g);
    }
    c_iterFree(arg.groups);

    if (complete) {
        (void)u_domainTransactionsPurge(domain);
    }
}

void
d_adminStoreGroup(
    d_admin admin,
    d_group group)
{
    d_store store;
    d_nameSpace nameSpace;
    d_durabilityKind kind, nskind;
    d_storeResult result;
    d_durability durability;

    assert(admin);
    assert(d_adminIsValid(admin));
    assert(group);
    assert(d_groupIsValid(group));

    d_lockLock(d_lock(admin));
    durability = admin->durability;
    nameSpace = d_adminGetNameSpaceForGroupNoLock(admin, group->partition, group->topic);
    store = d_subscriberGetPersistentStore(admin->subscriber);
    d_lockUnlock(d_lock(admin));

    if ((nameSpace) && (store)) {
        kind = d_groupGetKind(group);
        nskind = d_nameSpaceGetDurabilityKind(nameSpace);

        if ((kind == D_DURABILITY_PERSISTENT) &&
            ((nskind == D_DURABILITY_ALL) ||
             (nskind == D_DURABILITY_PERSISTENT))) {

            result = d_storeGroupStore(store, group, nameSpace);
            if (result == D_STORE_RESULT_OK) {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Persistent group %s.%s stored on disk.\n", group->partition, group->topic);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Storing persistent group %s.%s on disk failed (error code: %d).\n",
                    group->partition, group->topic, result);
            }
        }
    }
}

void
d_adminSetConflictResolver(
    d_admin admin,
    d_conflictResolver conflictResolver)
{
    assert(admin);
    assert(d_adminIsValid(admin));

    d_lockLock(d_lock(admin));
    assert(admin->conflictResolver == NULL);
    admin->conflictResolver = conflictResolver;
    d_lockUnlock(d_lock(admin));
}

void
d_adminInitialFellowsCreate(
    d_admin admin)
{
    d_tableIter tableIter;
    d_fellow fellow;

    assert(admin);
    assert(d_adminIsValid(admin));

    d_lockLock(d_lock(admin));
    if (admin->initial_fellows != NULL) {
        /* Initial fellow list already created */
        d_tableFree(admin->initial_fellows);
        admin->initial_fellows = NULL;
    }
    if (admin->fellows != NULL) {
        admin->initial_fellows = d_tableNew(d_fellowCompare, d_fellowFree);

        for (fellow = d_fellow(d_tableIterFirst(admin->fellows, &tableIter));
             fellow != NULL;
             fellow = d_fellow(d_tableIterNext(&tableIter))) {
            d_tableInsert(admin->initial_fellows, d_objectKeep(d_object(fellow)));
        }
    }
    d_lockUnlock(d_lock(admin));
}

void
d_adminInitialFellowsDestroy(
    d_admin admin)
{
    assert(d_adminIsValid(admin));

    d_lockLock(d_lock(admin));
    if (admin->initial_fellows != NULL) {
        d_tableFree(admin->initial_fellows);
        admin->initial_fellows = NULL;
    }
    d_lockUnlock(d_lock(admin));
}

char *
d_adminGetInitialFellowsString(
    d_admin admin)
{
    char *str = NULL;
    c_ulong size;
    d_tableIter tableIter;
    d_fellow fellow;
    size_t pos = 0;

    assert(d_adminIsValid(admin));

    d_lockLock(d_lock(admin));
    /* size = (number of initial fellow * (max character in decimal systemId + comma)) + terminator */
    size = (d_tableSize(admin->initial_fellows) * 11) + 1;
    str = os_malloc(size);
    str[0] = '\0';
    for (fellow = d_fellow(d_tableIterFirst(admin->initial_fellows, &tableIter));
         fellow != NULL;
         fellow = d_fellow(d_tableIterNext(&tableIter))) {
        int n = snprintf(str + pos, size - pos, "%s%u", str[0] == '\0' ? "" : ",", fellow->address->systemId);
        if (n > 0) {
            pos += (size_t)n;
        }
    }
    d_lockUnlock(d_lock(admin));

    return str;
}
