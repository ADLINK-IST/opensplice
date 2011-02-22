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
#ifndef OS_ABSTRACT_H
#define OS_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific header file                      */
#include "include/os_abstract.h"
#include "os_defs.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef PA__LITTLE_ENDIAN
#define PA_LITTLE_ENDIAN
#else
#define PA_BIG_ENDIAN
#endif

typedef enum {
    pa_endianLittle,
    pa_endianBig
} pa_endianNess;

OS_API pa_endianNess    pa_getEndianNess(void);

OS_API os_uint32        pa_increment(os_uint32 *count);
    
OS_API os_uint32        pa_decrement(os_uint32 *count);

#define PA_ADDRCAST	os_address

#ifdef PA__64BIT
#define PA_ADDRFMT	"%llx"
#define PA_SIZEFMT	"%llu"
#else
#define PA_ADDRFMT	"%lx"
#define PA_SIZEFMT	"%lu"
#endif

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_ABSTRACT_H */
