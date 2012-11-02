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

#define v_dataReaderInstanceOldest(_this) \
        v_dataReaderSample(v_dataReaderInstanceTemplate(_this)->oldest)

#define v_dataReaderInstanceNewest(_this) \
        v_dataReaderSample(v_dataReaderInstanceTemplate(_this)->sample)

/* Getter functions for the instance state */

#define v_dataReaderInstanceState(_this) \
        (v_dataReaderInstance(_this)->instanceState)

#define v_dataReaderInstanceStateTest(_this, state)    \
        (v_stateTest(v_dataReaderInstanceState(_this), state))

#define v_dataReaderInstanceStateTestOr(_this, state)    \
        (v_stateTestOr(v_dataReaderInstanceState(_this), state))

#define v_dataReaderInstanceSampleCount(_this) \
        (v_dataReaderInstance(_this)->sampleCount)

#if 0
#define v_dataReaderInstanceEmpty(_this) \
        ((v_dataReaderInstanceSampleCount(_this) == 0) && \
         (!v_dataReaderInstanceStateTest(_this, L_STATECHANGED)))
#else
#define v_dataReaderInstanceEmpty(_this) \
        (v_dataReaderInstanceOldest(_this) == NULL)
#endif

#define v_dataReaderInstanceNoWriters(_this) \
        (v_dataReaderInstance(_this)->liveliness == 0)

#define v_dataReaderInstanceReader(_this) \
        (v_dataReader(v_index(v_dataReaderInstance(_this)->index)->reader))

/*
 * Functions to set and get the datareaderInstance userdata field
 */
OS_API c_voidp v_dataReaderInstanceGetUserData
						(v_dataReaderInstance _this);

OS_API void v_dataReaderInstanceSetUserData
						(v_dataReaderInstance _this, c_voidp userDataDataReaderInstance);

/*
 * Function to create a new message (with instance key(s) filled-in)
 */
OS_API v_message v_dataReaderInstanceCreateMessage
                        (v_dataReaderInstance _this);

/*
 * Function to get the datareaderInstance index->notEmptyList field
 */
OS_API c_collection v_dataReaderInstanceGetNotEmptyInstanceSet
                        (v_dataReaderInstance _this);

/*
 * Function to get the datareaderInstance index->notEmptyList count
 */
OS_API c_ulong v_dataReaderInstanceGetNotEmptyInstanceCount
                        (v_dataReaderInstance _this);


#undef OS_API

#endif


