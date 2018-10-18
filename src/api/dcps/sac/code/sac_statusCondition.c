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

#include "sac_statusCondition.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_condition.h"
#include "u_statusCondition.h"
#include "sac_report.h"

#define DDS_StatusConditionClaim(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_STATUSCONDITION, (_Object *)condition)

#define DDS_StatusConditionClaimRead(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_STATUSCONDITION, (_Object *)condition)

#define DDS_StatusConditionRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_StatusConditionCheck(_this, condition) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_STATUSCONDITION, (_Object *)condition)

static DDS_boolean
_StatusConditionGetTriggerValue (
    _Condition condition)
{
    c_ulong triggerValue;
    u_statusCondition uCondition;

    triggerValue = 0;
    if (condition != NULL) {
        uCondition = u_statusCondition(_Condition_get_user_object(condition));
        if (uCondition != NULL) {
            u_statusCondition_get_triggerValue(uCondition, &triggerValue);
        }
    }
    return (triggerValue > 0);
}

static DDS_ReturnCode_t
_StatusCondition_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;
    _StatusCondition sc;

    sc = _StatusCondition(_this);
    if (sc != NULL) {
        _Condition_deinit(_this);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "StatusCondition = NULL");
    }
    return result;
}

DDS_StatusCondition
DDS_StatusConditionNew (
    DDS_Entity entity)
{
    DDS_ReturnCode_t result;
    _StatusCondition _this = NULL;
    u_entity uEntity;
    u_statusCondition uCondition;

    if (entity != NULL) {
        uEntity = _Entity_get_user_entity(entity);
        uCondition = u_statusConditionNew(uEntity);
        if (uCondition != NULL) {
            result = DDS_Object_new(DDS_STATUSCONDITION,
                                    _StatusCondition_deinit,
                                    (_Object *)&_this);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Condition_init(_this,
                                            u_object(uCondition),
                                            _StatusConditionGetTriggerValue);
            }   
            if (result == DDS_RETCODE_OK) {
                _this->enabledStatusMask = DDS_STATUS_MASK_ANY;
                _this->entity = entity;
            }
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Entity = NULL");
    }
    return (DDS_StatusCondition)_this;
}


/*     StatusKindMask
 *     get_enabled_statuses();
 */
DDS_StatusMask
DDS_StatusCondition_get_enabled_statuses (
    DDS_StatusCondition _this)
{
    DDS_ReturnCode_t result;
    _StatusCondition sc;
    DDS_StatusMask mask;

    SAC_REPORT_STACK();

    result = DDS_StatusConditionCheck(_this, &sc);
    if (result == DDS_RETCODE_OK) {
        mask = sc->enabledStatusMask;
    } else {
        mask = 0;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return mask;
}

/*     ReturnCode_t
 *     set_enabled_statuses(
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_StatusCondition_set_enabled_statuses (
    DDS_StatusCondition _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _StatusCondition sc;
    u_statusCondition uCondition;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_StatusConditionClaim(_this, &sc);
    if (result == DDS_RETCODE_OK) {
        sc->enabledStatusMask = mask;
        uCondition = u_statusCondition(_Condition_get_user_object(_Condition(sc)));
        if (uCondition != NULL) {
            uResult = u_statusCondition_set_mask(uCondition, DDS_StatusMask_get_eventMask(mask));
            result = DDS_ReturnCode_get(uResult);
        }
        DDS_StatusConditionRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     Entity
 *     get_entity();
 */
DDS_Entity
DDS_StatusCondition_get_entity (
    DDS_StatusCondition _this)
{
    DDS_ReturnCode_t result;
    _StatusCondition sc;
    DDS_Entity entity = NULL;

    SAC_REPORT_STACK();

    result = DDS_StatusConditionCheck(_this, &sc);
    if (result == DDS_RETCODE_OK) {
        entity = sc->entity;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return entity;
}
