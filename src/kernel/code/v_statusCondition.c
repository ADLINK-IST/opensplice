/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "v_statusCondition.h"
#include "v_public.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v__entity.h"
#include "v__status.h"

#include "os_report.h"

v_statusCondition
v_statusConditionNew(
    v_entity entity)
{
    v_statusCondition _this = NULL;
    v_kernel kernel;

    if (entity) {
        kernel = v_objectKernel(v_object(entity));
        _this = v_statusCondition(v_objectNew(kernel,K_STATUSCONDITION));
        if (_this != NULL) {
            v_observerInit(v_observer(_this));
            OSPL_ADD_OBSERVER(entity, _this, V_EVENTMASK_ALL, NULL);
            _this->entity = v_publicHandle(v_public(entity));
        }
    }
    return _this;
}

void
v_statusConditionFree(
   v_statusCondition _this)
{
    v_observable obs;
    v_handleResult result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_statusCondition));

    result = v_handleClaim(_this->entity, (v_object *)&obs);
    if (result == V_HANDLE_OK) {
        (void)OSPL_REMOVE_OBSERVER(obs, _this, V_EVENTMASK_ALL, NULL);
        (void)v_handleRelease(_this->entity);
    }
    v_observerFree(v_observer(_this));
}

void
v_statusConditionDeinit(
   v_statusCondition _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_statusCondition));
    v_observerDeinit(v_observer(_this));
}

void
v_statusConditionSetMask(
    v_statusCondition _this,
    v_eventMask mask)
{
    v_handleResult result;
    v_entity entity;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_statusCondition));
    OSPL_SET_EVENT_MASK(_this, mask);

    result = v_handleClaim(_this->entity, (v_object *)&entity);
    if (result == V_HANDLE_OK) {
        C_STRUCT(v_event) event;
        event.kind = v_entityGetTriggerValue(entity);
        event.source = v_observable(entity);
        event.data = NULL;
        v_observerNotify(v_observer(_this), &event, NULL);
        (void)v_handleRelease(_this->entity);
    }
}

v_eventMask
v_statusConditionGetTriggerValue (
    v_statusCondition _this)
{
    v_handleResult result;
    v_entity entity;
    v_eventMask triggerValue = 0;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_statusCondition));

    result = v_handleClaim(_this->entity, (v_object *)&entity);
    if (result == V_HANDLE_OK) {
        triggerValue = v_entityGetTriggerValue(entity);
        triggerValue &= v_observerGetEventMask(v_observer(_this));
        (void)v_handleRelease(_this->entity);
    }

    return triggerValue;
}

