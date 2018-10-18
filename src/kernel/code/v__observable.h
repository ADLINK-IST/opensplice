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

#ifndef V__OBSERVABLE_H
#define V__OBSERVABLE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "v_event.h"
#include "os_if.h"
#include "v_observable.h"
#include "v_observer.h"

#define OSPL_LOCK(_this) \
        v_observableLock(v_observable(_this))
#define OSPL_UNLOCK(_this) \
        v_observableUnlock(v_observable(_this))

#define OSPL_ASSERT_LOCK(_this)

#define OSPL_ADD_OBSERVER(_this,o,mask,userdata) \
        v_observableAddObserver(v_observable(_this), v_observer(o), mask, userdata)
#define OSPL_REMOVE_OBSERVER(_this,o,mask,userdata) \
        v_observableRemoveObserver(v_observable(_this), v_observer(o), mask, userdata)

#define OSPL_THROW_EVENT(_this,e) \
        v_observableNotify(v_observable(_this), e);

#define v_observableHasObservers(_this) \
        (v_observable(_this)->observers != NULL)

/* Following macros are private for v_observer and v_observable */
#define OSPL_BLOCK_EVENTS(_this) \
        v_observableBlock(v_observable(_this))
#define OSPL_UNBLOCK_EVENTS(_this) \
        v_observableUnblock(v_observable(_this))

void
v_observableFree (
    v_observable _this);

/**
 * The initialisation of an observable object.
 * This method initialises all attributes of the observable class and must
 * be called by every derived class.
 *
 * \param _this the reference to an observable object.
 * \param name  the name of the observable.
 */
void
v_observableInit (
    v_observable _this);

/**
 * The de-initialisation of an observable object.
 * This method releases all used resources by the observable object and must
 * be called by every derived class.
 *
 * \param _this the reference to an observable object.
 */
void
v_observableDeinit (
    v_observable _this);

/**
 * Notifies all observers of the observable.
 *
 * \param _this the reference to an observable object.
 * \param event the event describes the reason of notification and
 *              is passed to the observers.
 */
void
v_observableNotify(
    v_observable _this,
    v_event event);

/**
 * Add an observer to the observable.
 * By adding an observer to the observable, the observer is notified on
 * state changes of the observable. With this notification the given
 * userData is also sent.
 *
 * \param _this    the reference to an observable object.
 * \param observer the reference to the observer object to add.
 * \param mask     this mask specifies the events of interest, these events will now trigger the observer.
 * \param userData this data also sent when notify the given observer.
 * \return         TRUE if the observer is added, otherwise FALSE.
 */
#if 0 /* TODO temporary declared public in v_observable.h until dependency in cmx API is removed. */
c_bool
v_observableAddObserver(
    v_observable _this,
    v_observer observer,
    v_eventMask mask,
    c_voidp userData);
#endif

/**
 * Removes an observer from the observable.
 * By removing an observer from the observable, the observer is no longer
 * notified on state changes of the observable.
 *
 * \param _this    the reference to an observable object.
 * \param observer the reference to the observer object to remove.
 * \param mask     this mask specifies the events of no interest, these events will no
 *                 longer trigger the observer.
 * \param userData the reference to the userdata belonging to the proxy of the observer object to remove.
 * \return         TRUE if the observer is removed, otherwise FALSE.
 */
c_bool
v_observableRemoveObserver (
    v_observable _this,
    v_observer observer,
    v_eventMask mask,
    void** userData);

void v_observableLock(v_observable o);
void v_observableUnlock(v_observable o);
void v_observableBlock(v_observable o);
void v_observableUnblock(v_observable o);

#if defined (__cplusplus)
}
#endif

#endif /* V__OBSERVABLE_H */
