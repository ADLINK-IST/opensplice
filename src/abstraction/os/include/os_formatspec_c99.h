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
#ifndef OS_C99_FORMATSPEC_H
#define OS_C99_FORMATSPEC_H

#if defined (__cplusplus) && !defined (__GNUC__)
#include <cinttypes>
#else
#if defined (__cplusplus) && defined (__GNUC__)
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#endif

#include <stdint.h>

#define PA_PA_PRId16 PRId16
#define PA_PA_PRIo16 PRIo16
#define PA_PA_PRIu16 PRIu16
#define PA_PA_PRIx16 PRIx16
#define PA_PA_PRIX16 PRIX16

#define PA_PA_PRId32 PRId32
#define PA_PA_PRIo32 PRIo32
#define PA_PA_PRIu32 PRIu32
#define PA_PA_PRIx32 PRIx32
#define PA_PA_PRIX32 PRIX32

#define PA_PA_PRId64 PRId64
#define PA_PA_PRIo64 PRIo64
#define PA_PA_PRIu64 PRIu64
#define PA_PA_PRIx64 PRIx64
#define PA_PA_PRIX64 PRIX64

#define PA_PA_PRIdADDR PRIdPTR
#define PA_PA_PRIoADDR PRIoPTR
#define PA_PA_PRIuADDR PRIuPTR
#define PA_PA_PRIxADDR PRIxPTR
#define PA_PA_PRIXADDR PRIXPTR

#define PA_PA_PRIdSIZE "zd"
#define PA_PA_PRIoSIZE "zo"
#define PA_PA_PRIuSIZE "zu"
#define PA_PA_PRIxSIZE "zx"
#define PA_PA_PRIXSIZE "zX"

#define PA_PA_PRIoSIZE "zo"
#define PA_PA_PRIuSIZE "zu"
#define PA_PA_PRIxSIZE "zx"
#define PA_PA_PRIXSIZE "zX"

#define PA_PA_SCNd16 SCNd16
#define PA_PA_SCNi16 SCNi16
#define PA_PA_SCNo16 SCNo16
#define PA_PA_SCNu16 SCNu16
#define PA_PA_SCNx16 SCNx16

#define PA_PA_SCNd32 SCNd32
#define PA_PA_SCNi32 SCNi32
#define PA_PA_SCNo32 SCNo32
#define PA_PA_SCNu32 SCNu32
#define PA_PA_SCNx32 SCNx32

#define PA_PA_SCNd64 SCNd64
#define PA_PA_SCNi64 SCNi64
#define PA_PA_SCNo64 SCNo64
#define PA_PA_SCNu64 SCNu64
#define PA_PA_SCNx64 SCNx64

#define PA_PA_INT64_C(x) INT64_C(x)
#define PA_PA_UINT64_C(x) UINT64_C(x)

#endif /* OS_FORMATSPEC_H */
