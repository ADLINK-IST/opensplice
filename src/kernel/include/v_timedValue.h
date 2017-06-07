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
#ifndef V_TIMEDVALUE_H
#define V_TIMEDVALUE_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void     v_timedValueInit      (v_timedValue *_this);
OS_API void     v_timedValueReset     (v_timedValue *_this);
OS_API c_ulong  v_timedValueGetValue  (v_timedValue *_this);
OS_API os_timeW v_timedValueGetTime   (v_timedValue *_this);
OS_API void     v_timedValueSetValue  (v_timedValue *_this, c_ulong value);
OS_API void     v_timedValueValuePlus (v_timedValue *_this);
OS_API void     v_timedValueValueMin  (v_timedValue *_this);

#undef OS_API

#endif
