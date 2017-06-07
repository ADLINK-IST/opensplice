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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_condition.h"
#include "sac_report.h"

#define DDS_GuardConditionClaim(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_GUARDCONDITION, (_Object *)condition)

#define DDS_GuardConditionRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

static DDS_ReturnCode_t
_GuardCondition_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    _GuardCondition guard;

    guard = _GuardCondition(_this);
    if (guard != NULL) {
        _Condition_deinit(_this);
        result = DDS_RETCODE_OK;
    }
    return result;
}

static DDS_boolean
_GuardConditionGetTriggerValue (
    _Condition condition)
{
    DDS_boolean result = FALSE;

    if (condition != NULL) {
        result = ((_GuardCondition)condition)->triggerValue;
    }
    return result;
}

/*     ReturnCode_t
 *     set_trigger_value(
 *         in boolean value);
 */
DDS_ReturnCode_t
DDS_GuardCondition_set_trigger_value (
    DDS_GuardCondition _this,
    const DDS_boolean value)
{
    DDS_ReturnCode_t result;
    _GuardCondition guard;

    SAC_REPORT_STACK();

    result = DDS_GuardConditionClaim(_this, &guard);
    if (result == DDS_RETCODE_OK) {
        guard->triggerValue = value;
        DDS_GuardConditionRelease(_this);
        DDS_Condition_trigger(DDS_Condition(_this));
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     GuardCondition
 *     GuardCondition__alloc 
 *         void);
 */
DDS_GuardCondition
DDS_GuardCondition__alloc (
    void)
{
    DDS_ReturnCode_t result;
    _GuardCondition guard;

    result = DDS_Object_new(DDS_GUARDCONDITION, _GuardCondition_deinit, (_Object *)&guard);
    if (result == DDS_RETCODE_OK) {
        result = DDS_Condition_init(guard, NULL, _GuardConditionGetTriggerValue);
    }
    if (result == DDS_RETCODE_OK) {
        guard->triggerValue = FALSE;
    }
    return guard;
}
