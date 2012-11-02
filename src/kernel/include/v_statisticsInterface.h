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
#ifndef V_STATISTICSINTERFACE_H
#define V_STATISTICSINTERFACE_H

#include "v_statisticsHelpers.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* The untyped getter interface function */

#define v_statisticsGetRef(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? entityName##StatisticsGetRef(fieldName, entity) : NULL)
    
#define v_statisticsGetIndexedRef(entityName, indexedName, index, fieldName, entity) \
    (v_statisticsValid(entity) ? entityName##StatisticsGetIndexedRef(indexedName, index, fieldName, entity) : NULL)


/* Typed versions */

/* ULong */

#define v_statisticsGetULongValue(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? *v_statisticsGetRef(entityName, fieldName, entity) : 0)


/* v_maxValue */

#define v_statisticsGetMaxValue(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? v_maxValueGetValue(v_statisticsGetRef(entityName, fieldName, entity)) : 0)


/* v_fullCounter */

#define v_statisticsGetFullCounterValue(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? v_fullCounterGetValue(v_statisticsGetRef(entityName, fieldName, entity)) : 0)
    
#define v_statisticsGetFullCounterMin(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? v_fullCounterGetMin(v_statisticsGetRef(entityName, fieldName, entity)) : 0)
    
#define v_statisticsGetFullCounterMax(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? v_fullCounterGetMax(v_statisticsGetRef(entityName, fieldName, entity)) : 0)
    
#define v_statisticsGetFullCounterAvg(entityName, fieldName, entity) \
    (v_statisticsValid(entity) ? v_fullCounterGetAvg(v_statisticsGetRef(entityName, fieldName, entity)) : 0)
    

/* Units */

#define v_statisticsGetUnit(entityName, fieldName) \
    unit_##entityName##_##fieldName
    

/* Reset functions */

/* By fieldName */

#define v_statisticsULongReset(entityName, fieldName, entity) \
    if (v_statisticsValid(entity) && v_statisticsResettable(entityName, fieldName)) \
        *(v_statisticsGetRef(entityName, fieldName, entity)) = 0

#define v_statisticsMaxValueReset(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        v_maxValueReset(v_statisticsGetRef(entityName, fieldName, entity))

#define v_statisticsFullCounterReset(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        v_fullCounterReset(v_statisticsGetRef(entityName, fieldName, entity))
        
        
/* By complete entity */
 
#define v_statisticsResetAll(entityName, entity) \
    (v_statisticsValid(entity) ? \
        entityName##StatisticsReset(entityName##Statistics(v_entity(entity)->statistics), NULL) : FALSE)

#undef OS_API

#endif /*V_STATISTICSINTERFACE_H*/
