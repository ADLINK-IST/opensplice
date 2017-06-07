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
#ifndef V_RNRGROUPSTATISTICS_H
#define V_RNRGROUPSTATISTICS_H

#include "v_rnr.h"
#include "v_rnrStorageStatistics.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_rnrGroupStatistics(_this) (C_CAST(_this, v_rnrGroupStatistics))

OS_API v_rnrGroupStatistics
v_rnrGroupStatisticsNew(
    v_kernel k,
    const c_char *name);

OS_API void
v_rnrGroupStatisticsInit(
    v_rnrGroupStatistics _this,
    c_string name);

OS_API void
v_rnrGroupStatisticsDeinit(
    v_rnrGroupStatistics _this);

OS_API void
v_rnrGroupStatisticsFree(
    v_rnrGroupStatistics _this);

OS_API void
v_rnrGroupStatisticsReset(
    v_rnrGroupStatistics _this,
    c_string fieldName);

#undef OS_API
#endif /* V_RNRGROUPSTATISTICS_H */
