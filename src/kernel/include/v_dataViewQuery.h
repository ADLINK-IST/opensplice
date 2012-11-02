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
#ifndef V_DATAVIEWQUERY_H
#define V_DATAVIEWQUERY_H

/** \file kernel/include/v_dataViewQuery.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_event.h"
#include "v_dataView.h"
#include "v_dataViewSample.h"

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
 * \brief The <code>v_dataViewQuery</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewQuery</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewQuery</code> or
 * one of its subclasses.
 */
#define v_dataViewQuery(q) (C_CAST(q,v_dataViewQuery))

OS_API v_dataViewQuery
v_dataViewQueryNew(
    v_dataView view,
    const c_char *name,
    q_expr predicate,
    c_value params[]);

OS_API void
v_dataViewQueryFree(
    v_dataViewQuery _this);

OS_API c_bool
v_dataViewQueryTest(
    v_dataViewQuery _this);

OS_API c_bool
v_dataViewQueryRead(
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataViewQueryTake(
    v_dataViewQuery _this,
    v_readerSampleAction action,
     c_voidp arg);

OS_API c_bool
v_dataViewQueryReadInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataViewQueryTakeInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataViewQueryReadNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataViewQueryTakeNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg);

OS_API void
v_dataViewQueryDeinit(
    v_dataViewQuery _this);

OS_API c_bool
v_dataViewQueryNotifyDataAvailable(
    v_dataViewQuery _this,
    v_event e);

OS_API c_bool
v_dataViewQuerySetParams(
    v_dataViewQuery _this,
    q_expr predicate,
    c_value params[]);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
