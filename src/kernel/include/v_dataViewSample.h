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
#ifndef V_DATAVIEWSAMPLE_H
#define V_DATAVIEWSAMPLE_H

/** \file kernel/include/v_dataViewSample.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_readerSample.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_dataViewSample</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewSample</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewSample</code> or
 * one of its subclasses.
 */
#define v_dataViewSample(_this) (C_CAST(_this,v_dataViewSample))

/**
 * \brief The <code>v_dataViewSampleList</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewSampleList</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewSampleList</code> or
 * one of its subclasses.
 */
#define v_dataViewSampleList(_this) (C_CAST(_this,v_dataViewSampleList))

#define v_dataViewSampleTemplate(_this) ((v_dataViewSampleTemplate)(_this))

OS_API v_dataViewSample
v_dataViewSampleNew (
    v_dataViewInstance instance,
    v_readerSample sample);

OS_API void
v_dataViewSampleFree (
    v_dataViewSample _this);

OS_API void
v_dataViewSampleRemove(
    v_dataViewSample _this);

OS_API void
v_dataViewSampleListRemove(
    v_dataViewSampleList _this);

#undef OS_API

#endif
