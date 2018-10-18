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
#ifndef V_SPLICED_H
#define V_SPLICED_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"
#include "v_builtin.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_service</code> cast method.
 *
 * This method casts an object to a <code>v_service</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_service</code> or
 * one of its subclasses.
 */
#define v_spliced(o) (C_CAST(o, v_spliced))

OS_API void v_splicedFree(v_spliced spliced);

OS_API void
v_splicedGarbageCollector(
    v_spliced spliced);

OS_API void
v_splicedKernelManager(
    v_spliced spliced);

OS_API void
v_splicedBuiltinResendManager(
    v_spliced spliced);

OS_API v_leaseManager
v_splicedGetHeartbeatManager(
    v_spliced _this,
    c_bool create);

OS_API void
v_splicedPrepareTermination(
    v_spliced spliced);

OS_API c_bool
v_splicedStartHeartbeat(
    v_spliced spliced,
    os_duration period,
    os_duration renewal);

OS_API c_bool
v_splicedStopHeartbeat(
    v_spliced spliced);

OS_API void
v_splicedCAndMCommandDispatcherQuit(
   v_spliced spliced);

OS_API void
v_splicedBuiltinCAndMCommandDispatcher(
   v_spliced spliced);

/**
 * \brief Setup the durability client within spliced.
 *
 * This method will setup the kernel durability client feature within spliced.
 *
 * Communication channel:
 * partitionRequest     - The partition to send requests in.
 * partitionDataGlobal  - The partition to expect 'global' data.
 * partitionDataPrivate - The partition to expect 'private' data.
 */
OS_API v_result
v_splicedDurabilityClientSetup(
    v_spliced spliced,
    c_iter durablePolicies,
    const char* partitionRequest,
    const char* partitionDataGlobal,
    const char* partitionDataPrivate);

/**
 * \brief Acquire durability client from spliced.
 */
OS_API v_durabilityClient
v_splicedDurabilityClientGet(
    v_spliced spliced);

/**
 * \brief Durability client main thread loop.
 *
 * p - u_spliced
 * arg - NULL
 */
OS_API void
v_splicedDurabilityClientMain(
    v_public p,
    void *arg);

/**
 * \brief Terminate request for the durability client main thread loop.
 *
 * It is possible that the thread still runs when this function returns.
 * The only guarantee is that the thread will terminate as quickly as
 * possible.
 */
OS_API void
v_splicedDurabilityClientTerminate(
    v_spliced spliced);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_SPLICED_H */
