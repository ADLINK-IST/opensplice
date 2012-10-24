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
#ifndef OS_DIAG_H
#define OS_DIAG_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include <stdarg.h>
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef OS_DIAG_CLUSTERS

#define OS_DIAG(cluster,level,description) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description)

#define OS_DIAG_1(cluster,level,description,a1) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1)

#define OS_DIAG_2(cluster,level,description,a1,a2) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2)

#define OS_DIAG_3(cluster,level,description,a1,a2,a3) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3)

#define OS_DIAG_4(cluster,level,description,a1,a2,a3,a4) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3,a4)

#define OS_DIAG_5(cluster,level,description,a1,a2,a3,a4,a5) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3,a4,a5)

#define OS_DIAG_6(cluster,level,description,a1,a2,a3,a4,a5,a6) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3,a4,a5,a6)

#define OS_DIAG_7(cluster,level,description,a1,a2,a3,a4,a5,a6,a7) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3,a4,a5,a6,a7)

#define OS_DIAG_8(cluster,level,description,a1,a2,a3,a4,a5,a6,a7,a8) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3,a4,a5,a6,a7,a8)

#define OS_DIAG_9(cluster,level,description,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
    os_diag(OS_DIAG_CLUSTERS,cluster,level,description,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#else

#define OS_DIAG(cluster,level,description)

#define OS_DIAG_1(cluster,level,description,a1)

#define OS_DIAG_2(cluster,level,description,a1,a2)

#define OS_DIAG_3(cluster,level,description,a1,a2,a3)

#define OS_DIAG_4(cluster,level,description,a1,a2,a3,a4)

#define OS_DIAG_5(cluster,level,description,a1,a2,a3,a4,a5)

#define OS_DIAG_6(cluster,level,description,a1,a2,a3,a4,a5,a6)

#define OS_DIAG_7(cluster,level,description,a1,a2,a3,a4,a5,a6,a7)

#define OS_DIAG_8(cluster,level,description,a1,a2,a3,a4,a5,a6,a7,a8)

#define OS_DIAG_9(cluster,level,description,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#endif

typedef void * os_IDiagService_s;

typedef enum os_diagLevel {
    OS_FENTRY,
    OS_FEXIT,
    OS_FTRACE
} os_diagLevel;

OS_API void
os_diag(
    const char   *diag_clusters,
    const char   *cluster,
    os_diagLevel  level,
    const char   *description,
    ...);

OS_API os_int32
os_registerDiagService(
    os_IDiagService_s diagServiceContext,
    void (*diagService)(
        os_IDiagService_s diagServiceContext,
        const char *cluster,
        os_diagLevel level,
        const char *description,
        va_list args));

OS_API os_int32
os_unregisterDiagService(
    os_IDiagService_s reportDiagContext);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_DIAG_H */
