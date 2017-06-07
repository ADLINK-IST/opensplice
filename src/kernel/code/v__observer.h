/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#define v__observerClearEventFlags(_this) \
        (v_observer(_this)->eventFlags = 0)

#define v_observerEventMask(_this) \
        (v_observer(_this)->eventMask)

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
    v_observer _this);

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

/**
 * Retrieves and stores event data in the observer in one transaction.
 *
 * Retrieves and stores event data in the observer in a thread-safe manner.
 * It must be thread-safe, since the data can be used to determine whether
 * the observer must be triggered or not.
 *
 * \param _this     the reference to an observer object.
 * \param eventData the event data to store in the observer.
 * \return          the overwritten event data in the observer.
 */
c_voidp
v_observerSetEventData(
    v_observer _this,
    c_voidp eventData);

c_ulong
v__observerWait(
    v_observer _this);

c_ulong
v__observerTimedWait(
    v_observer _this,
    const os_duration time);

c_ulong
v_observerGetEventFlags(
    v_observer _this);

c_ulong
v__observerSetEvent(
    v_observer _this,
    c_ulong event);

#if defined (__cplusplus)
}
#endif

#endif
