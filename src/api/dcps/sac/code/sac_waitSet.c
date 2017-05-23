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
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_objManag.h"
#include "sac_entity.h"
#include "sac_condition.h"
#include "sac_readCondition.h"
#include "u_observable.h"
#include "sac_report.h"

#define DDS_WaitSetClaim(_this, waitset) \
        DDS_Object_claim(DDS_Object(_this), DDS_WAITSET, (_Object *)waitset)

#define DDS_WaitSetClaimRead(_this, waitset) \
        DDS_Object_claim(DDS_Object(_this), DDS_WAITSET, (_Object *)waitset)

#define DDS_WaitSetRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define _WaitSet_get_user_entity(_this) \
        u_waitset(_Entity_get_user_entity(_Entity(_WaitSet(_this))))

typedef struct {
    DDS_Condition condition;
    void *alternative;
} _WaitSet_ConditionInfo;


static _WaitSet_ConditionInfo*
_WaitSet_ConditionInfo_alloc(DDS_Condition cond, void *alt)
{
    _WaitSet_ConditionInfo *wsci =
            (_WaitSet_ConditionInfo*)os_malloc(sizeof(_WaitSet_ConditionInfo));
    /*
     * In some cases (like C99 API) it is required that instead of the
     * condition, another pointer is return when Waitset wait is called.
     * However, this is normally not the case and conditions are
     * expected to be returned by the wait.
     * To keep the wait as simple as possible, it only returns the
     * alternative pointer. So, in normal cases, this alternative pointer
     * isn't really alternative, but the actual condition itself.
     *
     * See also
     * DDS_WaitSet_attach_condition()
     * DDS_WaitSet_attach_condition_alternative()
     */
    wsci->condition = cond;
    wsci->alternative = alt;
    return wsci;
}

static void
_WaitSet_ConditionInfo_free(_WaitSet_ConditionInfo *wsci)
{
    if (wsci) {
        os_free(wsci);
    }
}

static c_bool
_WaitSet_compare_condition(void *o, c_iterActionArg arg)
{
    assert(o);
    assert(arg);
    return (((_WaitSet_ConditionInfo*)o)->condition == (DDS_Condition)arg);
}

static _WaitSet_ConditionInfo*
_WaitSet_read_condition(c_iter iter, DDS_Condition cond)
{
    return (_WaitSet_ConditionInfo*)c_iterReadAction(iter,
                                                     _WaitSet_compare_condition,
                                                     (c_iterActionArg)cond);
}

static _WaitSet_ConditionInfo*
_WaitSet_take_condition(c_iter iter, DDS_Condition cond)
{
    return (_WaitSet_ConditionInfo*)c_iterTakeAction(iter,
                                                     _WaitSet_compare_condition,
                                                     (c_iterActionArg)cond);
}

static c_bool
_WaitSet_contains_condition(c_iter iter, DDS_Condition cond)
{
    return (_WaitSet_read_condition(iter, cond) != NULL);
}



/**
 * This operation inserts a condition into the condition sequence.
 * It automatically increases the buffer size if allowed or else return
 * FALSE in case resizing fails because it isn't allowed.
 */
static DDS_boolean
DDS_ConditionSeq_insert (
    DDS_ConditionSeq *seq,
    DDS_Condition condition)
{
    DDS_boolean status = TRUE;
    DDS_unsigned_long length;
    void *oldbuf;

    if (seq->_length == seq->_maximum) {
        if (seq->_maximum > 0) {
            if (seq->_release) {
                oldbuf = seq->_buffer;
                length = seq->_length + 1;
                seq->_buffer = DDS_ConditionSeq_allocbuf(length);
                seq->_maximum = length;
                memcpy(seq->_buffer, oldbuf, sizeof(oldbuf));
                DDS_free(oldbuf);
            }
        } else {
            length = 1;
            seq->_buffer = DDS_ConditionSeq_allocbuf(length);
            seq->_maximum = length;
            seq->_release = TRUE;
        }
    }
    if (seq->_length < seq->_maximum) {
        seq->_buffer[seq->_length] = condition;
        seq->_length++;
    } else {
        status = FALSE;
        assert(FALSE);
    }
    return status;
}

static void
collect_detached_conditions(
    c_voidp o,
    c_iterActionArg arg)
{
    _WaitSet_ConditionInfo* wsci = (_WaitSet_ConditionInfo*)o;
    DDS_ConditionSeq *seq = (DDS_ConditionSeq *)arg;

    if (DDS_Condition_is_alive(wsci->condition) == DDS_RETCODE_ALREADY_DELETED) {
        DDS_ConditionSeq_insert(seq, wsci->alternative);
    }
}


#define MAX_GUARDS (256)

struct wait_actionArg {
    DDS_ConditionSeq *active_conditions;
    os_uint32 nrOfGuards;
    _WaitSet_ConditionInfo **guards;
};

static os_boolean
wait_action (
    c_voidp userData,
    c_voidp arg)
{
    os_uint32 i;
    os_boolean proceed = OS_TRUE;
    struct wait_actionArg *a = (struct wait_actionArg *)arg;

    if (userData == NULL) {
        /* test guard conditions. */
        for (i=0; i<a->nrOfGuards; i++) {
            if (DDS_Condition_get_trigger_value(a->guards[i]->condition) == TRUE) {
                DDS_ConditionSeq_insert(a->active_conditions, a->guards[i]->alternative);
            }
        }
        proceed = (a->active_conditions->_length == 0);
    } else {
        DDS_ConditionSeq_insert(a->active_conditions,
                                ((_WaitSet_ConditionInfo*)userData)->alternative);
    }
    return proceed;
}

/*     ReturnCode_t
 *     wait(
 *         out ConditionSeq active_conditions,
 *         in Duration_t timeout);
 */

DDS_ReturnCode_t
DDS_WaitSet_wait (
    DDS_WaitSet _this,
    DDS_ConditionSeq *active_conditions,
    const DDS_Duration_t *timeout)
{
    DDS_ReturnCode_t result;
    DDS_unsigned_long length;
    os_duration uTimeout;
    u_result uResult;
    u_waitset uWaitset;
    _WaitSet ws;
    struct wait_actionArg arg;
    _WaitSet_ConditionInfo *guards[MAX_GUARDS];

    SAC_REPORT_STACK();

    if ((active_conditions != NULL) && (DDS_Duration_copyIn(timeout, &uTimeout) == DDS_RETCODE_OK)) {
        length = 0;
        active_conditions->_length = 0; /* will be increased during the search */
        result = DDS_WaitSetClaim(_this, &ws);
        if (result == DDS_RETCODE_OK) {
            length = c_iterLength(ws->conditions);
            length += c_iterLength(ws->guards);
            DDS_WaitSetRelease(_this);
        }
    } else {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "ConditionSeq active_conditions = 0x%x and Duration_t timeout = 0x%x",
                    active_conditions, timeout);
        SAC_REPORT_FLUSH(_this, TRUE);
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (length > active_conditions->_maximum) {
        if (active_conditions->_maximum > 0) {
            if (active_conditions->_release) {
                DDS_free(active_conditions->_buffer);
                active_conditions->_buffer = DDS_ConditionSeq_allocbuf(length);
                active_conditions->_maximum = length;
            }
        } else {
            active_conditions->_buffer = DDS_ConditionSeq_allocbuf(length);
            active_conditions->_maximum = length;
            active_conditions->_release = TRUE;
        }
    }

    /* The following while loop waits on the waitset until it has a
     * true condition or the waitset is deleted. The wait can also return
     * in case a change has occurred in the attached conditions, in that case
     * this loop will potentially realloc the guard buffer and reenter the wait.
     */
    arg.guards = guards;
    arg.nrOfGuards = 0;
    arg.active_conditions = active_conditions;
    active_conditions->_length = 0; /* will be increased during the search */
    while ((result == DDS_RETCODE_OK) && (active_conditions->_length == 0))
    {
        os_uint32 nrOfGuards;
        uWaitset = ws->uWaitset;

        if (uWaitset != NULL) {
            result = DDS_WaitSetClaim(_this, &ws);
            if (result == DDS_RETCODE_OK) {
                nrOfGuards = c_iterLength(ws->guards);
                if (nrOfGuards > MAX_GUARDS) {
                    if (arg.nrOfGuards > MAX_GUARDS) {
                        os_free(arg.guards);
                    }
                    arg.guards = os_malloc(arg.nrOfGuards * sizeof(_WaitSet_ConditionInfo*));
                }
                if (nrOfGuards > 0) {
                    c_iterArray(ws->guards, (void**)arg.guards);
                }
                DDS_WaitSetRelease(_this);

                arg.nrOfGuards = nrOfGuards;
                uResult = u_waitsetWaitAction2(uWaitset, wait_action, &arg, uTimeout);
                if (uResult == U_RESULT_DETACHING) {
                    result = DDS_WaitSetClaim(_this, &ws);
                    if (result == DDS_RETCODE_OK) {
                        c_iterWalk(ws->conditions, collect_detached_conditions, active_conditions);
                        DDS_WaitSetRelease(_this);
                        result = DDS_RETCODE_OK;
                    }
                } else {
                    result = DDS_ReturnCode_get(uResult);
                }
            }
        } else if (active_conditions->_length == 0) {
            result = DDS_RETCODE_ALREADY_DELETED;
            SAC_REPORT(result, "Waitset is already deleted");
        }
    }
    if (arg.nrOfGuards > MAX_GUARDS) {
        os_free(arg.guards);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

/*     ReturnCode_t
 *     attach_condition(
 *         in Condition cond);
 */
DDS_ReturnCode_t
DDS_WaitSet_attach_condition (
    DDS_WaitSet _this,
    const DDS_Condition cond)
{
    /* To not bother the wait code with the question if an alternative
     * is set or not, set the condition as 'alternative'.
     * Now the wait code can always just return the alternative. */
    return DDS_WaitSet_attach_condition_alternative(_this, cond, cond);
}

DDS_ReturnCode_t
DDS_WaitSet_attach_condition_alternative (
    DDS_WaitSet _this,
    const DDS_Condition cond,
    void *alternative)
{
    DDS_ReturnCode_t result;
    DDS_ObjectKind kind;
    u_object uCondition;
    u_result uResult;
    _WaitSet ws;

    SAC_REPORT_STACK();

    result = DDS_WaitSetClaim(_this, &ws);
    if (result == DDS_RETCODE_OK) {
        kind = DDS_Object_get_kind(cond);
        if (kind == DDS_GUARDCONDITION) {
            if (!_WaitSet_contains_condition(ws->guards, cond)) {
                _WaitSet_ConditionInfo *wsci = _WaitSet_ConditionInfo_alloc(cond, alternative);
                /* Unblock any blocking wait call to evaluate the
                   attached guard condition. */
                uResult = u_waitsetNotify(ws->uWaitset, wsci);
                result = DDS_ReturnCode_get(uResult);
                if (result == DDS_RETCODE_OK) {
                    result = DDS_Condition_attach_waitset(cond, _this);
                    ws->guards = c_iterInsert(ws->guards, wsci);
                } else {
                    _WaitSet_ConditionInfo_free(wsci);
                }
            }
        } else {
            if (!_WaitSet_contains_condition(ws->conditions, cond)) {
                _WaitSet_ConditionInfo *wsci = _WaitSet_ConditionInfo_alloc(cond, alternative);
                switch (kind) {
                case DDS_STATUSCONDITION :
                    uCondition = DDS_Condition_get_user_object(cond);
                    uResult = u_waitsetAttach(ws->uWaitset, u_observable(uCondition), wsci);
                    result = DDS_ReturnCode_get(uResult);
                break;
                case DDS_READCONDITION :
                case DDS_QUERYCONDITION :
                    uCondition = u_object(DDS_ReadCondition_get_uQuery(DDS_ReadCondition(cond)));
                    uResult = u_waitsetAttach(ws->uWaitset, u_observable(uCondition), wsci);
                    result = DDS_ReturnCode_get(uResult);
                break;
                default :
                    result = DDS_RETCODE_BAD_PARAMETER;
                    SAC_REPORT(result, "Condition parameter 'cond' is of type %s",
                            DDS_ObjectKind_image(kind));
                break;
                }
                if (result == DDS_RETCODE_OK) {
                    result = DDS_Condition_attach_waitset(cond, _this);
                    ws->conditions = c_iterInsert(ws->conditions, wsci);

                    DDS_Object_set_domain_id(_Object(ws), u_waitsetGetDomainId(ws->uWaitset));
                } else {
                    _WaitSet_ConditionInfo_free(wsci);
                }
            }
        }
        DDS_WaitSetRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     detach_condition(
 *         in Condition cond);
 */
DDS_ReturnCode_t
DDS_WaitSet_detach_condition(
    DDS_WaitSet _this,
    const DDS_Condition cond)
{
    DDS_ReturnCode_t result;
    DDS_ObjectKind kind;
    _WaitSet_ConditionInfo *found;
    u_object uCondition;
    u_result uResult;
    _WaitSet ws;

    SAC_REPORT_STACK();

    result = DDS_WaitSetClaim(_this, &ws);
    if (result == DDS_RETCODE_OK) {
        kind = DDS_Object_get_kind(cond);
        if (kind == DDS_GUARDCONDITION) {
            found = _WaitSet_take_condition(ws->guards, cond);
            if (found) {
                result = DDS_Condition_detach_waitset(cond, _this);
                _WaitSet_ConditionInfo_free(found);
            } else {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "GuardCondition is not attached to this Waitset");
            }
        } else {
            found = _WaitSet_take_condition(ws->conditions, cond);
            if (found) {
                switch (kind) {
                case DDS_STATUSCONDITION :
                    uCondition = DDS_Condition_get_user_object(cond);
                    if (uCondition != NULL) {
                        uResult = u_waitsetDetach_s(ws->uWaitset, u_observable(uCondition));
                        result = DDS_ReturnCode_get(uResult);
                    }
                break;
                case DDS_READCONDITION :
                case DDS_QUERYCONDITION :
                    uCondition = u_object(DDS_ReadCondition_get_uQuery(DDS_ReadCondition(cond)));
                    if (uCondition != NULL) {
                        uResult = u_waitsetDetach_s(ws->uWaitset, u_observable(uCondition));
                        result = DDS_ReturnCode_get(uResult);
                    }
                break;
                default :
                    result = DDS_RETCODE_BAD_PARAMETER;
                    SAC_REPORT(result, "Condition parameter 'cond' is of type %s",
                                DDS_ObjectKind_image(kind));
                break;
                }
                if ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_ALREADY_DELETED)) {
                    result = DDS_Condition_detach_waitset(cond, _this);
                    _WaitSet_ConditionInfo_free(found);

                    DDS_Object_set_domain_id(_Object(ws), u_waitsetGetDomainId(ws->uWaitset));

                } else {
                    (void)c_iterInsert(ws->conditions, found);
                }
            } else {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Condition is not attached to this Waitset");
            }
        }
        DDS_WaitSetRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_conditions(
 *         out ConditionSeq attached_conditions);
 */
static void
copy_to_sequence(
    c_voidp o,
    c_iterActionArg arg)
{
    DDS_ConditionSeq *seq = (DDS_ConditionSeq *)arg;

    if (seq->_length < seq->_maximum) {
        seq->_buffer[seq->_length++] = ((_WaitSet_ConditionInfo*)o)->condition;
    }
}

DDS_ReturnCode_t
DDS_WaitSet_get_conditions(
    DDS_WaitSet _this,
    DDS_ConditionSeq *attached_conditions)
{
    DDS_ReturnCode_t result;
    DDS_unsigned_long length;
    _WaitSet ws;

    SAC_REPORT_STACK();

    if (attached_conditions != NULL) {
        result = DDS_WaitSetClaimRead(_this, &ws);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    if (result == DDS_RETCODE_OK) {
        length = c_iterLength(ws->conditions);
        length += c_iterLength(ws->guards);
        if (length > attached_conditions->_maximum) {
            if (attached_conditions->_maximum > 0) {
                if (attached_conditions->_release) {
                    DDS_free(attached_conditions->_buffer);
                    attached_conditions->_buffer = DDS_ConditionSeq_allocbuf(length);
                    attached_conditions->_maximum = length;
                } else {
                    result = DDS_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                attached_conditions->_buffer = DDS_ConditionSeq_allocbuf(length);
                attached_conditions->_maximum = length;
                attached_conditions->_release = TRUE;
            }
        }
        if (result == DDS_RETCODE_OK) {
            attached_conditions->_length = 0; /* will be increased during the walk */
            c_iterWalk(ws->conditions, copy_to_sequence, attached_conditions);
            c_iterWalk(ws->guards, copy_to_sequence, attached_conditions);
        }
        DDS_WaitSetRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_WaitSet_trigger(
    DDS_WaitSet _this,
    void *user_data)
{
    DDS_ReturnCode_t result;
    u_result uResult;
    _WaitSet ws;

    result = DDS_WaitSetClaim(_this, &ws);
    if (result == DDS_RETCODE_OK) {
        uResult = u_waitsetNotify(ws->uWaitset, user_data);
        result = DDS_ReturnCode_get(uResult);
        DDS_WaitSetRelease(_this);
    }
    return result;
}


static DDS_ReturnCode_t
_WaitSet_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;
    _WaitSet_ConditionInfo *wsci;
    _WaitSet ws;
    u_result uResult;
#if 0
    DDS_ObjectKind kind;
    u_object uCondition;
#endif

    result = DDS_RETCODE_OK;
    ws = _WaitSet(_this);
    wsci = c_iterTakeFirst(ws->conditions);
    if (wsci == NULL) {
        wsci = c_iterTakeFirst(ws->guards);
    }
    while ((wsci != NULL) && (result == DDS_RETCODE_OK)) {
#if 0

Some problems occurred when trying to detach the lower user layer conditions from the waitset.
Need to determine if still and why these problems occur.
Commenting this out is no problem since the deletion of the waitset will automatically fix
all loose ends.

        kind = DDS_Object_get_kind(cond);
        switch (kind) {
        case DDS_STATUSCONDITION :
            uCondition = u_object(DDS_Condition_get_user_object(wsci->condition));
            if (uCondition != NULL) {
                uResult = u_waitsetDetach_s(ws->uWaitset, u_observable(uCondition));
                result = DDS_ReturnCode_get(uResult);
            }
        break;
        case DDS_READCONDITION :
        case DDS_QUERYCONDITION :
            uCondition = u_object(DDS_ReadCondition_get_uQuery(DDS_ReadCondition(wsci->condition)));
            if (uCondition != NULL) {
                uResult = u_waitsetDetach_s(ws->uWaitset, u_observable(uCondition));
                result = DDS_ReturnCode_get(uResult);
            }
        break;
        case DDS_GUARDCONDITION :
        break;
        default :
            result = DDS_RETCODE_BAD_PARAMETER;
        break;
        }
#endif
        if (result == DDS_RETCODE_OK) {
            result = DDS_Condition_detach_waitset(wsci->condition, ws);
        }
        _WaitSet_ConditionInfo_free(wsci);
        wsci = c_iterTakeFirst(ws->conditions);
        if (wsci == NULL) {
            wsci = c_iterTakeFirst(ws->guards);
        }
    }

    if (result == DDS_RETCODE_OK) {
        c_iterFree(ws->guards);
        c_iterFree(ws->conditions);
        uResult = u_objectFree_s (u_object (ws->uWaitset));
        ws->uWaitset = NULL;
        result = DDS_ReturnCode_get(uResult);
    }
    return result;
}

DDS_ReturnCode_t
DDS_WaitSet_free (
    DDS_WaitSet _this)
{
    DDS_ReturnCode_t result;
    result = DDS__free(_this);
    return result;
}

/*     WaitSet
 *     WaitSet__alloc (
 *         void);
 */
DDS_WaitSet
DDS_WaitSet__alloc (
    void)
{
    DDS_ReturnCode_t result;
    _WaitSet waitset;

    result = DDS_Object_new(DDS_WAITSET, _WaitSet_deinit, (_Object *)&waitset);
    if (result == DDS_RETCODE_OK) {
        waitset->uWaitset = u_waitsetNew2();
        waitset->conditions = NULL;
        waitset->guards = NULL;
        u_waitsetSetEventMask(waitset->uWaitset, V_EVENTMASK_ALL);
    } else {
        waitset = DDS_OBJECT_NIL;
    }
    return (DDS_WaitSet)waitset;
}
