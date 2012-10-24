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
#ifndef V_NETWORKCHANNELSTATISTICS_H
#define V_NETWORKCHANNELSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_networkChannelStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_networkChannelStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networkChannelStatistics</code> or
 * one of its subclasses.
 */
#define v_networkChannelStatistics(_this) (C_CAST(_this,v_networkChannelStatistics))
#define v_networkChannelStatisticsAdd(fieldName, entity) \
        entity->fieldName++;
#define v_networkChannelStatisticsDec(fieldName, entity) \
        entity->fieldName--;
#define v_networkChannelStatisticsMaxValueSet(fieldName, entity) \
        v_maxValueSetValue(&(entity->fieldName));

OS_API v_networkChannelStatistics
v_networkChannelStatisticsNew(
    v_kernel k, const c_char *name);

OS_API void
v_networkChannelStatisticsInit(
    v_networkChannelStatistics _this,  c_string name);
    
OS_API void
v_networkChannelStatisticsDeinit(
    v_networkChannelStatistics _this);
    
OS_API void
v_networkChannelStatisticsFree(
    v_networkChannelStatistics _this);

OS_API c_bool
v_networkChannelStatisticsReset(
    v_networkChannelStatistics _this);

#undef OS_API

#endif
