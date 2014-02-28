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
#ifndef OS_ABSTRACT_H
#define OS_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific header file                      */
#include "include/os_abstract.h"
#include "os_defs.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
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


#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40100 && !defined (__cplusplus)
#define pa_increment(c) __sync_add_and_fetch ((c), 1)
#define pa_decrement(c) __sync_sub_and_fetch ((c), 1)
#else
OS_API os_uint32        pa_increment(os_uint32 *count);
OS_API os_uint32        pa_decrement(os_uint32 *count);
#endif

#define PA_ADDRCAST     os_address

#define PA_PRId16 PA_PA_PRId16
#define PA_PRIo16 PA_PA_PRIo16
#define PA_PRIu16 PA_PA_PRIu16
#define PA_PRIx16 PA_PA_PRIx16
#define PA_PRIX16 PA_PA_PRIX16

#define PA_PRId32 PA_PA_PRId32
#define PA_PRIo32 PA_PA_PRIo32
#define PA_PRIu32 PA_PA_PRIu32
#define PA_PRIx32 PA_PA_PRIx32
#define PA_PRIX32 PA_PA_PRIX32

#define PA_PRId64 PA_PA_PRId64
#define PA_PRIo64 PA_PA_PRIo64
#define PA_PRIu64 PA_PA_PRIu64
#define PA_PRIx64 PA_PA_PRIx64
#define PA_PRIX64 PA_PA_PRIX64

#define PA_PRIdADDR PA_PA_PRIdADDR
#define PA_PRIoADDR PA_PA_PRIoADDR
#define PA_PRIuADDR PA_PA_PRIuADDR
#define PA_PRIxADDR PA_PA_PRIxADDR
#define PA_PRIXADDR PA_PA_PRIXADDR

#define PA_PRIdSIZE PA_PA_PRIdSIZE
#define PA_PRIoSIZE PA_PA_PRIoSIZE
#define PA_PRIuSIZE PA_PA_PRIuSIZE
#define PA_PRIxSIZE PA_PA_PRIxSIZE
#define PA_PRIXSIZE PA_PA_PRIXSIZE

#define PA_ADDRFMT  "%" PA_PRIxADDR
#define PA_SIZEFMT  "%" PA_PRIuSIZE
#define PA_SIZESPEC     PA_PRIuSIZE

#define PA_INT64_C(x)   PA_PA_INT64_C(x)
#define PA_UINT64_C(x)  PA_PA_UINT64_C(x)

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_ABSTRACT_H */
