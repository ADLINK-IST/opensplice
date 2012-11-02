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
#ifndef V_DURABILITYSTATISTICS_H
#define V_DURABILITYSTATISTICS_H

/** \file kernel/include/v_durabilityStatistics.h
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

/**
 * \brief The <code>v_durabilityStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_durabilityStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_durabilityStatistics</code> or
 * one of its subclasses.
 */
#define v_durabilityStatistics(_this) (C_CAST(_this,v_durabilityStatistics))

OS_API v_durabilityStatistics
v_durabilityStatisticsNew(
    v_kernel k);

OS_API void
v_durabilityStatisticsInit(
    v_durabilityStatistics _this);

OS_API void
v_durabilityStatisticsDeinit(
    v_durabilityStatistics _this);

OS_API void
v_durabilityStatisticsFree(
    v_durabilityStatistics _this);

OS_API c_bool
v_durabilityStatisticsReset(
    v_durabilityStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
