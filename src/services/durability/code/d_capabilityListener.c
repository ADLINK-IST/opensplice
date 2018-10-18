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

#include "d__capabilityListener.h"
#include "d__readerListener.h"
#include "d__subscriber.h"
#include "d__durability.h"
#include "d__admin.h"
#include "d__capability.h"
#include "d__misc.h"
#include "d__fellow.h"
#include "d__configuration.h"
#include "d__publisher.h"
#include "d_message.h"

#define d_capabilityListener(_this) ((d_capabilityListener)(_this))
#define d_capabilityListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_CAPABILITY_LISTENER)

C_STRUCT(d_capabilityListener){
    C_EXTENDS(d_readerListener);
    d_subscriber subscriber;
};

static void
d_capabilityListenerAction(
    d_listener listener,
    d_message message)
{
    d_admin admin;
    d_durability durability;
    d_networkAddress fellowAddr;
    d_capability capability;
    char *capabilityStr;
    d_fellow fellow;

    assert(d_capabilityListenerIsValid(listener));

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    fellowAddr = d_networkAddressNew(
                    message->senderAddress.systemId,
                    message->senderAddress.localId,
                    message->senderAddress.lifecycleId);
    /* Get the capability and determine if it was a request
     * or response
     */

    capability = d_capability(message);

    /* The fellow has sent its capabilities to me.
     * I can only send my capabilities if I have detected
     * the capabilityListener of the fellow. In case
     * the fellow's capabilityListener is not yet detected
     * I must postpone sending my capabilities until the
     * capabilityListener of the fellow is detected.
     */
    capabilityStr = d_capabilityToString(capability);
    d_printTimedEvent(durability, D_LEVEL_FINE,
         "Received capabilities from fellow %u %s\n",fellowAddr->systemId, capabilityStr);
    os_free(capabilityStr);

    /* Get or create the fellow, set its capability and check its responsiveness */
    fellow = d_durabilityGetOrCreateFellowFromMessage(admin, fellowAddr, message);
    if (fellow) {
        /* I received a capability from the fellow, so the fellow must have capability support */
        assert(d_fellowHasCapabilitySupport(fellow));
        d_fellowSetCapability(fellow, capability);
        d_fellowCheckInitialResponsiveness(fellow);
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "No fellow retrieved or created when capabilities from fellow %u were received\n",
            fellowAddr->systemId);
    }
    d_networkAddressFree(fellowAddr);
    return;
}

static void
d_capabilityListenerDeinit(
    d_capabilityListener listener)
{
    assert(d_capabilityListenerIsValid(listener));

    /* Stop the listener */
    d_capabilityListenerStop(listener);
    /* Nothing to deallocate, call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static c_bool
d_capabilityListenerInit(
    d_capabilityListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed
     */

    assert(d_subscriberIsValid(subscriber));
    assert(listener);

    /* Call super-init */
    (void)os_threadAttrInit(&attr);
    d_readerListenerInit(d_readerListener(listener),
                         D_CAPABILITY_LISTENER,
                         d_capabilityListenerAction,
                         subscriber,
                         D_CAPABILITY_TOPIC_NAME,
                         D_CAPABILITY_TOP_NAME,
                         V_RELIABILITY_RELIABLE,
                         V_HISTORY_KEEPLAST,
                         1,
                         attr,
                         (d_objectDeinitFunc)d_capabilityListenerDeinit);
    return TRUE;
}

d_capabilityListener
d_capabilityListenerNew(
    d_subscriber subscriber)
{
    d_capabilityListener listener;
    c_bool success;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate capabilityListener */
    listener = d_capabilityListener(os_malloc(C_SIZEOF(d_capabilityListener)));
    if (listener) {
        /* Initialize the capabilityListener */
        success = d_capabilityListenerInit(listener, subscriber);
        if (!success) {
            os_free(listener);
            listener = NULL;
        }
    }
    return listener;
}

void
d_capabilityListenerFree(
    d_capabilityListener listener)
{
    assert(d_capabilityListenerIsValid(listener));

    d_objectFree(d_object(listener));
}


c_bool
d_capabilityListenerStart(
    d_capabilityListener listener)
{
    return d_readerListenerStart(d_readerListener(listener));
}


c_bool
d_capabilityListenerStop(
    d_capabilityListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}

