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
#ifndef U_SPLICED_H
#define U_SPLICED_H

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

#define u_spliced(sd) ((u_spliced)(sd))

OS_API u_result
u_splicedNew(
    u_spliced *spliced,
    const os_char *uri);

OS_API u_result
u_splicedGarbageCollector(
    const u_spliced spliced);

OS_API u_result
u_splicedKernelManager(
    const u_spliced spliced);

OS_API u_result
u_splicedBuiltinResendManager(
    const u_spliced spliced);

OS_API u_result
u_splicedBuiltinCAndMCommandDispatcher(
   const u_spliced spliced);

OS_API u_result
u_splicedCAndMCommandDispatcherQuit(
   const u_spliced spliced);

OS_API u_result
u_splicedPrepareTermination(
   const u_spliced spliced);

OS_API v_leaseManager
u_splicedGetHeartbeatManager(
    u_spliced spliced,
    c_bool create);

OS_API u_result
u_splicedStartHeartbeat(
    const u_spliced spliced,
    os_duration period,
    os_duration renewal);

OS_API u_result
u_splicedStopHeartbeat(
   const u_spliced spliced);

OS_API void
u_splicedSetInProcess(
    c_bool flag);

OS_API u_bool
u_splicedInProcess(void);

OS_API u_result
u_splicedCleanupProcessInfo(
    const u_spliced spliced,
    os_procId procId);

/**
 * \brief Setup the durability client within spliced.
 *
 * This method will setup the durability client feature within spliced.
 *
 * Communication channel:
 * partitionRequest     - The partition to send requests in.
 * partitionDataGlobal  - The partition to expect 'global' data.
 * partitionDataPrivate - The partition to expect 'private' data.
 */
OS_API u_result
u_splicedDurabilityClientSetup(
    u_spliced spliced,
    c_iter durablePolicies,
    const char* partitionRequest,
    const char* partitionDataGlobal,
    const char* partitionDataPrivate);

/**
 * \brief Durability client main thread loop.
 */
OS_API void*
u_splicedDurabilityClientMain(
    void *spliced);

/**
 * \brief Terminate request for the durability client main thread loop.
 *
 * It is possible that the thread still runs when this function returns.
 * The only guarantee is that the thread will terminate as quickly as
 * possible.
 */
OS_API u_result
u_splicedDurabilityClientTerminate(
    u_spliced spliced);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_SPLICED_H */
