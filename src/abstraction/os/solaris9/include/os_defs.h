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
#ifndef OS_SOLARIS_DEFS_H
#define OS_SOLARIS_DEFS_H

#define OS_SOLARIS_VER 9

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/types.h>

#define OS_SOCKET_USE_FCNTL 1
#define OS_SOCKET_USE_IOCTL 0
#define OS_HAS_UCONTEXT_T

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

typedef double os_os_timeReal;
typedef long int os_os_timeSec;
typedef uid_t os_os_uid;
typedef gid_t os_os_gid;
typedef size_t os_os_size_t;
typedef mode_t os_os_mode_t;

#if defined (__cplusplus)
}
#endif

#endif /* OS_SOLARIS_DEFS_H */
