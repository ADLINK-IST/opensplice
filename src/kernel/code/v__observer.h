/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef V__OBSERVER_H
#define V__OBSERVER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_observer.h"

#define OSPL_TRIGGER_EVENT(_this,e, userdata) \
        v_observerNotify(v_observer(_this), e, userdata);
#define OSPL_CATCH_EVENT(_this, timeout) \
        v_observerTimedWait(v_observer(_this), timeout)
#define OSPL_CATCH_EVENT_ACTION(_this, timeout, action, arg) \
        v_observerTimedWaitAction(v_observer(_this), timeout, action, arg)
#define OSPL_CLEAR_EVENT_FLAGS(_this) \
        v_observerClearEventFlags(v_observer(_this))

#define OSPL_SET_EVENT_MASK(_this,mask) \
        v_observerSetEventMask(v_observer(_this),mask)
#define OSPL_ADD_EVENT_MASK(_this,mask) \
        v_observerSetEvent(v_observer(_this),mask)
#define OSPL_CLEAR_EVENT_MASK(_this,mask) \
        v_observerClearEvent(v_observer(_this),mask)
#define OSPL_GET_EVENT_MASK(_this) \
        v_observerGetEventMask(v_observer(_this))

/**
 * The initialisation of an observer object.
 * This method initialises all attributes of the observer class and
 * is called by all derived classes.
 *
 * \param _this the reference to an observer object.
 * \param name  the name of the observer.
 */
void
v_observerInit (
    _Inout_ v_observer _this);

/**
 * The de-initialisation of an observer object.
 * This method releases all used resources by the observer object and
 * is called by all derived classes.
 *
 * \param _this  the reference to an observer object.
 */
void
v_observerDeinit (
    v_observer _this);

void
v_observerFree (
    v_observer _this);

c_ulong
v_observerTimedWait(
    v_observer _this,
    const os_duration time);

c_ulong
v_observerGetEventFlags(
    v_observer _this);

c_ulong
v_observerSetEvent(
    v_observer _this,
    c_ulong event);

#if defined (__cplusplus)
}
#endif

#endif
