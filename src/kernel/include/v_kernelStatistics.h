/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef V_KERNELSTATISTICS_H
#define V_KERNELSTATISTICS_H

/** \file kernel/include/v_kernelStatistics.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_kernelStatistics(s) \
        (C_CAST(s,v_kernelStatistics))

OS_API v_kernelStatistics
v_kernelStatisticsNew(
    v_kernel k);

OS_API void
v_kernelStatisticsInit(
    v_kernelStatistics _this);

OS_API void
v_kernelStatisticsDeinit(
    v_kernelStatistics _this);

OS_API void
v_kernelStatisticsFree(
    v_kernelStatistics _this);

OS_API c_bool
v_kernelStatisticsReset(
    v_kernelStatistics _this, 
    const c_char * fieldName);

OS_API c_type
v_kernelStatisticsCachedType(
    void);

#undef OS_API

#endif
