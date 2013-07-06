/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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

v_result
v_dataViewSetQos(
    v_dataView _this,
    v_dataViewQos qos);

void
v_dataViewWipeSamples(
    v_dataView _this);

void
v_dataViewNotifyDataAvailable(
    v_dataView _this,
    v_dataViewSample sample);

#if defined (__cplusplus)
}
#endif

#endif
