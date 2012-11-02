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
#ifndef V_QUERYSTATISTICS_H
#define V_QUERYSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_queryStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_queryStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_queryStatistics</code> or
 * one of its subclasses.
 */
#define v_queryStatistics(s) (C_CAST(s,v_queryStatistics))

OS_API v_queryStatistics
v_queryStatisticsNew(
    v_kernel k);

OS_API void
v_queryStatisticsInit(
    v_queryStatistics _this);

OS_API void
v_queryStatisticsDeinit(
    v_queryStatistics _this);

OS_API void
v_queryStatisticsFree(
    v_queryStatistics _this);

OS_API c_bool
v_queryStatisticsReset(
    v_queryStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
