#include "os.h"
#include "d_admin.h"
#include "d__admin.h"
#include "d_actionQueue.h"
#include "d_fellow.h"
#include "d_group.h"
#include "d_table.h"
#include "d_listener.h"
#include "d_durability.h"
#include "d_publisher.h"
#include "d_subscriber.h"
#include "d_group.h"
#include "d_newGroup.h"
#include "d_nameSpaces.h"
#include "d_groupsRequest.h"
#include "d_nameSpacesRequest.h"
#include "d_message.h"
#include "d_configuration.h"
#include "d_qos.h"
#include "d_misc.h"
#include "d_lock.h"
#include "d_store.h"
#include "d_networkAddress.h"
#include "d_eventListener.h"
#include "d_readerRequest.h"
#include "d__eventListener.h"
#include "d__statistics.h"
#include "v_kernel.h"
#include "v_event.h"
#include "v_reader.h"
#include "v_service.h"
#include "v_group.h"
#include "v_public.h"
#include "u_entity.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_defs.h"
#include "c_base.h"

/**
 * TODO: Determine the compatibility of the namespaces of two fellows before
 * allowing communication between them.
 */


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
    os_mutexAttr attr;
    os_condAttr ca;
    os_threadAttr ta;
    os_result osr;
    os_time sleepTime;
    d_configuration config;

    admin = NULL;

    if(durability){
        admin = d_admin(os_malloc(C_SIZEOF(d_admin)));
        d_lockInit(d_lock(admin), D_ADMIN, d_adminDeinit);

        u_entityAction(u_entity(d_durabilityGetService(durability)),
                                d_adminInitAddress, admin);
        assert(admin->myAddress);

        admin->durability = durability;
        assert(admin->durability);

        config = d_durabilityGetConfiguration(durability);

        d_printTimedEvent(durability, D_LEVEL_FINER,
                          D_THREAD_MAIN, "Initializing administration...\n");

        admin->cachedFellow = d_fellowNew(admin->myAddress, D_STATE_INIT);
        assert(admin->cachedFellow);

        admin->fellows = d_tableNew(d_fellowCompare, d_fellowFree);
        assert(admin->fellows);

        admin->groups = d_tableNew(d_groupCompare, d_groupFree);
        assert(admin->groups);

        admin->readerRequests = d_tableNew(d_readerRequestCompare, d_readerRequestFree);
        assert(admin->readerRequests);

        admin->alignerGroupCount = 0;

        d_printTimedEvent(durability, D_LEVEL_FINER,
                            D_THREAD_MAIN, "Initializing protocol topics...\n");

        admin->statusRequestTopic       = d_adminInitTopic(
                                            admin, D_STATUS_REQ_TOPIC_NAME,
                                            D_STATUS_REQ_TYPE_NAME,
                                            D_STATUS_REQ_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->groupsRequestTopic       = d_adminInitTopic(
                                            admin, D_GROUPS_REQ_TOPIC_NAME,
                                            D_GROUPS_REQ_TYPE_NAME,
                                            D_GROUPS_REQ_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->sampleRequestTopic       = d_adminInitTopic(
                                            admin, D_SAMPLE_REQ_TOPIC_NAME,
                                            D_SAMPLE_REQ_TYPE_NAME,
                                            D_SAMPLE_REQ_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->newGroupTopic            = d_adminInitTopic(
                                            admin, D_NEWGROUP_TOPIC_NAME,
                                            D_NEWGROUP_TYPE_NAME,
                                            D_NEWGROUP_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->statusTopic              = d_adminInitTopic(
                                            admin, D_STATUS_TOPIC_NAME,
                                            D_STATUS_TYPE_NAME,
                                            D_STATUS_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPLAST,
                                            1);

        admin->sampleChainTopic         = d_adminInitTopic(
                                            admin, D_SAMPLE_CHAIN_TOPIC_NAME,
                                            D_SAMPLE_CHAIN_TYPE_NAME,
                                            D_SAMPLE_CHAIN_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->nameSpacesTopic          = d_adminInitTopic(
                                            admin, D_NAMESPACES_TOPIC_NAME,
                                            D_NAMESPACES_TYPE_NAME,
                                            D_NAMESPACES_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->nameSpacesRequestTopic   = d_adminInitTopic(
                                            admin, D_NAMESPACES_REQ_TOPIC_NAME,
                                            D_NAMESPACES_REQ_TYPE_NAME,
                                            D_NAMESPACES_REQ_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);

        admin->deleteDataTopic          = d_adminInitTopic(
                                            admin, D_DELETE_DATA_TOPIC_NAME,
                                            D_DELETE_DATA_TYPE_NAME,
                                            D_DELETE_DATA_KEY_LIST,
                                            V_RELIABILITY_RELIABLE,
                                            V_HISTORY_KEEPALL,
                                            V_LENGTH_UNLIMITED);
        assert(admin->groupsRequestTopic);
        assert(admin->sampleRequestTopic);
        assert(admin->statusTopic);
        assert(admin->statusRequestTopic);
        assert(admin->newGroupTopic);
        assert(admin->sampleChainTopic);
        assert(admin->nameSpacesTopic);
        assert(admin->nameSpacesRequestTopic);
        assert(admin->deleteDataTopic);

        /* 100 ms */
        sleepTime.tv_sec  = 0;
        sleepTime.tv_nsec = 100000000;

        admin->actionQueue = d_actionQueueNew("d_adminActionQueue", sleepTime,
                config->heartbeatScheduling);
        assert(admin->actionQueue);

        osr = os_mutexAttrInit(&attr);

        if(osr == os_resultSuccess){
            attr.scopeAttr = OS_SCOPE_PRIVATE;

            osr = os_mutexInit(&admin->eventMutex, &attr);

            if(osr == os_resultSuccess){
                osr = os_condAttrInit(&ca);

                if(osr == os_resultSuccess){
                    ca.scopeAttr = OS_SCOPE_PRIVATE;
                    osr = os_condInit(&admin->eventCondition, &admin->eventMutex, &ca);
                    if(osr == os_resultSuccess){
                        admin->eventListeners = c_iterNew(NULL);
                        admin->eventQueue = c_iterNew(NULL);
                        admin->eventThreadTerminate = FALSE;
                        osr = os_threadAttrInit(&ta);

                        if(osr == os_resultSuccess){
                            osr = os_threadCreate(&(admin->eventThread),
                                              "AdminEventDispatcher",
                                              &ta,
                                              (void*(*)(void*))d_adminEventThreadStart,
                                              (void*)admin);
                        }
                    }
                }
            }
        }

        if(osr == os_resultSuccess){
            d_printTimedEvent(durability, D_LEVEL_FINER,
                        D_THREAD_MAIN,
                        "Initializing protocol publisher and writers...\n");
            admin->publisher  = d_publisherNew(admin);
            assert(admin->publisher);

            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                "Initializing protocol subscriber...\n");
            admin->subscriber = d_subscriberNew(admin);
            assert(admin->subscriber);
        } else {
            d_adminFree(admin);
            admin = NULL;
        }
    }
    return admin;
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
        d_lockUnlock(d_lock(admin));
        /**
         * In some cases a group might be registered multiple times. Of
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
            d_adminNotifyListeners(admin, D_GROUP_LOCAL_NEW, NULL, group);
            result = TRUE;
        }
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
d_adminGetLocalGroup(
    d_admin admin,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    d_group dummy;
    d_group found;
    d_quality quality;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    found = NULL;

    if(admin){
        quality.seconds = 0;
        quality.nanoseconds = 0;
        dummy = d_groupNew(partition, topic, kind, D_GROUP_KNOWLEDGE_UNDEFINED, quality);
        d_lockLock(d_lock(admin));
        found = d_tableFind(admin->groups, dummy);
        d_lockUnlock(d_lock(admin));
        d_groupFree(dummy);
    }
    return found;
}

void
d_adminDeinit(
    d_object object)
{
    d_admin admin;
    d_durability durability;
    d_adminEvent event;

    assert(d_objectIsValid(object, D_ADMIN) == TRUE);

    if(object){
        admin = d_admin(object);
        durability = admin->durability;

        if(admin->subscriber){
            d_printTimedEvent(durability, D_LEVEL_FINER,
                                    D_THREAD_MAIN, "Destroying subscriber...\n");
            d_subscriberFree(admin->subscriber);
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                "Subscriber destroyed\n");
            admin->subscriber = NULL;
        }
        if(admin->publisher){
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                    "Destroying publisher...\n");
            d_publisherFree(admin->publisher);
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                    "Publisher destroyed\n");
            admin->publisher = NULL;
        }

        os_mutexLock(&admin->eventMutex);
        admin->eventThreadTerminate = TRUE;
        os_condSignal(&admin->eventCondition);
        os_mutexUnlock(&admin->eventMutex);
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                "Waiting for admin event dispatcher thread to terminate...\n");
        os_threadWaitExit(admin->eventThread, NULL);
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                "Thread destroyed.\n");
        os_condDestroy(&admin->eventCondition);

        os_mutexLock(&admin->eventMutex);

        if(admin->eventListeners){
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                            "Removing event listeners.\n");
            c_iterFree(admin->eventListeners);
            admin->eventListeners = NULL;
        }
        if(admin->eventQueue){
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                                "Clearing event queue...\n");
            event = d_adminEvent(c_iterTakeFirst(admin->eventQueue));

            while(event){
                d_adminEventFree(event);
                event = d_adminEvent(c_iterTakeFirst(admin->eventQueue));
            }
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                                "Destroying event queue...\n");
            c_iterFree(admin->eventQueue);
        }
        os_mutexUnlock(&admin->eventMutex);
        os_mutexDestroy(&admin->eventMutex);

        if(admin->readerRequests){
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                            "Destroying readerRequests...\n");
            d_tableFree(admin->readerRequests);
            admin->readerRequests = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                            "ReaderRequests destroyed\n");
        }

        if(admin->fellows){
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "Destroying fellow admin...\n");
            d_tableFree(admin->fellows);
            admin->fellows = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "Fellows destroyed\n");
        }
        if(admin->groups){
            d_tableFree(admin->groups);
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "My groups destroyed\n");
        }
        if(admin->cachedFellow){
            d_fellowFree(admin->cachedFellow);
            admin->cachedFellow = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                                "Cached fellow destroyed\n");
        }

        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "Destroying topics...\n");

        if(admin->statusTopic){
            u_topicFree(admin->statusTopic);
            admin->statusTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_status topic destroyed\n");
        }
        if(admin->newGroupTopic){
            u_topicFree(admin->newGroupTopic);
            admin->newGroupTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_newGroup topic destroyed\n");
        }
        if(admin->statusRequestTopic){
            u_topicFree(admin->statusRequestTopic);
            admin->statusRequestTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_statusRequest topic destroyed\n");
        }
        if(admin->groupsRequestTopic){
            u_topicFree(admin->groupsRequestTopic);
            admin->groupsRequestTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_groupsRequest topic destroyed\n");
        }
        if(admin->sampleRequestTopic){
            u_topicFree(admin->sampleRequestTopic);
            admin->sampleRequestTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_sampleRequest topic destroyed\n");
        }
        if(admin->sampleChainTopic){
            u_topicFree(admin->sampleChainTopic);
            admin->sampleChainTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_sampleChain topic destroyed\n");
        }
        if(admin->nameSpacesTopic){
            u_topicFree(admin->nameSpacesTopic);
            admin->nameSpacesTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                        "d_nameSpaces topic destroyed\n");
        }
        if(admin->nameSpacesRequestTopic){
            u_topicFree(admin->nameSpacesRequestTopic);
            admin->nameSpacesRequestTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                    "d_nameSpacesRequest topic destroyed\n");
        }
        if(admin->deleteDataTopic){
            u_topicFree(admin->deleteDataTopic);
            admin->deleteDataTopic = NULL;
            d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                    "d_deleteData topic destroyed\n");
        }
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                    "Topics destroyed\n");

        if(admin->actionQueue) {
            d_actionQueueFree(admin->actionQueue);
        }

        if(admin->myAddress){
            d_networkAddressFree(admin->myAddress);
            admin->myAddress = NULL;
        }
        d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                                                "Admin destroyed\n");
    }
}

void
d_adminFree(
    d_admin admin)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        d_lockFree(d_lock(admin), D_ADMIN);
    }
}

d_fellow
d_adminGetFellow(
    d_admin admin,
    d_networkAddress address)
{
    d_fellow found;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    found = NULL;

    d_lockLock(d_lock(admin));

    d_fellowSetAddress(admin->cachedFellow, address);
    found = d_tableFind(admin->fellows, admin->cachedFellow);

    if(found){
        found = d_fellow(d_objectKeep(d_object(found)));
    }
    d_lockUnlock(d_lock(admin));

    return found;
}

c_bool
d_adminAddFellow(
    d_admin admin,
    d_fellow fellow)
{
    d_networkAddress addr;
    c_bool added;
    d_fellow duplicate;
    d_adminStatisticsInfo info;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    added = FALSE;

    if(admin && fellow){
        d_lockLock(d_lock(admin));
        duplicate = d_tableInsert(admin->fellows, fellow);

        if(!duplicate){
            addr = d_fellowGetAddress(fellow);

            d_printTimedEvent(admin->durability, D_LEVEL_INFO,
                    D_THREAD_MAIN,
                    "New fellow '%u' added to admin.\n",
                    addr->systemId);
            d_adminNotifyListeners(admin, D_FELLOW_NEW, fellow, NULL);
            d_networkAddressFree(addr);
            added = TRUE;

            info = d_adminStatisticsInfoNew();
            info->fellowsKnownDif = 1;
            d_durabilityUpdateStatistics(admin->durability, d_statisticsUpdateAdmin, info);
            d_adminStatisticsInfoFree(info);
        }
        d_lockUnlock(d_lock(admin));
    }
    return added;
}

static c_bool
clearMaster(
    d_fellow fellow,
    c_voidp userData)
{
    d_fellowClearMaster(fellow, d_networkAddress(userData));

    return TRUE;
}
d_fellow
d_adminRemoveFellow(
    d_admin admin,
    d_fellow fellow)
{
    d_fellow result;
    d_networkAddress fellowAddr;
    d_adminStatisticsInfo info;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    result = NULL;

    if(admin && fellow){
        d_lockLock(d_lock(admin));
        result = d_tableRemove(admin->fellows, fellow);

        if(result){
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
            fellowAddr = d_fellowGetAddress(result);
            d_tableWalk(admin->fellows, clearMaster, fellowAddr);
            d_networkAddressFree(fellowAddr);

            d_durabilityUpdateStatistics(admin->durability, d_statisticsUpdateAdmin, info);
            d_adminStatisticsInfoFree(info);
        }
        d_lockUnlock(d_lock(admin));
        d_adminNotifyListeners(admin, D_FELLOW_REMOVED, fellow, NULL);
    }
    return result;
}

d_durability
d_adminGetDurability(
    d_admin admin)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

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
d_adminGetStatusRequestTopic(
    d_admin admin)
{
    u_topic topic = NULL;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        topic = admin->statusRequestTopic;
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

void
d_adminInitAddress(
    v_entity entity,
    c_voidp args)
{
    d_admin admin;
    v_gid gid;

    admin = d_admin(args);
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    gid = v_kernel(v_object(entity)->kernel)->GID;
    admin->myAddress = d_networkAddressNew(
        v_gidSystemId(gid), v_gidLocalId(gid), v_gidLifecycleId(gid));
}

u_topic
d_adminInitTopic(
    d_admin admin,
    const c_char* topicName,
    const c_char* typeName,
    const c_char* keyList,
    v_reliabilityKind reliability,
    v_historyQosKind historyKind,
    c_long historyDepth)
{
    v_topicQos topicQos;
    u_topic topic;
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    topic = NULL;

    topicQos = d_topicQosNew(V_DURABILITY_VOLATILE, reliability);

    if(topicQos){
        topicQos->history.kind = historyKind;
        topicQos->history.depth = historyDepth;

        topic = u_topicNew(
                    u_participant(d_durabilityGetService(
                                    d_adminGetDurability(admin))),
                    topicName, typeName, keyList, topicQos);
        d_topicQosFree(topicQos);
    }
    return topic;
}

c_bool
d_adminAreLocalGroupsComplete(
    d_admin admin)
{
    c_bool complete;


    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    d_lockLock(d_lock(admin));
    if(d_tableSize(admin->groups) >= 1){
        complete = TRUE;

        if(admin->subscriber) {
            complete = d_subscriberAreRemoteGroupsHandled(admin->subscriber);
        }

        if(complete == TRUE) {
            d_tableWalk(admin->groups, d_adminLocalGroupsCompleteAction, &complete);
        }
    } else {
        complete = FALSE;
    }
    d_lockUnlock(d_lock(admin));

    return complete;
}

c_bool
d_adminLocalGroupsCompleteAction(
    d_group group,
    c_voidp userData)
{
    c_bool result;
    c_bool *boolData;
    d_completeness c;

    c = d_groupGetCompleteness(group);
    boolData = (c_bool*)userData;

    if(c != D_GROUP_COMPLETE){
        result = FALSE;
        *boolData = FALSE;
    } else {
        *boolData = TRUE;
        result = TRUE;
    }
    return result;
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

void
d_adminCleanupFellows(
    d_admin admin,
    d_timestamp timestamp)
{
    struct cleanupData data;
    d_fellow fellow;
    d_networkAddress address;
    d_durability durability;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    durability      = admin->durability;
    data.fellows    = c_iterNew(NULL);
    data.stamp      = timestamp;

    d_lockLock(d_lock(admin));
    d_tableWalk(admin->fellows,d_adminCleanupFellowsAction , &data);
    d_lockUnlock(d_lock(admin));

    fellow = c_iterTakeFirst(data.fellows);

    while(fellow){
        address = d_fellowGetAddress(fellow);
        d_printTimedEvent(durability, D_LEVEL_FINE, D_THREAD_STATUS_LISTENER,
                                    "Removing fellow: %u\n", address->systemId);
        d_networkAddressFree(address);
        fellow = d_adminRemoveFellow(admin, fellow);
        d_adminNotifyListeners(admin, D_FELLOW_LOST, fellow, NULL);
        d_fellowFree(fellow);
        fellow = c_iterTakeFirst(data.fellows);
    }
    c_iterFree(data.fellows);
}

c_bool
d_adminCleanupFellowsAction(
    d_fellow fellow,
    c_voidp args)
{
    d_timestamp s;
    c_equality eq;
    struct cleanupData *data;

    data = (struct cleanupData*)(args);

    s = d_fellowGetLastStatusReport(fellow);
    eq = c_timeCompare(s, data->stamp);

    if(eq == C_LT){
        data->fellows = c_iterInsert(data->fellows, fellow);
    }
    return TRUE;
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

    if(admin && listener){
        os_mutexLock(&admin->eventMutex);
        admin->eventListeners = c_iterInsert(admin->eventListeners, listener);
        os_mutexUnlock(&admin->eventMutex);
    }
}

void
d_adminRemoveListener(
    d_admin admin,
    d_eventListener listener)
{
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(listener), D_EVENT_LISTENER) == TRUE);

    if(admin && listener){
        os_mutexLock(&admin->eventMutex);
        c_iterTake(admin->eventListeners, listener);
        os_mutexUnlock(&admin->eventMutex);
    }
}

void
d_adminNotifyListeners(
    d_admin admin,
    c_ulong mask,
    d_fellow fellow,
    d_group group)
{
    d_adminEvent event;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    if(admin){
        event = d_adminEventNew(mask, fellow, group);
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
    d_group group)
{
    d_adminEvent evt = NULL;
    d_networkAddress addr;
    c_char *topic, *partition;
    v_group vgroup;

    evt = d_adminEvent(os_malloc(C_SIZEOF(d_adminEvent)));
    d_objectInit(d_object(evt), D_ADMIN_EVENT, d_adminEventDeinit);
    evt->event = event;

    if(fellow){
        assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
        addr = d_fellowGetAddress(fellow);
        evt->fellow = d_fellowNew(addr, d_fellowGetState(fellow));
        d_networkAddressFree(addr);
    } else {
        evt->fellow = NULL;
    }
    if(group){
        assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
        partition = d_groupGetPartition(group);
        topic = d_groupGetTopic(group);
        evt->group = d_groupNew (partition, topic, d_groupGetKind(group),
                                    d_groupGetCompleteness(group),
                                    d_groupGetQuality(group));
        vgroup = d_groupGetKernelGroup(group);

        if(vgroup){
            d_groupSetKernelGroup(evt->group, vgroup);
            c_free(vgroup);
        }
        os_free(partition);
        os_free(topic);
    } else {
        evt->group = NULL;
    }
    return evt;
}

void
d_adminEventDeinit(
    d_object object)
{
    d_adminEvent event;

    assert(d_objectIsValid(object, D_ADMIN_EVENT) == TRUE);

    if(object){
        event = d_adminEvent(object);

        if(event->fellow){
            d_fellowFree(event->fellow);
        }
        if(event->group){
            d_groupFree(event->group);
        }
    }
}

void
d_adminEventFree(
    d_adminEvent event)
{
    assert(d_objectIsValid(d_object(event), D_ADMIN_EVENT) == TRUE);

    if(event){
        d_objectFree(d_object(event), D_ADMIN_EVENT);
    }
}

void*
d_adminEventThreadStart(
    void* arg)
{
    d_admin admin;
    d_adminEvent event;
    d_eventListener listener;
    int i;
    c_bool result;

    admin = d_admin(arg);

    while(admin->eventThreadTerminate == FALSE){
        os_mutexLock(&admin->eventMutex);
        event = c_iterTakeFirst(admin->eventQueue);

        while(event){
            for(i=0; i<c_iterLength(admin->eventListeners); i++){
                listener = d_eventListener(c_iterObject(admin->eventListeners, i));

                if((listener->interest & event->event) == event->event){
                    result = listener->func(event->event, event->fellow,
                                                event->group, listener->args);
                }
            }
            d_adminEventFree(event);
            event = c_iterTakeFirst(admin->eventQueue);
        }
        if(admin->eventThreadTerminate == FALSE){
            os_condWait(&admin->eventCondition, &admin->eventMutex);
        }
        os_mutexUnlock(&admin->eventMutex);
    }
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

    if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL){
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
    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

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
        handle.server = NULL;
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
        handle.server = NULL;
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
                    handle = d_readerRequestGetHandle(found);
                    handleResult = v_handleClaim(handle, (v_object*)&reader);

                    if(handleResult == V_HANDLE_OK){
                        v_readerNotifyHistoricalDataAvailable(reader);
                        v_handleRelease(handle);
                    }
                    d_readerRequestFree(found);
                    result = TRUE;
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
