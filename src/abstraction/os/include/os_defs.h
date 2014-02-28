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
/****************************************************************
 * Interface definition for common definitions of SPLICE-DDS    *
 ****************************************************************/

/** \file os_defs.h
 *  \brief General OS layer definitions
 */

#ifndef OS_DEFS_H
#define OS_DEFS_H

#if defined (__cplusplus)
extern "C" {
#endif

/* Include OS specific header file				*/
#include "include/os_defs.h"
#include "os_if.h"

#ifndef OS_SOCKET_USE_FCNTL
#error OS_SOCKET_USE_FCNTL must be defined for this platform.
#endif

#ifndef OS_SOCKET_USE_IOCTL
#error OS_SOCKET_USE_IOCTL must be defined for this platform.
#endif

#if ( OS_SOCKET_USE_IOCTL == 1 ) && ( OS_SOCKET_USE_FCNTL == 1 )
#error this platform must set only one of OS_SOCKET_USE_IOCTL and OS_SOCKET_USE_FCNTL to 1
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* a somewhat complex, but efficient way to calculate the max size in
 * a platform independent manner. For unsigned numerical types the first part  up to
 * the XOR is enough. The second part is to make up for signed numerical types.
 */
#define OS_MAX_INTEGER(T) ((T)(((T)~0) ^ ((T)!((T)~0 > 0) << (CHAR_BIT * sizeof(T) - 1))))
#define OS_MIN_INTEGER(T) ((-OS_MAX_INTEGER(T)) - 1)

/* Bounds for integers of known size. These are defined in C99 but aren't
 * always provided.
 */

#ifndef INT64_MAX
#define INT64_MAX (9223372036854775807LL)
#endif
#ifndef INT64_MIN
#define INT64_MIN (-INT64_MAX - 1LL)
#endif
#ifndef UINT64_MAX
#define UINT64_MAX (18446744073709551615ULL)
#endif
#ifndef INT32_MAX
#define INT32_MAX (2147483647)
#endif
#ifndef INT32_MIN
#define INT32_MIN (-2147483647-1)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX (4294967295U)
#endif

/* Compiler Silencing macros
 * Some compilers complain about parameters that are not used.
 */
#if !defined (OS_UNUSED_ARG)
#define OS_UNUSED_ARG(a) (void) (a)
#endif

/** @def TRUE
 * @bug OSPL-2272 */
/** @def FALSE
 * @bug OSPL-2272 */
/* VERSION is the deprecated form of OSPL_VERSION
 * This is only defined when building OpenSplice itself */
#if defined (VERSION) || defined (OSPL_VERSION)
/* TRUE and FALSE macro's */
#if !defined FALSE
#define FALSE              (0)
#elif FALSE != (0)
#error "FALSE macro not defined as 0"
#endif
#if !defined TRUE
#define TRUE               (!FALSE)
#elif TRUE != !FALSE
#error "TRUE macro not defined as !FALSE"
#endif
#endif

#if !defined OS_C_FALSE
#define OS_C_FALSE              (0)
#elif OS_C_FALSE != (0)
#error "OS_C_FALSE macro not defined as 0"
#endif
#if !defined OS_C_TRUE
#define OS_C_TRUE               (!OS_C_FALSE)
#elif OS_C_TRUE != !FALSE
#error "OS_C_TRUE macro not defined as !OS_C_FALSE"
#endif


/** \brief OS layer primitive type definitions
 */
typedef os_os_char      os_char;      /* 1 byte   signed integer*/
typedef os_os_uchar     os_uchar;     /* 1 byte unsigned integer*/
typedef os_os_short     os_short;     /* 2 byte   signed integer */
typedef os_os_ushort    os_ushort;    /* 2 byte unsigned integer */
typedef os_os_int32     os_int32;     /* 4 byte   signed integer */
typedef os_os_uint32    os_uint32;    /* 4 byte unsigned integer */
typedef os_os_int64     os_int64;     /* 8 byte   signed integer */
typedef os_os_uint64    os_uint64;    /* 8 byte unsigned integer */
typedef os_os_float     os_float;     /* 4 byte float */
typedef os_os_double    os_double;    /* 8 byte float */
typedef os_os_address   os_address;   /* integer with size of pointer on the platform */
typedef os_os_saddress  os_saddress;  /* signed version of os_address */

/* Platform specific integers, 32-bit on most 32-bit and also 64-bit
 * archs (LP64), but it could be also 64-bit wide (ILP64 &
 * SILP64). This type is required for systems calls relying on 'int'
 * parameters, such as setsockopt */
typedef os_os_int       os_int;
typedef os_os_uint      os_uint;
typedef os_os_ulong_int os_ulong_int;

/** \brief OS layer real definition to represent time
 */
typedef os_os_timeReal os_timeReal;

/** \brief OS layer seconds definition to represent
 *         seconds since 1-jan-1970 00:00
 */
typedef os_os_timeSec os_timeSec;

/** \brief OS layer user identification
 */
typedef os_os_uid os_uid;

/** \brief OS layer group identification
 */
typedef os_os_gid os_gid;

/** \brief OS layer size_t definition
 */
typedef os_os_size_t os_size_t;

/** \brief OS layer ssize_t definition
 */
#ifdef os_os_ssize_t
typedef os_os_ssize_t os_ssize_t;
#else
typedef ssize_t os_ssize_t;
#endif

/** \brief OS layer mode_t definition
 */
typedef os_os_mode_t os_mode_t;

/** \brief Definition of the scope attribute */
typedef enum os_scopeAttr {
    /** The scope of the service is system wide */
    OS_SCOPE_SHARED,
    /** The scope of the service is process wide */
    OS_SCOPE_PRIVATE
} os_scopeAttr;

/** \brief Definition of the page locking policy */
typedef enum os_lockPolicy {
    /** Page locking policy for processes and shared memory
     *  according a platform default. OS_LOCKED for realtime
     *  platforms and OS_UNLOCKED for timesharing platforms
     */
    OS_LOCK_DEFAULT,
    /** Lock pages in physical memory
     */
    OS_LOCKED,
    /** Pages may be paged by the pager on will
     */
    OS_UNLOCKED
} os_lockPolicy;

/** \brief Definition of memory page locking
 * \todo the definition os_lockPolicy will be removed in V4.1
 */
#define OS_MEMLOCK_CURRENT (0x1)
#define OS_MEMLOCK_FUTURE  (0x2)

/** \brief Definition of the scheduling class */
typedef enum os_schedClass {
    /** Schedule processes and threads according a platform default.
     *  OS_SCHED_REALTIME for timesharing platforms and
     *  OS_SCHED_TIMESHARE for realtime platforms
     */
    OS_SCHED_DEFAULT,
    /** Schedule processes and threads on realtime basis,
     *  on most platforms implying:
     *  - Fixed Priority
     *  - Preemption
     *  - Either "First In First Out" or "Round Robin"
     */
    OS_SCHED_REALTIME,
    /** Schedule processes and threads on timesharing basis,
     *  on most platforms implying:
     *  - Dynamic Priority to guarantee fair share
     *  - Preemption
     */
    OS_SCHED_TIMESHARE
} os_schedClass;

/** \brief Definition of the compare results */
typedef enum os_compare {
    OS_LESS,
    OS_EQUAL,
    OS_MORE
} os_compare;

/** \brief Definition of the service return values */
typedef enum os_result {
    /** The service is successfuly completed */
    os_resultSuccess,
    /** A resource was not found */
    os_resultUnavailable,
    /** The service is timed out */
    os_resultTimeout,
    /** The requested resource is busy */
    os_resultBusy,
    /** An invalid argument is passed */
    os_resultInvalid,
    /** The operating system returned a failure */
    os_resultFail
} os_result;

/** \brief Definition of user credentials */
typedef struct os_userCred {
    /** \brief User identification */
    os_uid uid;
    /** \brief Group identification */
    os_gid gid;
} os_userCred;

/** \brief Definition of boolean with it's possible values */
typedef enum os_boolean {
    OS_FALSE = 0,
    OS_TRUE = 1
} os_boolean;

typedef void (*os_fptr)(void);

/** \brief Operation to convert the enum value to a string (no os_free required)
 */
OS_API os_char *
os_scopeAttrImage(
    os_scopeAttr _this);

/** \brief Operation to convert the enum value to a string (no os_free required)
 */
OS_API os_char *
os_lockPolicyImage(
    os_lockPolicy _this);

/** \brief Operation to convert the enum value to a string (no os_free required)
 */
OS_API os_char *
os_schedClassImage(
    os_schedClass _this);

/** \brief Operation to convert the enum value to a string (no os_free required)
 */
OS_API os_char *
os_compareImage(
    os_compare _this);

/** \brief Operation to convert the enum value to a string (no os_free required)
 */
OS_API os_char *
os_resultImage(
    os_result _this);

/** \brief Operation to convert the enum value to a string (no os_free required)
 */
OS_API os_char *
os_booleanImage(
    os_boolean _this);

#define os_fptr(v) os__fptr(v)

OS_API os_fptr
os__fptr(
    void* ptr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_DEFS_H */
