/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OS_WIN32_TYPES_H
#define OS_WIN32_TYPES_H

#if defined (__cplusplus)
extern "C" {
#endif

/* Included as part of VC7 include dir */
/* #include <sys/types.h> /* needed for _off_t */
#include <BaseTsd.h>

#if defined (_MSC_VER) && (_MSC_VER < 1600)
// VS 2005 & 2008 confirmed to have no stdint.h; Predecessors presumed likewise
#else
        #include <stdint.h>
#endif
#include <STDDEF.H>
#include <CRTDEFS.H>

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
/* MS standard pointer precision types... */
typedef DWORD_PTR          os_os_address;   /* word length of the platform */
/* long on 32; __int64 on 64 */
typedef LONG_PTR           os_os_saddress;  /* signed version of os_os_address */

/** Platform specific integers, 32-bit on most 32-bit and also 64-bit archs (LP64), but it could
 * be also 64-bit wide (ILP64 & SILP64). This type is required for systems calls relying on
 * 'int' parameters, such as setsockopt */
typedef unsigned int      os_os_uint;
typedef          int      os_os_int;
typedef unsigned long int os_os_ulong_int;

typedef double os_os_timeReal;
/* Int with pointer precision: int on 32 or __int64 on 64 bit */
typedef os_os_int64 os_os_timeSec;
typedef unsigned int os_os_uid;
typedef unsigned int os_os_gid;
typedef SIZE_T os_os_size_t;
/* keep in sync with st_mode field def in struct stat in sys/stat.h */
typedef unsigned short os_os_mode_t;

/* Specific pointer types as defined in stdint.h */
typedef uintptr_t os_os_uintptr;

#ifndef os_os_ssize_t
#define os_os_ssize_t SSIZE_T
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_TYPES_H */
