/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OS_WIN32_IF_H
#define OS_WIN32_IF_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <winsock2.h>
#include <iptypes.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifndef OS_API_EXPORT
#define OS_API_EXPORT __declspec(dllexport)
#endif

#ifndef OS_API_IMPORT
#define OS_API_IMPORT __declspec(dllimport)
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_IF_H */
