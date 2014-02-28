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
#ifndef V_RNRSTORAGESTATISTICS_H
#define V_RNRSTORAGESTATISTICS_H

#include "v_kernel.h"
#include "v_rnrGroupStatistics.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
#define v_rnrStorageStatistics(_this) (C_CAST(_this, v_rnrStorageStatistics))

OS_API v_rnrStorageStatistics
v_rnrStorageStatisticsNew(
    v_kernel k,
    const c_char *name);

OS_API void
v_rnrStorageStatisticsInit(
    v_rnrStorageStatistics _this,
    v_kernel k,
    c_string name);

OS_API void
v_rnrStorageStatisticsDeinit(
    v_rnrStorageStatistics _this);

OS_API void
v_rnrStorageStatisticsFree(
    v_rnrStorageStatistics _this);

OS_API void
v_rnrStorageStatisticsReset(
    v_rnrStorageStatistics _this,
    c_string fieldName);

OS_API v_rnrGroupStatistics
v_rnrStorageStatisticsGroup(
        v_rnrStorageStatistics _this,
        v_service service,
        const c_char* name);

#undef OS_API
#endif /* V_RNRSTORAGESTATISTICS_H */
