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
#ifndef V__DATAVIEW_H
#define V__DATAVIEW_H

#include "v_dataView.h"
#include "v_dataReader.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define v_dataViewKeyList(_this) \
        c_tableKeyList(v_dataView(_this)->instances)

#define v_dataViewReader(_this) \
        v_dataReader(v_dataView(_this)->reader)

#define v_dataViewLock(_this) \
        v_observerLock(v_dataViewReader(_this))

#define v_dataViewUnlock(_this) \
        v_observerUnlock(v_dataViewReader(_this))

/* The trigger-value stores a sample but needs access to the unreferenced
 * instance-pointer of the sample, which thus needs to be explicitly kept and
 * freed.
 *
 * This macro returns the parameter sample
 *
 * @return sample */
#define v_dataViewTriggerValueKeep(sample) \
        (c_keep(v_readerSample(sample)->instance), \
         c_keep(sample))

/* The sample stored in the trigger-value has its (otherwise unreferenced)
 * instance-pointer explicitly kept, so it has to be freed. */
#define v_dataViewTriggerValueFree(triggerValue)            \
    {                                                       \
        v_dataViewInstance instance;                        \
                                                            \
        assert(C_TYPECHECK(triggerValue, v_dataViewSample));\
        instance = v_readerSample(triggerValue)->instance;  \
        c_free(triggerValue);                               \
        c_free(instance);                                   \
    }

void
v_dataViewDeinit(
    v_dataView _this);
                     
void
v_dataViewFreeUnsafe(
    v_dataView _this);

void
v_dataViewWipeSamples(
    v_dataView _this);

void
v_dataViewNotifyDataAvailable(
    v_dataView _this,
    v_dataViewSample sample);

c_type
dataViewSampleTypeNew(
    v_dataReader dataReader);

c_type
dataViewInstanceTypeNew(
    v_kernel kernel,
    c_type viewSampleType);

#if defined (__cplusplus)
}
#endif

#endif
