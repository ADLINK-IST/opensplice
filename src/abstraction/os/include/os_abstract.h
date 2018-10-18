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
#ifndef OS_ABSTRACT_H
#define OS_ABSTRACT_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

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

#define PA_SCNd16 PA_PA_SCNd16
#define PA_SCNi16 PA_PA_SCNi16
#define PA_SCNo16 PA_PA_SCNo16
#define PA_SCNu16 PA_PA_SCNu16
#define PA_SCNx16 PA_PA_SCNx16

#define PA_SCNd32 PA_PA_SCNd32
#define PA_SCNi32 PA_PA_SCNi32
#define PA_SCNo32 PA_PA_SCNo32
#define PA_SCNu32 PA_PA_SCNu32
#define PA_SCNx32 PA_PA_SCNx32

#define PA_SCNd64 PA_PA_SCNd64
#define PA_SCNi64 PA_PA_SCNi64
#define PA_SCNo64 PA_PA_SCNo64
#define PA_SCNu64 PA_PA_SCNu64
#define PA_SCNx64 PA_PA_SCNx64

#define PA_ADDRFMT  "%" PA_PRIxADDR
#define PA_SIZEFMT  "%" PA_PRIuSIZE
#define PA_SIZESPEC     PA_PRIuSIZE

#define PA_INT64_C(x)   PA_PA_INT64_C(x)
#define PA_UINT64_C(x)  PA_PA_UINT64_C(x)

#define PA_PRIduration  PA_PRId64".%09u"
#define PA_PRItime      PA_PRIu64".%09u"

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_ABSTRACT_H */
