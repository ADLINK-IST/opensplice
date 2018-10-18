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
#ifndef CMN_LISTENERDISPATCHER_H
#define CMN_LISTENERDISPATCHER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "vortex_os.h"
#include "v_kernel.h"
#include "u_user.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#define cmn_listenerDispatcher(o) ((cmn_listenerDispatcher)(o))

OS_CLASS(cmn_listenerDispatcher);

typedef void(*cmn_listenerDispatcher_callback)(
    v_listenerEvent event, c_voidp argument);

/**
 * \brief Get stack size to use for dispatcher thread
 *
 * @return Stack size in bytes
 */
OS_API os_uint32
cmn_listenerDispatcher_stack_size (
    u_participant uParticipant);

/**
 * \brief Allocate and initialize dispatcher
 *
 * @param uParticipant User-layer participant entity
 * @param scheduling_class Scheduling class
 * @param scheduling_priority Scheduling priority
 * @param callback Function to invoke for event
 * @param combine if true then trigger once on multiple data available events.
 */
OS_API cmn_listenerDispatcher
cmn_listenerDispatcher_new (
    u_participant uParticipant,
    os_schedClass scheduling_class,
    os_int32 scheduling_priority,
    cmn_listenerDispatcher_callback callback,
    void *callback_data,
    os_boolean combine);

/**
 * \brief Free memory occupied by dispatcher
 *
 * Dispatcher thread is instructed to stop if invoked on a running dispatcher.
 *
 * @param _this Dispatcher
 * @return OS_RETCODE_OK on success, other OS_RETCODE_* constant on failure
 */
OS_API os_int32
cmn_listenerDispatcher_free (
    cmn_listenerDispatcher _this);

/**
 * \brief Append observable to set of entities watched by dispatcher
 *
 * The set is used as a reference counting mechanism. The advantage to using a
 * set here is easy deduplication, or rather, registering an entity that's
 * already in the set is a noop.
 *
 * Dispatcher thread is guaranteed to be started on successful return.
 *
 * @param _this Dispatcher
 * @param observable Entity to register with dispatcher
 * @param mask Event mask with entity interest
 * @return OS_RETCODE_OK on success, other OS_RETCODE_* constant on failure
 */
OS_API os_int32
cmn_listenerDispatcher_add (
    cmn_listenerDispatcher _this,
    u_entity observable,
    cmn_listenerDispatcher_callback callback,
    void *callback_data,
    v_eventMask mask);

/**
 * \brief Remove observable from set of entities watched by dispatcher
 *
 * Dispatcher thread is instructed to stop if last Entity is deregistered.
 *
 * @param _this Dispatcher
 * @param observable Entity to deregister from dispatcher
 * @return OS_RETCODE_OK on success, other OS_RETCODE_* constant on failure
 */
OS_API os_int32
cmn_listenerDispatcher_remove (
    cmn_listenerDispatcher _this,
    u_entity observable);

/**
 * \brief Get scheduling class and priority currently used by dispatcher
 *
 * @param _this Dispatcher
 * @param scheduling_class Pointer to variable where scheduling class should be
 *                         stored.
 * @param scheduling_priority Pointer to variable where scheduling priority
 *                            should be stored.
 * @return OS_RETCODE_OK on success, other OS_RETCODE_* constant on failure
 */
OS_API void
cmn_listenerDispatcher_get_scheduling (
    cmn_listenerDispatcher _this,
    os_schedClass *scheduling_class,
    os_int32 *scheduling_priority);

/**
 * \brief Set scheduling class and scheduling priority for dispatcher
 *
 * The dispatcher thread is restarted if scheduling class is changed while a
 * dispatcher thread is already running.
 *
 * @param _this Dispatcher
 * @param scheduling_class Scheduling class to use
 * @param scheduling_priority Scheduling priority to use
 * @return OS_RETCODE_OK on success, other OS_RETCODE_* constant on failure
 */
OS_API os_int32
cmn_listenerDispatcher_set_scheduling (
    cmn_listenerDispatcher _this,
    os_schedClass scheduling_class,
    os_int32 scheduling_priority);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMN_LISTENERDISPATCHER_H */
