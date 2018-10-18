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
#ifndef UT_HEX_H
#define UT_HEX_H

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

/**
 *  \brief Encodes input as hexadecimal and returns number of bytes written.
 *
 * os_hexenc encodes the input as hexadecimal ascii and returns the number of
 * bytes that are required to encode the input, even if the output buffer is
 * not large enough. If the output buffer is large enough it's null terminated.
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 */
OS_API int
ut_hexenc(
    char *xbuf,
    unsigned int xlen,
    const unsigned char *buf,
    unsigned int len);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_HEX_H */
