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
#include "d__groupRemoteListener.h"
#include "d__sampleChainListener.h"
#include "d__readerListener.h"
#include "d__listener.h"
#include "d__admin.h"
#include "d__fellow.h"
#include "d__actionQueue.h"
#include "d__subscriber.h"
#include "d__configuration.h"
#include "d__publisher.h"
#include "d__nameSpace.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d__groupCreationQueue.h"
#include "d__group.h"
#include "d__eventListener.h"
#include "d_newGroup.h"
#include "d_message.h"
#include "d_nameSpacesRequest.h"
#include "d_networkAddress.h"
#include "u_group.h"
#include "u_participant.h"
#include "u_entity.h"
#include "os_heap.h"
#include "os_thread.h"

/**
 * Macro that checks the d_groupRemoteListener validity.
 * Because d_groupRemoteListener is a concrete class typechecking is required.
 */
#define             d_groupRemoteListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_GROUP_REMOTE_LISTENER)

/**
 * \brief The d_groupRemoteListener cast macro.
 *
 * This macro casts an object to a d_groupRemoteListener object.
 */
#define d_groupRemoteListener(_this) ((d_groupRemoteListener)(_this))


C_STRUCT(d_groupRemoteListener){
    C_EXTENDS(d_readerListener);
    d_groupCreationQueue groupCreationQueue;
};

/**
 * \brief Handle the reception of a d_group message
 */
static void
d_groupRemoteListenerAction(
    d_listener listener,
    d_message message)
{
    d_newGroup remote;
    d_durability durability;
    d_admin admin;
    d_group group, group2;
    d_fellow fellow;
    c_bool createLocally, added;
    u_group ugroup;
    d_networkAddress addr;
    c_bool result;
    d_subscriber subscriber;
    d_sampleChainListener sampleChainListener;
    d_completeness localCompleteness;
    d_quality quality;

    assert(d_groupRemoteListenerIsValid(listener));

    remote     = d_newGroup(message);
    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    addr       = d_networkAddressNew(message->senderAddress.systemId,
                                     message->senderAddress.localId,
                                     message->senderAddress.lifecycleId);

    fellow     = d_adminGetFellow(admin, addr);

    d_qualityExtToQuality(&quality, &remote->quality, IS_Y2038READY(remote));
    if (remote->partition && remote->topic) {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                          "Received remote group '%s.%s' with completeness=%d and quality=%"PA_PRItime" from fellow: %u.\n",
                          remote->partition, remote->topic,
                          remote->completeness,
                          OS_TIMEW_PRINT(quality),
                          message->senderAddress.systemId);
    }
    if (fellow) {
        if (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED) {
            /* When a  remote group message is received with both
             * partition and topic NULL, this indicates that the
             * fellow does not have any groups. The expected number
             * of groups is transferred always, even the partition
             * and topic are NULL.
             */
            d_fellowSetExpectedGroupCount(fellow, (c_long) remote->alignerCount);
            if ((remote->partition != NULL) && (remote->topic != NULL)) {
                group = d_adminGetLocalGroup(admin, remote->partition,
                                             remote->topic, remote->durabilityKind);
                if (!group) {
                    /* The remote group is not yet locally known, so try to create it.*/
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                            "Received remote group %s.%s which is locally unknown.\n",
                                            remote->partition, remote->topic);
                    group = d_groupNew(remote->partition, remote->topic,
                                       remote->durabilityKind,
                                       remote->completeness, quality);
                    added = d_fellowAddGroup(fellow, group);
                    d_fellowSetExpectedGroupCount(fellow, (c_long) remote->alignerCount);
                    if (added == FALSE) {
                        /* Remote group already present in the fellow administration */
                        d_groupFree(group);
                        group = d_fellowGetGroup(fellow, remote->partition,
                                                 remote->topic,
                                                 remote->durabilityKind);
                        if (group) {
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                "Remote group %s.%s already known for fellow %u, updating quality=%"PA_PRItime" and completeness=%d.\n",
                                remote->partition, remote->topic, message->senderAddress.systemId,
                                OS_TIMEW_PRINT(quality), remote->completeness);
                            /* Update the group */
                            d_groupUpdate(group, remote->completeness, quality, admin);
                            d_groupFree(group);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Remote group %s.%s not added for fellow %u, but also not found.\n",
                                remote->partition, remote->topic,
                                message->senderAddress.systemId);
                        }
                    } else {
                        /* Remote group was not known yet in the fellow administration,
                         * register the group with the fellow.
                         */
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Remote group %s.%s registered for fellow %u with quality=%"PA_PRItime" and completeness=%d.\n",
                            remote->partition, remote->topic, message->senderAddress.systemId,
                            OS_TIMEW_PRINT(quality), remote->completeness);
                    }

                    /* Even though the group didn't exist when entering this function,
                     * this might no longer be the case.
                     * A complete group might be interesting in case there are
                     * still unfullfilled chain requests.
                     */
                    if (remote->completeness == D_GROUP_COMPLETE) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                                  "Remote group '%s.%s' complete, check for unfulfilled chains.\n",
                                                  remote->partition, remote->topic);
                        subscriber = d_adminGetSubscriber(admin);
                        sampleChainListener = d_subscriberGetSampleChainListener(subscriber);
                        group = d_groupNew(remote->partition, remote->topic,
                                           remote->durabilityKind,
                                           remote->completeness, quality);
                        d_sampleChainListenerTryFulfillChains(sampleChainListener, group);
                        d_groupFree(group);
                    }

                    /* Group unknown locally, check if it should be aligned
                     * initially. In case DDSI is responsible for builtin
                     * topics then durability must NOT create the local
                     * group.
                     */
                    createLocally = d_adminGroupInInitialAligneeNS(
                                                admin, remote->partition,
                                                remote->topic);

                    if (createLocally == TRUE) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                            "Remote group %s.%s in initial alignee namespace, so creating locally...\n",
                                            remote->partition, remote->topic);

                        ugroup = u_groupNew(
                                u_participant(d_durabilityGetService(durability)),
                                remote->partition, remote->topic, 10*OS_DURATION_MILLISECOND);

                        if(ugroup){
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                            "Remote group %s.%s with quality %"PA_PRItime" created locally.\n",
                                            remote->partition, remote->topic, OS_TIMEW_PRINT(quality));
                            u_objectFree(u_object(ugroup));
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Remote group %s.%s with quality %"PA_PRItime" could NOT be created locally.\n",
                                remote->partition, remote->topic, OS_TIMEW_PRINT(quality));
                            /* TODO: quality must not be taken over from remote. */
                            group2 = d_groupNew(remote->partition, remote->topic,
                                           remote->durabilityKind, D_GROUP_INCOMPLETE, quality);
                            result = d_groupCreationQueueAdd(
                                d_groupRemoteListener(listener)->groupCreationQueue,
                                group2);

                            if(result == FALSE){
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                    "Remote group %s.%s already in creation queue. Skipping this one.\n",
                                    remote->partition, remote->topic);
                                d_groupFree(group2);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Remote group %s.%s NOT created locally.\n",
                            remote->partition, remote->topic);
                    }
                } else {
                    /* The group is already locally known */
                    localCompleteness = d_groupGetCompleteness(group);
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                          "Remote group '%s.%s' already known locally with completeness=%d.\n",
                          remote->partition, remote->topic, localCompleteness);
                    group = d_fellowGetGroup(fellow, remote->partition,
                         remote->topic, remote->durabilityKind);
                    if (group) {
                        /* The fellow already knows the group, update the group */
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                             "Updating remote group '%s.%s' for fellow %u with completeness=%d and quality=%"PA_PRItime".\n",
                             remote->partition, remote->topic, message->senderAddress.systemId,
                             remote->completeness, OS_TIMEW_PRINT(quality));
                        d_groupUpdate(group, remote->completeness, quality, admin);
                        d_groupFree(group);
                    } else {
                        /* The fellow does not know the group, add it to the fellow */
                        group = d_groupNew(remote->partition, remote->topic, remote->durabilityKind, remote->completeness, quality);
                        added = d_fellowAddGroup(fellow, group);
                        if (added == FALSE) {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  "Remote group '%s.%s' with completeness=%d and quality=%"PA_PRItime" could not be registered for fellow %u.\n",
                                  remote->partition, remote->topic, remote->completeness,
                                  OS_TIMEW_PRINT(quality), message->senderAddress.systemId);
                            d_groupFree(group);
                            group = d_fellowGetGroup(fellow, remote->partition, remote->topic, remote->durabilityKind);
                            if (group) {
                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                      "Updating remote group '%s.%s' for fellow %u with completeness=%d and quality=%"PA_PRItime".\n",
                                      remote->partition, remote->topic, message->senderAddress.systemId,
                                      remote->completeness, OS_TIMEW_PRINT(quality));
                                d_groupUpdate(group, remote->completeness, quality, admin);
                                d_groupFree(group);
                            }
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                "Remote group '%s.%s' with completeness=%d and quality=%"PA_PRItime" registered for fellow %u.\n",
                                remote->partition, remote->topic, remote->completeness,
                                OS_TIMEW_PRINT(quality), message->senderAddress.systemId);
                        }
                    }
                    /* A complete group might be interesting in case there are
                     * still unfullfilled chain requests.
                     */
                    if (remote->completeness == D_GROUP_COMPLETE) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Remote group '%s.%s' complete, check for unfulfilled chains.\n",
                            remote->partition, remote->topic);
                        subscriber = d_adminGetSubscriber(admin);
                        sampleChainListener = d_subscriberGetSampleChainListener(subscriber);
                        group = d_groupNew(remote->partition, remote->topic, remote->durabilityKind,
                                           remote->completeness, quality);
                        d_sampleChainListenerTryFulfillChains(sampleChainListener, group);
                        d_groupFree(group);
                    }
                }
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                "Fellow %u not approved, so ignoring remote group message.\n",
                message->senderAddress.systemId);
        }
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
            "Fellow %u unknown so far, so ignoring remote group message.\n",
            message->senderAddress.systemId);
    }
    d_networkAddressFree(addr);

    return;
}
static void
d_groupRemoteListenerDeinit(
    d_groupRemoteListener listener)
{
    assert(d_groupRemoteListenerIsValid(listener));

    /* Stop the groupRemoteListener before cleaning up. */
    d_groupRemoteListenerStop(listener);
    /* Destroy the groupCreationQueue */
    if (listener->groupCreationQueue) {
        d_groupCreationQueueFree(listener->groupCreationQueue);
    }
    /* Call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_groupRemoteListenerInit(
    d_groupRemoteListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;
    d_admin admin;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed
     */

    assert(d_subscriberIsValid(subscriber));

    /* Call super-init */
    os_threadAttrInit(&attr);
    d_readerListenerInit(   d_readerListener(listener),
                            D_GROUP_REMOTE_LISTENER,
                            d_groupRemoteListenerAction,
                            subscriber,
                            D_NEWGROUP_TOPIC_NAME,
                            D_NEWGROUP_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            (d_objectDeinitFunc)d_groupRemoteListenerDeinit);
    /* Create the groupCreationQueue */
    admin = d_listenerGetAdmin(d_listener(listener));
    assert(d_adminIsValid(admin));
    listener->groupCreationQueue = d_groupCreationQueueNew(admin);
}

d_groupRemoteListener
d_groupRemoteListenerNew(
    d_subscriber subscriber)
{
    d_groupRemoteListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate groupRemoteListener object */
    listener = d_groupRemoteListener(os_malloc(C_SIZEOF(d_groupRemoteListener)));
    if (listener) {
        /* Initialize the groupsRequestListener */
        d_groupRemoteListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_groupRemoteListenerFree(
    d_groupRemoteListener listener)
{
    assert(d_groupRemoteListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

c_bool
d_groupRemoteListenerStart(
    d_groupRemoteListener listener)
{
    assert(d_groupRemoteListenerIsValid(listener));

    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_groupRemoteListenerStop(
    d_groupRemoteListener listener)
{
    c_bool result;

    result = d_readerListenerStop(d_readerListener(listener));

    return result;
}



c_bool
d_groupRemoteListenerAreRemoteGroupsHandled(
    d_groupRemoteListener listener)
{
    return d_groupCreationQueueIsEmpty(listener->groupCreationQueue);
}
