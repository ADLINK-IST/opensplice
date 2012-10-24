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
#ifndef V_NETWORKQUEUESTATISTICS_H
#define V_NETWORKQUEUESTATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_networkQueueStatistics</code> cast method.
 *
 * This method casts an object to a <code>v_networkQueueStatistics</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networkQueueStatistics</code> or
 * one of its subclasses.
 */
#define v_networkQueueStatistics(_this) (C_CAST(_this,v_networkQueueStatistics))
#define v_networkQueueStatisticsAdd(fieldName, entity) \
		if (entity) { 								   \
			entity->fieldName++; 					   \
		}
#define v_networkQueueStatisticsCounterInc(fieldName, entity) \
		if (entity) { 								   		  \
			v_fullCounterValueInc(&(entity->fieldName));	  \
		}
#define v_networkQueueStatisticsCounterDec(fieldName, entity) \
		if (entity) { 								   		  \
        v_fullCounterValueDec(&(entity->fieldName));		  \
		}

OS_API v_networkQueueStatistics
v_networkQueueStatisticsNew(
    v_kernel k, const c_char *name);

OS_API void
v_networkQueueStatisticsInit(
    v_networkQueueStatistics _this, c_string name);
    
OS_API void
v_networkQueueStatisticsDeinit(
    v_networkQueueStatistics _this);
    
OS_API void
v_networkQueueStatisticsFree(
    v_networkQueueStatistics _this);

OS_API c_bool
v_networkQueueStatisticsReset(
    v_networkQueueStatistics _this,
    const c_char *fieldName);

#undef OS_API

#endif
