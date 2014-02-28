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
#include "d_nameSpace.h"
#include "d__mergeState.h"
#include "d_nameSpaces.h"
#include "d_groupsRequest.h"
#include "d_nameSpacesRequest.h"
#include "d_nameSpacesRequestListener.h"
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
#include "v_builtin.h"
#include "u_entity.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_defs.h"
#include "os_report.h"
#include "c_base.h"

/**
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

        admin->nameSpaces = c_iterNew(NULL);

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
		assert(admin->nameSpaces);

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

        } else {
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
    d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN,
                        "Initializing protocol subscriber...\n");
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
            d_adminNotifyListeners(admin, D_GROUP_LOCAL_NEW, NULL, NULL, group, NULL);
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
    d_nameSpace nameSpace;

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

        if (admin->nameSpaces)
        {
            nameSpace = d_nameSpace(c_iterTakeFirst(admin->nameSpaces));

            while(nameSpace){
                d_nameSpaceFree(nameSpace);
                nameSpace = d_nameSpace(c_iterTakeFirst(admin->nameSpaces));
            }
            c_iterFree (admin->nameSpaces);
            admin->nameSpaces = NULL;
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
            d_adminNotifyListeners(admin, D_FELLOW_NEW, fellow, NULL, NULL, NULL);
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

struct fellowsExistForRoleHelper
{
    d_name role;
    c_bool found;
};

struct checkAlignerForNameSpaceHelper {
    d_fellow fellow;         /* the fellow to check*/
    d_nameSpace nameSpace;   /* the namespace for find a master for */
    c_char *role;            /* the role of the namespace to find a master for */
};

/* Check if the fellow is an aligner for the namespace and role
 * that are specified in the userData.
 */
c_bool
checkAlignerForNameSpace (d_fellow fellow, struct checkAlignerForNameSpaceHelper *userData) {

    c_char * role;
    c_bool result;

    assert(userData->fellow);
    assert(userData->nameSpace);

    role = userData->role;
    if (role) {
        if  (d_fellowIsAlignerForNameSpace(fellow, userData->nameSpace) &&
             (strcmp(d_fellowGetRole(fellow), role) == 0)) {
            result = FALSE;
        } else {
            result = TRUE;
        }
    } else {
        result = TRUE;
    }
    return result;
}


struct checkAlignerForRoleHelper {
    d_admin admin;     /* the admininistration */
    d_fellow fellow;   /* the fellow */
    c_char *role;      /* the fellow role */
};


/* Check if there exists an alternative aligner for the
 * given namespace. If no alternative aligner exists the
 * merge state for the namespace and role is cleared,
 * indicating that no master can be found.
 * If an alternative aligner is found nothing happens
 * because the new master already has an up-to-date state.
 *
 * return values:
 *   FALSE if no alternative aligner has been found for
 *       my namespace and the merge state of my namespace
 *       is cleared successfully
 *   TRUE otherwise
 */
c_bool
checkAlignerForRole (d_nameSpace nameSpace, void *userData) {

    struct checkAlignerForNameSpaceHelper nameSpaceHelper;
    struct checkAlignerForRoleHelper *roleHelper;
    c_bool noAlignerFound, result;
    d_nameSpace myNameSpace;

    roleHelper = (struct checkAlignerForRoleHelper *)userData;

    assert(roleHelper->admin);
    assert(roleHelper->fellow);

    nameSpaceHelper.fellow = roleHelper->fellow;
    nameSpaceHelper.nameSpace = nameSpace;
    nameSpaceHelper.role = roleHelper->role;

    result = TRUE;
    /* Retrieve the local namespace corresponding to the fellow's namespace 
     * from my own administration. */
    myNameSpace = d_adminGetNameSpaceNoLock(roleHelper->admin, d_nameSpaceGetName(nameSpace));
    if (myNameSpace != NULL) {
        /* Check if there is an alternative aligner */
        if (!d_nameSpaceIsAligner(myNameSpace)) {
            noAlignerFound = d_tableWalk(roleHelper->admin->fellows, checkAlignerForNameSpace, &nameSpaceHelper);
            if (noAlignerFound) {
                /* No alternative aligner has been found.
                 * Clear the current state of the namespace for this role */
                d_nameSpaceClearMergeState (myNameSpace, roleHelper->role);
                result = FALSE;
                /* Log the clearing of the state because no aligner is available for the namespace. */
                d_printTimedEvent(roleHelper->admin->durability, D_LEVEL_FINER, D_THREAD_UNSPECIFIED, 
                                  "State of namespace '%s' for role '%s' cleared\n",
                                  d_nameSpaceGetName(nameSpace),
                                  roleHelper->role);
            }
        }
    }
    return result;
}


/* Remove a fellow.
 * If the fellow was a master for a namespace and a role
 * then check if an alternative master can be found.
 * If no alternative aligner can be found then the state
 * is cleared.
 */
d_fellow
d_adminRemoveFellow(
    d_admin admin,
    d_fellow fellow)
{
    d_fellow result;
    d_networkAddress fellowAddr;
    d_adminStatisticsInfo info;
    struct checkAlignerForRoleHelper helper;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    result = NULL;

    if(admin && fellow){

        d_lockLock(d_lock(admin));

        /* first remove the fellow from the admin */
        result = d_tableRemove(admin->fellows, fellow);

        /* Walk over all namespaces of the remaining fellows
         * and check if an alternative aligner can be found.
         * If not found the merge state for this namespace
         * and role is cleared.
         */
        helper.fellow = fellow;
        helper.admin = admin;
        helper.role = d_fellowGetRole(fellow);
        d_fellowNameSpaceWalk(fellow, checkAlignerForRole, &helper);

        if(result) {
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
        d_adminNotifyListeners(admin, D_FELLOW_REMOVED, fellow, NULL, NULL, NULL);
    }
    return result;
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

/* Dynamic namespace functionality */

c_bool
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
                /* Keep ns */
                d_objectKeep (d_object(nameSpace));

                d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN,
                        "Add namespace '%s' to administration with policy {aligner=%d, alignee=%s, durability=%s}\n",
                        d_nameSpaceGetName(nameSpace),
                        d_nameSpaceIsAligner(nameSpace),
                        akindStr[d_nameSpaceGetAlignmentKind(nameSpace)],
                        dkindStr[d_nameSpaceGetDurabilityKind(nameSpace)]);

                /* Add namespace to admin */
                c_iterAppend (admin->nameSpaces, nameSpace);

                d_printTimedEvent(durability, D_LEVEL_FINER, D_THREAD_MAIN, "Namespace '%s' added to administration, notifying listeners...\n", d_nameSpaceGetName(nameSpace));

                /* New namespace event */
                d_adminNotifyListeners(admin, D_NAMESPACE_NEW, NULL, nameSpace, NULL, NULL);
            }
        }

        d_lockUnlock(d_lock(admin));
    }

    return TRUE;
}

d_nameSpace
d_adminGetNameSpaceForGroup(
        d_admin admin,
        d_partition partition,
        d_topic topic)
{
    d_nameSpace nameSpace;
    c_long i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    nameSpace = NULL;

    d_lockLock (d_lock(admin));

    for(i=0; (i<c_iterLength(admin->nameSpaces)) && (nameSpace == NULL); i++){
        nameSpace = d_nameSpace(c_iterObject(admin->nameSpaces, i));

        if(d_nameSpaceIsIn(nameSpace, partition, topic) == TRUE){
         /* do nothing */
        } else {
            nameSpace = NULL;
        }
    }

    d_lockUnlock (d_lock(admin));

    return nameSpace;
}

static c_bool
isBuiltinGroup(
    d_partition partition,
    d_topic topic)
{
    c_bool result = FALSE;
    assert(partition);
    assert(topic);

    if(strcmp(partition, V_BUILTIN_PARTITION) == 0){
        if( (strcmp(topic, V_PARTICIPANTINFO_NAME) == 0) ||
            (strcmp(topic, V_TOPICINFO_NAME) == 0) ||
            (strcmp(topic, V_PUBLICATIONINFO_NAME) == 0) ||
            (strcmp(topic, V_SUBSCRIPTIONINFO_NAME) == 0))
        {
            result = TRUE;
        }
    }
    return result;
}

c_bool
d_adminInNameSpace(
        d_nameSpace ns,
        d_partition partition,
        d_topic topic,
        c_bool aligner)
{
    c_bool result;

    result = FALSE;

    if(d_nameSpaceIsIn(ns, partition, topic) == TRUE){
        if(aligner == TRUE) {
           if(d_nameSpaceIsAligner(ns) == TRUE){
                result = TRUE;
           }
        } else {
            result = TRUE;
        }
    }

    if(result == FALSE){
        result = isBuiltinGroup(partition, topic);
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

    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }

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

        if(inNameSpace){
            if(d_nameSpaceGetAlignmentKind(ns) == D_ALIGNEE_ON_REQUEST){
                inNameSpace = FALSE;
            }
        }
    }
    d_lockUnlock (d_lock(admin));

    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
    return inNameSpace;
}

c_bool
d_adminGroupInAlignerNS(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);

    inNameSpace = FALSE;

    d_lockLock(d_lock(admin));

    count       = c_iterLength(admin->nameSpaces);
    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(admin->nameSpaces, i));
        inNameSpace = d_adminInNameSpace(ns, partition, topic, TRUE);
    }

    d_lockUnlock(d_lock(admin));

    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
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

        if(inNameSpace == TRUE){
            switch(d_nameSpaceGetAlignmentKind(ns)){
                case D_ALIGNEE_INITIAL:
                    break;
                default:
                    inNameSpace = FALSE;
                    break;
            }

        }
    }

    d_lockUnlock(d_lock(admin));

    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
    return inNameSpace;
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

    if((c != D_GROUP_COMPLETE) && (c != D_GROUP_UNKNOWN)){
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
    os_int32 i;

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
        c_iterInsert (nameSpaces, ns);
    }
}

static void
deleteNsWalk(
   void* o, void* userData)
{
    OS_UNUSED_ARG(userData);
    d_objectFree(d_object(o), D_NAMESPACE);
}

c_iter d_adminNameSpaceCollect(
    d_admin admin)
{
    c_iter result;
    result = c_iterNew(NULL);
    d_adminNameSpaceWalk(admin, collectNsWalk, result);
    return result;
}

void d_adminNameSpaceCollectFree(
    d_admin admin,
    c_iter nameSpaces)
{
    OS_UNUSED_ARG(admin);
    c_iterWalk(nameSpaces, deleteNsWalk, NULL);
    c_iterFree(nameSpaces);
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
        d_adminNotifyListeners(admin, D_FELLOW_LOST, fellow, NULL, NULL, NULL);
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

    evt = d_adminEvent(os_malloc(C_SIZEOF(d_adminEvent)));
    d_objectInit(d_object(evt), D_ADMIN_EVENT, d_adminEventDeinit);
    evt->event = event;
    evt->userData = userData;

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
    if (nameSpace) {
        assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE) == TRUE);
        evt->nameSpace = nameSpace;
    }else {
        evt->nameSpace = NULL;
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
    os_result waitResult;

    admin = d_admin(arg);

    while(admin->eventThreadTerminate == FALSE){
        os_mutexLock(&admin->eventMutex);
        event = c_iterTakeFirst(admin->eventQueue);
        os_mutexUnlock(&admin->eventMutex);

        while(event){
            for(i=0; i<c_iterLength(admin->eventListeners); i++){
                listener = d_eventListener(c_iterObject(admin->eventListeners, i));

                if((listener->interest & event->event) == event->event){
                    listener->func(event->event, event->fellow, event->nameSpace,
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
            waitResult = os_condWait(&admin->eventCondition, &admin->eventMutex);
            if (waitResult == os_resultFail)
            {
                OS_REPORT(OS_CRITICAL, "d_adminEventThreadStart", 0,
                          "os_condWait failed - terminating thread");
                admin->eventThreadTerminate = TRUE;
            }
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
    v_reader reader, *readerPtr;
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
                    readerPtr = &reader;
                    handleResult = v_handleClaim(handle, (v_object*)readerPtr);

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

struct masterCountForRoleHelper
{
    d_name role;
    d_nameSpace nameSpace;
    c_ulong masterCount;
};

static c_bool
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

struct nameSpaceConflictHelper
{
    d_durability durability;
    const char* name;
    d_nameSpace fellowNameSpace;
    d_nameSpace oldFellowNameSpace;
    c_iter stateConflicts;
    c_ulong conflictEvent;
};

static void
d_adminNameSpaceCheckConflicts(
    d_admin admin,
    d_nameSpace nameSpace,
    struct nameSpaceConflictHelper* userData)
{
    struct nameSpaceConflictHelper* helper;
    struct masterCountForRoleHelper walkData;
    d_networkAddress fellowMaster, ownMaster;
    d_mergeState fellowState, oldFellowState, ownState;
    char *role, *fellowRole;
    char *nameSpaceName;
    c_iter stateConflicts;
    c_bool fellowNativeStateChanged;
    c_bool fellowOtherStatesChanged;
    d_subscriber subscriber;
    d_nameSpacesRequestListener nsrListener;

    helper = (struct nameSpaceConflictHelper*)userData;
    role = d_nameSpaceGetRole (nameSpace);
    fellowRole = d_nameSpaceGetRole(helper->fellowNameSpace);
    subscriber = d_adminGetSubscriber(admin);
    nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);

    /* Check if there is a master conflict on role, in which case no action should be taken */
    walkData.masterCount = 0;
    walkData.nameSpace = nameSpace;
    walkData.role = fellowRole;

    /* Role for fellow should always be present at this point */
    assert (walkData.role);

    d_adminFellowWalk (admin, d_nameSpaceCountMastersForRoleWalk, &walkData);

    /* Only take action when fellow mastercount for role is exactly one */
    if (walkData.masterCount == 1) {
        nameSpaceName = d_nameSpaceGetName (nameSpace);
        fellowMaster = d_nameSpaceGetMaster (helper->fellowNameSpace);
        fellowState = d_nameSpaceGetMergeState (helper->fellowNameSpace, fellowRole);
        ownState = d_nameSpaceGetMergeState (nameSpace, fellowRole);
        fellowNativeStateChanged = FALSE;
        fellowOtherStatesChanged = FALSE;
        oldFellowState = NULL;
        stateConflicts = NULL;

        /* If old fellow namespace did not exist, this can mean a
         * reconnection (fellow is new) or fellow was not yet aware
         * of the namespace
         */
        if (!helper->oldFellowNameSpace) {
            fellowNativeStateChanged = TRUE;
            fellowOtherStatesChanged = TRUE;
        } else {
            oldFellowState = d_nameSpaceGetMergeState(helper->oldFellowNameSpace, fellowRole);

            if (fellowState->value != oldFellowState->value) {
                fellowNativeStateChanged = TRUE;
            }

            stateConflicts = d_nameSpaceGetMergedStatesDiff (helper->fellowNameSpace, helper->oldFellowNameSpace);
            if (stateConflicts) {
                fellowOtherStatesChanged = TRUE;
            }
        }

        d_printTimedEvent(
                helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                "Check for conflicts in namespace %s (nativeStateChanged=%d, otherStatesChanged=%d)\n",
                nameSpaceName,
                fellowNativeStateChanged,
                fellowOtherStatesChanged);

        /* Fellow is in own role */
        if (strcmp (role, fellowRole) == 0) {
            /* Check if masters are different for namespace in admin and from fellow */
            ownMaster = d_nameSpaceGetMaster(nameSpace);
            if (d_networkAddressCompare (ownMaster, fellowMaster) && d_nameSpaceIsMasterConfirmed(nameSpace)){
                d_printTimedEvent(
                        helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "Conflicting master found for namespace %s\n",
                        nameSpaceName);

                helper->conflictEvent = D_NAMESPACE_MASTER_CONFLICT;

                /* Report namespaces to let other fellows know that there was a conflicting master */
                d_nameSpacesRequestListenerReportNameSpaces(nsrListener);
                /* Namespace master is pending */
                d_nameSpaceMasterPending(nameSpace);

            /* Conflict in own state */
            } else if (fellowNativeStateChanged || (!ownState || (fellowState->value != ownState->value))) {
                d_printTimedEvent(
                        helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "Conflicting (or new) state %d found for namespace %s from own role '%s'\n",
                        fellowState->value,
                        nameSpaceName,
                        fellowRole);

                helper->conflictEvent = D_NAMESPACE_STATE_CONFLICT;

            /* Conflict in other state? (note that fellowOtherStatesChanged without explicit stateConflicts is already covered by the above statement) */
            } else if (fellowOtherStatesChanged && stateConflicts) {
                d_printTimedEvent(
                        helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "Conflicting state %d found for namespace %s in one or more mergedStates\n",
                        fellowState->value,
                        nameSpaceName);

                helper->stateConflicts = stateConflicts;
                helper->conflictEvent = D_NAMESPACE_STATE_CONFLICT;
            } else {
                d_printTimedEvent(
                        helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "No conflicts found in own or other states for namespace %s\n",
                        nameSpaceName);
            }
            d_free (ownMaster);

        /* In other role */
        } else {
            if (fellowNativeStateChanged && (!ownState || (fellowState->value != ownState->value))) {
                d_printTimedEvent(
                        helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "Conflicting (or new) state %d found for namespace %s from role %s\n",
                        fellowState->value,
                        nameSpaceName,
                        fellowRole);

                helper->conflictEvent = D_NAMESPACE_STATE_CONFLICT;
            } else {
                d_printTimedEvent(
                        helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "No conflicts found in other role for namespace %s\n",
                        nameSpaceName);
            }
        }

        d_free (fellowMaster);
        d_mergeStateFree (fellowState);

        if (oldFellowState) {
            d_mergeStateFree (oldFellowState);
        }
        if (ownState) {
            d_mergeStateFree (ownState);
        }
    } else {
        d_printTimedEvent(
                helper->durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                "Inconsistent number of masters (%d) found in role %s, no action required for now.\n",
                walkData.masterCount,
                fellowRole);
    }

    os_free (role);
    os_free (fellowRole);

}

void
d_adminReportMaster(
    d_admin admin,
    d_fellow fellow,
    d_nameSpace fellowNameSpace,
    d_nameSpace oldFellowNameSpace)
{
    char* nameSpaceName;
    struct nameSpaceConflictHelper helper;
    d_nameSpace nameSpace, nameSpaceCopy;
    d_durability durability;
    d_serviceState durabilityState;
    d_networkAddress master;

    durability = d_adminGetDurability(admin);
    durabilityState = d_durabilityGetState (durability);

    nameSpaceName = d_nameSpaceGetName (fellowNameSpace);
    nameSpace = d_adminGetNameSpace(admin, nameSpaceName);
    master = d_nameSpaceGetMaster(nameSpace);

    if (nameSpace) {
        helper.durability = d_adminGetDurability(admin);
        helper.name = nameSpaceName;
        helper.fellowNameSpace = fellowNameSpace;
        helper.oldFellowNameSpace = oldFellowNameSpace;
        helper.conflictEvent = D_NONE;
        helper.stateConflicts = 0;

        /* Only check when I'm complete and fellow is past injecting persistent data, or
         * when I have a confirmed but non-existent (0) master.
         */
        if ( ((durabilityState >= D_STATE_DISCOVER_PERSISTENT_SOURCE) && (d_fellowGetState(fellow) >= D_STATE_INJECT_PERSISTENT)) ||
             ((d_nameSpaceIsMasterConfirmed(nameSpace) && (master->systemId == 0)))) {
            d_adminNameSpaceCheckConflicts (admin, nameSpace, &helper);

            /* If a conflict occured, create D_NAMESPACE_MASTER_CONFLICT or D_NAMESPACE_STATE_CONFLICT event */
            if (helper.conflictEvent) {
                /* Create copy from namespace (fellow namespace is likely to change) */
                nameSpaceCopy = d_nameSpaceCopy (fellowNameSpace);

                /* New conflict event */
                d_adminNotifyListeners(admin, helper.conflictEvent, NULL, nameSpaceCopy, NULL, helper.stateConflicts);
            }
        }

        d_nameSpaceFree (nameSpace);
    }
    if(master){
        d_networkAddressFree(master);
    }
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
        if(!(q.seconds || q.nanoseconds)) {

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

                d_printTimedEvent(
                        durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                        "Delayed initial set discovered for namespace '%s'.\n",
                        d_nameSpaceGetName(nameSpace));

                /* Notify others that a delayed initial set is available */
                d_adminNotifyListeners(admin, D_NAMESPACE_DELAYED_INITIAL, fellow, localNameSpace, NULL, NULL);
            }
        }else {
            d_printTimedEvent(
                    durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                    "No delayed alignment for local namespace '%s', local quality is non-zero.\n",
                    d_nameSpaceGetName(nameSpace));
        }
    }else {
        d_printTimedEvent(
                durability, D_LEVEL_INFO, D_THREAD_NAMESPACES_LISTENER,
                "No delayed alignment for local namespace '%s', namespace does not exist locally, or delayed alignment is not enabled.\n",
                d_nameSpaceGetName(nameSpace));
    }
}
