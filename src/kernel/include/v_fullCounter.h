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
#ifndef V_FULLCOUNTER_H
#define V_FULLCOUNTER_H

/** \file kernel/include/v_fullCounter.h
 *  \brief This file defines the interface
 */

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void     v_fullCounterInit       (v_fullCounter *_this);
OS_API void     v_fullCounterReset      (v_fullCounter *_this);
OS_API c_ulong  v_fullCounterGetValue   (v_fullCounter *_this);
OS_API void     v_fullCounterSetValue   (v_fullCounter *_this, c_ulong value);
OS_API void     v_fullCounterValueInc   (v_fullCounter *_this);
OS_API void     v_fullCounterValueDec   (v_fullCounter *_this);
OS_API os_timeW v_fullCounterGetMaxTime (v_fullCounter *_this);
OS_API c_ulong  v_fullCounterGetMax     (v_fullCounter *_this);
OS_API os_timeW v_fullCounterGetMinTime (v_fullCounter *_this);
OS_API c_ulong  v_fullCounterGetMin     (v_fullCounter *_this);
OS_API c_float  v_fullCounterGetAvg     (v_fullCounter *_this);

#undef OS_API

#endif
