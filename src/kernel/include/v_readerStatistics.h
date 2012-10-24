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
#ifndef V_READERSTATISTICS_H
#define V_READERSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_readerStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_readerStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_readerStatistics</code> or
 * one of its subclasses.
 */
#define v_readerStatistics(s) (C_CAST(s,v_readerStatistics))

OS_API v_readerStatistics
v_readerStatisticsNew(
    v_kernel k);

OS_API void
v_readerStatisticsInit(
    v_readerStatistics _this);

OS_API void
v_readerStatisticsDeinit(
    v_readerStatistics _this);

OS_API void
v_readerStatisticsFree(
    v_readerStatistics _this);

OS_API c_bool
v_readerStatisticsReset(
    v_readerStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
