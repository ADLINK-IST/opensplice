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
#ifndef V_READERSAMPLE_H
#define V_READERSAMPLE_H

#include "v_kernel.h"
#include "v_lifespanSample.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_readerSample</code> cast method.
 *
 * This method casts an object to a <code>v_readerSample</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_readerSample</code> or
 * one of its subclasses.
 */
#define v_readerSample(o) \
        (C_CAST(o,v_readerSample))

#define v_readerSampleState(_this) \
        (v_readerSample(_this)->sampleState)

#define v_readerSampleSetState(_this,mask) \
        v_stateSet(v_readerSampleState(_this),mask)

#define v_readerSampleTestState(_this,mask) \
        v_stateTest(v_readerSampleState(_this),mask)

#define v_readerSampleTestStateOr(_this,mask) \
        v_stateTestOr(v_readerSampleState(_this),mask)

#define v_readerSampleClearState(_this,mask) \
        v_stateClear(v_readerSampleState(_this),mask)

#define v_readerSampleInstance(_this) \
        (v_readerSample(_this)->instance)

typedef c_bool
(*v_readerSampleAction)(
    v_readerSample _this,
    c_voidp arg);

#undef OS_API

#endif
