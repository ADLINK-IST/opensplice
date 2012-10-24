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
#ifndef V_DATAVIEW_H
#define V_DATAVIEW_H

/** \file kernel/include/v_dataView.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_reader.h"
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
 * \brief The <code>v_dataView</code> cast method.
 *
 * This method casts an object to a <code>v_dataView</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataView</code> or
 * one of its subclasses.
 */
#define  v_dataView(o) (C_CAST(o,v_dataView))

OS_API v_dataView 
v_dataViewNew(
    v_dataReader reader,
    const c_char *name,
    v_dataViewQos qos,
    c_bool enable);

OS_API void
v_dataViewFree(
    v_dataView dataView);

OS_API v_dataReader     
v_dataViewGetReader(
    v_dataView dataView);

OS_API c_bool           
v_dataViewWrite(
    v_dataView dataView,
    v_readerSample sample);

OS_API c_bool           
v_dataViewRead(
    v_dataView dataView,
    v_readerSampleAction action,
    c_voidp arg);
                     
OS_API c_bool           
v_dataViewTake(
    v_dataView dataView,
    v_readerSampleAction action,
    c_voidp arg);
                                         
OS_API c_bool
v_dataViewReadInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg);
                 
OS_API c_bool           
v_dataViewTakeInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg);
                                         
OS_API c_bool           
v_dataViewReadNextInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg);
                     
OS_API c_bool           
v_dataViewTakeNextInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

OS_API v_dataViewInstance
v_dataViewLookupInstance (
    v_dataView view, 
    v_message keyTemplate);

OS_API c_bool
v_dataViewContainsInstance (
    v_dataView view,
    v_dataViewInstance instance);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
