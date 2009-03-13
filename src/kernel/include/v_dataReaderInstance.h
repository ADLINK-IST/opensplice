#ifndef V_DATAREADERINSTANCE_H
#define V_DATAREADERINSTANCE_H

/** \file kernel/include/v_dataReaderInstance.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_state.h"
#include "v_dataReader.h"
#include "v_dataReaderSample.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_dataReaderInstance</code> cast method.
 *
 * This method casts an object to a <code>v_dataReaderInstance</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataReaderInstance</code> or
 * one of its subclasses.
 */
#define v_dataReaderInstance(o) (C_CAST(o,v_dataReaderInstance))

#define v_dataReaderInstanceTemplate(o) ((v_dataReaderInstanceTemplate)(o))

#define v_dataReaderInstanceHead(_this) \
        v_dataReaderSample(v_dataReaderInstanceTemplate(_this)->sample)

#define v_dataReaderInstanceTail(_this) \
        v_dataReaderSample(v_dataReaderInstanceTemplate(_this)->tail)

/* Getter functions for the instance state */

#define v_dataReaderInstanceState(instance) \
        (v_dataReaderInstance(instance)->instanceState)
    
#define v_dataReaderInstanceStateTest(instance, state)    \
        (v_stateTest(v_dataReaderInstanceState(instance), state))

#define v_dataReaderInstanceSampleCount(i) \
        (v_dataReaderInstance(i)->sampleCount)

#define v_dataReaderInstanceEmpty(i) \
        ((v_dataReaderInstanceSampleCount(i) == 0) && \
         (!v_dataReaderInstanceStateTest(i, L_STATECHANGED)))

#define v_dataReaderInstanceNoWriters(i) \
        (v_dataReaderInstanceStateTest(i, L_NOWRITERS))

#undef OS_API

#endif


