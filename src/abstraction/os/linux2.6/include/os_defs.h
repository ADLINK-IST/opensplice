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
#ifndef OS_LINUX_DEFS_H
#define OS_LINUX_DEFS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>

typedef char               os_os_char;      /* 1 byte   signed integer*/
typedef uint8_t            os_os_uchar;     /* 1 byte unsigned integer*/
typedef int16_t            os_os_short;     /* 2 byte   signed integer */
typedef uint16_t           os_os_ushort;    /* 2 byte unsigned integer */
typedef int32_t            os_os_int32;     /* 4 byte   signed integer */
typedef uint32_t           os_os_uint32;    /* 4 byte unsigned integer */
typedef int64_t            os_os_int64;     /* 8 byte   signed integer */
typedef uint64_t           os_os_uint64;    /* 8 byte unsigned integer */
typedef float              os_os_float;     /* 4 byte float */
typedef double             os_os_double;    /* 8 byte float */

#ifdef __x86_64__
typedef unsigned long long os_os_address;   /* word length of the platform */
#else
typedef unsigned long      os_os_address;   /* word length of the platform */
#endif

typedef double os_os_timeReal;
typedef int os_os_timeSec;
typedef uid_t os_os_uid;
typedef gid_t os_os_gid;
typedef size_t os_os_size_t;
typedef mode_t os_os_mode_t;

/* Platform specific integers, 32-bit on most 32-bit and also 64-bit archs (LP64), but it could
 * be also 64-bit wide (ILP64 & SILP64). This type is required for systems calls relying on 
 * 'int' parameters, such as setsockopt */
typedef unsigned int      os_os_uint;
typedef          int      os_os_int;
typedef unsigned long int os_os_ulong_int;


#if defined (__cplusplus)
}
#endif

#endif /* OS_LINUX_DEFS_H */

