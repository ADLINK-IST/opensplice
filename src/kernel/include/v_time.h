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
#ifndef V_TIME_H
#define V_TIME_H

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
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define V_TIME_MAX_NANOSECONDS (1000000000)

#define v_timeIsZero(t) (c_timeCompare((t),C_TIME_ZERO) == C_EQ)
#define v_timeIsInfinite(t) (c_timeCompare((t),C_TIME_INFINITE) == C_EQ)

OS_API c_time
v_timeGet(
    void);

OS_API c_equality
v_timeCompare(
    c_time t1,
    c_time t2);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
