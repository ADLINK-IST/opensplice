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
#ifndef U_LISTENER_H
#define U_LISTENER_H

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_listener(o) \
        ((u_listener)u_objectCheckType(u_object(o), U_LISTENER))

typedef void (*u_listenerAction)(v_listenerEvent e, void *arg);

/** \brief Creates a listener object that can wait for events.
 *
 * The listener object provides a framework to register a callback operation
 * to wait for a specific event. A listener object belongs to a participant so
 * its lifecycle is bound to the participants lifecycle, this doen mean that it
 * will receive events from the participant. The listener will only receive
 * events from entities that are associated to the listener by the
 * u_entitySetListener method.
 * if combine is true then trigger once on multiple data availability events.
 */
OS_API u_listener
u_listenerNew(
    const u_entity entity,
    u_bool combine);

/**
 * This method blocks until an event occurs or the timeout period has passed.
 *
 * When an event occurs the action callback is invoked and the event together
 * with the given arg is passed to the callback. After execution of the callback
 * this method will return.
 * TODO: evaluate if the action should have a return value that controls if the
 *       wait should return or continue.
 */
OS_API u_result
u_listenerWait(
    const u_listener _this,
    u_listenerAction action,
    void *arg,
    os_duration timeout);

/**
 * This method will wakeup blocking wait calls without passing an event.
 */
OS_API u_result
u_listenerNotify(
    const u_listener _this);

/**
 * \brief Wake-up blocking wait call by passing a trigger event
 */
OS_API u_result
u_listenerTrigger(
    const u_listener _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
