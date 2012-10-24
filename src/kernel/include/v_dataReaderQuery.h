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
#ifndef V_DATAREADERQUERY_H
#define V_DATAREADERQUERY_H

/** \file kernel/include/v_dataReaderQuery.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_event.h"
#include "v_dataReader.h"
#include "v_dataReaderSample.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_dataReaderQuery</code> cast method.
 *
 * This method casts an object to a <code>v_dataReaderQuery</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataReaderQuery</code> or
 * one of its subclasses.
 */
#define v_dataReaderQuery(q) (C_CAST(q,v_dataReaderQuery))

OS_API v_dataReaderQuery
v_dataReaderQueryNew(
    v_dataReader r,
    const c_char *name,
    q_expr predicate,
    c_value params[]);

OS_API void
v_dataReaderQueryFree(
    v_dataReaderQuery _this);

OS_API c_bool
v_dataReaderQueryTest(
    v_dataReaderQuery _this);

OS_API c_bool
v_dataReaderQueryTriggerTest(
    v_dataReaderQuery _this);

OS_API c_bool
v_dataReaderQueryRead(
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderQueryTake(
    v_dataReaderQuery _this,
    v_readerSampleAction action,
     c_voidp arg);

OS_API c_bool
v_dataReaderQueryReadInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderQueryTakeInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderQueryReadNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderQueryTakeNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API void
v_dataReaderQueryDeinit(
    v_dataReaderQuery query);

c_bool
v_dataReaderQueryNotifyDataAvailable(
    v_dataReaderQuery _this,
    v_event e);

OS_API c_bool
v_dataReaderQuerySetParams(
    v_dataReaderQuery _this,
    q_expr predicate,
    c_value params[]);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif
