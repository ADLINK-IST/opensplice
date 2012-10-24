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
