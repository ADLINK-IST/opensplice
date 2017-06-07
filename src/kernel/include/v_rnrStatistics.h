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
#ifndef V_RNRSTATISTICS_H
#define V_RNRSTATISTICS_H

/** \file kernel/include/v_rnrStatistics.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_rnrStorageStatistics.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_rnrStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_rnrStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_rnrStatistics</code> or
 * one of its subclasses.
 */
#define v_rnrStatistics(_this) (C_CAST(_this,v_rnrStatistics))

OS_API v_rnrStatistics
v_rnrStatisticsNew(
    v_kernel k,
    const c_char *name);

OS_API void
v_rnrStatisticsInit(
    v_rnrStatistics _this,
    v_kernel kernel,
    c_string name);

OS_API void
v_rnrStatisticsDeinit(
    v_rnrStatistics _this);

OS_API void
v_rnrStatisticsFree(
    v_rnrStatistics _this);

OS_API c_bool
v_rnrStatisticsReset(
    v_rnrStatistics _this,
    const c_char *fieldName);

OS_API v_rnrStorageStatistics
v_rnrStatisticsStorageStatistics(
    v_rnrStatistics _this,
    v_service service,
    const c_char* storageName);

#undef OS_API

#endif /* V_RNRSTATISTICS_H */
