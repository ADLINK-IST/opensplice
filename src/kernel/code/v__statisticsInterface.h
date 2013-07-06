/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V__STATISTICSINTERFACE_H
#define V__STATISTICSINTERFACE_H

#include "v_statisticsInterface.h"
#include "v_maxValue.h"
#include "v_fullCounter.h"


/* Macro for ULong statistics values */

#define v_statisticsULongSetValue(entityName, fieldName, entity, value) \
    if (v_statisticsValid(entity)) \
        *v_statisticsGetRef(entityName, fieldName, entity) = value

#define v_statisticsULongInit(entityName, fieldName, entity) \
    v_statisticsULongSetValue(entityName, fieldName, entity, 0)

#define v_statisticsULongValueInc(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        (*v_statisticsGetRef(entityName, fieldName, entity))++

#define v_statisticsULongValueAdd(entityName, fieldName, entity, value) \
    if (v_statisticsValid(entity)) \
        (*v_statisticsGetRef(entityName, fieldName, entity)) += value;

#define v_statisticsULongValueDec(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        (*v_statisticsGetRef(entityName, fieldName, entity))--

#define v_statisticsULongValueSub(entityName, fieldName, entity, value) \
    if (v_statisticsValid(entity)) \
        (*v_statisticsGetRef(entityName, fieldName, entity)) -= value;

/* Macros for MaxValue statistics values */

#define v_statisticsMaxValueInit(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        v_maxValueInit(v_statisticsGetRef(entityName, fieldName, entity))

#define v_statisticsMaxValueSetValue(entityName, fieldName, entity, value) \
    if (v_statisticsValid(entity)) \
        v_maxValueSetValue(v_statisticsGetRef(entityName, fieldName, entity), value)

/* Macros for FullCounter statistics values */

#define v_statisticsFullCounterInit(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        v_fullCounterInit(v_statisticsGetRef(entityName, fieldName, entity))

#define v_statisticsFullCounterSetValue(entityName, fieldName, entity, value) \
    if (v_statisticsValid(entity)) \
        v_fullCounterSetValue(v_statisticsGetRef(entityName, fieldName, entity), value)

#define v_statisticsFullCounterValuePlus(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        v_fullCounterValueInc(v_statisticsGetRef(entityName, fieldName, entity))

#define v_statisticsFullCounterValueMin(entityName, fieldName, entity) \
    if (v_statisticsValid(entity)) \
        v_fullCounterValueDec(v_statisticsGetRef(entityName, fieldName, entity))


/* Temporary helper macro which has to be removed at the moment that all
 * statistics filling is done via the macro's above */

 #define v_statisticsULongResetInternal(entityName, fieldName, statistics) \
    if (statistics && v_statisticsResettable(entityName, fieldName)) \
        statistics->fieldName = 0

#endif /*V__STATISTICSINTERFACE_H*/
