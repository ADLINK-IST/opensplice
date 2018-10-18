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
#ifndef OS_VXWORKS_KM_OS_OS_TYPES_H
#define OS_VXWORKS_KM_OS_OS_TYPES_H

#include <sys/types.h>
#include <stdarg.h>

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef LLONG_MAX
#define LLONG_MAX INT64_MAX
#endif
#ifndef LLONG_MIN
#define LLONG_MIN INT64_MIN
#endif
#ifndef INT_MAX
#define INT_MAX INT32_MAX
#endif
#ifndef INT_MIN
#define INT_MIN INT32_MIN
#endif

typedef char               os_os_char;      /* 1 byte   signed integer*/
typedef unsigned char      os_os_uchar;     /* 1 byte unsigned integer*/
typedef short              os_os_short;     /* 2 byte   signed integer */
typedef unsigned short     os_os_ushort;    /* 2 byte unsigned integer */
typedef int                os_os_int32;     /* 4 byte   signed integer */
typedef unsigned int       os_os_uint32;    /* 4 byte unsigned integer */
typedef long long          os_os_int64;     /* 8 byte   signed integer */
typedef unsigned long long os_os_uint64;    /* 8 byte unsigned integer */
typedef float              os_os_float;     /* 4 byte float */
typedef double             os_os_double;    /* 8 byte float */
typedef unsigned long      os_os_address;   /* word length of the platform */
typedef long               os_os_saddress;  /* signed version of os_os_address */

#include "os_formatspec_ilp32_i32lp64.h" /* common printf format specifiers */

/** Platform specific integers, 32-bit on most 32-bit and also 64-bit archs (LP64), but it could
 * be also 64-bit wide (ILP64 & SILP64). This type is required for systems calls relying on
 * 'int' parameters, such as setsockopt */
typedef unsigned int      os_os_uint;
typedef          int      os_os_int;
typedef unsigned long int os_os_ulong_int;

#ifdef VXWORKS_GTE_6_9
typedef uintptr_t os_os_uintptr;
#else
typedef unsigned int os_os_uintptr;
#endif
  
typedef double       os_os_timeReal;
typedef int          os_os_timeSec;
typedef uid_t        os_os_uid;
typedef gid_t        os_os_gid;
typedef size_t       os_os_size_t;
typedef mode_t       os_os_mode_t;

#if defined( OSPL_VXWORKS653 )
typedef struct os_if_struct {
	char* name;
	char* address;
	char* netmask;
	char* broadcast;
	int flags;
} os_if_struct;


/* There is no file access in vx653 so this is only for it to compile
   NONE of the file access functions will be called */
#define R_OK     0
#define X_OK     1

#define AF_INET6 !AF_INET

#endif

#if defined (__cplusplus)
}
#endif

#endif
