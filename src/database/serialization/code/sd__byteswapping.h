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
/** \file services/serialization/code/sd__byteswapping.h
 *  \brief Byteswapping and copying macros to be used by
 *         \b serializer descendants only.
 */

#ifndef SD__BYTESWAPPING_H
#define SD__BYTESWAPPING_H

#include "c_typebase.h"

#define SD_SWAP_1(dst,src) \
        (*(c_char *)dst = *(c_char *)src)

#define SD_SWAP_2(dst,src)                     \
        ((c_char *)dst)[0] = ((c_char *)src)[1]; \
        ((c_char *)dst)[1] = ((c_char *)src)[0]

#define SD_SWAP_4(dst,src)                     \
        ((c_char *)dst)[0] = ((c_char *)src)[3]; \
        ((c_char *)dst)[1] = ((c_char *)src)[2]; \
        ((c_char *)dst)[2] = ((c_char *)src)[1]; \
        ((c_char *)dst)[3] = ((c_char *)src)[0]

#define SD_SWAP_8(dst,src)                     \
        ((c_char *)dst)[0] = ((c_char *)src)[7]; \
        ((c_char *)dst)[1] = ((c_char *)src)[6]; \
        ((c_char *)dst)[2] = ((c_char *)src)[5]; \
        ((c_char *)dst)[3] = ((c_char *)src)[4]; \
        ((c_char *)dst)[4] = ((c_char *)src)[3]; \
        ((c_char *)dst)[5] = ((c_char *)src)[2]; \
        ((c_char *)dst)[6] = ((c_char *)src)[1]; \
        ((c_char *)dst)[7] = ((c_char *)src)[0]

#define SD_SWAP_N(dst,src,n) \
        { \
            c_address i; \
            c_address m = n-1; \
            for (i=0; i<n; i++) { \
                ((c_char *)dst)[i] = ((c_char *)src)[m-i]; \
            } \
        }

#define SD_COPY_1(dst,src) \
        (*(c_char *)dst = *(c_char *)src)

#define SD_COPY_2(dst,src) \
        SD_COPY_N(dst,src,2)

#define SD_COPY_4(dst,src) \
        SD_COPY_N(dst,src,4)

#define SD_COPY_8(dst,src) \
        SD_COPY_N(dst,src,8)

#define SD_COPY_N(dst,src,n) \
        memcpy(dst,src,n)

#ifdef PA_LITTLE_ENDIAN

#define SD_COPY2BIG_1(dst,src)        SD_SWAP_1(dst,src)
#define SD_COPY2BIG_2(dst,src)        SD_SWAP_2(dst,src)
#define SD_COPY2BIG_4(dst,src)        SD_SWAP_4(dst,src)
#define SD_COPY2BIG_8(dst,src)        SD_SWAP_8(dst,src)
#define SD_COPY2BIG_N(dst,src,n)      SD_SWAP_N(dst,src,n)

#define SD_COPY2LITTLE_1(dst,src)     SD_COPY_1(dst,src)
#define SD_COPY2LITTLE_2(dst,src)     SD_COPY_2(dst,src)
#define SD_COPY2LITTLE_4(dst,src)     SD_COPY_4(dst,src)
#define SD_COPY2LITTLE_8(dst,src)     SD_COPY_8(dst,src)
#define SD_COPY2LITTLE_N(dst,src,n)   SD_COPY_N(dst,src,n)

#endif /* PA_LITTLE_ENDIAN */

#ifdef PA_BIG_ENDIAN

#define SD_COPY2BIG_1(dst,src)        SD_COPY_1(dst,src)
#define SD_COPY2BIG_2(dst,src)        SD_COPY_2(dst,src)
#define SD_COPY2BIG_4(dst,src)        SD_COPY_4(dst,src)
#define SD_COPY2BIG_8(dst,src)        SD_COPY_8(dst,src)
#define SD_COPY2BIG_N(dst,src,n)      SD_COPY_N(dst,src,n)

#define SD_COPY2LITTLE_1(dst,src)     SD_SWAP_1(dst,src)
#define SD_COPY2LITTLE_2(dst,src)     SD_SWAP_2(dst,src)
#define SD_COPY2LITTLE_4(dst,src)     SD_SWAP_4(dst,src)
#define SD_COPY2LITTLE_8(dst,src)     SD_SWAP_8(dst,src)
#define SD_COPY2LITTLE_N(dst,src,n)   SD_SWAP_N(dst,src,n)

#endif  /* PA_BIG_ENDIAN */

#endif  /* SD__BYTESWAPPING_H */
