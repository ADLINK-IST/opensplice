#ifndef V_HISTORICALDATAREQUEST_H
#define V_HISTORICALDATAREQUEST_H

/** \file kernel/include/v_historicalDataRequest.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API v_historicalDataRequest
v_historicalDataRequestNew(
    v_kernel kernel,
    c_char* filter,
    c_char* params[],
    c_ulong nofParams,
    c_time minSourceTime,
    c_time maxSourceTime,
    struct v_resourcePolicy *resourceLimits);

OS_API c_bool
v_historicalDataRequestEquals(
    v_historicalDataRequest req1,
    v_historicalDataRequest req2);

OS_API c_bool
v_historicalDataRequestIsValid(
    v_historicalDataRequest request,
    v_reader reader);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
