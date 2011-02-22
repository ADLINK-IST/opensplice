/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V_SPLICED_H
#define V_SPLICED_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"
#include "os_if.h"
#include "v_status.h"

#ifdef OSPL_BUILD_KERNEL
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

OS_API void
v_splicedPrepareTermination(
    v_spliced spliced);

OS_API c_bool
v_splicedStartHeartbeat(
    v_spliced spliced,
    v_duration period,
    v_duration renewal);

OS_API c_bool
v_splicedStopHeartbeat(
    v_spliced spliced);

OS_API void
v_splicedCAndMCommandDispatcherQuit(
   v_spliced spliced);

OS_API void
v_splicedBuiltinCAndMCommandDispatcher(
   v_spliced spliced);

OS_API v_result
v_splicedGetMatchedSubscriptions(
	v_spliced spliced,
    v_writer w,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_splicedGetMatchedSubscriptionData(
	v_spliced spliced,
    v_writer w,
    v_gid subscription,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_splicedGetMatchedPublications(
	v_spliced spliced,
    v_dataReader r,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_splicedGetMatchedPublicationData(
	v_spliced spliced,
    v_dataReader r,
    v_gid publication,
    v_statusAction action,
    c_voidp arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_SPLICED_H */
