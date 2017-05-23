/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef OS_FORMATSPEC_H
#define OS_FORMATSPEC_H

/* printf() format specifiers that work for Win32/Win64/WinCE */

#define PA_PA_PRId16 "hd"
#define PA_PA_PRIo16 "ho"
#define PA_PA_PRIu16 "hu"
#define PA_PA_PRIx16 "hx"
#define PA_PA_PRIX16 "hX"

#define PA_PA_PRId32 "d"
#define PA_PA_PRIo32 "o"
#define PA_PA_PRIu32 "u"
#define PA_PA_PRIx32 "x"
#define PA_PA_PRIX32 "X"

#define PA_PA_PRId64 "I64d"
#define PA_PA_PRIo64 "I64o"
#define PA_PA_PRIu64 "I64u"
#define PA_PA_PRIx64 "I64x"
#define PA_PA_PRIX64 "I64X"

#ifdef PA__64BIT
#define PA_PA_PRIdADDR "I64d"
#define PA_PA_PRIoADDR "I64o"
#define PA_PA_PRIuADDR "I64u"
#define PA_PA_PRIxADDR "I64x"
#define PA_PA_PRIXADDR "I64X"
#else
#define PA_PA_PRIdADDR "ld"
#define PA_PA_PRIoADDR "lo"
#define PA_PA_PRIuADDR "lu"
#define PA_PA_PRIxADDR "lx"
#define PA_PA_PRIXADDR "lX"
#endif

#define PA_PA_PRIdSIZE PA_PA_PRIdADDR
#define PA_PA_PRIoSIZE PA_PA_PRIoADDR
#define PA_PA_PRIuSIZE PA_PA_PRIuADDR
#define PA_PA_PRIxSIZE PA_PA_PRIxADDR
#define PA_PA_PRIXSIZE PA_PA_PRIXADDR

#define PA_PA_SCNd16 "hd"
#define PA_PA_SCNi16 "hi"
#define PA_PA_SCNo16 "ho"
#define PA_PA_SCNu16 "hu"
#define PA_PA_SCNx16 "hx"

#define PA_PA_SCNd32 "ld"
#define PA_PA_SCNi32 "li"
#define PA_PA_SCNo32 "lo"
#define PA_PA_SCNu32 "lu"
#define PA_PA_SCNx32 "lx"

#define PA_PA_SCNd64 "I64d"
#define PA_PA_SCNi64 "I64i"
#define PA_PA_SCNo64 "I64o"
#define PA_PA_SCNu64 "I64u"
#define PA_PA_SCNx64 "I64x"

#define PA_PA_INT64_C(x) x##i64
#define PA_PA_UINT64_C(x) x##ui64

#endif /* OS_FORMATSPEC_H */
