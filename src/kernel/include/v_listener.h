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
#ifndef V_LISTENER_H
#define V_LISTENER_H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \class v_listener
 *
 * The listener extends the <code>v_observer</code> class and can be used to wait
 * on events. While waiting for events, every event is administrated
 */

/**
 * \brief The <code>v_listener</code> cast method.
 *
 * This method casts a kernel object to a <code>v_listener</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_listener</code> or one of its subclasses.
 */
#define v_listener(o)   (C_CAST(o,v_listener))

#include "v_participant.h"
#include "v_event.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * The definition of the action callback method, which is called when
 * one or more events are received.
 *
 * \param e   a received event.
 * \param arg a pointer to user data needed to perform the action.
 */
typedef void (*v_listenerAction)(v_listenerEvent e, c_voidp arg);

/**
 * Creates a new listener.
 *
 * \param p a reference to the participant
 * \param combine : if true then trigger once on multiple data available events.
 * \return       the reference to the newly created listener.
 */
OS_API v_listener
v_listenerNew(
    v_participant p,
    c_bool combine);

/**
 * Destroys the referenced listener.
 *
 * \param _this a reference to the listener object.
 */
OS_API void
v_listenerFree(
    v_listener _this);

/**
 * Wait until at least one event is received or the specified timeout has
 * elapsed.
 * The caller is blocked until at least one event is received or the specified
 * timeout ('time') parameter has elapsed. At receiving event(s) the given
 * action routine is called for each received event. The given 'arg' parameter
 * is passed-through as-is. On timeout the action routine is not called.
 *
 * \param _this  a reference to the listener object.
 * \param action reference to the action callback method.
 * \param arg    reference to data passed on to the action method.
 * \param time   the maximum time to wait until an event is received.
 */
OS_API v_result
v_listenerWait(
    v_listener _this,
    v_listenerAction action,
    c_voidp arg,
    const os_duration time);

/**
 * Notifies the listener on a state change of one of the conditions.
 * Every event the listener receives is stored in a list. The events
 * are grouped on the origin of the event.
 *
 * \param _this    a reference to the listener object.
 * \param e        a reference to the event describing the reason of the
 *                 notification. A NULL event only triggers blocking wait
 *                 calls to wakeup.
 * \param listener reference to listening entity.
 */
OS_API void
v_listenerNotify(
    v_listener _this,
    v_event e,
    v_entity listener);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
