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
#ifndef V_SCHEDULE_H
#define V_SCHEDULE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API v_scheduleKind
v_scheduleProcessCurrentKind(void);

OS_API c_long
v_scheduleProcessCurrentPriority(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
