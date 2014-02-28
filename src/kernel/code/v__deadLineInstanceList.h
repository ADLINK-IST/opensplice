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
#ifndef V__DEADLINEINSTANCE_H
#define V__DEADLINEINSTANCE_H

#include "v_time.h"
#include "v_leaseManager.h"
#include "v_status.h"
#include "v_public.h"
#include "v_instance.h"

#define v_deadLineInstanceList(o) \
        (C_CAST(o,v_deadLineInstanceList))

v_deadLineInstanceList
v_deadLineInstanceListNew(
    c_base base,
    v_leaseManager leaseManager,
    v_duration leaseDuration,
    v_leaseActionId actionId,
    v_public o);

void
v_deadLineInstanceListFree(
    v_deadLineInstanceList _this);

void
v_deadLineInstanceListSetDuration(
    v_deadLineInstanceList _this,
    v_duration duration);

void
v_deadLineInstanceListInsertInstance(
    v_deadLineInstanceList _this,
    v_instance instance,
    c_time timestamp);

void
v_deadLineInstanceListRemoveInstance(
    v_deadLineInstanceList _this,
    v_instance instance);

void
v_deadLineInstanceListUpdate(
    v_deadLineInstanceList _this,
    v_instance instance,
    c_time timestamp);

c_iter
v_deadLineInstanceListCheckDeadlineMissed(
    v_deadLineInstanceList _this,
    v_duration deadlineTime,
    c_time now);

c_bool
v_deadLineInstanceListEmpty(
    v_deadLineInstanceList _this);

#endif
