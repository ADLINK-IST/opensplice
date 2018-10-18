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

#ifndef OS_WINCE_TYPES_H
#define OS_WINCE_TYPES_H

#if defined (__cplusplus)
extern "C" {
#endif

// mha: _off_t (or off_t) not supported under winCE, added the definition here
#ifndef _OFF_T_DEFINED
#if     !__STDC__
/* Non-ANSI name for compatibility */
typedef long off_t;
#endif
#define _OFF_T_DEFINED
#endif
// mha - end-------------------------------------------------------------------

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

#include "os_formatspec_win32_win64.h" /* common printf format specifiers */

/** Platform specific integers, 32-bit on most 32-bit and also 64-bit archs (LP64), but it could
 * be also 64-bit wide (ILP64 & SILP64). This type is required for systems calls relying on
 * 'int' parameters, such as setsockopt */
typedef unsigned int	  os_os_uint;
typedef          int	  os_os_int;
typedef unsigned long int os_os_ulong_int;

typedef uintptr_t os_os_uintptr;

typedef double os_os_timeReal;
typedef DWORD64 os_os_timeSec;
typedef unsigned int os_os_uid;
typedef unsigned int os_os_gid;
typedef long os_os_size_t;
typedef DWORD os_os_mode_t;

#ifndef os_os_ssize_t
#define os_os_ssize_t signed long
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_WINCE_TYPES_H */
