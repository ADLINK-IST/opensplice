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
#include "vortex_os.h"
#include "d_nameSpacesRequest.h"
#include "d__configuration.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__groupLocalListener.h"
#include "d__sampleChainListener.h"
#include "d__nameSpacesRequestListener.h"
#include "d__group.h"
#include "d__thread.h"
#include "d__nameSpace.h"
#include "d__admin.h"
#include "d__fellow.h"
#include "d__misc.h"
#include "d__table.h"
#include "d__listener.h"
#include "d__nameSpace.h"
#include "d__actionQueue.h"
#include "d__publisher.h"
#include "d__subscriber.h"
#include "d__waitset.h"
#include "d__readerRequest.h"
#include "d__group.h"
#include "d__eventListener.h"
#include "d_newGroup.h"
#include "d_deleteData.h"
#include "d_message.h"
#include "d_sampleRequest.h"
#include "d_groupsRequest.h"
#include "d_deleteData.h"
#include "d_networkAddress.h"
#include "d_store.h"
#include "u_observable.h"
#include "u_group.h"
#include "v_event.h"
#include "v_observer.h"
#include "v_waitset.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_entity.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_group.h"
#include "v_time.h"
#include "c_time.h"
#include "c_iterator.h"
#include "os_heap.h"
#include "os_report.h"
#include "d_durability.h"
#include "d__mergeAction.h"
#include "d__mergeState.h"
#include "d__conflictResolver.h"

/**
 * Macro that checks the d_groupLocalListener validity.
 * Because d_groupLocalListener is a concrete class typechecking is required.
 */
#define d_groupLocalListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_GROUP_LOCAL_LISTENER)

/**
 * \brief The d_groupLocalListener cast macro.
 *
 * This macro casts an object to a d_groupLocalListener object.
 */
#define d_groupLocalListener(_this) ((d_groupLocalListener)(_this))

#define D_FLOOR_SEQUENCE_NUMBER  (-1)

C_CLASS(d_groupIncomplete);

C_STRUCT(d_groupIncomplete){
    d_group dgroup;
    v_group vgroup;
};

/**
 * \brief The d_groupIncomplete cast macro.
 *
 * This macro casts an object to a d_groupIncomplete object.
 */
#define d_groupIncomplete(g) ((d_groupIncomplete)(g))

C_STRUCT(d_groupLocalListener){
    C_EXTENDS(d_listener);
    c_bool initialGroupsAdministrated;
    c_long lastSequenceNumber;
    os_mutex masterLock;
    d_eventListener fellowListener;
    d_eventListener nameSpaceListener;
    d_sampleChainListener sampleChainListener;
    d_waitsetEntity waitsetData;
    d_actionQueue actionQueue;
    d_actionQueue masterMonitor;
    c_long snapshotRequestNumber;
};

struct findAligner{
    d_fellow fellow;
    d_group group;
};

struct createPersistentSnapshotHelper {
    c_char* partExpr;
    c_char* topicExpr;
    c_char* uri;
    d_listener listener;
};

struct fellowState {
    d_networkAddress address;
    d_serviceState state;
    d_table nameSpaces; /* Namespace states */
};

/* TODO: d_groupLocalListenerHandleAlignment: take appropriate action on
 * lazy alignment where injection of data fails.
 */


struct checkNameSpacesHelper {
    d_nameSpacesRequest request;
    d_publisher publisher;
    c_iter retryFellows;
    os_timeE retryTime;
    os_duration retryDelay;
};

static c_bool
checkNameSpaces(
    d_fellow fellow,
    c_voidp args)
{
    struct checkNameSpacesHelper *helper;
    d_communicationState state;
    d_networkAddress fellowAddress;

    helper = (struct checkNameSpacesHelper*)args;
    state = d_fellowGetCommunicationState (fellow);

    /* If an incomplete fellow is found, send a namespaces request to that fellow, and reset the retryTime.
     * The fellow is added to the retryFellows list and removed from the administration (by the caller function),
     * when retryTime eventually exceeds and the fellow is still in the list.
     */
    if (state == D_COMMUNICATION_STATE_APPROVED) {
        d_fellow retryFellow;
        retryFellow = c_iterTake(helper->retryFellows, fellow);
        if(retryFellow) {
            d_objectFree(d_object(retryFellow));
        }
    } else {
        if (!c_iterContains(helper->retryFellows, fellow)) {
            d_durability durability = d_threadsDurability();

            fellowAddress = d_fellowGetAddress(fellow);
            d_messageSetAddressee(d_message(helper->request), fellowAddress);
            d_publisherNameSpacesRequestWrite(helper->publisher, helper->request, fellowAddress, d_durabilityGetState(durability));
            /* Increase retrytime, in case this is a new fellow give it a fair chance to respond */
            helper->retryTime = os_timeEAdd(os_timeEGet(), helper->retryDelay);
            d_objectKeep((d_object)fellow);
            helper->retryFellows = c_iterAppend(helper->retryFellows, fellow);
            d_networkAddressFree(fellowAddress);
        }
    }

    return TRUE;
}

struct masterHelper {
    d_groupLocalListener listener;
    c_iter nameSpaces;
};

static c_bool
determineNewMastersAction(
    d_action action,
    c_bool terminate)
{
    struct masterHelper* helper;

    helper = (struct masterHelper*)(d_actionGetArgs(action));

    if(terminate == FALSE){
        d_groupLocalListenerDetermineNewMasters (helper->listener, helper->nameSpaces);
    }

    c_iterFree(helper->nameSpaces);
    os_free(helper);

    return FALSE;
}

static c_bool
notifyFellowEvent(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    d_groupLocalListener listener;
    d_admin admin;
    d_durability durability;
    c_ulong length, i;
    d_nameSpace nameSpace;
    d_action masterAction;
    os_duration sleepTime = OS_DURATION_INIT(0, 100000000);  /* 100 ms */
    d_networkAddress masterAddress, fellowAddress;
    d_serviceState fellowState;
    c_iter nameSpaces, nsCollect;
    struct masterHelper *helper;

    OS_UNUSED_ARG(ns);
    OS_UNUSED_ARG(group);
    OS_UNUSED_ARG(eventUserData);

    listener      = d_groupLocalListener(userData);
    admin         = d_listenerGetAdmin(d_listener(listener));
    durability    = d_adminGetDurability(admin);
    fellowAddress = d_fellowGetAddress(fellow);

    if (event == D_FELLOW_NEW) {
        fellowState   = d_fellowGetState(fellow);

        d_printTimedEvent(durability, D_LEVEL_INFO,
                    "New fellow '%u' with state %s\n",
                    fellowAddress->systemId,
                    d_fellowStateText(fellowState));

        d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Potentially I need merging with fellow '%u' with state %s\n",
                    fellowAddress->systemId,
                    d_fellowStateText(fellowState));

        /* The fellow might have published data that I missed.
         * To retrieve the data the I am going to schedule a conflict
         */
        d_conflictMonitorCheckFellowConnected(admin->conflictMonitor, fellow);

    } else if(event == D_FELLOW_REMOVED){
        nameSpaces    = c_iterNew(NULL);

        d_printTimedEvent(durability, D_LEVEL_INFO,
                    "Fellow '%d' removed, checking whether new master must be determined.\n",
                    fellowAddress->systemId);

        nsCollect = d_adminNameSpaceCollect(admin);
        length        = c_iterLength(nsCollect);

        for(i=0; (i<length) && (d_durabilityMustTerminate(durability) == FALSE); i++) {
            nameSpace = d_nameSpace(c_iterObject(nsCollect, i));
            masterAddress = d_nameSpaceGetMaster(nameSpace);

            if (d_networkAddressEquals(masterAddress, fellowAddress) && d_nameSpaceIsMasterConfirmed(nameSpace)) {
                /* The confirmed master of the namespace has been removed, need to find another master */
                d_printTimedEvent(durability, D_LEVEL_FINE,
                                "Need to find a new master for nameSpace '%s'.\n",
                                d_nameSpaceGetName(nameSpace));
                nameSpaces = c_iterAppend(nameSpaces, nameSpace);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                                "No need to find a new master for nameSpace '%s', current master is %u (confirmed=%d)\n",
                                    d_nameSpaceGetName(nameSpace), masterAddress->systemId, d_nameSpaceIsMasterConfirmed(nameSpace));
            }
            d_networkAddressFree(masterAddress);
        }
        length = c_iterLength(nameSpaces);

        /* TODO: Remove merge action from sampleChainListener if it exists */
        if(length > 0){
            helper = (struct masterHelper*)(os_malloc(sizeof(struct masterHelper)));
            helper->listener = listener;
            helper->nameSpaces = nameSpaces;
            masterAction = d_actionNew(os_timeMGet(), sleepTime, determineNewMastersAction, helper);
            d_actionQueueAdd(listener->masterMonitor, masterAction);
        } else {
            c_iterFree(nameSpaces);
        }

        /* Free namespace list */
        d_adminNameSpaceCollectFree(admin, nsCollect);
    }

    d_networkAddressFree(fellowAddress);

    return TRUE;

}

struct nsGroupAlignWalkData
{
    d_durability durability;
    d_nameSpace nameSpace;
    d_groupLocalListener listener;
    c_iter groups;
};

/* Start alignment for existing groups in specific namespace */
static c_bool
nsCollectGroupWalk(
    d_group group,
    c_voidp userData)
{
    c_bool inNameSpace;
    d_partition partition;
    d_topic topic;
    struct nsGroupAlignWalkData* walkData = (struct nsGroupAlignWalkData*)userData;

    assert(walkData);

    partition = d_groupGetPartition(group);
    topic = d_groupGetTopic(group);

    if(walkData->nameSpace){
        inNameSpace = d_nameSpaceIsIn(walkData->nameSpace, partition, topic);

        if(inNameSpace){
            d_printTimedEvent(walkData->durability, D_LEVEL_FINEST,
                "-Group %s.%s.\n", partition, topic);
            c_iterAppend (walkData->groups, group);
        }
    } else {
        d_printTimedEvent(walkData->durability, D_LEVEL_FINEST,
               "- Group %s.%s.\n", partition, topic);
        c_iterAppend (walkData->groups, group);
    }
    os_free(partition);
    os_free(topic);

    return TRUE;
}

static void
handleGroupAlignmentWalk (
    void* o,
    c_voidp userData)
{
    struct nsGroupAlignWalkData* walkData = (struct nsGroupAlignWalkData*)userData;

    d_group group = d_group(o);
    d_groupLocalListener listener;
    d_admin admin;
    d_nameSpace nameSpace;
    d_partition partition;
    d_topic topic;

    admin       = d_listenerGetAdmin(d_listener(walkData->listener));
    partition   = d_groupGetPartition(group);
    topic       = d_groupGetTopic(group);
    listener    = walkData->listener;

    /* Compare namespace in walkdata with namespace from group */
    nameSpace = d_adminGetNameSpaceForGroup(admin, partition, topic);
    if (!d_nameSpaceCompare (walkData->nameSpace, nameSpace))
    {
        /* Start alignment of group when namespaces are equal */
        d_groupLocalListenerHandleAlignment(
            listener,
            group,
            NULL);
    }

    os_free (partition);
    os_free (topic);
}


typedef struct applyDelayedAlignment_t {
    d_groupLocalListener listener;
    d_nameSpace nameSpace;
    d_fellow fellow;
}applyDelayedAlignment_t;

static void
setGroupIncomplete (
    void* o,
    c_voidp userData)
{
    d_group group = d_group(o);
    d_admin admin= d_admin(userData);
    d_groupSetIncomplete(group, admin);
}

static c_bool
applyDelayedAlignment (
    d_action action,
    c_bool terminate)
{
    c_iter nameSpaces;
    d_admin admin;
    d_durability durability;
    d_groupLocalListener listener;
    d_nameSpace ns;
    applyDelayedAlignment_t* actionData;
    d_networkAddress master;
    d_fellow masterFellow;
    d_quality q;
    c_bool callAgain = TRUE;
    struct nsGroupAlignWalkData walkData;

    actionData = (applyDelayedAlignment_t*)d_actionGetArgs(action);
    listener = actionData->listener;
    durability  = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));
    admin = d_listenerGetAdmin(d_listener(listener));
    ns = d_nameSpace(d_objectKeep(d_object(actionData->nameSpace)));

    if (terminate == FALSE) {
        /* Only do action when fellow has reached discover_persistent_source state, so master-selection is synced. */
        if(d_fellowGetState(actionData->fellow) >= D_STATE_DISCOVER_PERSISTENT_SOURCE) {

            /* Re-determine master for namespace */
            nameSpaces = c_iterNew(ns);
            d_groupLocalListenerDetermineNewMasters (listener, nameSpaces);
            c_iterFree(nameSpaces);

            /* If I am not the new master, and the namespace quality of the new master is not infinite, I will
             * request alignment for the namespace.
             */

            /* Get master */
            master = d_nameSpaceGetMaster(ns);

            /* Get fellow namespace */
            masterFellow = d_adminGetFellow(admin, master);

            if (masterFellow) {
                /* Re-align groups */
                if(d_durabilityMustTerminate(durability) == FALSE){
                    walkData.durability = durability;
                    walkData.listener = listener;
                    walkData.nameSpace = ns;
                    walkData.groups = c_iterNew(NULL);
                    admin = d_listenerGetAdmin(d_listener(listener));

                d_printTimedEvent(durability, D_LEVEL_FINER,
                   "Collecting groups for namespace %s to apply delayed alignment.\n",
                   d_nameSpaceGetName(ns));
                    /* Collect groups */
                    d_adminGroupWalk (admin, nsCollectGroupWalk, &walkData);

                    /* Set completeness of group back to incomplete again */
                    c_iterWalk(walkData.groups, setGroupIncomplete, admin);

                    /* Align groups */
                    c_iterWalk (walkData.groups, handleGroupAlignmentWalk, &walkData);

                    /* Namespace has no longer zero-quality, set it to infinite so it can't be written to anymore. */
                    q = D_QUALITY_INFINITE;
                    d_nameSpaceSetInitialQuality(ns, q);
                }
                d_fellowFree(masterFellow);
            } else {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                            "Fellow '%d' lost before starting delayed alignment, or I have become master.\n",
                            master->systemId);
            }
            d_networkAddressFree(master);
            d_fellowFree(actionData->fellow);
            os_free(actionData);
            callAgain = FALSE;
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                       "Redo applyDelayedAlignment (namespace %s) - fellow not yet in DISCOVER_PERSISTENT_SOURCE state.\n",
                       d_nameSpaceGetName(ns));
        }
    } else {
        d_fellowFree(actionData->fellow);
        os_free(actionData);
        callAgain = FALSE;
    }
    d_nameSpaceFree(ns);

    return callAgain;
}

/* Mark namespace with state of fellow */
static c_bool
notifyNameSpaceEvent(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    d_durability durability;
    d_admin admin;
    d_groupLocalListener listener;
    d_action action;

    OS_UNUSED_ARG(group);
    OS_UNUSED_ARG(eventUserData);

    listener = d_groupLocalListener(userData);
    assert(listener);
    admin = d_listenerGetAdmin(d_listener(listener));
    durability  = d_adminGetDurability(admin);
    /* New namespace detected */
    if (event == D_NAMESPACE_NEW) {
        /* A new namespace has been learned. To ensure that data is acquired for this namespace we
         * generate a master conflict. When this master conflict is resolved alignment will occur
         * automatically. */
        d_conflict conflict;
        d_nameSpace nameSpaceCopy;
        nameSpaceCopy = d_nameSpaceCopy(ns);

        conflict = d_conflictNew(D_CONFLICT_NAMESPACE_MASTER, NULL, nameSpaceCopy, NULL);
        if (!d_conflictResolverConflictExists(admin->conflictResolver, conflict)) {
            d_conflictSetId(conflict, durability);

            d_printTimedEvent(durability, D_LEVEL_FINE,
                            "New nameSpace '%s' detected, master conflict %d created\n",
                            ns->name, conflict->id);
            d_conflictResolverAddConflict(admin->conflictResolver, conflict);
        } else {
            d_conflictFree(conflict);
        }
        d_nameSpaceFree(nameSpaceCopy);

    /* Late-joining node with initial data joined */
    } else if (event & D_NAMESPACE_DELAYED_INITIAL) {
        applyDelayedAlignment_t* actionData;
        d_fellow adminFellow;
        d_networkAddress fellowAddr;
        os_duration sleepTime = OS_DURATION_INIT(1, 0);

        listener = d_groupLocalListener(userData);
        admin = d_listenerGetAdmin(d_listener(listener));

        fellowAddr = d_fellowGetAddress(fellow);
        adminFellow = d_adminGetFellow(admin, fellowAddr);
        d_networkAddressFree(fellowAddr);

        actionData = os_malloc(sizeof(applyDelayedAlignment_t));
        actionData->listener = listener;
        actionData->nameSpace = ns;
        actionData->fellow = adminFellow;

        /* Post delayed alignment action */
        action = d_actionNew(os_timeMGet(), sleepTime, applyDelayedAlignment, actionData);
        d_actionQueueAdd(listener->actionQueue, action);
    }
    return TRUE;
}

static void
d_groupLocalListenerDeinit(
    d_groupLocalListener listener)
{
    assert(d_groupLocalListenerIsValid(listener));

    /* Stop the groupLocalListener */
    d_groupLocalListenerStop(listener);
    /* Deallocate */
    if (listener->nameSpaceListener) {
        d_eventListenerFree(listener->nameSpaceListener);
    }
    if (listener->fellowListener) {
        d_eventListenerFree(listener->fellowListener);
    }
    if (listener->actionQueue) {
        d_actionQueueFree(listener->actionQueue);
    }
    if (listener->masterMonitor) {
        d_actionQueueFree(listener->masterMonitor);
    }
    (void)os_mutexDestroy(&(listener->masterLock));
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));
}

static void
d_groupLocalListenerInit(
    d_groupLocalListener listener)
{
    os_duration sleepTime;
    os_threadAttr ta;
    d_durability durability;

    /* Initialize groupLocalListener */
    sleepTime = OS_DURATION_INIT(0, 100000000);  /* 100 ms */
    listener->lastSequenceNumber         = D_FLOOR_SEQUENCE_NUMBER;
    listener->initialGroupsAdministrated = FALSE;
    os_threadAttrInit(&ta);
    listener->masterMonitor              = d_actionQueueNew(
                                                "masterMonitor",
                                                 sleepTime, ta);

    durability = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));
    /* The groupLocalListerActionQueue thread has access to the persistent store
     * and should therefore set the persistentStoreStackSize
     */
    ta.stackSize = durability->configuration->persistentStoreStackSize;
    listener->actionQueue                = d_actionQueueNew(
                                                "groupLocalListenerActionQueue",
                                                sleepTime, ta);
    os_mutexInit(&(listener->masterLock), NULL);
    listener->fellowListener = d_eventListenerNew(
                                        D_FELLOW_NEW | D_FELLOW_REMOVED,
                                        notifyFellowEvent,
                                        listener);
    listener->nameSpaceListener = d_eventListenerNew(
                                        D_NAMESPACE_NEW | D_NAMESPACE_DELAYED_INITIAL,
                                        notifyNameSpaceEvent,
                                        listener);
    listener->snapshotRequestNumber = 0;
}

d_groupLocalListener
d_groupLocalListenerNew(
    d_subscriber subscriber)
{
    d_groupLocalListener listener = NULL;

    assert(d_subscriberIsValid(subscriber));

    if (subscriber->sampleChainListener) {
        assert(d_sampleChainListenerIsValid(subscriber->sampleChainListener));
        /* Allocate grouplocalListener object */
        listener = d_groupLocalListener(os_malloc(C_SIZEOF(d_groupLocalListener)));
        if (listener) {
            /* Call super-init */
            d_listenerInit(d_listener(listener), D_GROUP_LOCAL_LISTENER, subscriber, NULL,
                           (d_objectDeinitFunc)d_groupLocalListenerDeinit);
            /* Initialize the d_groupLocalListener */
            listener->sampleChainListener = subscriber->sampleChainListener;
            d_groupLocalListenerInit(listener);
        }
    }
    return listener;
}

void
d_groupLocalListenerFree(
    d_groupLocalListener listener)
{
    assert(d_groupLocalListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

/*************** START NEW IMPL *************************/

/**
 * \brief Check if all groups of the fellow have been received
 *
 * If no groupsRequest message has been sent to the fellow yet
 * then TRUE is returned.
 *
 * @return TRUE, if all groups have been received, FALSE otherwise
 */
static c_bool
checkFellowGroupsKnown(
    d_fellow fellow,
    c_voidp args)
{
    c_long expected;
    c_ulong actual;
    c_bool* known;
    c_bool requested;

    known = (c_bool*)args;
    requested = d_fellowGetGroupsRequested(fellow);
    if (requested) {
        expected = d_fellowGetExpectedGroupCount(fellow);
        if (expected != -1) {
            actual = d_fellowGetGroupCount(fellow);
            if (actual >= ((c_ulong)expected)) {
                *known = TRUE;
            } else {
                *known = FALSE;
            }
        } else if ( (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_STATE) ||
                    (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL)) {
            *known = TRUE;
        } else {
            *known = FALSE;
        }
    } else {
        *known = TRUE;
    }
    return *known;

}


struct groupInfo {
    c_iter nameSpaces;
    c_bool iAmAMaster;
    d_groupsRequest request;
    d_publisher publisher;
    d_durability durability;
    c_bool requested;    /* Indicates if a groupsRequest has been sent */
};


static c_bool
requestGroups(
    d_fellow fellow,
    c_voidp args)
{
    c_bool toRequest = TRUE;
    d_networkAddress fellowAddr, master;
    struct groupInfo *info;
    c_ulong i;
    d_nameSpace ns;
    c_bool notInitial;
    d_admin admin;
    d_subscriber subscriber;
    d_sampleChainListener sampleChainListener;
    c_bool allFellowGroupsKnown = FALSE;

    /* Only attempt to request groups if never requested before or the groups have not yet been received */
    info = (struct groupInfo*)args;
    info->requested = FALSE;
    /* groupsRequests only moves from FALSE to TRUE so the race condition on groupsRequested can
     * only lead to requesting the groups more often than strictly necessary
     * As long as not all fellows groups are known keep requesting all groups are requested.
     * This is a precondition for handleMergeAlignment to wait for.
     * Eventually groupsRequested and allFellowGroupsKnown will become TRUE, at which point
     * no further requests will be issued
     */
    checkFellowGroupsKnown(fellow, &allFellowGroupsKnown);
    if ((!fellow->groupsRequested) || (!allFellowGroupsKnown)) {
        fellowAddr = d_fellowGetAddress(fellow);
        if (!info->iAmAMaster) {
            /* I am not the master. If the fellow is master for one of my
             * nameSpaces then request groups from this fellow in case
             * alignment is initial
             */
            admin = info->durability->admin;
            subscriber = d_adminGetSubscriber(admin);
            sampleChainListener = d_subscriberGetSampleChainListener(subscriber);
            toRequest = FALSE;
            for(i=0; i<c_iterLength(info->nameSpaces) && (!toRequest); i++) {
                d_networkAddress unAddressed = d_networkAddressUnaddressed();

                ns = d_nameSpace(c_iterObject(info->nameSpaces, i));
                master = d_nameSpaceGetMaster(ns);

                /* Request groups from the fellow if the fellow is the master */

                if (d_networkAddressEquals(master, fellowAddr)) {
                    notInitial = d_nameSpaceIsAlignmentNotInitial(ns);
                    if (notInitial) {
                        d_printTimedEvent(info->durability, D_LEVEL_FINER,
                              "I am very lazy and will not request groups from master %u\n",
                              fellowAddr->systemId);
                        /* Request groups that are still being aligned */
                        d_sampleChainListenerCheckUnfulfilled(sampleChainListener, ns, master);
                        toRequest = FALSE;
                    } else {
                        toRequest = TRUE;
                    }

                /* Request groups from the fellow if my master is 'nobody' and
                 * I cannot act as aligner myself.
                 */
                } else if (d_networkAddressEquals(master, unAddressed) && !d_nameSpaceIsAligner(ns)) {
                    d_printTimedEvent(info->durability, D_LEVEL_FINER,
                          "I have no master and cannot act as aligner, but fellow %u may be a potential master\n",
                          fellowAddr->systemId);
                    toRequest = TRUE;
                }

                d_networkAddressFree(master);
                d_networkAddressFree(unAddressed);
            } /* for */
            if (!toRequest) {
                d_printTimedEvent(info->durability, D_LEVEL_FINER,
                                  "No need to request groups from fellow %u\n",
                                  fellowAddr->systemId);
            }
        } else {
            /* Groups are only requested if not requested already */
            toRequest = !(fellow->groupsRequested);
        }
        if (toRequest) {
            /* Actually do the request. */
            d_printTimedEvent(info->durability, D_LEVEL_FINE,
                              "Requesting all groups from fellow %u\n",
                              fellowAddr->systemId);
            fellow->groupsRequested = TRUE;
            info->requested = TRUE;
            d_messageSetAddressee(d_message(info->request), fellowAddr);
            d_publisherGroupsRequestWrite(info->publisher, info->request, fellowAddr);
        }
        d_networkAddressFree(fellowAddr);
    }
    return TRUE;
}


static void
doGroupsRequest(
    d_groupLocalListener listener,
    c_iter nameSpaces,
    c_bool iAmAMaster)
{
    d_admin admin;
    struct groupInfo info;

    if(listener){
        admin           = d_listenerGetAdmin(d_listener(listener));

        info.nameSpaces = nameSpaces;
        info.durability = d_adminGetDurability(admin);
        info.iAmAMaster = iAmAMaster;
        info.publisher  = d_adminGetPublisher(admin);
        info.request    = d_groupsRequestNew(admin, NULL, NULL);
        /* Request groups from all known fellows */
        d_adminFellowWalk(admin, requestGroups, &info);
        d_groupsRequestFree(info.request);
    }
    return;
}


struct addressList {
    d_networkAddress address;
    c_ulong count;
    c_voidp next;
};

struct masterInfo {
    d_nameSpace nameSpace;
    d_networkAddress master;
    c_bool masterConfirmed;
    d_quality masterQuality;
    c_bool conflicts;           /* Indicator if all fellows agree with the proposal */
    d_serviceState masterState;
    d_durability durability;
    c_bool conflictsAllowed;
    struct addressList* list;
    c_bool fellowExists;        /* Indicator to determine if the fellow exists */
    c_bool initial;             /* boolean to decide to use the fixed fellow table admin->initial_fellows or the dynamically changing table admin->fellows */
};

static void
addMajorityMaster(
    struct masterInfo* info,
    d_networkAddress address)
{
    struct addressList *list, *prevList = NULL;
    c_bool found;

    assert(info);

    found = FALSE;

    if(info->list){
        list = info->list;

        while(list && !found){
            prevList = list;
            if(d_networkAddressEquals(address, list->address)){
                found = TRUE;
                list->count++;
            }
            list = list->next;
        }
        if(!found){
            prevList->next = (struct addressList*)(os_malloc(sizeof(struct addressList)));
            list = prevList->next;
        }
    } else {
        info->list = (struct addressList*)(os_malloc(sizeof(struct addressList)));
        list = info->list;
    }

    if(!found){
        list->address = d_networkAddressNew(address->systemId, address->localId, address->lifecycleId);
        list->count = 1;
        list->next = NULL;
    }
    return;
}

static void
removeMajorityMaster(
    struct masterInfo* info,
    d_networkAddress address)
{
    struct addressList *list, *prev;
    c_bool found;

    assert(info);

    prev = NULL;
    list = info->list;
    found = FALSE;

    while(list && !found){
        if(d_networkAddressEquals(list->address, address)){
            if(prev){
                prev->next = list->next;
            } else {
                info->list = list->next;
            }
            d_networkAddressFree(list->address);
            os_free(list);
            found = TRUE;
        }

        if (!found) {
            prev = list;
            list = list->next;
        }
    }
    return;
}

static d_networkAddress
getMajorityMaster(
    struct masterInfo* info)
{
    d_networkAddress master;
    c_ulong count;
    c_bool replace;
    struct addressList* list;

    assert(info);

    if(info->list){
        master = d_networkAddressNew(info->list->address->systemId,
                info->list->address->localId, info->list->address->lifecycleId);
        count = info->list->count;

        list = (struct addressList*)(info->list->next);

        while(list){
            replace = FALSE;

            if(list->count > count){
                replace = TRUE;
            } else if (list->count == count){
                if(d_networkAddressCompare(list->address, master) > 0){
                    replace = TRUE;
                }
            }

            if(replace){
                d_networkAddressFree(master);
                master = d_networkAddressNew(list->address->systemId,
                        list->address->localId, list->address->lifecycleId);
                count = list->count;
            }
            list = (struct addressList*)list->next;
        }
    } else {
        master = d_networkAddressUnaddressed();
    }
    return master;
}

static void
freeMajorityMasters(
    struct masterInfo* info)
{
    struct addressList *list, *prevList;

    assert(info);

    list = info->list;

    while(list){
        d_networkAddressFree(list->address);
        prevList = list;
        list = list->next;
        os_free(prevList);
    }
    return;
}

static c_bool
determineExistingMaster(
    d_fellow fellow,
    c_voidp userData)
{
    struct masterInfo* m;
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowMaster, fellowAddress;
    char *role, *fellowRole;

    m = (struct masterInfo*)userData;
    fellowNameSpace = d_fellowGetNameSpace(fellow, m->nameSpace);
    role = d_nameSpaceGetRole(m->nameSpace);

    /* Check if fellow has compatible nameSpace. If not go to the next one */
    if (fellowNameSpace) {
        fellowMaster = d_nameSpaceGetMaster(fellowNameSpace);
        fellowRole = d_nameSpaceGetRole(fellowNameSpace);

        if (strcmp(fellowRole, role) == 0) {
            /* If I haven't found a potential master so far, check whether the
             * fellow determined a (potential) master already. If so, conform
             * to the selected master.
             */
            if(d_networkAddressIsUnaddressed(m->master)){
                if(!d_networkAddressIsUnaddressed(fellowMaster)){
                    d_networkAddressFree(m->master);
                    m->master = d_networkAddressNew(
                            fellowMaster->systemId,
                            fellowMaster->localId,
                            fellowMaster->lifecycleId);
                    addMajorityMaster(m, fellowMaster);
                }
            } else if(!d_networkAddressIsUnaddressed(fellowMaster)){
                /* If the fellow has determined a (potential) master
                 * that doesn't match my current one, there's a conflict and
                 * I need to drop out of this function.
                 */
                d_printTimedEvent(m->durability, D_LEVEL_FINEST,
                    "m->master %u:%s fellowMaster %u:%s\n",
                    m->master->systemId, m->masterConfirmed?"TRUE":"FALSE",
                    fellowMaster->systemId, d_nameSpaceIsMasterConfirmed(fellowNameSpace)?"TRUE":"FALSE");

                if(!d_networkAddressEquals(m->master, fellowMaster)){
                    c_bool fellowMasterConfirmed = d_nameSpaceIsMasterConfirmed(fellowNameSpace);

                    if ((fellowMasterConfirmed) &&
                        (!m->masterConfirmed)) {

                        fellowAddress = d_fellowGetAddress(fellow);
                        d_printTimedEvent(m->durability, D_LEVEL_FINEST,
                            "Fellow '%u' reports confirmed master '%u' for nameSpace '%s'. " \
                            "while I found unconfirmed master '%u'.\n",
                            fellowAddress->systemId,
                            fellowMaster->systemId,
                            d_nameSpaceGetName(m->nameSpace),
                            m->master->systemId);
                        d_networkAddressFree(fellowAddress);

                        d_networkAddressFree(m->master);
                        m->master = d_networkAddressNew(
                                fellowMaster->systemId,
                                fellowMaster->localId,
                                fellowMaster->lifecycleId);
                        m->masterConfirmed = TRUE;
                        m->conflicts = FALSE;
                    } else if ((fellowMasterConfirmed) &&
                               (m->masterConfirmed)) {
                        m->conflicts = TRUE;

                        fellowAddress = d_fellowGetAddress(fellow);
                        d_printTimedEvent(m->durability, D_LEVEL_FINEST,
                            "Fellow '%u' reports master '%u' for nameSpace '%s'. " \
                            "while I found master '%u'.\n",
                            fellowAddress->systemId,
                            fellowMaster->systemId,
                            d_nameSpaceGetName(m->nameSpace),
                            m->master->systemId);
                        d_networkAddressFree(fellowAddress);
                    } else if ((!fellowMasterConfirmed) &&
                               (!m->masterConfirmed)) {
                        fellowAddress = d_fellowGetAddress(fellow);
                        d_printTimedEvent(m->durability, D_LEVEL_FINEST,
                            "Fellow '%u' reports unconfirmed master '%u' for nameSpace '%s'. " \
                            "while I found unconfirmed master '%u', dropping mine\n",
                            fellowAddress->systemId,
                            fellowMaster->systemId,
                            d_nameSpaceGetName(m->nameSpace),
                            m->master->systemId);
                        d_networkAddressFree(fellowAddress);

                        d_networkAddressFree(m->master);
                        m->master = d_networkAddressUnaddressed();
                    } else {
                        /* No need to do anything, I already am my own confirmed master */
                    }
                }
                addMajorityMaster(m, fellowMaster);
            }
        }

        d_networkAddressFree(fellowMaster);
        os_free (fellowRole);
    /* If fellow namespace is from other role, it can't be master for this node */
    }

    os_free (role);

    return TRUE;
}


static c_bool
determineNewMasterLegacy(
    d_fellow fellow,
    c_voidp userData)
{
    struct masterInfo* m;
    d_nameSpace fellowNameSpace;
    d_networkAddress fellowAddress;
    c_bool isAligner, replace;
    d_quality quality;
    d_serviceState fellowState;
    char *role, *fellowRole;
    c_bool contEval = FALSE;

    m = (struct masterInfo*)userData;
    replace = FALSE;
    fellowNameSpace = d_fellowGetNameSpace(fellow, m->nameSpace);

    if(fellowNameSpace){

        role = d_nameSpaceGetRole (m->nameSpace);
        fellowRole = d_nameSpaceGetRole (fellowNameSpace);

        if (strcmp (role, fellowRole) == 0) {
            isAligner = d_nameSpaceIsAligner(fellowNameSpace);
            quality = d_nameSpaceGetInitialQuality(fellowNameSpace);
            fellowState = d_fellowGetState(fellow);

            if(isAligner){
                if(d_networkAddressIsUnaddressed(m->master)){
                    replace = TRUE;

                /* This behavior is undesired when delayedAlignment is active */
                } else if(!d_nameSpaceGetDelayedAlignment(m->nameSpace)) {
                    if ((m->masterState <= D_STATE_DISCOVER_PERSISTENT_SOURCE) &&
                        (fellowState > D_STATE_DISCOVER_PERSISTENT_SOURCE))
                    {
                        replace = TRUE;
                    } else if ((m->masterState > D_STATE_DISCOVER_PERSISTENT_SOURCE) &&
                               (fellowState <= D_STATE_DISCOVER_PERSISTENT_SOURCE))
                    {
                        replace = FALSE;
                    } else {
                        contEval = TRUE;
                    }
                } else {
                    contEval = TRUE;
                }

                if (contEval) {
                    os_compare eq;
                    eq = d_qualityCompare(quality, m->masterQuality);
                    if (eq == OS_MORE) {
                        replace = TRUE;
                    } else if (eq == OS_EQUAL) {
                        fellowAddress = d_fellowGetAddress(fellow);
                        if(d_networkAddressCompare(fellowAddress, m->master) > 0){
                            replace = TRUE;
                        }
                        d_networkAddressFree(fellowAddress);
                    }
                }
            }
        }
        os_free (role);
        os_free (fellowRole);
    }

    if(replace){
        if(m->master){
            d_networkAddressFree(m->master);
        }
        m->master = d_fellowGetAddress(fellow);
        m->masterQuality = quality;
        m->masterState = fellowState;
    }
    return TRUE;
}

static int
determineNewMasterPriorityCompare(
        struct masterInfo *m,
        d_fellow fellow,
        d_nameSpace fellowNameSpace)
{
    d_quality quality = d_nameSpaceGetInitialQuality(fellowNameSpace);
    os_compare qcmp;


    /* Replace if priority of proposed master is worse than that of fellow */
    if (d_nameSpaceGetMasterPriority(m->nameSpace) < d_nameSpaceGetMasterPriority(fellowNameSpace)) {
       return -1;
    } else if (d_nameSpaceGetMasterPriority(m->nameSpace) > d_nameSpaceGetMasterPriority(fellowNameSpace)) {
       return 1;

    /* Replace if quality of proposed master is worse than that of fellow */
    } else if ((qcmp = d_qualityCompare(m->masterQuality, quality)) == OS_LESS) {
       return -1;
    } else if (qcmp == OS_MORE) {
       return 1;

    /* Replace if systemId of proposed master is lower than that of fellow */
    } else {
        d_networkAddress fellowAddress = d_fellowGetAddress(fellow);
        int cmp = d_networkAddressCompare(m->master, fellowAddress);
        d_networkAddressFree(fellowAddress);
        return cmp;
    }
}

static int
determineNewMasterPriorityIsFellowCandidate(
    struct masterInfo* m,
    d_nameSpace fellowNameSpace)
{
    char *role = d_nameSpaceGetRole (m->nameSpace);
    char *fellowRole = d_nameSpaceGetRole (fellowNameSpace);
    int samerole = (strcmp (role, fellowRole) == 0);
    os_free (role);
    os_free (fellowRole);
    /* A fellow is a candidate master for me if it has the same role as me, is aligner and has a priority > 0 */
    return samerole && d_nameSpaceIsAligner(fellowNameSpace) && (d_nameSpaceGetMasterPriority(fellowNameSpace) > 0);
}

static void
determineNewMasterPriorityNoreturn(
    d_fellow fellow,
    struct masterInfo* m)
{
    d_nameSpace fellowNameSpace;
    c_bool replace;
    d_quality quality;

    assert(m);

    /* Skip if no fellow nameSpace found */
    if ((fellowNameSpace = d_fellowGetNameSpace(fellow, m->nameSpace)) == NULL) {
        d_trace(D_TRACE_MASTER_SELECTION, "%s - REJECT fellow %u as candidate master for nameSpace %s. Reason: nameSpace unknown\n",
                OS_FUNCTION, fellow->address->systemId, m->nameSpace->name);
        return;
    }

    /* Skip if fellow is not a replacement candidate */
    if (!determineNewMasterPriorityIsFellowCandidate(m, fellowNameSpace)) {
        d_trace(D_TRACE_MASTER_SELECTION, "%s - REJECT fellow %u as candidate master for nameSpace %s. Reason: Not a suitable candidate\n",
                OS_FUNCTION, fellow->address->systemId, m->nameSpace->name);
        return;
    }

    quality = d_nameSpaceGetInitialQuality(fellowNameSpace);

    d_trace(D_TRACE_MASTER_SELECTION, "%s - Checking fellow '%u' (priority: %u, quality: %"PA_PRItime")\n",
        OS_FUNCTION, fellow->address->systemId, d_nameSpaceGetMasterPriority(fellowNameSpace), OS_TIMEW_PRINT(quality));

    if (d_networkAddressIsUnaddressed(m->master)) {
        d_trace(D_TRACE_MASTER_SELECTION, "%s - ACCEPT fellow %u as candidate master for nameSpace %s. Reason: current master is unAddressed\n",
                OS_FUNCTION, fellow->address->systemId, m->nameSpace->name);
        replace = TRUE;
    /* This behavior is undesired when delayedAlignment is active */
    } else if(d_nameSpaceGetDelayedAlignment(m->nameSpace)) {
        d_trace(D_TRACE_MASTER_SELECTION, "%s - REJECT fellow %u as candidate master for nameSpace %s. Reason: delayedAlignment configured\n",
                OS_FUNCTION, fellow->address->systemId, m->nameSpace->name);
        replace = FALSE;
    } else if (determineNewMasterPriorityCompare(m, fellow, fellowNameSpace) < 0) {
        d_trace(D_TRACE_MASTER_SELECTION, "%s - ACCEPT fellow %u as candidate master for nameSpace %s. Reason: fellow is better candidate than current master\n",
                OS_FUNCTION, fellow->address->systemId, m->nameSpace->name);
        replace = TRUE;
    } else {
        d_trace(D_TRACE_MASTER_SELECTION, "%s - REJECT fellow %u as candidate master for nameSpace %s. Reason: fellow is worse candidate than current master\n",
                OS_FUNCTION, fellow->address->systemId, m->nameSpace->name);
        replace = FALSE;
    }

    if (replace) {

        if(m->master){
            d_networkAddressFree(m->master);
        }
        m->master = d_fellowGetAddress(fellow);
        m->masterQuality = quality;
        m->masterState = d_fellowGetState(fellow);
        d_trace(D_TRACE_MASTER_SELECTION, "%s - Current master candidate for nameSpace %s: %u\n",
                OS_FUNCTION, m->nameSpace->name, fellow->address->systemId);
        m->conflicts = TRUE;

    }
}

static c_bool
determineNewMasterPriority(
    d_fellow fellow,
    c_voidp userData)
{
   determineNewMasterPriorityNoreturn(fellow, userData);
   return TRUE;
}

static c_bool
determineNewMaster(
    d_fellow fellow,
    c_voidp userData)
{
    c_bool result = FALSE;
    d_durability durability = d_threadsDurability();
    d_configuration config = d_durabilityGetConfiguration(durability);
    struct masterInfo *mastership = (struct masterInfo *)userData;

    /* Determine the master selection algorithm to use from the config. */

    if (config->masterSelectionLegacy) {
        result = determineNewMasterLegacy(fellow, mastership);
    } else {
        result = determineNewMasterPriority(fellow, mastership);
    }

    return result;
}


static void
wait_for_responses(
    os_duration d)
{
    d_durability durability = d_threadsDurability();
    os_result osr;
    d_thread const self = d_threadLookupSelf ();

    /* Do not wait for fellows */
    if (OS_DURATION_ISZERO(d)) {
        return;
    }

    d_printTimedEvent(durability, D_LEVEL_FINER, "Waiting %f seconds for responses.\n", os_durationToReal(d));

    os_mutexLock(&(durability->terminateMutex));
    if ((osr = d_condTimedWait(self, &durability->terminateCondition, &durability->terminateMutex, d)) == os_resultFail) {
        OS_REPORT(OS_CRITICAL, D_CONTEXT_DURABILITY, 0, "d_condTimedWait failed; terminating");
        /* terminate durability */
        os_mutexUnlock(&(durability->terminateMutex));
        d_durabilityTerminate(durability, TRUE);
        os_mutexLock(&(durability->terminateMutex));
    }
    os_mutexUnlock(&(durability->terminateMutex));
}


static void
propose_master_for_namespace(
    d_admin admin,
    d_nameSpace nameSpace,
    struct masterInfo *mastership)
{
    char master_string[18];   /* maximum representation 'fellow ' (7) + c_ulong (10)  + '\0' (1) */
    d_networkAddress myAddress = d_adminGetMyAddress(admin);
    d_networkAddress unAddressed = d_networkAddressUnaddressed();
    d_durability durability = d_threadsDurability();

    /* Determine the best possible master given the current state of the fellows
     * Include myself if I am aligner and my master priority > 0
     */
    mastership->nameSpace = nameSpace;
    mastership->conflicts = FALSE;
    mastership->masterState = d_durabilityGetState(durability);
    mastership->list = NULL;

    if (d_nameSpaceIsAligner(nameSpace) && (d_nameSpaceGetMasterPriority(nameSpace) > 0)) {
        mastership->master = d_adminGetMyAddress(admin);
        d_trace(D_TRACE_MASTER_SELECTION, "%s - initial master selected for %s: myself (priority: %u, quality: %"PA_PRItime")\n",
                OS_FUNCTION, nameSpace->name, d_nameSpaceGetMasterPriority(nameSpace), OS_TIMEW_PRINT(d_nameSpaceGetInitialQuality(nameSpace)));

    } else {
        mastership->master = d_networkAddressUnaddressed();
        d_trace(D_TRACE_MASTER_SELECTION, "%s - initial master selected for %s: nobody (priority: %u, quality: %"PA_PRItime")\n",
                OS_FUNCTION, nameSpace->name, d_nameSpaceGetMasterPriority(nameSpace), OS_TIMEW_PRINT(d_nameSpaceGetInitialQuality(nameSpace)));
    }

    /* Determine the best master among the available fellows */
    if (mastership->initial) {
        d_adminInitialFellowWalk(admin, determineNewMaster, mastership);
    } else {
        d_adminFellowWalk(admin, determineNewMaster, mastership);
    }

    /* Create a log entry of the proposal */
    if (d_networkAddressEquals(mastership->master, myAddress)) {
        snprintf(master_string, sizeof(master_string), "myself");
    } else if (d_networkAddressEquals(mastership->master, unAddressed)) {
        snprintf(master_string, sizeof(master_string), "nobody");
    } else {
        snprintf(master_string, sizeof(master_string), "fellow %u", mastership->master->systemId);
    }

    d_printTimedEvent(durability, D_LEVEL_FINEST,
        "Proposing %s as master for nameSpace '%s' (confirmed: %d, conflicts: %d)\n",
        master_string, d_nameSpaceGetName(mastership->nameSpace), d_nameSpaceIsMasterConfirmed(mastership->nameSpace), mastership->conflicts);

    d_networkAddressFree(myAddress);
    d_networkAddressFree(unAddressed);
}



c_bool
d_groupLocalListenerDetermineMastersImproved(
    d_groupLocalListener listener,
    c_iter nameSpaces,
    c_bool initial)
{
    /* Select a master for each of the nameSpaces.
     * When initial is set the master is determined for fellows in admin->initial_fellows
     * (which is set during the resolution of the initial conflict in
     * d_conflictResolver.c:resolveInitialConflict().
     *
     * Return TRUE if I have become master for one of the nameSpaces
     */

    d_admin admin;
    d_durability durability;
    d_configuration configuration;
    d_nameSpace nameSpace;
    d_subscriber subscriber;
    d_nameSpacesRequestListener nsrListener;
    d_networkAddress unAddressed, myAddress;
    c_iterIter iter;
    c_bool IAmMaster = FALSE;
    struct masterInfo mastership;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    configuration = d_durabilityGetConfiguration(durability);
    subscriber = d_adminGetSubscriber(admin);
    nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);
    unAddressed = d_networkAddressUnaddressed();
    myAddress = d_adminGetMyAddress(admin);
    mastership.initial = initial;

    /* Wait the masterElectionWaitTime (default: zero). */
    wait_for_responses(configuration->masterElectionWaitTime);

    mastership.masterQuality = D_QUALITY_ZERO;

    iter = c_iterIterGet(nameSpaces);
    while (((nameSpace = (d_nameSpace)c_iterNext(&iter)) != NULL) && (!d_durabilityMustTerminate(durability))) {

        propose_master_for_namespace(admin, nameSpace, &mastership);

        /* Set and confirm the chosen master */
        d_nameSpaceSetMaster(nameSpace, mastership.master);
        d_nameSpaceMasterConfirmed(nameSpace);

        /* Indicate who has become master */
        if (d_networkAddressEquals(nameSpace->master, myAddress)) {
             d_printTimedEvent(durability, D_LEVEL_INFO,
                "Confirming master: I am the master for nameSpace '%s'.\n",
                 d_nameSpaceGetName(nameSpace));
             IAmMaster = TRUE;
        } else {
             d_printTimedEvent(durability, D_LEVEL_INFO,
                "Confirming master: Fellow '%u' is the master for nameSpace '%s'.\n",
                mastership.master->systemId, d_nameSpaceGetName(nameSpace));
        }

        d_networkAddressFree(mastership.master);
    }

    /* Set quality of namespace to quality of master */
    if (d_qualityCompare(mastership.masterQuality, D_QUALITY_ZERO) == OS_MORE) {
        iter = c_iterIterGet(nameSpaces);
        while (((nameSpace = (d_nameSpace)c_iterNext(&iter)) != NULL)) {
            d_nameSpaceSetInitialQuality(nameSpace, mastership.masterQuality);
        }
    }

    /* Report my namespace choices to others */
    d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

    if (!d_durabilityMustTerminate(durability)) {
        /* Request groups */
        doGroupsRequest(listener, nameSpaces, IAmMaster);
    }

    d_networkAddressFree(myAddress);
    d_networkAddressFree(unAddressed);

    return IAmMaster;
}


/* PRECONDTION: listener->masterLock is locked
 *
 * @return Returns TRUE if I have become a master for one or more nameSpaces.
 */
static c_bool
d_groupLocalListenerDetermineMastersLegacy(
    d_groupLocalListener listener,
    c_iter nameSpaces,
    c_bool initial)
{
    d_thread self = d_threadLookupSelf ();
    d_admin admin;
    d_durability durability;
    d_configuration configuration;
    c_ulong length, i;
    d_nameSpace nameSpace;
    d_subscriber subscriber;
    d_publisher publisher;
    d_nameSpacesRequestListener nsrListener;
    d_networkAddress unaddressed, myAddress, master, lastMaster, dummyAddress;
    struct checkNameSpacesHelper checkNsHelper;
    struct masterInfo mastership;
    os_duration sleepTime = OS_DURATION_INIT(0, 100000000); /* 100 ms */
    os_timeE endTime;
    c_bool conflicts, firstTime, cont, proceed;
    d_quality myQuality;
    c_bool iAmAMaster;
    d_serviceState fellowState;
    d_fellow fellow, retryFellow, dummy;
    c_ulong tries, maxTries;
    c_bool initialUnaddressed;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    configuration = d_durabilityGetConfiguration(durability);
    length = c_iterLength(nameSpaces);
    subscriber = d_adminGetSubscriber(admin);
    publisher = d_adminGetPublisher(admin);
    nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);
    unaddressed = d_networkAddressUnaddressed();
    myAddress = d_adminGetMyAddress(admin);
    firstTime = TRUE;
    iAmAMaster = FALSE;
    maxTries = configuration->majorityVotingThreshold;
    tries = 0;

    /* The legacy algorithm can already handle the situation where
     * new nodes appear while masters are being selected. For that reason
     * the 'initial' argument is not used. */
    OS_UNUSED_ARG(initial);

    mastership.durability = durability;
    mastership.masterQuality = D_QUALITY_ZERO;

    checkNsHelper.request = d_nameSpacesRequestNew(admin);
    checkNsHelper.retryFellows = NULL;
    checkNsHelper.publisher = publisher;
    checkNsHelper.retryDelay = configuration->heartbeatExpiryTime;

    do {
        conflicts = FALSE;
        tries++;

        /* Check if namespaces still have the same master */
        for(i=0; (i < length) && (d_durabilityMustTerminate(durability) == FALSE); i++){
            nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));

            /* Remember the last master I selected. If the 'existing'
             * one I find now is different, I need to do this whole loop again.
             */
            if (firstTime) {
                lastMaster = d_nameSpaceGetMaster(nameSpace);

                if (d_networkAddressIsUnaddressed(lastMaster)) {
                    initialUnaddressed = TRUE;
                } else {
                    d_networkAddressFree(lastMaster);
                    lastMaster = d_networkAddressUnaddressed();
                    initialUnaddressed = FALSE;
                }
            } else {
                lastMaster = d_nameSpaceGetMaster(nameSpace);
                if (d_networkAddressIsUnaddressed(lastMaster)) {
                    initialUnaddressed = TRUE;
                } else {
                    initialUnaddressed = FALSE;
                }
            }

            mastership.nameSpace = nameSpace;
            mastership.master = d_nameSpaceGetMaster(nameSpace);
            mastership.masterConfirmed = d_nameSpaceIsMasterConfirmed(nameSpace);
            mastership.conflicts = FALSE;
            mastership.list = NULL;

            /* Always use un-addressed as my master when looking for an
             * existing master.
             */
            d_nameSpaceSetMaster(nameSpace, unaddressed);
            d_nameSpaceMasterPending (nameSpace);

            if(tries >= maxTries){
                mastership.conflictsAllowed = TRUE;
            } else {
                mastership.conflictsAllowed = FALSE;
            }

            if (initialUnaddressed || !firstTime) {
                /* Walk over all fellows that are approved */
                d_adminFellowWalk(admin, determineExistingMaster, &mastership);
            }

            if((!mastership.conflicts) && (!d_networkAddressIsUnaddressed(mastership.master))){
                /* Check whether the found fellow is still alive and kicking. */

                /* Some node(s) could already have chosen me as master */
                if(!d_networkAddressEquals(mastership.master, myAddress)){
                    fellow = d_adminGetFellow(admin, mastership.master);

                    /*Fellow may be gone already or is currently terminating*/
                    if(!fellow){
                        d_networkAddressFree(mastership.master);
                        mastership.master = d_networkAddressUnaddressed();
                    } else {
                        fellowState = d_fellowGetState(fellow);

                        if((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)){
                            d_networkAddressFree(mastership.master);
                            mastership.master = d_networkAddressUnaddressed();
                        } else if(!d_networkAddressEquals(lastMaster, mastership.master)){
                            /* Check whether I determined a master already
                             * in the previous loop.
                             */
                            if(!d_networkAddressIsUnaddressed(lastMaster)) {
                                /* Existing master is not the same as the one I found before
                                 * so make sure I'll check once more.
                                 */
                                conflicts = TRUE;
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                    "The existing master '%u' I found now for nameSpace '%s'" \
                                    "doesn't match the one '%u' I found before. " \
                                    "Waiting for confirmation...\n",
                                    mastership.master->systemId,
                                    d_nameSpaceGetName(nameSpace),
                                    lastMaster->systemId);
                            }
                        }
                        d_fellowFree(fellow);
                    }
                }
            } else if(mastership.conflictsAllowed){
                cont = TRUE;
                mastership.conflicts = FALSE;

                OS_REPORT(OS_WARNING, OS_FUNCTION, 0, "Determining master based on majority voting, this may cause alignment issues.");

                while(cont){
                    if(mastership.master){
                        d_networkAddressFree(mastership.master);
                    }
                    mastership.master = getMajorityMaster(&mastership);

                    if(!d_networkAddressIsUnaddressed(mastership.master)){
                        fellow = d_adminGetFellow(admin, mastership.master);

                        /*Fellow may be gone already or is currently terminating*/
                        if(!fellow){
                            if (d_networkAddressEquals(mastership.master, myAddress)) {
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                     "Found myself as majority master '%d' for nameSpace '%s'.\n",
                                     mastership.master->systemId,
                                     d_nameSpaceGetName(nameSpace));
                                 cont = FALSE;
                            } else {
                                removeMajorityMaster(&mastership, mastership.master);
                            }
                        } else {
                            fellowState = d_fellowGetState(fellow);

                            if((fellowState == D_STATE_TERMINATING) || (fellowState == D_STATE_TERMINATED)){
                                removeMajorityMaster(&mastership, mastership.master);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                    "Found majority master '%d' for nameSpace '%s'.\n",
                                    mastership.master->systemId,
                                    d_nameSpaceGetName(nameSpace));
                                cont = FALSE;
                            }
                            d_fellowFree(fellow);
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Tried to find majority master for nameSpace '%s', but found none.\n",
                            d_nameSpaceGetName(nameSpace));
                        cont = FALSE;
                    }
                }
            }
            freeMajorityMasters(&mastership);

            if((mastership.conflicts) || d_networkAddressIsUnaddressed(mastership.master)){
                if(mastership.conflicts){
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Found conflicting masters for nameSpace '%s'. " \
                        "Determining new master now...\n",
                        d_nameSpaceGetName(nameSpace));
                    conflicts = TRUE;
                    mastership.conflicts = FALSE;
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Found no existing master for nameSpace '%s'. " \
                        "Determining new master now...\n",
                        d_nameSpaceGetName(nameSpace));
                }
                d_networkAddressFree(mastership.master);

                if(d_nameSpaceIsAligner(nameSpace)) {
                    /* Advertise myself as candidate master */
                    myQuality = d_nameSpaceGetInitialQuality(nameSpace);
                    mastership.master = d_adminGetMyAddress(admin);
                    mastership.masterQuality = myQuality;
                    mastership.masterState = d_durabilityGetState(durability);
                } else {
                    /* Do not advertise myself as candidate master */
                    mastership.master = d_networkAddressUnaddressed();
                    mastership.masterQuality = D_QUALITY_ZERO;
                    mastership.masterState = D_STATE_INIT;
                }
                /*7. Walk over all fellows that are approved again.*/
                d_adminFellowWalk(admin, determineNewMaster, &mastership);

                if(d_networkAddressIsUnaddressed(mastership.master)){
                    /* Depending on the configuration setting TimeToWaitForAligner
                     * the service should keep waiting until an aligner becomes
                     * available. If no aligner is available within the specified time
                     * the system should move to state INCOMPATIBLE_CONFIGURATION and
                     * gracefully exit.
                     * The INCOMPATIBLE_CONFIGURATION is then picked up by the splice
                     * daemon that can execute the configured FailureAction.
                     * Note: at the moment only 0 is supported, any other value
                     * will result in waiting indefinitely for an aligner.
                     */
                    if (os_durationCompare(configuration->timeToWaitForAligner, OS_DURATION_ZERO) != OS_EQUAL) {
                        /* For the moment it is assumed that in case timeToWaitForAligner != 0
                         * then the service will wait indefinitely until an aligner becomes available.
                         */
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                           "There's no new master available for nameSpace '%s'. " \
                           "Awaiting availability of a new master...\n",
                           d_nameSpaceGetName(nameSpace));
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                           "There's no new master available for nameSpace '%s'. " \
                           "Incompatible configuration.\n",
                           d_nameSpaceGetName(nameSpace));
                        /* Move the state of the durability service to
                         * STATE_INCOMPATIBLE_CONFIGURATION and terminate
                         * gracefully.
                         */
                        d_durabilityTerminate(durability, FALSE);
                    }
                } else if(d_networkAddressEquals(mastership.master, myAddress)){
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                       "I want to be the new master for nameSpace '%s'. " \
                       "Waiting for confirmation...\n",
                       d_nameSpaceGetName(nameSpace));
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "I want fellow '%u' to be the new master for nameSpace '%s'. " \
                        "Waiting for confirmation...\n",
                        mastership.master->systemId,
                        d_nameSpaceGetName(nameSpace));
                }
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Found existing master '%u' for nameSpace '%s'. " \
                    "Waiting for confirmation...\n",
                    mastership.master->systemId,
                    d_nameSpaceGetName(nameSpace));
            }
            d_nameSpaceSetMaster(nameSpace, mastership.master);
            d_networkAddressFree(mastership.master);
            d_networkAddressFree(lastMaster);
        } /* for */

        d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

        /* Do the same thing again if there were conflicts.
         * Since this step must be taken at least once (even if there are
         * no conflicts), 'firstTime' is checked also.
         */
        if(conflicts || firstTime){
            if(firstTime){
                firstTime = FALSE;
                /*To make sure the do-while is done at least once more: */
                conflicts = TRUE;
            }

            /* Wait the heartbeat expiry time. */
            endTime = os_timeEGet();
            endTime = os_timeEAdd(endTime, configuration->heartbeatExpiryTime);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Waiting the heartbeat expiry period: %f seconds.\n",
                    os_durationToReal(configuration->heartbeatExpiryTime));

            while ((d_durabilityMustTerminate(durability) == FALSE) &&
                    (os_timeECompare(os_timeEGet(), endTime) == OS_LESS)) {
                d_sleep(self, sleepTime);
            }

            proceed = FALSE;
            while((d_durabilityMustTerminate(durability) == FALSE) && (proceed == FALSE)) {
                d_adminFellowWalk(admin, checkNameSpaces, &checkNsHelper);
                if (c_iterLength(checkNsHelper.retryFellows) > 0) {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Found %d incomplete fellow(s)\n", c_iterLength(checkNsHelper.retryFellows));
                    proceed = FALSE;
                    if (os_timeECompare(os_timeEGet(), checkNsHelper.retryTime) == OS_MORE) {
                        /* There hasn't been a new incomplete fellow for the last 2*heartbeat period, remove any remaining incomplete fellows */
                        retryFellow = d_fellow(c_iterTakeFirst(checkNsHelper.retryFellows));

                        while(retryFellow){
                            d_fellowSetCommunicationState(retryFellow, D_COMMUNICATION_STATE_TERMINATED);
                            dummy = d_adminRemoveFellow(admin, retryFellow, FALSE);
                            if (dummy) {
                                dummyAddress = d_fellowGetAddress(dummy);
                                d_printTimedEvent(durability, D_LEVEL_INFO,
                                    "Removed incomplete fellow %u from admin\n",
                                    dummyAddress->systemId);
                                d_networkAddressFree(dummyAddress);
                                d_fellowFree(dummy);
                            }
                            d_fellowFree(retryFellow);
                            retryFellow = d_fellow(c_iterTakeFirst(checkNsHelper.retryFellows));
                        }
                        c_iterFree(checkNsHelper.retryFellows);
                        checkNsHelper.retryFellows = NULL;
                        proceed = TRUE;
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                        "All fellows' namespaces complete\n");
                    c_iterFree(checkNsHelper.retryFellows);
                    checkNsHelper.retryFellows = NULL;
                    proceed = TRUE;
                }
                if (proceed == FALSE) {
                    d_sleep(self, sleepTime);
                }
            }
            if((d_durabilityMustTerminate(durability) == TRUE) && checkNsHelper.retryFellows){
                retryFellow = d_fellow(c_iterTakeFirst(checkNsHelper.retryFellows));

                while(retryFellow){
                    d_fellowFree(retryFellow);
                    retryFellow = d_fellow(c_iterTakeFirst(checkNsHelper.retryFellows));
                }
                c_iterFree(checkNsHelper.retryFellows);
                checkNsHelper.retryFellows = NULL;
            }

        } else if(d_durabilityMustTerminate(durability) == FALSE){
            assert(conflicts == FALSE);
            /*No more conflicts; all masters have been confirmed*/
            for(i=0; i<length; i++){
                nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));
                master = d_nameSpaceGetMaster(nameSpace);

                if(d_networkAddressEquals(master, myAddress)){
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                       "Confirming master: I am the master for nameSpace '%s'.\n",
                       d_nameSpaceGetName(nameSpace));
                    iAmAMaster = TRUE;
                } else {
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                           "Confirming master: Fellow '%u' is the master for nameSpace '%s'.\n",
                           master->systemId,
                           d_nameSpaceGetName(nameSpace));

                    /* Get masterfellow */
                    fellow = d_adminGetFellow(admin, master);
                    if(fellow) {
                        d_nameSpace fellowNamespace;
                        d_quality q;

                        /* Get fellow namespace */
                        fellowNamespace = d_fellowGetNameSpace(fellow, nameSpace);

                        q = d_nameSpaceGetInitialQuality(fellowNamespace);

                        /* Set quality of namespace to quality of master */
                        d_nameSpaceSetInitialQuality(nameSpace, q);

                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                               "Quality of namespace '%s' set to %"PA_PRItime".\n",
                               d_nameSpaceGetName(nameSpace), OS_TIMEW_PRINT(q));
                        d_fellowFree(fellow);
                    }
                }
                d_networkAddressFree(master);
                d_nameSpaceMasterConfirmed(nameSpace);

            }
        }
    } while ((conflicts == TRUE) && (d_durabilityMustTerminate(durability) == FALSE));

    d_nameSpacesRequestFree(checkNsHelper.request);
    assert(!checkNsHelper.retryFellows);

    d_durabilityUpdateStatistics(durability, d_statisticsUpdateConfiguration, admin);

    /* Re-report namespaces with correct confirmed status */
    d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

    /* Request groups */
    doGroupsRequest(listener, nameSpaces, iAmAMaster);

    d_networkAddressFree(unaddressed);
    d_networkAddressFree(myAddress);

    return iAmAMaster;
}

c_bool
d_groupLocalListenerDetermineMasters(
    d_groupLocalListener listener,
    c_iter nameSpaces,
    c_bool initial)
{
    d_admin admin = d_listenerGetAdmin(d_listener(listener));
    d_durability durability = d_adminGetDurability(admin);
    d_configuration config = d_durabilityGetConfiguration(durability);
    c_bool result;

    /* Determine whether or not to use the legacy algorithm */
    if (config->masterSelectionLegacy) {
        result = d_groupLocalListenerDetermineMastersLegacy(listener, nameSpaces, initial);
    } else {
        result = d_groupLocalListenerDetermineMastersImproved(listener, nameSpaces, initial);
    }
    return result;
}


/*****************************************************************************/

static void
initPersistentData(
    d_groupLocalListener listener)
{
    d_thread self = d_threadLookupSelf ();
    d_admin admin;
    d_subscriber subscriber;
    d_store store;
    d_durability durability;
    d_group group;
    u_participant participant;
    d_storeResult result;
    d_groupList list, next;
    c_ulong i, length;
    d_nameSpace nameSpace;
    d_durabilityKind dkind;
    c_iter nameSpaces;
    c_bool attached;
    v_group vgroup;
    c_ulong count;
    d_mergeState mergeState;

    admin         = d_listenerGetAdmin(d_listener(listener));
    durability    = d_adminGetDurability(admin);
    subscriber    = d_adminGetSubscriber(admin);
    store         = d_subscriberGetPersistentStore(subscriber);
    participant   = u_participant(d_durabilityGetService(durability));
    result        = d_storeGroupsRead(store, &list);

    /* Collect namespaces from admin */
    nameSpaces = d_adminNameSpaceCollect(admin);
    length        = c_iterLength(nameSpaces);

    if(result == D_STORE_RESULT_OK){

        /* Loop namespaces */
        for(i=0; (i<length) && (d_durabilityMustTerminate(durability) == FALSE); i++) {
            nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));

            if(d_nameSpaceMasterIsMe(nameSpace, admin)){
                /* This durability service is master for the nameSpace */
                dkind = d_nameSpaceGetDurabilityKind(nameSpace);

                if((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)){
                    os_timeM t = os_timeMGet();

                    next = list;

                    d_durabilitySetState(durability, D_STATE_INJECT_PERSISTENT);

                    /* Loop (persistent) groups, inject data from group */
                    while(next) {
                        d_threadAwake(self);
                        if(d_durabilityMustTerminate(durability) == FALSE){
                            if(d_nameSpaceIsIn(nameSpace, next->partition, next->topic) == TRUE) {
                                result = d_storeGroupInject(store, next->partition, next->topic, participant, &group);

                                if(result == D_STORE_RESULT_OK) {
                                    d_printTimedEvent(durability, D_LEVEL_FINER,
                                        "Group %s.%s locally created\n",
                                        next->partition, next->topic);


                                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                        "Data from group %s.%s must now be injected\n",
                                        next->partition, next->topic);

                                    vgroup = d_groupGetKernelGroup(group);
                                    attached = d_durabilityWaitForAttachToGroup(durability, vgroup);


                                    count = 0;

                                    while(  (c_count(vgroup->streams) == 0) &&
                                            (count < 30)){
                                        d_sleep(self, OS_DURATION_INIT(0, 100000000));  /* 100 ms */
                                        count++;
                                    }
                                    c_free(vgroup);

                                    result = d_storeMessagesInject(store, group);

                                    if(result == D_STORE_RESULT_OK) {
                                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                            "All data for group %s.%s has been injected from local store.\n",
                                            next->partition, next->topic);
                                    } else {
                                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                            "All data for group %s.%s could not be injected.\n",
                                            next->partition, next->topic);
                                    }
                                    if(!attached){
                                        d_groupSetPrivate(group, TRUE);
                                        d_groupSetKernelGroupCompleteness(group, TRUE);
                                    }
                                    d_groupSetComplete(group, admin);

                                    d_adminAddLocalGroup(admin, group);
                                    d_sampleChainListenerReportGroup(listener->sampleChainListener, group);
                                } else {
                                    d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                        "Group %s.%s could NOT be created locally (%d)\n",
                                        next->partition, next->topic, result);
                                }
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                            "Group %s.%s not in nameSpace.\n",
                                            next->partition, next->topic);
                            }
                        }
                        next = d_groupList(next->next);
                    }
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                      "Initializing persistent data took %f sec\n",
                                      os_durationToReal(os_timeMDiff(os_timeMGet(), t)));

                    /* Update the nameSpace state to trigger alignment */
                    mergeState = d_nameSpaceGetMergeState(nameSpace, NULL);
                    d_mergeStateSetValue(mergeState, mergeState->value+1);
                    d_nameSpaceSetMergeState(nameSpace, mergeState);

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                         "Persistent data injected for nameSpace %s, state  bumped to %d\n", nameSpace->name, mergeState->value);

                    d_mergeStateFree(mergeState);
                }
            } else {
                /* If not master, backup old persistent data (would be overwritten otherwise) */
                result = d_storeBackup(store, nameSpace);

                if(result != D_STORE_RESULT_OK) {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE,
                        "Namespace could NOT be backupped in local persistent store (%d)\n",
                        result);
                }

                /* Mark namespace incomplete */
                d_storeNsMarkComplete (store, nameSpace, FALSE);
            }
         }
    } else {
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                            "Could not read groups from persistent store. Persistent data not injected.\n");
    }

    d_storeGroupListFree(store, list);

    /* Free namespace list */
    d_adminNameSpaceCollectFree(admin, nameSpaces);
}

/*
 * PRECONDTION: listener->masterLock is locked
 */
static void
initMasters(
    d_groupLocalListener listener)
{
    d_thread self = d_threadLookupSelf ();
    d_admin admin;
    d_durability durability;
    c_bool fellowGroupsKnown, terminate;
    c_iter nameSpaces;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    /* Collect namespaces from admin */
    nameSpaces = d_adminNameSpaceCollect(admin);

    /* Initialize with namespaces from configuration */
    d_groupLocalListenerDetermineMasters(listener, nameSpaces, TRUE);

    fellowGroupsKnown = FALSE;
    terminate = d_durabilityMustTerminate(durability);

    /*now wait for completion of groups*/
    while((fellowGroupsKnown == FALSE) && (!terminate) && (d_adminGetInitialFellowCount(admin) > 0)){
        d_adminInitialFellowWalk(admin, checkFellowGroupsKnown, &fellowGroupsKnown);
        d_sleep(self, OS_DURATION_INIT(0, 100000000));  /* 100 ms */
        terminate = d_durabilityMustTerminate(durability);
    }

    if(!terminate){
        d_printTimedEvent(durability, D_LEVEL_FINE, "Fellow groups complete.\n");
    }

    /* Free namespace list */
    d_adminNameSpaceCollectFree(admin, nameSpaces);
}

void
d_groupLocalListenerDetermineNewMasters(
    d_groupLocalListener listener,
    c_iter nameSpaces)
{
    c_bool tryChains;
    os_result osr;
    d_durability durability;
    d_thread self = d_threadLookupSelf ();

    tryChains = FALSE;
    if(d_objectIsValid(d_object(listener), D_LISTENER)){
        durability = d_adminGetDurability(d_listener(listener)->admin);
        /* Because master determination can take quite some time
         * liveliness of this thread must regularly be asserted
         * to prevent that this thread is declared dead.
         * To ensure thread liveness we use a tryLock icw
         * d_sleep instead of an ordinary lock (the latter
         * does not asserts liveliness in case another thread
         * holds the lock for a very long time).
         */
        do {
            osr = os_mutexTryLock(&listener->masterLock);
            if (osr == os_resultFail) {
                OS_REPORT(OS_ERROR, "d_groupLocalListenerStart", 0,
                     "Failure to try to acquire the masterlock");
                d_durabilityTerminate(durability, FALSE);
                break;
            } else if (osr == os_resultBusy) {
                d_sleep(self, OS_DURATION_INIT(0,100000000));  /* 100 ms */
            }
        } while (osr != os_resultSuccess);

        if (osr == os_resultSuccess) {
            if(c_iterLength(nameSpaces) > 0){
                tryChains = d_groupLocalListenerDetermineMasters(listener, nameSpaces, FALSE);
            }
            os_mutexUnlock(&listener->masterLock);

            if(tryChains){
                d_sampleChainListenerTryFulfillChains(listener->sampleChainListener, NULL);
            }
        }
    }
}

static c_bool
collectGroupsToAlign(
    d_group group,
    c_voidp args)
{
    d_durability durability = d_threadsDurability();
    d_configuration config = d_durabilityGetConfiguration(durability);

    if (d_groupIsPrivate(group)) {
        return TRUE;
    }
    if (d_groupIsBuiltinGroup(group) && (!config->mustAlignBuiltinTopics)) {
        return TRUE;
    }
    (void)c_iterInsert((c_iter)args, d_objectKeep(d_object(group)));
    return TRUE;
}


/**
 * \brief Wait until the fellows have sent their groups.
 *
 * No response is required from a fellow that has prematurely left.
 */
static void
waitForGroups(
    c_iter fellowAddresses,
    d_durability durability)
{
    c_bool allFellowGroupsKnown;
    c_bool fellowGroupsKnown;
    c_bool terminate;
    d_networkAddress fellowAddr;
    d_fellow fellow;
    c_ulong i;
    d_thread self = d_threadLookupSelf ();

    if (c_iterLength(fellowAddresses) > 0) {
        /* There are pending groupsRequests.
         * Wait until all groups of the fellows are known.
         * We currently use a polling loop for this, but
         * maybe a condWait is a better approach.
         */
        allFellowGroupsKnown = FALSE;
        terminate = d_durabilityMustTerminate(durability);
        while ((!allFellowGroupsKnown) && (!terminate)) {
            allFellowGroupsKnown = TRUE;
            for(i=0; ((i < c_iterLength(fellowAddresses)) && allFellowGroupsKnown); i++) {
                fellowAddr = d_networkAddress(c_iterObject(fellowAddresses, i));
                fellow = d_adminGetFellow(durability->admin, fellowAddr);
                if (fellow) {
                    /* The fellow still exists, let's check if its groups are known */
                    fellowGroupsKnown = FALSE;
                    (void)checkFellowGroupsKnown(fellow, &fellowGroupsKnown);
                    d_fellowFree(fellow);
                } else {
                    /* The fellow is not present anymore, skip */
                    fellowGroupsKnown = TRUE;
                }
                allFellowGroupsKnown = allFellowGroupsKnown && fellowGroupsKnown;
            } /* for */
            d_sleep(self,OS_DURATION_INIT(0, 100000000));  /* 100 ms */
            terminate = d_durabilityMustTerminate(durability);
        }
        /* Clean up the list of pending groupsRequests */
        fellowAddr = d_networkAddress(c_iterTakeFirst(fellowAddresses));
        while (fellowAddr) {
            d_networkAddressFree(fellowAddr);
            fellowAddr = d_networkAddress(c_iterTakeFirst(fellowAddresses));
        }
    }
    (void)c_iterFree(fellowAddresses);
}


/**
 * \brief Request alignment data for the nameSpace from a set of fellows.
 *
 * When all alignment data is retrieved the new merge state is advertised.
 */
void
handleMergeAlignment(
    d_groupLocalListener listener,
    d_conflict conflict,
    d_nameSpace nameSpace,
    c_iter fellows,               /* fellows to request samples from */
    d_mergeState newState         /* new merge state AFTER alignment has succeeded */)
{
    d_mergeAction mergeAction;
    d_admin admin;
    d_durability durability;
    d_configuration config;
    c_iter groups;
    d_group group;
    d_partition partition;
    d_topic topic;
    d_durabilityKind dkind;
    c_bool inNameSpace, success;
    d_chain chain;
    os_timeW stamp, networkAttachTime;
    d_sampleRequest request;
    c_ulong i;
    c_ulong fellowCount = c_iterLength(fellows);
    c_long chainCount = 0;
    d_networkAddress source;
    d_fellow fellow;
    c_iter pendingGroupsRequest;
    struct groupInfo info;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);

    if (fellowCount == 0) {
        /* No fellows to request */
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                          "Not inserting merge request for nameSpace '%s' because there are no fellows to merge with.\n",
                          d_nameSpaceGetName(nameSpace));
        return;
    }

    groups = c_iterNew(NULL);
    source = d_networkAddressUnaddressed();

    /* We need to request data from the fellows. To ensure
     * that a sampleChain is generated for all groups known by
     * the fellow we must first be sure that we know all groups
     * from the fellow. Therefore, request all groups from the
     * fellows for which no previous groupsRequest has been sent.
     */
    pendingGroupsRequest = c_iterNew(NULL);
    for(i=0; i < fellowCount; i++) {
        fellow = d_fellow(c_iterObject(fellows, i));
        assert(d_fellowIsValid(fellow));
        /* Request all groups from the fellow */
        info.nameSpaces = c_iterNew(nameSpace);
        info.durability = durability;
        info.iAmAMaster = d_nameSpaceMasterIsMe(nameSpace, admin);
        info.publisher  = d_adminGetPublisher(admin);
        info.request    = d_groupsRequestNew(admin, NULL, NULL);
        (void)requestGroups(fellow, &info);
        d_groupsRequestFree(info.request);
        c_iterFree(info.nameSpaces);
        if (info.requested) {
            /* A groupsRequest has actually been sent to this fellow */
            c_iterAppend(pendingGroupsRequest, d_fellowGetAddress(fellow));
            d_printTimedEvent(durability, D_LEVEL_FINER,
                              "Waiting for groups from fellow %u\n", fellow->address->systemId);
        }
    }
    /* Wait for the pending groups requests */
    waitForGroups(pendingGroupsRequest, durability);

    /* Now we are sure that all groups of the fellows are known.
     * Create a merge action. The mergeAction is freed in
     * d_sampleChainListenerCheckChainComplete once
     * all samples in the chain have been received.
     */

    mergeAction = d_mergeActionNew(conflict, nameSpace, fellows, newState);
    /* For each group create a request for data for the
     * fellows. Also create a corresponding chain where all
     * responses (beads, links) from all fellows to the request
     * will be stored. Because responses from multiple fellows
     * are stored in a single chain we have to make sure
     * that they use the same source.
     */
    d_adminGroupWalk(admin, collectGroupsToAlign, groups);
    group = d_group(c_iterTakeFirst(groups));
    while (group) {
        partition = d_groupGetPartition(group);
        topic = d_groupGetTopic(group);
        dkind = d_groupGetKind(group);
        inNameSpace = d_adminInNameSpace(nameSpace, partition, topic, FALSE);
        if (inNameSpace) {
            stamp = os_timeWGet();
            if (config->timeAlignment) {
                networkAttachTime = stamp;
            } else {
                networkAttachTime = OS_TIMEW_INFINITE;
            }
            /* Now create sampleRequests and corresponding chains
             * to collect the beads for these sampleRequests.
             * We use the address (0,0,0) as the source of the
             * sampleRequests. This ensures that responses from
             * multiple fellows to the request are collected
             * in the same set, and data for the same instance
             * is only stored once (on a by-reception-time policy).
             */
            request = d_sampleRequestNew(
                                         admin, partition, topic,
                                         dkind, stamp, FALSE,
                                         OS_TIMEW_ZERO, networkAttachTime);
            d_sampleRequestSetSource(request,source);
            chain = d_chainNew(admin, request);
            if (d_mergeActionAddChain(mergeAction, chain)) {
                /* Chain is successfully added to the mergeAction */
                chainCount++;
            } else {
                /* The mergeAction already contains a chain
                 * for this partition/topic to the fellow, no
                 * need to added it again.
                 */
                d_chainFree(chain);
            }
        }
        os_free(partition);
        os_free(topic);
        d_groupFree(group);
        group = d_group(c_iterTakeFirst(groups));
    } /* while */

    d_traceMergeAction(mergeAction, "Created");

    /* Now a single mergeAction exists that contains chains
     * for all groups that match the namespace.
     */
    if (chainCount > 0) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
                          "Inserting merge requests to merge %d groups to %u fellows for nameSpace '%s'\n",
                          chainCount, fellowCount, d_nameSpaceGetName(nameSpace));
        /* Publish the sampleRequest(s) for this chain */
        success = d_sampleChainListenerInsertMergeAction(listener->sampleChainListener, mergeAction);
        if (success == FALSE) {
            if (d_tableSize(mergeAction->fellows) == 0) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  "All fellows targeted for the merge request have disappeared, so canceling the merge action.\n");
            } else {
                /* A merge action already exists for this */
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  "Merge requests to merge %d groups for nameSpace '%s' to %u fellows "\
                                  "are already in progress, so not issuing a new "\
                                  "merge action.\n",
                                  chainCount, d_nameSpaceGetName(nameSpace), fellowCount);
            }
            d_mergeActionFree(mergeAction);
        }
    } else {
        /* The mergeAction does not contain any chain, so no
         * data needs to be requested. That means we're done
         * with this mergeAction.
         */
        d_printTimedEvent(durability, D_LEVEL_FINER,
                          "No merge requests inserted to %u fellows for nameSpace '%s'\n",
                          fellowCount, d_nameSpaceGetName(nameSpace));
        d_mergeActionFree(mergeAction);
    }

    c_iterFree(groups);
    d_networkAddressFree(source);
}


struct applyMergePolicyHelper {
    d_groupLocalListener listener;
    d_fellow fellow;            /* When set this fellow is master for the namespace. */
    d_nameSpace nameSpace;      /* If set, applyMergePolicy will only check this namespace, otherwise all namespaces are evaluated */
    d_table fellowStates;       /* Fellow states at the time of the namespace event */
    c_iter conflictStates;      /* Result of mergeState compare between own namespace and fellow namespace */
    c_ulong event;              /* the event that triggered the merge action */
    d_mergeState oldMergeState; /* the merge state of the namespace before a new master was determined */
};

/*************** END NEW IMPL *************************/

/**
 * \brief Handle the appearance of a new local group
 */
void
d_groupLocalListenerHandleNewGroupsLocal(
    v_public entity,
    c_voidp args)
{
    d_listener           listener;
    d_groupLocalListener groupListener;
    v_service            kservice;
    c_iter               groups;
    d_admin              admin;
    v_group              group;
    v_group              group2;
    d_group              dgroup;
    d_durabilityKind     kind;
    d_durability         durability;
    c_bool               added, attached, groupAlreadyKnown;
    v_topicQos           qos;
    d_adminStatisticsInfo info;

    listener      = d_listener(args);
    groupListener = d_groupLocalListener(args);
    kservice      = v_service(entity);
    admin         = d_listenerGetAdmin(listener);
    durability    = d_adminGetDurability(admin);

    groups        = v_serviceTakeNewGroups(kservice);
    while ((group = v_group(c_iterTakeFirst(groups))) != NULL) {
        dgroup = NULL;
        qos = v_topicQosRef(group->topic);
        kind = d_durabilityKindFromKernel(qos->durability.v.kind);
        d_reportLocalGroup(durability, group);

        /* Check durability kind. Only transient, transient local
         * and persistent groups require actions
         */
        if (kind == D_DURABILITY_VOLATILE){
            d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Ignoring group %s.%s.\n",
                        v_partitionName(v_groupPartition(group)),
                        v_topicName(v_groupTopic(group)));
            /* update statistics */
            info = d_adminStatisticsInfoNew();
            info->kind = D_ADMIN_STATISTICS_GROUP;
            info->groupsKnownVolatileDif += 1;
            info->groupsIgnoredVolatileDif +=1;
            d_adminUpdateStatistics(admin, info);
            d_adminStatisticsInfoFree(info);
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Wait for services to attach.\n");
            attached = d_durabilityWaitForAttachToGroup(durability, group);
            d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Administrating group %s.%s.\n",
                    v_partitionName(v_groupPartition(group)),
                    v_topicName(v_groupTopic(group)));

            /* New groups may be notified multiple times by the kernel.
             * Therefore no alignment should be done if group is
             * already known in the durability administration.
             */
            groupAlreadyKnown = TRUE;
            dgroup = d_adminGetLocalGroup(
                         admin,
                         v_partitionName(v_groupPartition(group)),
                         v_topicName(v_groupTopic(group)),
                         kind);

            if(!dgroup){
                dgroup = d_groupNew(v_partitionName(v_groupPartition(group)),
                                    v_topicName(v_groupTopic(group)),
                                    kind, D_GROUP_INCOMPLETE, D_QUALITY_ZERO);
                d_groupSetKernelGroup(dgroup, group);

                if(!attached){
                   d_groupSetPrivate(dgroup, TRUE);
                   d_groupSetKernelGroupCompleteness(dgroup, TRUE);
                }
                added = d_adminAddLocalGroup(admin, dgroup);

                if(added == FALSE){
                    d_groupFree(dgroup);
                    dgroup = d_adminGetLocalGroup(
                                 admin,
                                 v_partitionName(v_groupPartition(group)),
                                 v_topicName(v_groupTopic(group)),
                                 kind);
                    if(!attached){
                        d_groupSetPrivate(dgroup, TRUE);
                        d_groupSetKernelGroupCompleteness(dgroup, TRUE);
                    }
                } else {
                    d_adminStoreGroup(admin, dgroup);
                    groupAlreadyKnown = FALSE;
                }
            } else if(!attached){
                d_groupSetPrivate(dgroup, TRUE);
                d_groupSetKernelGroupCompleteness(dgroup, TRUE);
            }

            if ((d_groupGetCompleteness(dgroup) != D_GROUP_COMPLETE) && (!groupAlreadyKnown)) {
                /* Make sure that a group is associated with a d_group */
                group2 = d_groupGetKernelGroup(dgroup);

                if(group2){
                    c_free(group2);
                } else {
                    d_groupSetKernelGroup(dgroup, group);
                }

                if(d_durabilityMustTerminate(durability) == FALSE){
                    d_groupLocalListenerHandleAlignment(groupListener, dgroup, NULL);
                }
            } else if(groupAlreadyKnown){
                d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Group %s.%s already known in admin.\n",
                        v_partitionName(v_groupPartition(group)),
                        v_topicName(v_groupTopic(group)));
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                    "Group %s.%s already complete.\n",
                    v_partitionName(v_groupPartition(group)),
                    v_topicName(v_groupTopic(group)));
            }
        }
        c_free(group);
    }
    c_iterFree(groups);
}

static c_bool
d_groupLocalAction(
    d_action action,
    c_bool terminate)
{
    d_listener listener;
    d_durability durability;
    d_admin admin;
    u_entity service;

    listener = d_listener(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(listener), D_LISTENER)){
        if(terminate == FALSE){
            admin = d_listenerGetAdmin(listener);
            durability = d_adminGetDurability(admin);
            service = u_entity(d_durabilityGetService(durability));

            (void)u_observableAction(u_observable(service), d_groupLocalListenerHandleNewGroupsLocal, listener);
        }
    }
    return FALSE;
}


static c_bool
d_groupCreatePersistentSnapshotAction(
    d_action action,
    c_bool terminate)
{
    d_durability durability;
    d_admin admin;
    struct createPersistentSnapshotHelper* cps;
    u_result result;

    cps = (struct createPersistentSnapshotHelper*)(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(cps->listener), D_LISTENER))
    {
        if(terminate == FALSE)
        {
            admin = d_listenerGetAdmin(cps->listener);
            durability = d_adminGetDurability(admin);

            result = d_durabilityTakePersistentSnapshot(
                durability,
                cps->partExpr,
                cps->topicExpr,
                cps->uri);
            if (result == U_RESULT_OK) {
                OS_REPORT(
                    OS_INFO,
                    "d_groupCreatePersistentSnapshotAction",
                    0,
                    "Creation of persistent snapshot number %d successfully completed."
                    "Snapshot was requested for partition expression '%s',"
                    " topic expression '%s' and was to be stored at location '%s'.",
                    d_groupLocalListener(cps->listener)->snapshotRequestNumber++,
                    cps->partExpr,
                    cps->topicExpr,
                    cps->uri);
            } else {
                OS_REPORT(
                    OS_ERROR,
                    "d_groupCreatePersistentSnapshotAction",
                    0,
                    "Creation of persistent snapshot failed with result "
                    "'%s'. Snapshot was requested for partition expression '%s',"
                    " topic expression '%s' and was to be stored at location '%s'.",
                    u_resultImage(result),
                    cps->partExpr,
                    cps->topicExpr,
                    cps->uri);
            }

        }
    }
    os_free(cps->partExpr);
    os_free(cps->topicExpr);
    os_free(cps->uri);
    os_free(cps);

    return FALSE;
}

struct deleteHistoricalDataHelper {
    os_timeW deleteTimeW;
    c_char* partExpr;
    c_char* topicExpr;
    d_listener listener;
};

static c_bool
d_groupDeleteHistoricalDataAction(
    d_action action,
    c_bool terminate)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_networkAddress unaddressed;
    d_deleteData delData;
    struct deleteHistoricalDataHelper* dhd;

    dhd = (struct deleteHistoricalDataHelper*)(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(dhd->listener), D_LISTENER)){
        if(terminate == FALSE){
            admin = d_listenerGetAdmin(dhd->listener);
            durability = d_adminGetDurability(admin);
            publisher = d_adminGetPublisher(admin);

            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Notifying fellows to delete historical data for partition/topic expression '%s.%s' up to time %"PA_PRItime"\n",
                dhd->partExpr, dhd->topicExpr, OS_TIMEW_PRINT(dhd->deleteTimeW));

            unaddressed = d_networkAddressUnaddressed();

            delData = d_deleteDataNew(admin, dhd->deleteTimeW, dhd->partExpr, dhd->topicExpr);
            d_publisherDeleteDataWrite(publisher, delData, unaddressed);
            d_networkAddressFree(unaddressed);
            d_deleteDataFree(delData);
        }
    }
    os_free(dhd->partExpr);
    os_free(dhd->topicExpr);
    os_free(dhd);

    return FALSE;
}

struct readerRequestHelper {
    d_readerRequest request;
    d_admin admin;
    d_groupLocalListener listener;
};

static c_bool
d_groupLocalReaderRequestAction(
    d_action action,
    c_bool terminate)
{
    c_bool callAgain, fulfilled;
    d_table groups;
    d_group group, localGroup;
    c_char *partition, *topic;
    d_durabilityKind kind;
    d_durability durability;
    v_handle handle;
    struct readerRequestHelper* helper;

    helper = (struct readerRequestHelper*)d_actionGetArgs(action);

    if(!terminate){
        callAgain  = FALSE;
        groups     = d_readerRequestGetGroups(helper->request);
        durability = d_adminGetDurability(helper->admin);
        handle     = d_readerRequestGetHandle(helper->request);

        group = d_tableTake(groups);

        while(group && !callAgain){
            partition  = d_groupGetPartition(group);
            topic      = d_groupGetTopic(group);
            kind       = d_groupGetKind(group);
            localGroup = d_adminGetLocalGroup(helper->admin, partition, topic, kind);

            if(!localGroup){
                callAgain = TRUE;
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Handling alignment of group %s.%s as part of "\
                    "historicalDataRequest from reader [%d, %d]\n",
                    partition, topic, handle.index, handle.serial);
                d_groupLocalListenerHandleAlignment(helper->listener,
                        localGroup, helper->request);
            }
            os_free(partition);
            os_free(topic);

            d_groupFree(group);

            if(!callAgain){
                group = d_tableTake(groups);
            } else {
                group = NULL;
            }
        }
        d_tableFree(groups);

        if (!callAgain) {
            d_printTimedEvent(durability, D_LEVEL_FINER,
				"Alignment for historicalDataRequest from "\
				"reader [%d, %d] in progress\n",
				handle.index, handle.serial);
            fulfilled = d_adminCheckReaderRequestFulfilled(
                                helper->admin, helper->request);

            if (fulfilled) {
                d_printTimedEvent(durability, D_LEVEL_FINER,
                        "historicalDataRequest from reader [%d, %d] fulfilled.\n",
                         handle.index, handle.serial);
            }
        }
    } else {
        callAgain = FALSE;
    }


    if(!callAgain){
        d_readerRequestFree(helper->request);
        os_free(helper);
    }

    return callAgain;
}

/* Lookup namespace */
typedef struct lookupNameSpace_t {
    v_group group;
    d_nameSpace namespace;
}lookupNameSpace_t;
static void lookupNamespace(d_nameSpace ns, void* data) {
    lookupNameSpace_t* userData;

    userData = (lookupNameSpace_t*)data;

    if(!userData->namespace) {
        if(d_nameSpaceIsIn(ns, v_entity(userData->group->partition)->name, v_entity(userData->group->topic)->name)) {
            userData->namespace = ns;
        }
    }
}

/* Set namespace quality to infinite. This behavior supports delayed alignment functionality. */
static void markGroupNamespaceWritten(d_admin admin, v_group group) {
    lookupNameSpace_t walkData;
    d_quality q;
    d_durability durability;

    durability = d_adminGetDurability(admin);

    /* Lookup namespace */
    walkData.group = group;
    walkData.namespace = 0;
    d_adminNameSpaceWalk(admin, lookupNamespace, &walkData);

    /* Check if namespace is found */
    if(!walkData.namespace) {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
                "Namespace for group '%s.%s' not found in administration (cannot update namespace quality).\n",
                v_entity(group->partition)->name, v_entity(group->topic)->name);
        return;
    }

    /* Set quality to infinite when delayed alignment is enabled. */
    if(d_nameSpaceGetDelayedAlignment(walkData.namespace)) {
        q = d_nameSpaceGetInitialQuality(walkData.namespace);
        if (!D_QUALITY_ISINFINITE(q)) {
            q = D_QUALITY_INFINITE;
            /* Set quality to infinite */
            d_nameSpaceSetInitialQuality(walkData.namespace, q);
            /* Report that quality of namespace is set to infinite */
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Quality of namespace '%s' is set to infinite.\n",
                    d_nameSpaceGetName(walkData.namespace));
        }
    }
}

static void
d_groupLocalListenerHandleReaderRequest(
    d_groupLocalListener listener,
    v_waitsetEvent event)
{
    d_admin admin;
    d_durability durability;
    d_readerRequest readerRequest;
    os_duration sleepTime = OS_DURATION_INIT(0, 500000000);  /* 500 ms */
    struct readerRequestHelper* requestHelper;
    d_action action;
    c_bool added;

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    readerRequest = d_readerRequestNew(admin, event);
    added = d_adminAddReaderRequest(admin, readerRequest);

    if (added) {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received historicalDataRequest from reader [%d, %d]\n",
                event->source.index, event->source.serial);

        requestHelper = (struct readerRequestHelper*)
                            os_malloc(sizeof(struct readerRequestHelper));
        requestHelper->admin = admin;
        requestHelper->listener = d_groupLocalListener(listener);
        requestHelper->request = readerRequest;

        action = d_actionNew(os_timeMGet(),
                sleepTime, d_groupLocalReaderRequestAction, requestHelper);
        d_actionQueueAdd(listener->actionQueue, action);
    }
    return;
}

/**
 * \brief Schedules local actions such as the appearance of a new local group,
 *        a request for historical data, a request to delete data, and a
 *        a request to take a snapshot.
 */
c_ulong
d_groupLocalListenerAction(
    u_object o,
    v_waitsetEvent event,
    c_voidp userData)
{
    d_listener listener;
    d_admin admin;
    d_durability durability;
    d_actionQueue queue;
    d_action action;
    os_duration sleepTime = OS_DURATION_INIT(1, 0);
    os_duration duration;
    os_timeW deleteTimeW;
    struct deleteHistoricalDataHelper* data;
    struct createPersistentSnapshotHelper* snapshotHelper;

    if (o && userData) {
        listener   = d_listener(userData);
        assert(d_groupLocalListenerIsValid(listener));
        admin      = d_listenerGetAdmin(listener);
        durability = d_adminGetDurability(admin);
        queue      = d_groupLocalListener(listener)->actionQueue;

        if((event->kind & V_EVENT_NEW_GROUP) == V_EVENT_NEW_GROUP){
            action = d_actionNew(os_timeMGet(), sleepTime, d_groupLocalAction, listener);
            d_actionQueueAdd(queue, action);
        }
        if((event->kind & V_EVENT_HISTORY_REQUEST) == V_EVENT_HISTORY_REQUEST){
            d_groupLocalListenerHandleReaderRequest(
                d_groupLocalListener(listener), event);
        }

        if((event->kind & V_EVENT_HISTORY_DELETE) == V_EVENT_HISTORY_DELETE){
            data  = (struct deleteHistoricalDataHelper*)(os_malloc(
                                    sizeof(struct deleteHistoricalDataHelper)));
            /* To notify fellows a d_deleteData message will generated. This
             * message requires the wallclock timestamp that is used as discriminator
             * which data to purge. The event, however, has this data available as
             * an elapsed time. Therefore, we need to calculate the wall clock time
             * from the elapsed time.
             * Note that this calculation requires time alignment between nodes.
             */
            duration = os_timeEDiff(os_timeEGet(), v_waitsetEventHistoryDeleteTime(event));
            deleteTimeW = os_timeWAdd(os_timeWGet(), duration);

            data->deleteTimeW = deleteTimeW;
            data->partExpr    = os_strdup(v_waitsetEventHistoryDeletePartitionExpr(event));
            data->topicExpr   = os_strdup(v_waitsetEventHistoryDeleteTopicExpr(event));
            data->listener    = listener;

            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received local deleteHistoricalData event for partition topic %s.%s upto time %"PA_PRItime". Going to notify fellows ...\n",
                data->partExpr, data->topicExpr, OS_TIMEW_PRINT(deleteTimeW));

            action = d_actionNew(os_timeMGet(), sleepTime, d_groupDeleteHistoricalDataAction, data);
            d_actionQueueAdd(queue, action);
        }
        if((event->kind & V_EVENT_PERSISTENT_SNAPSHOT) == V_EVENT_PERSISTENT_SNAPSHOT)
        {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received a request for a persistent snapshot for partition "
                "expression '%s' and topic expression '%s' to be stored at"
                "destination '%s'.\n",
                v_waitsetEventPersistentSnapshotPartitionExpr(event),
                v_waitsetEventPersistentSnapshotTopicExpr(event),
                v_waitsetEventPersistentSnapshotURI(event));

            snapshotHelper  = (struct createPersistentSnapshotHelper*)(os_malloc(
                                    sizeof(struct createPersistentSnapshotHelper)));
            snapshotHelper->partExpr = os_strdup(v_waitsetEventPersistentSnapshotPartitionExpr(event));
            snapshotHelper->topicExpr = os_strdup(v_waitsetEventPersistentSnapshotTopicExpr(event));
            snapshotHelper->uri = os_strdup(v_waitsetEventPersistentSnapshotURI(event));
            snapshotHelper->listener = listener;

            action = d_actionNew(os_timeMGet(), sleepTime, d_groupCreatePersistentSnapshotAction, snapshotHelper);
            d_actionQueueAdd(queue, action);
        }
        if((event->kind & V_EVENT_CONNECT_WRITER) == V_EVENT_CONNECT_WRITER) {
            admin = d_listenerGetAdmin(listener);

            /* Set namespace quality to infinite */
            markGroupNamespaceWritten(admin, v_waitsetEventConnectWriterGroup(event));
        }
    }

    return event->kind;
}

#define _MASK_ (V_EVENT_CONNECT_WRITER | V_EVENT_NEW_GROUP | \
                V_EVENT_HISTORY_DELETE | V_EVENT_HISTORY_REQUEST | \
                V_EVENT_PERSISTENT_SNAPSHOT | V_EVENT_TRIGGER)

c_bool
d_groupLocalListenerStart(
    d_groupLocalListener listener)
{
    c_bool result;
    u_object object;
    u_result ur;
    u_eventMask mask;
    d_durability durability;

    c_bool wsResult;
    d_waitset waitset;
    d_admin admin;
    d_subscriber subscriber;
    d_waitsetAction action;
    d_store store;
    os_threadAttr attr;
    os_result osr;
    d_thread self = d_threadLookupSelf ();

    result = FALSE;

    assert(d_groupLocalListenerIsValid(listener));

    /* Setup listener for CONNECT_WRITER, NEW_GROUP, HISTORY_DELETE, HISTORY_REQUEST and PERSISTENT_SNAPSHOT events */
    if(listener){
        d_listenerLock(d_listener(listener));
        durability  = d_adminGetDurability(d_listenerGetAdmin(d_listener(listener)));
        object      = u_object( d_durabilityGetService(durability));
        action      = d_groupLocalListenerAction; /* callback function */

        if(d_listener(listener)->attached == FALSE){
            ur = u_observableGetListenerMask(u_observable(object), &mask);

            if(ur == U_RESULT_OK){
                mask = mask | _MASK_;
                ur = u_observableSetListenerMask(u_observable(object), mask);

                if(ur == U_RESULT_OK){
                    admin      = d_listenerGetAdmin(d_listener(listener));
                    subscriber = d_adminGetSubscriber(admin);
                    store      = d_subscriberGetPersistentStore(subscriber);
                    waitset    = d_subscriberGetWaitset(subscriber);

                    /* Create and attach waitset for listener */
                    os_threadAttrInit(&attr);
                    listener->waitsetData = d_waitsetEntityNew(
                                    "groupLocalListener",
                                    object, action,
                                    _MASK_,
                                    attr, listener);
                    wsResult = d_waitsetAttach(waitset, listener->waitsetData);

                    if(wsResult == TRUE) {
                        ur = U_RESULT_OK;
                    } else {
                        ur = U_RESULT_ILL_PARAM;
                    }

                    if (listener->initialGroupsAdministrated == FALSE) {
                        d_durabilitySetState(durability, D_STATE_DISCOVER_PERSISTENT_SOURCE);
                        /* Because master determination can take quite some time
                         * liveliness of this thread must regularly be asserted
                         * to prevent that this thread is declared dead.
                         * To ensure thread liveness we use a tryLock icw
                         * d_sleep instead of an ordinary lock (the latter
                         * does not asserts liveliness in case another thread
                         * holds the lock for a very long time).
                         */
                        do {
                            osr = os_mutexTryLock(&listener->masterLock);
                            if (osr == os_resultFail) {
                                OS_REPORT(OS_ERROR, "d_groupLocalListenerStart", 0,
                                     "Failure to try to acquire the masterlock");
                                d_durabilityTerminate(durability, FALSE);
                                break;
                            } else if (osr == os_resultBusy) {
                                d_sleep(self, OS_DURATION_INIT(0, 100000000));  /* 100 ms */
                            }
                        } while (osr != os_resultSuccess);
                        d_adminAddListener(admin, listener->fellowListener);
                        d_adminAddListener(admin, listener->nameSpaceListener);

                        initMasters(listener);

                        if(store != NULL){
                            initPersistentData(listener);
                            d_printTimedEvent(durability, D_LEVEL_FINER, "Persistency has been enabled...\n");
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINER, "Persistency has not been enabled...\n");
                        }
                        os_mutexUnlock(&listener->masterLock);
                        d_durabilitySetState(durability, D_STATE_DISCOVER_LOCAL_GROUPS);
                        d_printTimedEvent(durability, D_LEVEL_FINER, "Initializing local groups...\n");

                        if(d_durabilityMustTerminate(durability) == FALSE){
                            (void)u_serviceFillNewGroups(u_service(object));
                            (void)u_observableAction(u_observable(object), d_groupLocalListenerHandleNewGroupsLocal, listener);
                        }
                        d_durabilitySetState(durability, D_STATE_FETCH_INITIAL);
                        listener->initialGroupsAdministrated = TRUE;
                        d_printTimedEvent(durability, D_LEVEL_FINEST, "Local groups initialized.\n");
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
            }
        } else {
            d_listenerUnlock(d_listener(listener));
            result = TRUE;
        }
    }
    return result;
}

c_bool
d_groupLocalListenerStop(
    d_groupLocalListener listener)
{
    c_bool result;
    u_result ur;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_groupLocalListenerIsValid(listener));

    result = FALSE;

    if(listener){
        d_listenerLock(d_listener(listener));

        if(d_listener(listener)->attached == TRUE){
            admin = d_listenerGetAdmin(d_listener(listener));
            subscriber = d_adminGetSubscriber(admin);

            /* Remove the listeners */
            d_adminRemoveListener(admin, listener->nameSpaceListener);
            d_adminRemoveListener(admin, listener->fellowListener);

            waitset = d_subscriberGetWaitset(subscriber);
            result = d_waitsetDetach(waitset, listener->waitsetData);

            if(result == TRUE) {
                d_waitsetEntityFree(listener->waitsetData);
                ur = U_RESULT_OK;
            } else {
                ur = U_RESULT_ILL_PARAM;
            }
            if(ur == U_RESULT_OK) {
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
d_groupLocalListenerNewGroupLocalAction(
    u_object o,
    c_ulong event,
    c_voidp userData)
{
    d_admin admin;
    d_durability durability;
    d_listener listener;

    if (o && (event & V_EVENT_NEW_GROUP)) {
        if (userData) {
            listener   = d_listener(userData);
            assert(d_groupLocalListenerIsValid(listener));
            admin = d_listenerGetAdmin(listener);
            durability = d_adminGetDurability(admin);

            (void)u_observableAction(u_observable(d_durabilityGetService(durability)), d_groupLocalListenerHandleNewGroupsLocal, userData);

        }
    }
    return V_EVENT_NEW_GROUP;
}

/**
 * \brief Called when a new local group is created.
 */
void
d_groupLocalListenerHandleAlignment(
    d_groupLocalListener listener,
    d_group dgroup,
    d_readerRequest readerRequest)
{
    os_timeW stamp, networkAttachTime;
    d_sampleRequest     request;
    d_admin             admin;
    d_durability        durability;
    d_subscriber        subscriber;
    d_store             store;
    d_completeness      completeness;
    d_configuration     config;
    d_durabilityKind    dkind;
    c_bool              timeRangeActive, requestRemote, inject;
    d_group             localGroup;
    c_char              *partition, *topic;
    d_nameSpace         nameSpace;
    d_alignmentKind     akind;
    d_chain             chain;
    d_adminStatisticsInfo info;
    d_nameSpace          groupNameSpace;

    assert(d_groupLocalListenerIsValid(listener));

    admin      = d_listenerGetAdmin(d_listener(listener));
    subscriber = d_adminGetSubscriber(admin);
    store      = d_subscriberGetPersistentStore(subscriber);
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);
    partition  = d_groupGetPartition(dgroup);
    topic      = d_groupGetTopic(dgroup);
    localGroup = d_adminGetLocalGroup(admin, partition, topic, d_groupGetKind(dgroup));

    if(localGroup){
        completeness = d_groupGetCompleteness(localGroup);
        dkind        = d_groupGetKind(localGroup);
        nameSpace    = d_adminGetNameSpaceForGroup(admin, partition, topic);

        if(readerRequest){
            requestRemote = FALSE;

            if(nameSpace){
                d_storeResult result;
                akind = d_nameSpaceGetAlignmentKind(nameSpace);

                if((akind == D_ALIGNEE_ON_REQUEST) && (completeness != D_GROUP_COMPLETE)){
                    if(dkind == D_DURABILITY_PERSISTENT){
                        inject = d_nameSpaceMasterIsMe(nameSpace, admin);

                        if(inject || d_groupIsPrivate(localGroup)){
                            result = d_storeMessagesInject(store, localGroup);
                            if(result == D_STORE_RESULT_OK){
                                if(d_groupIsPrivate(localGroup)) {
                                    d_groupSetComplete(localGroup, admin);
                                    d_groupSetKernelGroupCompleteness(localGroup, TRUE);
                                }
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                                  "Data for group %s.%s injected from disk. Group is complete now (0).\n",
                                                  partition, topic);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                                  "Injecting data from disk for group %s.%s failed (0).\n",
                                                  partition, topic);
                            }
                            d_readerRequestRemoveGroup(readerRequest, localGroup);
                        } else {
                            requestRemote = TRUE;
                        }
                    } else if(!d_groupIsPrivate(localGroup)){
                        requestRemote = TRUE;
                    } else {
                        /* All data available, so we're done for this group*/
                    }

                } else if(completeness == D_GROUP_COMPLETE){
                    d_readerRequestRemoveGroup(readerRequest, localGroup);
                } else {
                    /* Simply wait for group completeness later on */
                }
            } else {
                /* Not in nameSpace so ignore request for this group */
                d_readerRequestSetGroupIgnored(readerRequest);

                d_readerRequestRemoveGroup(readerRequest, localGroup);

                d_printTimedEvent(durability, D_LEVEL_WARNING,
                    "Received a historicalDataRequest from a reader for group %s.%s, "\
                    "but that is not in the nameSpace and therefore ignored.\n",
                    partition, topic);

            }

            if(requestRemote == TRUE){
                stamp = os_timeWGet();

                if(config->timeAlignment){
                    networkAttachTime = stamp;
                } else {
                    networkAttachTime = OS_TIMEW_INFINITE;
                }
                timeRangeActive      = FALSE;

                request = d_sampleRequestNew(
                                admin, partition, topic,
                                dkind, stamp, timeRangeActive,
                                OS_TIMEW_ZERO, networkAttachTime);

                d_sampleRequestSetCondition(request, readerRequest);
                chain = d_chainNew(admin, request);
                d_readerRequestAddChain(readerRequest, chain);
                d_readerRequestRemoveGroup(readerRequest, localGroup);

                if (akind == D_ALIGNEE_ON_REQUEST) {
                    /* set ignoreTransactionFlush on kernel group to ensure that the
                     * possible transaction messages are forwarded to the group
                     * but when the transaction becomes complete it is not flushed to
                     * the group.
                     */
                    v_groupSetOnRequest(dgroup->vgroup, TRUE);
                }

                d_sampleChainListenerInsertRequest(
                            d_groupLocalListener(listener)->sampleChainListener,
                            chain, TRUE);

            }

        /* No request has been received */
        } else if(d_adminGroupInActiveAligneeNS(admin, partition, topic) == TRUE){
            if(completeness != D_GROUP_COMPLETE){
                if(dkind == D_DURABILITY_PERSISTENT){
                    assert(nameSpace);
                    akind     = d_nameSpaceGetAlignmentKind(nameSpace);
                    inject    = d_nameSpaceMasterIsMe(nameSpace, admin);

                    if(inject == TRUE){
                        if(akind == D_ALIGNEE_LAZY){
                            d_storeResult result = d_storeMessagesInject(store, localGroup);

                            if(result == D_STORE_RESULT_OK){
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Data for group %s.%s injected from disk. Group is complete now (1).\n",
                                    partition, topic);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Injecting data from disk for group %s.%s failed (1).\n",
                                partition, topic);
                            }
                        } else if(akind == D_ALIGNEE_ON_REQUEST){
                            d_storeResult result = d_storeMessagesInject(store, localGroup);

                            if(result == D_STORE_RESULT_OK){
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                    "Data for group %s.%s injected from disk. Group is complete now (2).\n",
                                    partition, topic);
                            } else {
                                d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Injecting data from disk for group %s.%s failed (1).\n",
                                partition, topic);
                            }
                        } else if(akind == D_ALIGNEE_INITIAL){
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                "Persistent group %s.%s complete, because I am persistent source and already injected data.\n",
                                partition, topic);
                        } else {
                            assert(FALSE);
                        }
                        if(d_groupIsPrivate(localGroup)){
                            d_groupSetComplete(localGroup, admin);
                            d_groupSetKernelGroupCompleteness(localGroup, TRUE);
                            requestRemote = FALSE;
                        } else {
                            requestRemote = TRUE;
                        }
                    } else if(d_groupIsPrivate(localGroup)){
                        /* I am not the master, but this is a persistent group
                         * that exists on this node only, so inject data for
                         * group.
                         */
                        d_storeResult result = d_storeMessagesInject(store, localGroup);

                        if(result == D_STORE_RESULT_OK){
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                "Data for local group %s.%s injected from disk. Group is complete now (3).\n",
                                partition, topic);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Injecting data from disk for local group %s.%s failed (3).\n",
                                partition, topic);
                        }
                        d_groupSetComplete(localGroup, admin);
                        d_groupSetKernelGroupCompleteness(localGroup, TRUE);
                        requestRemote = FALSE;
                    } else {
                        /*Only request if complete.*/
                        requestRemote = TRUE;
                    }
                } else if(d_groupIsPrivate(localGroup)){
                    /* This group is private and transient so it is complete */
                    d_groupSetComplete(localGroup, admin);
                    d_groupSetKernelGroupCompleteness(localGroup, TRUE);
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Transient group %s.%s complete, because it is local.\n",
                        partition, topic);
                    requestRemote = FALSE;
                } else {
                    requestRemote = TRUE;
                }

                /* Request samples for this group */
                if(requestRemote == TRUE){
                    stamp = os_timeWGet();

                    if(config->timeAlignment){
                        networkAttachTime = stamp;
                    } else {
                        networkAttachTime = OS_TIMEW_INFINITE;
                    }
                    timeRangeActive      = FALSE;

                    if((dkind == D_DURABILITY_PERSISTENT) && store){

                    }

                    request = d_sampleRequestNew(admin, partition, topic,
                            dkind, stamp, timeRangeActive, OS_TIMEW_ZERO, networkAttachTime);

                    chain   = d_chainNew(admin, request);
                    d_sampleChainListenerInsertRequest(
                                d_groupLocalListener(listener)->sampleChainListener,
                                chain, TRUE);
                }
            }
        } else if(d_adminGroupInAligneeNS(admin, partition, topic) == TRUE){
            d_sampleChainListenerReportGroup(listener->sampleChainListener, localGroup);
            /* For those topics in a on_request namespace, they will be marked unaligned */
            groupNameSpace = d_adminGetNameSpaceForGroup(admin,
                                                         d_groupGetPartition(localGroup),
                                                         d_groupGetTopic(localGroup));

            if ((d_nameSpaceGetAlignmentKind(groupNameSpace) == D_ALIGNEE_ON_REQUEST)
                && (d_groupIsBuiltinGroup(localGroup) != TRUE)) {

                d_groupSetUnaligned(localGroup, admin);

                d_printTimedEvent(durability, D_LEVEL_FINE,
                "Group %s.%s is transient group in the namespace with " \
                "on_request policy will be marked unaligned.\n",
                partition, topic);
            }
        } else {
            d_configuration c = d_durabilityGetConfiguration(durability);

            d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Group %s.%s not in alignee namespace, so no alignment action taken.\n",
                    partition, topic);

            /* Always mark d_group as no-interest */
            d_groupSetNoInterest(localGroup, admin);

            /* If DDSI is running, it is responsible for both TRANSIENT_LOCAL as
             * well as built-in groups. In that case durability should not mark
             * v_group-alignstate. In all other cases, it should
             */
            if (d_isBuiltinGroup(partition, topic) || (d_groupGetKind(localGroup) == D_DURABILITY_TRANSIENT_LOCAL)) {
                /* The heartbeat-group should never be considered TRANSIENT, so
                 * it is marked NO_INTEREST in all cases.
                 */
                if (c->mustAlignBuiltinTopics || d_isHeartbeatGroup(partition, topic)) {
                    d_groupSetKernelGroupNoInterest(localGroup);
                }
            } else {
                d_groupSetKernelGroupNoInterest(localGroup);
            }

            /* update statistics */
            info = d_adminStatisticsInfoNew();
            info->kind = D_ADMIN_STATISTICS_GROUP;

            switch(dkind){
            case D_DURABILITY_VOLATILE:
                info->groupsIncompleteVolatileDif -= 1;
                info->groupsIgnoredVolatileDif +=1;
                break;
            case D_DURABILITY_TRANSIENT_LOCAL:
            case D_DURABILITY_TRANSIENT:
                info->groupsIncompleteTransientDif -= 1;
                info->groupsIgnoredTransientDif +=1;
                break;
            case D_DURABILITY_PERSISTENT:
                info->groupsIncompletePersistentDif -= 1;
                info->groupsIgnoredPersistentDif +=1;
                break;
            default:
                break;
            }

            d_adminUpdateStatistics(admin, info);
            d_adminStatisticsInfoFree(info);
        }
    }
    os_free(partition);
    os_free(topic);
}
