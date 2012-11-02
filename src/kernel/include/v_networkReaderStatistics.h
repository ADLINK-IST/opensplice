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
#ifndef V_NETWORKREADERSTATISTICS_H
#define V_NETWORKREADERSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_networkReaderStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_networkReaderStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networkReaderStatistics</code> or
 * one of its subclasses.
 */
#define v_networkReaderStatistics(_this) (C_CAST(_this,v_networkReaderStatistics))

OS_API v_networkReaderStatistics
v_networkReaderStatisticsNew(
    v_kernel k);

OS_API void
v_networkReaderStatisticsInit(
    v_networkReaderStatistics _this);
    
OS_API void
v_networkReaderStatisticsDeinit(
    v_networkReaderStatistics _this);
    
OS_API void
v_networkReaderStatisticsFree(
    v_networkReaderStatistics _this);

OS_API c_bool
v_networkReaderStatisticsReset(
    v_networkReaderStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
