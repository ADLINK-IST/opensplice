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
#include "sac_waitSet.h"
#include "sac_report.h"

#define DDS_ConditionClaim(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_CONDITION, (_Object *)condition)

#define DDS_ConditionClaimRead(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_CONDITION, (_Object *)condition)

#define DDS_ConditionRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_ConditionCheck(_this, condition) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_CONDITION, (_Object *)condition)

DDS_ReturnCode_t
_Condition_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_WaitSet waitset;
    _Condition condition;
    u_result uResult;

    condition = _Condition(_this);
    if (condition != NULL) {

/* TODO: The following loop to remove the condition from all associated waitsets
 * should be excecuted without unlocking the condition.
 * One option is to break encapsulation and give waitsets and conditions unmanaged
 * access to each other but this is probably not needed and a result of some other
 * fundamental issue.
 * Another approach is that deinit operations do their own locking instead that
 * the DDS_free performs the locking, this is likely from a conceptual perspective
 * a better solution but for now the impact of changing this is to much.
 */
        waitset = c_iterObject(condition->waitsets, 0);
        while ((waitset != NULL) && (result == DDS_RETCODE_OK)) {
DDS_ConditionRelease(_this);
            result = DDS_WaitSet_detach_condition(waitset, _this);
DDS_ConditionClaim(_this, &_this);
            waitset = c_iterObject(condition->waitsets, 0);
        }
        c_iterFree(condition->waitsets);

        if ((result == DDS_RETCODE_OK) && (condition->uObject != NULL)) {
            uResult = u_objectFree_s(condition->uObject);
            condition->uObject = NULL;
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    return result;
}


DDS_ReturnCode_t
DDS_Condition_init (
    _Condition _this,
    u_object uObject,
    GetTriggerValue getTriggerValue)
{
    _this->uObject = uObject;
    _this->waitsets = NULL;
    _this->getTriggerValue = getTriggerValue;
    if (uObject) {
        DDS_Object_set_domain_id(_Object(_this), u_observableGetDomainId(u_observable(uObject)));
    }
    return DDS_RETCODE_OK;
}

/*
 *     boolean
 *     get_trigger_value();
 */
DDS_boolean
DDS_Condition_get_trigger_value (
    DDS_Condition _this)
{
    DDS_boolean trigger;
    DDS_ReturnCode_t result;
    _Condition condition;

    SAC_REPORT_STACK();

    trigger = FALSE;
    result = DDS_ConditionCheck(_this, &condition);
    if (result == DDS_RETCODE_OK) {
        trigger = condition->getTriggerValue(condition);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return trigger;
}

u_object
_Condition_get_user_object (
    _Condition _this)
{
    return _this->uObject;
}

u_object
DDS_Condition_get_user_object_for_test (
    DDS_Condition _this)
{
    u_object object = NULL;
    DDS_ReturnCode_t result;
    _Condition condition;

    result = DDS_ConditionCheck(_this, &condition);
    if (result == DDS_RETCODE_OK) {
        if (((_Object)condition)->kind == DDS_READCONDITION) {
            object = u_object(((_ReadCondition)condition)->uQuery);
        } else if (((_Object)condition)->kind == DDS_QUERYCONDITION) {
            object = u_object(((_ReadCondition)condition)->uQuery);
        } else {
            object = condition->uObject;
        }
    }
    return object;
}

u_object
DDS_Condition_get_user_object (
    DDS_Condition _this)
{
    u_object object = NULL;
    DDS_ReturnCode_t result;
    _Condition condition;

    result = DDS_ConditionCheck(_this, &condition);
    if (result == DDS_RETCODE_OK) {
        object = condition->uObject;
    }
    return object;
}

DDS_ReturnCode_t
DDS_Condition_attach_waitset (
    DDS_Condition _this,
    DDS_WaitSet waitset)
{
    DDS_ReturnCode_t result;
    _Condition condition;

    result = DDS_ConditionClaim(_this, &condition);
    if (result == DDS_RETCODE_OK) {
        if (c_iterContains(condition->waitsets, waitset) == FALSE) {
            condition->waitsets = c_iterInsert(condition->waitsets, waitset);
        }
        DDS_ConditionRelease(_this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Condition_detach_waitset (
    DDS_Condition _this,
    DDS_WaitSet waitset)
{
    DDS_ReturnCode_t result;
    _Condition condition;
    DDS_WaitSet found;

    result = DDS_ConditionClaim(_this, &condition);
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(condition->waitsets, waitset);
        if (found != waitset) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Waitset is not associated to this Condition");
        }
        DDS_ConditionRelease(_this);
    }
    return result;
}

static void
trigger_waitset (
    c_voidp o,
    c_iterActionArg arg)
{
    DDS_WaitSet_trigger(DDS_WaitSet(o), arg);
}

DDS_ReturnCode_t
DDS_Condition_trigger (
    DDS_Condition _this)
{
    DDS_ReturnCode_t result;
    _Condition condition;
    c_iter list;

    result = DDS_ConditionClaimRead(_this, &condition);
    if (result == DDS_RETCODE_OK) {
        /* A copy of the list of waitsets is made to be triggered after
         * the condition is released. Triggering a Waitset will lock the
         * Waitset whereas the Waitset may lock the condition to get the
         * trigger value. So If the Condition is locked when triggering
         * a Waitset a deadlock may occur if the Waitset simultaneously
         * tries to get the conditions trigger value, by first taking a
         * copy of all waitsets and then releasing the condition before
         * triggering the Waitsets this situation is avoided.
         */
        list = c_iterCopy(condition->waitsets);
        DDS_ConditionRelease(_this);
        c_iterWalk(list, trigger_waitset, _this);
        c_iterFree(list);
    }
    return result;
}

static void
dummy_callback(
    v_public p,
    c_voidp arg)
{
    OS_UNUSED_ARG(p);
    OS_UNUSED_ARG(arg);
}

DDS_ReturnCode_t
DDS_Condition_is_alive(
   DDS_Condition _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    u_result uResult;
    u_object uObject = DDS_Condition_get_user_object(_this);

    if (uObject != NULL) {
        uResult = u_observableAction(u_observable(uObject), dummy_callback, NULL);
        result = DDS_ReturnCode_get(uResult);
    }

    return result;
}

DDS_ConditionKind_t
DDS_Condition_get_kind(
    DDS_Condition _this)
{
    DDS_ConditionKind_t kind = DDS_CONDITION_KIND_UNDEFINED;
    if (_this) {
        switch (((_Object)_this)->kind) {
        case DDS_STATUSCONDITION:
            kind = DDS_CONDITION_KIND_STATUS;
            break;
        case DDS_GUARDCONDITION:
            kind = DDS_CONDITION_KIND_GUARD;
            break;
        case DDS_READCONDITION:
            kind = DDS_CONDITION_KIND_READ;
            break;
        case DDS_QUERYCONDITION:
            kind = DDS_CONDITION_KIND_QUERY;
            break;
        default:
            kind = DDS_CONDITION_KIND_UNDEFINED;
            break;
        }
    }
    return kind;
}

