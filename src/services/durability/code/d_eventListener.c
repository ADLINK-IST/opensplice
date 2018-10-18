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

#include "d__eventListener.h"
#include "os_heap.h"

d_eventListener
d_eventListenerNew(
    c_ulong interest,
    d_eventListenerFunc func,
    c_voidp args)
{
    d_eventListener listener = NULL;

    if (func) {
        /* Allocate eventListener object */
        listener = d_eventListener(os_malloc(C_SIZEOF(d_eventListener)));
        if (listener) {
            /* Call super-init */
            d_objectInit(d_object(listener), D_EVENT_LISTENER,
                         (d_objectDeinitFunc)d_eventListenerDeinit);
            /* Initialize eventListener */
            listener->interest  = interest;
            listener->func      = func;
            listener->args      = args;
        }
    }
    return listener;
}

c_voidp
d_eventListenerGetUserData(
    d_eventListener listener)
{
    c_voidp userData = NULL;

    assert(d_eventListenerIsValid(listener));

    if(listener){
        userData = listener->args;
    }
    return userData;
}

void
d_eventListenerDeinit(
    d_eventListener listener)
{
    assert(d_eventListenerIsValid(listener));

    /* Nothing to deallocate, call super-deinit */
    d_objectDeinit(d_object(listener));
}

void
d_eventListenerFree(
    d_eventListener listener)
{
    assert(d_eventListenerIsValid(listener));

    d_objectFree(d_object(listener));
}
