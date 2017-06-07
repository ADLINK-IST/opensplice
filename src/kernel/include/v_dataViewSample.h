/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

#ifdef OSPL_BUILD_CORE
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

#define v_dataViewSampleState(_this) \
        (v_readerSample(_this)->sampleState)

#define v_dataViewSampleTestState(_this,mask) \
        v_stateTest(v_dataViewSampleState(_this),mask)

#define  v_dataViewSampleInstance(o) \
         ((v_dataViewInstance)(v_readerSampleInstance(v_readerSample(o))))

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
