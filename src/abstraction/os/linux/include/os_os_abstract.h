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

#ifndef PA_LINUX_ABSTRACT_H
#define PA_LINUX_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#ifdef __x86_64__
#define PA__64BIT
#endif

#if defined (__cplusplus)
}
#endif

#endif /* PA_LINUX_ABSTRACT_H */
