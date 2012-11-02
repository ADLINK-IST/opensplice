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
#ifndef V_NETWORKINGSTATISTICS_H
#define V_NETWORKINGSTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_networkingStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_networkingStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networkingStatistics</code> or
 * one of its subclasses.
 */
#define v_networkingStatistics(_this) (C_CAST(_this,v_networkingStatistics))

OS_API v_networkingStatistics
v_networkingStatisticsNew(
    v_kernel k);

OS_API void
v_networkingStatisticsInit(
    v_networkingStatistics _this, v_kernel k);

OS_API void
v_networkingStatisticsDeinit(
    v_networkingStatistics _this);

OS_API void
v_networkingStatisticsFree(
    v_networkingStatistics _this);

OS_API c_bool
v_networkingStatisticsReset(
    v_networkingStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
