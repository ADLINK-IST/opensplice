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

#include "d__statusListener.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__readerListener.h"
#include "d__configuration.h"
#include "d__fellow.h"
#include "d__publisher.h"
#include "d__misc.h"
#include "d_status.h"
#include "d__listener.h"
#include "d__actionQueue.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "d_nameSpacesRequest.h"
#include "d_groupsRequest.h"
#include "v_time.h"
#include "c_time.h"
#include "os_heap.h"
#include "os_thread.h"
#include "os_time.h"

/**
 * Macro that checks the d_statusListener validity.
 * Because d_statusListener is a concrete class typechecking is required.
 */
#define d_statusListenerIsValid(_this)   \
        d_listenerIsValidKind(d_listener(_this), D_STATUS_LISTENER)

/**
 * \brief The d_statusListener cast macro.
 *
 * This macro casts an object to a d_statusListener object.
 */
#define d_statusListener(_this) ((d_statusListener)(_this))

C_STRUCT(d_statusListener){
    C_EXTENDS(d_readerListener);
};

static void
d_statusListenerAction(
    d_listener listener,
    d_message message)
{
    d_admin admin;
    d_fellow fellow = NULL, fellow2;
    d_networkAddress fellowAddr;

    assert(d_statusListenerIsValid(listener));

    admin = d_listenerGetAdmin(listener);
    fellowAddr = d_networkAddressNew(
            message->senderAddress.systemId,
            message->senderAddress.localId,
            message->senderAddress.lifecycleId);
    fellow = d_durabilityGetOrCreateFellowFromMessage(admin, fellowAddr, message);
    if (fellow) {
        /* Remove fellow that is about to terminate or terminated */
        if ((message->senderState == D_STATE_TERMINATING) || (message->senderState == D_STATE_TERMINATED)) {
            d_fellowSetCommunicationState(fellow, D_COMMUNICATION_STATE_TERMINATED);
            fellow2 = d_adminRemoveFellow(admin, fellow, TRUE);
            if (fellow2) {
                d_fellowFree(fellow2);
            }
        }
        d_fellowFree(fellow);
    }
    d_networkAddressFree(fellowAddr);
    return;
}

static void
d_statusListenerDeinit(
    d_statusListener listener)
{
    assert(d_statusListenerIsValid(listener));

    d_statusListenerStop(listener);
    /* Call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_statusListenerInit(
    d_statusListener listener,
    d_subscriber subscriber)
{
    d_admin admin;
    d_durability durability;
    d_configuration config;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed */

    assert(d_subscriberIsValid(subscriber));

    admin = d_subscriberGetAdmin(subscriber);
    assert(d_adminIsValid(admin));
    durability = d_adminGetDurability(admin);
    assert(d_durabilityIsValid(durability));
    config = d_durabilityGetConfiguration(durability);
    assert(d_configurationIsValid(config));
    /* Call super-init */
    d_readerListenerInit(   d_readerListener(listener),
                            D_STATUS_LISTENER,
                            d_statusListenerAction,
                            subscriber,
                            D_STATUS_TOPIC_NAME,
                            D_STATUS_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPLAST,
                            1, config->heartbeatScheduling,
                            (d_objectDeinitFunc)d_statusListenerDeinit);
}


d_statusListener
d_statusListenerNew(
    d_subscriber subscriber)
{
    d_statusListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate statusListener object */
    listener = d_statusListener(os_malloc(C_SIZEOF(d_statusListener)));
    if (listener) {
        /* Initialize statusListener */
        d_statusListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_statusListenerFree(
    d_statusListener listener)
{
    assert(d_statusListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

c_bool
d_statusListenerStart(
    d_statusListener listener)
{
    assert(d_statusListenerIsValid(listener));

    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_statusListenerStop(
    d_statusListener listener)
{
    c_bool result;

    assert(d_statusListenerIsValid(listener));

    result = FALSE;

    if(listener){
        result = d_readerListenerStop(d_readerListener(listener));
    }
    return result;
}
