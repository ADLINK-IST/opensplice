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
#ifndef UT_ENTRYPOINT_H
#define UT_ENTRYPOINT_H

#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef int (*ut_entryPointFunc)(int,char **);

typedef struct ut_entryPointWrapperArg {
    ut_entryPointFunc entryPoint;
    os_int32 argc;
    os_char **argv;
} ut_entryPointWrapperArg;

OS_API void* ut_entryPointWrapper (void *arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_ENTRYPOINT_H */
