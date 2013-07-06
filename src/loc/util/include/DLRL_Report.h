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
#ifndef DLRL_REPORT_H
#define DLRL_REPORT_H

#if defined (__cplusplus)
extern "C" {
#endif
#include <stdio.h>
#include <stdarg.h>

#include "os_if.h"

#ifdef OSPL_BUILD_LOC_UTIL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef __ospl_func__
#undef __ospl_func__
#endif

#if ! defined ( _MSC_VER ) && ! defined ( INTEGRITY )
#define __ospl_func__ __func__
#else /* _MSC_VER */
#define __ospl_func__ __FUNCTION__

#endif /* _MSC_VER */

typedef enum DLRL_Report_InfoType {
    INF_API        = 1,     /*  API calls */
    INF_CALLBACK   = 1<<1,  /*  callbacks from the kernel */
    INF_ENTITY     = 1<<2,  /*  entity creation or deletion */
    INF_DCPS       = 1<<3,  /*  communication with DCPS */
    INF_ENTER      = 1<<4,  /*  entering functions */
    INF_EXIT       = 1<<5,  /*  exit functions */
    INF            = 1<<6,  /*  general trace info */
    INF_OBJECT     = 1<<7,  /*  log object pointers */
    INF_REF_COUNT  = 1<<8   /*  log reference counters */
} DLRL_Report_InfoType;


typedef enum DLRL_Report_ReportType {
    REPORT_WARNING,
    REPORT_ERROR,
    REPORT_CRITICAL,
    REPORT_FATAL,
    REPORT_REPAIRED,
    DLRL_Report_ReportType_elements
} DLRL_Report_ReportType;

/*  errors and warnings cannot be disabled */
/* #define DLRL_ERROR ... */
/* #define DLRL_WARNING ... */


#ifndef DLRL_DO_TRACE

/*  Unless specifically specified the DLRL info trace is disabled */
#define DLRL_INFO(...)    /*  replace with nothing */
#define DLRL_INFO_OBJECT(type)      /*  replace with nothing */

#else

  /* in NDEBUG mode the DLRL info trace is enabled*/
#define DLRL_INFO(...) DLRL_Report_info(__FILE__, __LINE__, __ospl_func__, __VA_ARGS__)
  /* DLRL_INFO_OBJECT creates an log similar to a DLRL_INFO but includes the "_this" pointer*/
#define DLRL_INFO_OBJECT(type) DLRL_Report_info(__FILE__, __LINE__, __ospl_func__, INF_OBJECT, ", object = %p", _this)

#endif

#define DLRL_REPORT(...) DLRL_Report_report(__FILE__, __LINE__, __ospl_func__, __VA_ARGS__)

/*  Always use the following macro to avoid segfaults on %s parameters */
#ifdef __GNUC__
#define DLRL_VALID_NAME(name) ((name) ? (name) : "???")
#else
#define DLRL_VALID_NAME(name) ((name) ? (name) : "???")
#endif

/* NOT IN DESIGN - removed argument */
OS_API void
DLRL_Report_info(
    const char *fileName,
    int lineNo,
    const char *function,
    ...);

/* NOT IN DESIGN - removed argument */
OS_API void
DLRL_Report_report(
    const char *fileName,
    int lineNo,
    const char *function,
    ...);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_REPORT_H */
