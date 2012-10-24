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
#ifndef OS_WIN32_DEFS_H
#define OS_WIN32_DEFS_H

// Suppress spurious 'child' : inherits 'parent::member' via dominance warnings
#if defined (_MSC_VER)
    #pragma warning(disable:4250)
#endif /* _MSC_VER */

#if defined (__cplusplus)
extern "C" {
#endif

/* Included as part of VC7 include dir */
#include <sys/types.h> /* needed for _off_t */

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

/** Platform specific integers, 32-bit on most 32-bit and also 64-bit archs (LP64), but it could
 * be also 64-bit wide (ILP64 & SILP64). This type is required for systems calls relying on
 * 'int' parameters, such as setsockopt */
typedef unsigned int      os_os_uint;
typedef          int      os_os_int;
typedef unsigned long int os_os_ulong_int;

typedef double os_os_timeReal;
typedef int os_os_timeSec;
typedef unsigned int os_os_uid;
typedef unsigned int os_os_gid;
typedef _off_t os_os_size_t;
/* keep in sync with st_mode field def in struct stat in sys/stat.h */
typedef unsigned short os_os_mode_t;

#ifndef ssize_t 
#define ssize_t signed long 
#endif 

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_DEFS_H */
