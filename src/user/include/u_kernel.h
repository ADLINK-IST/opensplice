/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef U_KERNEL_H
#define U_KERNEL_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"


#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_kernel(p) ((u_kernel)(p))

OS_API u_result
u_kernelCreatePersistentSnapshot(
    u_kernel _this,
    const c_char * partition_expression,
    const c_char * topic_expression,
    const c_char * uri);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
