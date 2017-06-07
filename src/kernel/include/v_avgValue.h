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
#ifndef V_AVGVALUE_H
#define V_AVGVALUE_H

/** \file kernel/include/v_avgValue.h
 *  \brief This file defines the interface of the avgValue type.
 *
 * The v_avgValue type defines average value objects.
 * Objects are set to zero by construction and values can
 * be added to the average by v_avgValueSetValue().
 * The v_avgValueGetValue() method will return the actual average value.
 * Objects can be reused by resetting the value using the v_avgValueReset
 * method.
 */


#include "kernelModuleI.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void
v_avgValueInit (
    v_avgValue *_this);

OS_API void
v_avgValueReset (
    v_avgValue *_this);

OS_API c_float
v_avgValueGetValue (
    v_avgValue *_this);

OS_API void
v_avgValueSetValue (
    v_avgValue *_this,
    c_ulong value);

/* Option:
 * OS_API os_timeM  v_avgValueGetTime    (v_avgValue *_this);
 */

#undef OS_API

#endif
