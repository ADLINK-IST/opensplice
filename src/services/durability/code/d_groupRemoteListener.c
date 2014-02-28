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
#include "d__groupRemoteListener.h"
#include "d_groupRemoteListener.h"
#include "d_sampleChainListener.h"
#include "d__readerListener.h"
#include "d_readerListener.h"
#include "d_groupCreationQueue.h"
#include "d_actionQueue.h"
#include "d_subscriber.h"
#include "d_eventListener.h"
#include "d_listener.h"
#include "d_admin.h"
#include "d_fellow.h"
#include "d_group.h"
#include "d_newGroup.h"
#include "d_configuration.h"
#include "d_publisher.h"
#include "d_nameSpace.h"
#include "d_durability.h"
#include "d_message.h"
#include "d_nameSpacesRequest.h"
#include "d_misc.h"
#include "d_networkAddress.h"
#include "u_group.h"
#include "u_participant.h"
#include "u_entity.h"
#include "os_heap.h"
#include "os_thread.h"

d_groupRemoteListener
d_groupRemoteListenerNew(
    d_subscriber subscriber)
{
    d_groupRemoteListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_groupRemoteListener(os_malloc(C_SIZEOF(d_groupRemoteListener)));
        d_listener(listener)->kind = D_GROUP_REMOTE_LISTENER;
        d_groupRemoteListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_groupRemoteListenerInit(
    d_groupRemoteListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    os_threadAttrInit(&attr);
    d_readerListenerInit(   d_readerListener(listener),
                            d_groupRemoteListenerAction, subscriber,
                            D_NEWGROUP_TOPIC_NAME, D_NEWGROUP_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            d_groupRemoteListenerDeinit);
}

void
d_groupRemoteListenerFree(
    d_groupRemoteListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_GROUP_REMOTE_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_groupRemoteListenerDeinit(
    d_object object)
{
    d_groupRemoteListener listener;

    assert(d_listenerIsValid(d_listener(object), D_GROUP_REMOTE_LISTENER));

    if(object){
        listener = d_groupRemoteListener(object);
        d_groupRemoteListenerStop(listener);
    }
}

c_bool
d_groupRemoteListenerStart(
    d_groupRemoteListener listener)
{
    d_admin admin;

    admin = d_listenerGetAdmin(d_listener(listener));
    listener->groupCreationQueue = d_groupCreationQueueNew(admin);

    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_groupRemoteListenerStop(
    d_groupRemoteListener listener)
{
    c_bool result;

    result = d_readerListenerStop(d_readerListener(listener));
    d_groupCreationQueueFree(listener->groupCreationQueue);

    return result;
}

void
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
    v_duration duration;
    u_group ugroup;
    d_networkAddress addr;
    c_bool result;
    d_subscriber subscriber;
    d_sampleChainListener sampleChainListener;
    d_completeness localCompleteness;

    assert(d_listenerIsValid(d_listener(listener), D_GROUP_REMOTE_LISTENER));

    remote     = d_newGroup(message);
    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    addr       = d_networkAddressNew(message->senderAddress.systemId,
                                     message->senderAddress.localId,
                                     message->senderAddress.lifecycleId);


    d_printTimedEvent(durability, D_LEVEL_FINEST,
                      D_THREAD_GROUP_REMOTE_LISTENER,
                      "DEBUG: Lookup fellow %u.%u\n",
                      message->senderAddress.systemId, message->senderAddress.localId);


    fellow     = d_adminGetFellow(admin, addr);

    if(remote->partition && remote->topic){
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                          D_THREAD_GROUP_REMOTE_LISTENER,
                          "Received remote group '%s.%s' with completeness=%d and quality=%d.%d from fellow: %u.\n",
                          remote->partition, remote->topic,
                          remote->completeness,
                          remote->quality.seconds,
                          remote->quality.nanoseconds,
                          message->senderAddress.systemId);
    }
    if(fellow){
        if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED){
            if(!(remote->partition) && !(remote->topic)){
                d_fellowSetExpectedGroupCount(fellow, remote->alignerCount);
            } else {
                group = d_adminGetLocalGroup(admin, remote->partition,
                                             remote->topic, remote->durabilityKind);
                if(!group){
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                                            D_THREAD_GROUP_REMOTE_LISTENER,
                                            "Received remote group %s.%s which is locally unknown.\n",
                                            remote->partition, remote->topic);

                    group = d_groupNew(remote->partition, remote->topic,
                                       remote->durabilityKind,
                                       remote->completeness, remote->quality);
                    added = d_fellowAddGroup(fellow, group);
                    d_fellowSetExpectedGroupCount(fellow, remote->alignerCount);

                    if(added == FALSE){
                        d_groupFree(group);
                        group = d_fellowGetGroup(fellow, remote->partition,
                                                 remote->topic,
                                                 remote->durabilityKind);

                        if(group){
                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                D_THREAD_GROUP_REMOTE_LISTENER,
                                "Remote group %s.%s already known for fellow %u, updating quality=%d.%d and completeness=%d.\n",
                                remote->partition, remote->topic,
                                message->senderAddress.systemId,
                                remote->quality.seconds,
                                remote->quality.nanoseconds,
                                remote->completeness);

                            d_groupUpdate(group, remote->completeness,
                                          remote->quality);
                            d_groupFree(group);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                D_THREAD_GROUP_REMOTE_LISTENER,
                                "Remote group %s.%s not added for fellow %u, but also not found.\n",
                                remote->partition, remote->topic,
                                message->senderAddress.systemId);
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                            D_THREAD_GROUP_REMOTE_LISTENER,
                            "Remote group %s.%s registered for fellow %u with quality=%d.%d and completeness=%d.\n",
                            remote->partition, remote->topic,
                            message->senderAddress.systemId,
                            remote->quality.seconds,
                            remote->quality.nanoseconds,
                            remote->completeness);
                    }
                    /* Group unknown locally, check if it should be aligned
                     * initially.
                     */
                    createLocally = d_adminGroupInInitialAligneeNS(
                                                admin, remote->partition,
                                                remote->topic);

                    if(createLocally == TRUE){
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                            D_THREAD_GROUP_REMOTE_LISTENER,
                                            "Remote group %s.%s in initial alignee namespace, so creating locally...\n",
                                            remote->partition, remote->topic);

                        duration.seconds = 0;
                        duration.nanoseconds = 10000000;

                        ugroup = u_groupNew(
                                u_participant(d_durabilityGetService(durability)),
                                remote->partition, remote->topic, duration);

                        if(ugroup){
                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                            D_THREAD_GROUP_REMOTE_LISTENER,
                                            "Remote group %s.%s with quality %d created locally.\n",
                                            remote->partition, remote->topic, remote->quality.seconds);
                            u_entityFree(u_entity(ugroup));
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                D_THREAD_GROUP_REMOTE_LISTENER,
                                "Remote group %s.%s with quality %d could NOT be created locally.\n",
                                remote->partition, remote->topic, remote->quality.seconds);
                            /**
                             * TODO: quality must not be taken over
                             * from remote.
                             */
                            group2 = d_groupNew(remote->partition, remote->topic,
                                           remote->durabilityKind,
                                           D_GROUP_INCOMPLETE,
                                           remote->quality);
                            result = d_groupCreationQueueAdd(
                                d_groupRemoteListener(listener)->groupCreationQueue,
                                group2);

                            if(result == FALSE){
                                d_printTimedEvent(durability, D_LEVEL_FINER,
                                        D_THREAD_GROUP_REMOTE_LISTENER,
                                        "Remote group %s.%s already in creation queue. Skipping this one.\n",
                                        remote->partition, remote->topic);
                                d_groupFree(group2);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                        D_THREAD_GROUP_REMOTE_LISTENER,
                                        "Remote group %s.%s in alignee namespace, but not initial.\n",
                                        remote->partition, remote->topic);
                    }
                } else {
                    localCompleteness = d_groupGetCompleteness(group);

                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                          D_THREAD_GROUP_REMOTE_LISTENER,
                          "Remote group '%s.%s' already known locally with completeness=%d.\n",
                          remote->partition, remote->topic,
                          localCompleteness);

                    group = d_fellowGetGroup(fellow, remote->partition,
                                                     remote->topic,
                                                     remote->durabilityKind);

                    if(group){
                        d_groupUpdate(group, remote->completeness,
                                      remote->quality);
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                              D_THREAD_GROUP_REMOTE_LISTENER,
                              "Updating remote group '%s.%s' for fellow %u with completeness=%d and quality=%d.%d.\n",
                              remote->partition, remote->topic,
                              message->senderAddress.systemId,
                              remote->completeness, remote->quality.seconds,
                              remote->quality.nanoseconds);
                        d_groupFree(group);
                    } else if(localCompleteness != D_GROUP_COMPLETE){
                        group = d_groupNew(remote->partition, remote->topic,
                                           remote->durabilityKind,
                                           remote->completeness, remote->quality);
                        added = d_fellowAddGroup(fellow, group);

                        if(added == FALSE){
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  D_THREAD_GROUP_REMOTE_LISTENER,
                                  "Remote group '%s.%s' with completeness=%d and quality=%d.%d could not be registered for fellow %u.\n",
                                  remote->partition, remote->topic,
                                  remote->completeness, remote->quality.seconds,
                                  remote->quality.nanoseconds,
                                  message->senderAddress.systemId);

                            d_groupFree(group);
                            group = d_fellowGetGroup(fellow, remote->partition,
                                                     remote->topic,
                                                     remote->durabilityKind);

                            if(group){
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                      D_THREAD_GROUP_REMOTE_LISTENER,
                                      "Updating remote group '%s.%s' for fellow %u with completeness=%d and quality=%d.%d.\n",
                                      remote->partition, remote->topic,
                                      message->senderAddress.systemId,
                                      remote->completeness, remote->quality.seconds,
                                      remote->quality.nanoseconds);
                                d_groupUpdate(group, remote->completeness,
                                              remote->quality);
                                d_groupFree(group);
                            }
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  D_THREAD_GROUP_REMOTE_LISTENER,
                                  "Remote group '%s.%s' with completeness=%d and quality=%d registered for fellow %u.\n",
                                  remote->partition, remote->topic,
                                  remote->completeness, remote->quality,
                                  message->senderAddress.systemId);
                        }
                    }
                    /* A complete group might be interesting in case there are
                     * still unfullfilled chain requests.
                     */
                    if(remote->completeness == D_GROUP_COMPLETE){
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                                  D_THREAD_GROUP_REMOTE_LISTENER,
                                                  "Remote group '%s.%s' complete, check for unfulfilled chains.\n",
                                                  remote->partition, remote->topic);
                        subscriber = d_adminGetSubscriber(admin);
                        sampleChainListener = d_subscriberGetSampleChainListener(subscriber);

                        group = d_groupNew(remote->partition, remote->topic,
                                           remote->durabilityKind,
                                           remote->completeness, remote->quality);

                        d_sampleChainListenerTryFulfillChains(sampleChainListener, group);

                        d_groupFree(group);
                    }
                }
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                       D_THREAD_GROUP_REMOTE_LISTENER,
                      "Fellow %u not approved, so ignoring remote group message.\n",
                      message->senderAddress.systemId);
        }
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
                D_THREAD_GROUP_REMOTE_LISTENER,
                "Fellow %u unknown so far, so ignoring remote group message.\n",
                message->senderAddress.systemId);
    }
    d_networkAddressFree(addr);

    d_printTimedEvent(durability, D_LEVEL_FINEST,
                      D_THREAD_GROUP_REMOTE_LISTENER,
                      "DEBUG: gruopRemoteListener action finished.\n");

    return;
}

c_bool
d_groupRemoteListenerAreRemoteGroupsHandled(
    d_groupRemoteListener listener)
{
    return d_groupCreationQueueIsEmpty(listener->groupCreationQueue);
}
