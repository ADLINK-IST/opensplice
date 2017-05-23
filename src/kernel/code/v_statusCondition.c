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
#include "v_statusCondition.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_public.h"
#include "v_proxy.h"
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
            v_observerSetEventData(v_observer(_this), NULL);
            v_observableAddObserver(v_observable(entity), v_observer(_this), NULL);
            _this->entity = v_proxyNew(kernel, v_publicHandle(v_public(entity)), NULL);
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

    result = v_handleClaim(_this->entity->source, (v_object *)&obs);
    if (result == V_HANDLE_OK) {
        (void)v_observableRemoveObserver(obs, v_observer(_this), NULL);
        (void)v_handleRelease(_this->entity->source);
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
    v_observerSetEventMask(v_observer(_this), mask);

    /* TODO: the following line is a shortcut to copy the trigger value from the status.
     * This is needed because the condition is actually true but no event will be generated to report this.
     * IMO we need to have a closer look to the whole event propagation in the near future.
     */

    result = v_handleClaim(_this->entity->source, (v_object *)&entity);
    if (result == V_HANDLE_OK) {
        v_observer(_this)->eventFlags = mask & v_entityGetTriggerValue(entity);
        (void)v_handleRelease(_this->entity->source);
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

    result = v_handleClaim(_this->entity->source, (v_object *)&entity);
    if (result == V_HANDLE_OK) {
#if 0
        triggerValue = v_observerGetEventMask(v_observer(_this));
#else
        /* test doing without locking */
        triggerValue = v_observerEventMask(v_observer(_this));
#endif

#if 1
        /* Sync all flags with the flags of the entity to which the condition is attached */
        triggerValue &= v_statusGetMask(entity->status);
#else
        /* The status condition may indicate Data Availability even if the data is already taken.
         * To avoid spurious wakeups verify if Data Availability is still set on the entity itself.
         */
        if ((V_EVENT_DATA_AVAILABLE & v_entityGetTriggerValue(entity)) == 0) {
            triggerValue &= ~(V_EVENT_DATA_AVAILABLE);
        }
#endif
        (void)v_handleRelease(_this->entity->source);
    }

    return triggerValue;
}

