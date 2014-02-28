/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
