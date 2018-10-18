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
#ifndef U_DATAVIEW_H
#define U_DATAVIEW_H

#include "u_types.h"
#include "u_reader.h"
#include "u_dataReader.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

#define u_dataView(o) \
        ((u_dataView)u_objectCheckType(u_object(o),U_DATAVIEW))

OS_API u_dataView
u_dataViewNew(
    const u_dataReader r,
    const os_char *name,
    const u_dataViewQos qos);

OS_API u_result
u_dataViewGetQos (
    const u_dataView _this,
    u_dataViewQos *qos);

OS_API u_result
u_dataViewSetQos (
    const u_dataView _this,
    const u_dataViewQos qos);

OS_API u_result
u_dataViewRead(
    const u_dataView _this,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_dataViewTake(
    const u_dataView _this,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_dataViewReadInstance(
    const u_dataView _this,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_dataViewTakeInstance(
    const u_dataView _this,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_dataViewReadNextInstance(
    const u_dataView _this,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_dataViewTakeNextInstance(
    const u_dataView _this,
    u_instanceHandle h,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_dataViewLookupInstance(
    const u_dataView _this,
    void *keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
