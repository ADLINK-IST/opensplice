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
#ifndef V_WRITERSTATISTICS_H
#define V_WRITERSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_writerStatistics</code> cast methods.
 *
 * This method casts an object to a <code>v_writerStatistics</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_writerStatistics</code> or
 * one of its subclasses.
 */
#define v_writerStatistics(_this) (C_CAST(_this,v_writerStatistics))

OS_API v_writerStatistics
v_writerStatisticsNew(
    v_kernel k);

OS_API void
v_writerStatisticsInit(
    v_writerStatistics _this);

OS_API void
v_writerStatisticsDeinit(
    v_writerStatistics _this);

OS_API void
v_writerStatisticsFree(
    v_writerStatistics _this);

#undef OS_API

#endif
