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
#ifndef V_READERSAMPLE_H
#define V_READERSAMPLE_H

#include "v_kernel.h"
#include "v_lifespanSample.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
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

#define v_readerSampleTestStateNot(_this,mask) \
        v_stateTestNot(v_readerSampleState(_this),mask)

#define v_readerSampleTestStateOr(_this,mask) \
        v_stateTestOr(v_readerSampleState(_this),mask)

#define v_readerSampleClearState(_this,mask) \
        v_stateClear(v_readerSampleState(_this),mask)

#define v_readerSampleInstance(_this) \
        (v_readerSample(_this)->instance)

/* Mask definitions for results of a v_readerSampleAction routine. */
typedef c_ulong v_actionResult;

#define V_STOP             (0x0000U)      /* 0 */
#define V_PROCEED          (0x0001U << 0) /* 1 */
#define V_SKIP             (0x0001U << 1) /* 2 */

/*
 * Sets all bits in actionResult that are set in mask.
 *
 * Example:
 *      actionResult = 101101;
 *      v_actionResultSet(actionResult, 000011);
 *      actionResult == 101111;
 *
 */
#define v_actionResultSet(actionResult,mask)    ((actionResult)|=(mask))

/*
 * Clears all bits in actionResult that are set in mask.
 *
 * Example:
 *      actionResult = 101111;
 *      v_actionResultClear(actionResult, 000011);
 *      actionResult == 101100;
 *
 */
#define v_actionResultClear(actionResult,mask)  ((actionResult)&=(~mask))

/*
 * Tests whether the supplied mask is set. If all bits in the mask are set in
 * the actionResult, then the result is TRUE.
 *
 * Example:
 *      v_actionResultTest(101001, 000101) == FALSE
 *      v_actionResultTest(101001, 001001) == TRUE
 *      v_actionResultTest(101001, 101001) == TRUE
 */
#define v_actionResultTest(actionResult,mask)   (((actionResult)&(mask))==(mask))

/*
 * Tests whether the supplied mask is NOT set. If all bits in the mask are not
 * set in the actionResult, then the result is TRUE.
 *
 * Example:
 *      v_actionResultTestNot(101001, 000110) == TRUE
 *      v_actionResultTestNot(101001, 000010) == TRUE
 *      v_actionResultTestNot(101001, 001110) == FALSE
 *
 * Note that this is different than calling v_actionResultTest with a negated mask:
 *      v_actionResultTest(101001, ~000110) == FALSE
 *      v_actionResultTestNot(101001, 000110) == TRUE
 * Or negating the output of v_actionResultTest:
 *      !v_actionResultTest(101001, 001110) == TRUE
 *      v_actionResultTestNot(101001, 001110) == FALSE
 * Or negating the output of v_actionResultTest with a negated mask:
 *      !v_actionResultTest(101001, ~001110) == TRUE
 *      v_actionResultTestNot(101001, 001110) == FALSE
 */
#define v_actionResultTestNot(state,mask)(((state)&(~mask))==(state))

typedef v_actionResult
(*v_readerSampleAction)(
    c_object _this,
    c_voidp arg);

#undef OS_API

#endif
