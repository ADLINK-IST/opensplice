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

#include "d__statusListener.h"
#include "d_statusListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_configuration.h"
#include "d_status.h"
#include "d_listener.h"
#include "d_actionQueue.h"
#include "d_admin.h"
#include "d_publisher.h"
#include "d_fellow.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "d_nameSpacesRequest.h"
#include "d_groupsRequest.h"
#include "d_misc.h"
#include "v_time.h"
#include "c_time.h"
#include "os_heap.h"
#include "os_thread.h"

static c_bool
d_statusListenerRemoveDeadFellows(
    d_action action,
    c_bool terminate)
{
    d_admin admin;
    d_configuration config;
    d_timestamp removeTime;
    d_durability durability;
    c_bool result = FALSE;

    admin      = d_admin(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE){
        if(terminate == FALSE){
            config = d_durabilityGetConfiguration(d_adminGetDurability(admin));

            removeTime.seconds = config->heartbeatExpiryTime.tv_sec;
            removeTime.nanoseconds = config->heartbeatExpiryTime.tv_nsec;

            while (removeTime.nanoseconds >= 1000000000) {
                removeTime.seconds += 1;
                removeTime.nanoseconds -= 1000000000;
            }
            /*
             * Remove the fellow if it didn't report since current time minus
             * the heartbeat period times the maximum nomber of heartbeat
             * misses.
             */
            durability = d_adminGetDurability(admin);

            if(d_durabilityMustTerminate(durability) == FALSE){
                removeTime = c_timeSub(v_timeGet(), removeTime);
                d_adminCleanupFellows(admin, removeTime);
            }
            result = TRUE;
        }
    }
    return result;
}

d_statusListener
d_statusListenerNew(
    d_subscriber subscriber)
{
    d_statusListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_statusListener(os_malloc(C_SIZEOF(d_statusListener)));
        d_listener(listener)->kind = D_STATUS_LISTENER;
        d_statusListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_statusListenerInit(
    d_statusListener listener,
    d_subscriber subscriber)
{
    d_admin admin;
    d_durability durability;
    d_configuration config;

    admin = d_subscriberGetAdmin(subscriber);
    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);
    assert(d_objectIsValid(d_object(config), D_CONFIGURATION) == TRUE);

    d_readerListenerInit(   d_readerListener(listener),
                            d_statusListenerAction, subscriber,
                            D_STATUS_TOPIC_NAME, D_STATUS_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPLAST,
                            1, config->heartbeatScheduling,
                            d_statusListenerDeinit);
    listener->cleanupAction = NULL;
}

void
d_statusListenerFree(
    d_statusListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_STATUS_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_statusListenerDeinit(
    d_object object)
{
    d_statusListener listener;

    assert(d_listenerIsValid(d_listener(object), D_STATUS_LISTENER));

    if(object){
        listener = d_statusListener(object);
        d_statusListenerStop(listener);
    }
}

c_bool
d_statusListenerStart(
    d_statusListener listener)
{
    c_bool result;
    d_actionQueue queue;
    d_admin admin;
    os_time sleepTime, execTime;
    assert(d_listenerIsValid(d_listener(listener), D_STATUS_LISTENER));

    result = d_readerListenerStart(d_readerListener(listener));

    if(result == TRUE){
        result = FALSE;

        d_listenerLock(d_listener(listener));

        /* 200 ms */
        sleepTime.tv_sec  = 0;
        sleepTime.tv_nsec = 200000000;
        execTime          = os_timeAdd(os_timeGet(), sleepTime);

        admin = d_listenerGetAdmin(d_listener(listener));
        queue = d_adminGetActionQueue(admin);

        listener->cleanupAction = d_actionNew(execTime, sleepTime,
                                              d_statusListenerRemoveDeadFellows,
                                              admin);

        result = d_actionQueueAdd(queue, listener->cleanupAction);
        d_listenerUnlock(d_listener(listener));

        if(result == FALSE){
            d_readerListenerStop(d_readerListener(listener));
            d_actionFree(listener->cleanupAction);
            listener->cleanupAction = NULL;
        }
    }
    return result;
}

c_bool
d_statusListenerStop(
    d_statusListener listener)
{
    c_bool result;
    d_admin admin;
    d_actionQueue queue;

    assert(d_listenerIsValid(d_listener(listener), D_STATUS_LISTENER));
    result = FALSE;

    if(listener){
        result = d_readerListenerStop(d_readerListener(listener));

        d_listenerLock(d_listener(listener));

        if(listener->cleanupAction){
            admin = d_listenerGetAdmin(d_listener(listener));
            queue = d_adminGetActionQueue(admin);

            result = d_actionQueueRemove(queue, listener->cleanupAction);

            if(result == TRUE){
                d_actionFree(listener->cleanupAction);
                listener->cleanupAction = NULL;
            }
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}




void
d_statusListenerAction(
    d_listener listener,
    d_message message)
{
    d_admin admin;
    d_durability durability;
    d_fellow fellow, fellow2;
    d_networkAddress sender;
    d_timestamp receptionTime;
    c_time t;
    d_nameSpacesRequest request;
    d_publisher publisher;
    c_bool added;
    d_serviceState oldState;

    assert(d_listenerIsValid(d_listener(listener), D_STATUS_LISTENER));
    t = v_timeGet();

    receptionTime.seconds     = t.seconds;
    receptionTime.nanoseconds = t.nanoseconds;

    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    publisher  = d_adminGetPublisher(admin);
    sender     = d_networkAddressNew(message->senderAddress.systemId,
                                     message->senderAddress.localId,
                                     message->senderAddress.lifecycleId);
    fellow     = d_adminGetFellow(admin, sender);

    if(!fellow){
        if((message->senderState != D_STATE_TERMINATING) &&
           (message->senderState != D_STATE_TERMINATED)){
            d_printTimedEvent (durability, D_LEVEL_FINE,
                           D_THREAD_STATUS_LISTENER,
                           "Fellow %d unknown, administrating it.\n",
                           message->senderAddress.systemId);
            fellow = d_fellowNew(sender, message->senderState);
            d_fellowUpdateStatus(fellow, message->senderState, receptionTime);
            added = d_adminAddFellow(admin, fellow);

            if(added == FALSE){
                d_fellowFree(fellow);
                fellow = d_adminGetFellow(admin, sender);
                d_fellowUpdateStatus(fellow, message->senderState, receptionTime);
                assert(fellow);
            } else {
                fellow = d_adminGetFellow(admin, sender); /* This allows free at the end in all cases */
                assert(fellow);
                request = d_nameSpacesRequestNew(admin);
                d_messageSetAddressee(d_message(request), sender);
                d_publisherNameSpacesRequestWrite(publisher, request, sender);
                d_nameSpacesRequestFree(request);
            }
            d_fellowFree(fellow);
        }
    } else {
        /* Update fellow state, or remove if it terminates */
        switch(message->senderState){
            case D_STATE_TERMINATING:
            case D_STATE_TERMINATED:
                d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_TERMINATED);
                fellow2 = d_adminRemoveFellow(admin, fellow);
                d_fellowFree(fellow);

                if(fellow2){
                    d_fellowFree(fellow2);
                }
                d_printTimedEvent(durability, D_LEVEL_INFO, D_THREAD_STATUS_LISTENER, "Fellow removed from admin.\n");
                break;
            default:
                /* Update the state of the fellow */
                oldState = d_fellowGetState(fellow);

                if(oldState != message->senderState){
                    d_printTimedEvent (durability, D_LEVEL_FINE,
                               D_THREAD_STATUS_LISTENER,
                               "Updating state of fellow '%d' to '%s'.\n",
                               message->senderAddress.systemId, d_fellowStateText(message->senderState));
                }
                d_fellowUpdateStatus(fellow, message->senderState, receptionTime);
                d_fellowFree(fellow);
                break;
        }
    }
    d_networkAddressFree(sender);

    return;
}
