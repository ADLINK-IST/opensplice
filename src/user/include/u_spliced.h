/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef U_SPLICED_H
#define U_SPLICED_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_spliced(sd) ((u_spliced)(sd))

OS_API u_spliced
u_splicedNew(
    const c_char *uri);
    
OS_API u_result
u_splicedFree(
    u_spliced spliced);

OS_API u_result
u_splicedGarbageCollector(
    u_spliced spliced);
    
OS_API u_result
u_splicedKernelManager(
    u_spliced spliced);

OS_API u_result
u_splicedBuiltinResendManager(
    u_spliced spliced);

OS_API u_result
u_splicedBuiltinCAndMCommandDispatcher(
   u_spliced spliced);

OS_API u_result
u_splicedCAndMCommandDispatcherQuit(
   u_spliced spliced);

OS_API u_result
u_splicedPrepareTermination(
    u_spliced spliced);

OS_API v_leaseManager
u_splicedGetHeartbeatManager(
    u_spliced spliced,
    c_bool create);

OS_API u_result
u_splicedStartHeartbeat(
    u_spliced spliced,
    v_duration period,
    v_duration renewal,
    c_long priority);
    
OS_API u_result
u_splicedStopHeartbeat(
    u_spliced spliced);

OS_API u_result
u_splicedSetInProcess();

OS_API c_bool
u_splicedInProcess();

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_SPLICED_H */
