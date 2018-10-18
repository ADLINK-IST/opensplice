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

#include "d__listener.h"
#include "d__subscriber.h"

void
d_listenerInit(
    d_listener listener,
    d_listenerKind kind,
    d_subscriber subscriber,
    d_listenerAction action,
    d_objectDeinitFunc deinit)
{
    /* Do not assert the listener because the initialization
     * of the listener has not yet completed */

    if (listener) {
        /* Call super-init */
        d_lockInit(d_lock(listener), D_LISTENER,
                   (d_objectDeinitFunc)deinit);
        /* Initialize the listener */
        listener->kind      = kind;
        listener->admin     = d_subscriberGetAdmin(subscriber);
        listener->action    = action;
        listener->attached  = FALSE;
    }
}


void
d_listenerDeinit (
    d_listener listener)
{
    assert(d_listenerIsValid(listener));

    /* Call super-deinit */
    d_lockDeinit(d_lock(listener));
}


c_bool
d_listenerIsValidKind(
    d_listener listener,
    d_listenerKind kind)
{
    c_bool result = FALSE;

    assert(d_listenerIsValid(listener));

    if (listener->kind == kind) {
        result = TRUE;
    }
    return result;
}


void
d_listenerLock(
    d_listener listener)
{
    assert(d_listenerIsValid(listener));

    d_lockLock(d_lock(listener));
}


void
d_listenerUnlock(
    d_listener listener)
{
    assert(d_listenerIsValid(listener));

    d_lockUnlock(d_lock(listener));
}


d_listenerAction
d_listenerGetAction(
    d_listener listener)
{
    assert(d_listenerIsValid(listener));

    return listener->action;
}


c_bool
d_listenerIsAttached(
    d_listener listener)
{
    assert(d_listenerIsValid(listener));

    return listener->attached;
}

d_admin
d_listenerGetAdmin(
    d_listener listener)
{
    assert(d_listenerIsValid(listener));

    return listener->admin;
}
