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

/* printf() format specifiers that work for most ILP32 and I32LP64 platforms --
   pretty much every modern Unix box out there for example.  Essentially a
   clone of the C99 stuff. */

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

#ifdef PA__64BIT
#define PA_PA_PRId64 "ld"
#define PA_PA_PRIo64 "lo"
#define PA_PA_PRIu64 "lu"
#define PA_PA_PRIx64 "lx"
#define PA_PA_PRIX64 "lX"
#else
#define PA_PA_PRId64 "lld"
#define PA_PA_PRIo64 "llo"
#define PA_PA_PRIu64 "llu"
#define PA_PA_PRIx64 "llx"
#define PA_PA_PRIX64 "llX"
#endif

#define PA_PA_PRIdADDR "ld"
#define PA_PA_PRIoADDR "lo"
#define PA_PA_PRIuADDR "lu"
#define PA_PA_PRIxADDR "lx"
#define PA_PA_PRIXADDR "lX"

#define PA_PA_PRIdSIZE "ld"
#define PA_PA_PRIoSIZE "lo"
#define PA_PA_PRIuSIZE "lu"
#define PA_PA_PRIxSIZE "lx"
#define PA_PA_PRIXSIZE "lX"

#define PA_PA_SCNd16 "hd"
#define PA_PA_SCNi16 "hi"
#define PA_PA_SCNo16 "ho"
#define PA_PA_SCNu16 "hu"
#define PA_PA_SCNx16 "hx"

#define PA_PA_SCNd32 "d"
#define PA_PA_SCNi32 "i"
#define PA_PA_SCNo32 "o"
#define PA_PA_SCNu32 "u"
#define PA_PA_SCNx32 "x"

#ifdef PA__64BIT
#define PA_PA_SCNd64 "ld"
#define PA_PA_SCNi64 "li"
#define PA_PA_SCNo64 "lo"
#define PA_PA_SCNu64 "lu"
#define PA_PA_SCNx64 "lx"
#else
#define PA_PA_SCNd64 "lld"
#define PA_PA_SCNi64 "lli"
#define PA_PA_SCNo64 "llo"
#define PA_PA_SCNu64 "llu"
#define PA_PA_SCNx64 "llx"
#endif
/* magic for creating compile-time 64-bit constants */

#ifdef PA__64BIT
#define PA_PA_INT64_C(x) x##l
#define PA_PA_UINT64_C(x) x##ul
#else
#define PA_PA_INT64_C(x) x##ll
#define PA_PA_UINT64_C(x) x##ull
#endif

#endif /* OS_FORMATSPEC_H */
