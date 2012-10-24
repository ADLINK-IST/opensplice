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
#ifndef V_WRITERSTATISTICS_H
#define V_WRITERSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
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

OS_API c_bool
v_writerStatisticsReset(
    v_writerStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
