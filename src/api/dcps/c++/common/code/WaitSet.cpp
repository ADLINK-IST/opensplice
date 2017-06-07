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
#include "u_user.h"
#include "u_observable.h"
#include "c_base.h"
#include "WaitSet.h"
#include "GuardCondition.h"
#include "StatusCondition.h"
#include "ReadCondition.h"
#include "ReportUtils.h"

DDS::WaitSet_ptr
DDS::WaitSet::_nil (void)
{
    return static_cast<DDS::WaitSet_ptr>(NULL);
}

DDS::WaitSet_ptr
DDS::WaitSet::_duplicate (DDS::WaitSet_ptr obj)
{
    return dynamic_cast<DDS::WaitSet_ptr>(
        DDS::WaitSetInterface::_duplicate (obj));
}

DDS::WaitSet_ptr
DDS::WaitSet::_narrow (DDS::WaitSet_ptr obj)
{
    return dynamic_cast<DDS::WaitSet_ptr>(
        DDS::WaitSetInterface::_narrow (obj));
}

DDS::WaitSet_ptr
DDS::WaitSet::_unchecked_narrow (DDS::WaitSet_ptr obj)
{
    return dynamic_cast<DDS::WaitSet_ptr>(
        DDS::WaitSetInterface::_narrow (obj)); /* _unchecked_narrow is not implemented by all ORBs. */
}

DDS::WaitSet::WaitSet (
) : DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::WAITSET),
    uWaitset(NULL),
    conditions(new DDS::OpenSplice::ObjSet(FALSE)),
    guards(new DDS::OpenSplice::ObjSet(TRUE))
{
    (void) init();
}

DDS::WaitSet::~WaitSet ()
{
    (void) deinit();
    delete conditions;
    delete guards;
}

DDS::ReturnCode_t
DDS::WaitSet::init()
{
    return nlReq_init();
}

DDS::ReturnCode_t
DDS::WaitSet::nlReq_init()
{
    DDS::ReturnCode_t result;

    result = conditions->init();
    if (result == DDS::RETCODE_OK) {
        result = guards->init();
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    }
    if (result == DDS::RETCODE_OK) {
        this->uWaitset = u_waitsetNew2 ();
        if (this->uWaitset == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_PANIC("Could not create WaitSet.");
        }
    }
    return result;
}

DDS::ReturnCode_t
DDS::WaitSet::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::ObjSeq_var condSeq = conditions->getObjSeq();
    DDS::ULong length = condSeq->length();

    result = DDS::OpenSplice::CppSuperClass::wlReq_deinit();
    if (result == DDS::RETCODE_OK) {
        u_waitsetAnnounceDestruction(uWaitset);
        for (DDS::ULong i = 0; i < length && result == DDS::RETCODE_OK; i++) {
            DDS::OpenSplice::Condition *cond;

            cond = dynamic_cast<DDS::OpenSplice::Condition *>(condSeq[i].in());
            result = cond->detachFromWaitset(this);
        }
    }

    if (result == DDS::RETCODE_OK) {
        condSeq = guards->getObjSeq();
        length = condSeq->length();
        for (DDS::ULong i = 0; i < length && result == DDS::RETCODE_OK; i++) {
            DDS::OpenSplice::Condition *cond;

            cond = dynamic_cast<DDS::OpenSplice::Condition *>(condSeq[i].in());
            result = cond->detachFromWaitset(this);
        }
        u_waitsetNotify (uWaitset, NULL);
        result = uResultToReturnCode(u_objectFree_s (u_object (uWaitset)));
        if (result == DDS::RETCODE_OK) {
            uWaitset = NULL;
            conditions->clear();
            guards->clear();
            result = conditions->deinit();
        }
        if (result == DDS::RETCODE_OK) {
            result = guards->deinit();
        }
    }

    return result;
}

DDS::Boolean
DDS::WaitSet::collect_detached_conditions(
    DDS::Object_ptr td,
    struct findObjectArg *arg)
{
    DDS::ConditionSeq *activeConditions = reinterpret_cast<DDS::ConditionSeq *>(arg);
    DDS::ULong length = activeConditions->length();
    DDS::OpenSplice::Condition *condition = dynamic_cast<DDS::OpenSplice::Condition *>(td);
    assert(condition); /* Type already checked during insertion. */

    if (condition->isAlive() == DDS::RETCODE_ALREADY_DELETED) {
        activeConditions->length(++length);
        (*activeConditions)[length - 1] = DDS::Condition::_duplicate(condition);
    }

    return TRUE;
}

typedef struct {
    DDS::ConditionSeq *activeConditions;// Don't clean up afterwards: this field is returned to the caller!!
    DDS::ObjSeq_var guards;             // _var type automatically cleans up afterward.
    DDS::Long maxConditions;
} WaitActionArg;

static os_boolean
wait_action (
    c_voidp userData,
    c_voidp arg)
{
    DDS::Condition_ptr condition;
    os_boolean proceed = OS_TRUE;
    WaitActionArg *waitArg = reinterpret_cast<WaitActionArg *>(arg);
    DDS::ConditionSeq &condSeq = *(waitArg->activeConditions);
    DDS::ULong length = condSeq.length();
    DDS::ULong max = condSeq.maximum();

    if (userData == NULL) {
        DDS::ULong nrGuards = waitArg->guards->length();
        for (DDS::ULong i = 0; i < nrGuards; i++)
        {
            condition = dynamic_cast<DDS::Condition_ptr>(waitArg->guards[i].in());
            assert(condition);
            if (condition->get_trigger_value()) {
                if (++length > max)
                {
                    // Stretch sequence up to worst-case, to avoid continuous re-allocations.
                    max = waitArg->maxConditions;
                    condSeq.length(max);
                }
                condSeq.length(length);
                condSeq[length - 1] = DDS::Condition::_duplicate(condition);
            }
        }
        proceed = (length == 0) ? OS_TRUE : OS_FALSE;
    } else {

        condition = reinterpret_cast<DDS::Condition_ptr>(userData);
        assert(condition);
        condSeq.length(++length);
        condSeq[length - 1] = DDS::Condition::_duplicate(condition);
    }
    return proceed;
}

DDS::ReturnCode_t
DDS::WaitSet::wait(
    ConditionSeq &active_conditions,
    const DDS::Duration_t &timeout
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    os_duration uTimeout;
    WaitActionArg arg;

    CPP_REPORT_STACK();

    active_conditions.length(0);
    arg.activeConditions = &active_conditions;
    arg.guards = NULL;

    result = DDS::OpenSplice::Utils::copyDurationIn(timeout, uTimeout);

    while (result == DDS::RETCODE_OK && active_conditions.length() == 0) {
        result = this->read_lock ();
        if (result == DDS::RETCODE_OK) {
            arg.maxConditions = conditions->getNrElements() + guards->getNrElements();
            /* Avoid re-allocating a new sequence every iteration, so only create it the first time. */
            if (arg.guards == NULL) {
                arg.guards = guards->getObjSeq();
            } else {
                guards->getObjSeq(arg.guards);
            }
            this->unlock ();

            u_result uResult = u_waitsetWaitAction2(uWaitset, wait_action, &arg, uTimeout);
            if (uResult == U_RESULT_DETACHING) {
                result = this->read_lock ();
                if(result == DDS::RETCODE_OK) {
                    (void) this->conditions->walk(
                            (DDS::OpenSplice::ObjSet::ObjSetActionFunc)collect_detached_conditions,
                            &active_conditions);
                    this->unlock ();
                }
            } else {
                result = uResultToReturnCode(uResult);
            }
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

/**
 * Condition specific callback from non-GuardConditions to WaitSet.
 */
DDS::ReturnCode_t
DDS::WaitSet::wlReq_attachGeneralCondition(
    DDS::Condition_ptr condition,
    u_observable uCondition)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    /* The WaitSet does not modify the reCounts of the conditions it points to: the destructor of
     * each condition is supposed to detach itself from the WaitSet. Keeping track of refCounts would
     * avoid some Conditions to ever reach 0 due to cyclic references.
     */
    assert (this->conditions->containsElement(condition) == FALSE);

    uResult = u_waitsetAttach(uWaitset, uCondition, condition);
    result = uResultToReturnCode(uResult);
    if (result == DDS::RETCODE_OK) {
        DDS::Boolean inserted = this->conditions->insertElement(condition);
        assert (inserted);
        OS_UNUSED_ARG(inserted);
        this->setDomainId(u_waitsetGetDomainId(this->uWaitset));
    } else {
        CPP_REPORT(result, "Could not attach Condition to WaitSet.");
    }

    return result;
}

/**
 * Condition specific callback from GuardConditions to WaitSet.
 */
DDS::ReturnCode_t
DDS::WaitSet::wlReq_attachGuardCondition(
    GuardCondition *condition)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    /* The WaitSet does not modify the reCounts of the conditions it points to: the destructor of
     * each condition is supposed to detach itself from the WaitSet. Keeping track of refCounts would
     * avoid some Conditions to ever reach 0 due to cyclic references.
     */
    assert (this->guards->containsElement(condition) == FALSE);

    uResult = u_waitsetNotify(uWaitset, NULL);
    result = uResultToReturnCode(uResult);
    if (result == DDS::RETCODE_OK) {
        DDS::Boolean inserted = this->guards->insertElement(condition);
        assert (inserted);
        OS_UNUSED_ARG(inserted);
    } else {
        CPP_REPORT(result, "Could not attach GuardCondition to WaitSet.");
    }

    return result;
}

/**
 * Condition specific callback from non-GuardConditions to WaitSet.
 */
DDS::ReturnCode_t
DDS::WaitSet::wlReq_detachGeneralCondition(
    DDS::OpenSplice::Condition *condition,
    u_observable uCondition)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    /* The WaitSet does not modify the reCounts of the conditions it points to: the destructor of
     * each condition is supposed to detach itself from the WaitSet. Keeping track of refCounts would
     * avoid some Conditions to ever reach 0 due to cyclic references.
     */
    assert (this->conditions->containsElement(condition));

    uResult = u_waitsetDetach_s(uWaitset, uCondition);
    result = uResultToReturnCode(uResult);
    if ((result == DDS::RETCODE_OK) || (result == DDS::RETCODE_ALREADY_DELETED)) {
        // Set takes care of its own refCounting.
        DDS::Boolean removed = this->conditions->removeElement(condition);
        assert (removed);
        OS_UNUSED_ARG(removed);
        this->setDomainId(u_waitsetGetDomainId(this->uWaitset));
    } else {
        CPP_REPORT(result, "Could not detach Condition from WaitSet.");
    }

    return result;
}

/**
 * Condition specific callback from GuardConditions to WaitSet.
 */
DDS::ReturnCode_t
DDS::WaitSet::wlReq_detachGuardCondition(
    GuardCondition *condition)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    /* The WaitSet does not modify the reCounts of the conditions it points to: the destructor of
     * each condition is supposed to detach itself from the WaitSet. Keeping track of refCounts would
     * avoid some Conditions to ever reach 0 due to cyclic references.
     */
    assert (this->guards->containsElement(condition));

    uResult = u_waitsetNotify(uWaitset, NULL);
    result = uResultToReturnCode(uResult);
    if (result == DDS::RETCODE_OK) {
        // Set takes care of its own refCounting.
        DDS::Boolean removed = this->guards->removeElement(condition);
        assert (removed);
        OS_UNUSED_ARG(removed);
    } else {
        CPP_REPORT(result, "Could not detach GuardCondition from WaitSet.");
    }

    return result;
}

DDS::ReturnCode_t
DDS::WaitSet::attach_condition(
    DDS::Condition_ptr cond
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::Condition *condition;

    CPP_REPORT_STACK();

    if (cond == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "cond '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::Condition *>(cond);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "cond is invalid, not of type '%s'.",
                "DDS::OpenSplice::Condition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            result = condition->attachToWaitset(this);

            // ALREADY_DELETED may only apply to the WaitSet in this context,
            // so for a deleted condition use BAD_PARAMETER instead.
            if (result == DDS::RETCODE_ALREADY_DELETED) {
                result = DDS::RETCODE_BAD_PARAMETER;
            }
            this->unlock();
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::WaitSet::detach_condition(
    DDS::Condition_ptr cond
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::Condition *condition;

    CPP_REPORT_STACK();

    if (cond == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "cond '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::Condition *>(cond);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "cond is invalid, not of type '%s'.",
                "DDS::OpenSplice::Condition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            result = condition->detachFromWaitset(this);

            // ALREADY_DELETED may only apply to the WaitSet in this context,
            // so for a deleted condition use BAD_PARAMETER instead.
            if (result == DDS::RETCODE_ALREADY_DELETED) {
                result = DDS::RETCODE_BAD_PARAMETER;
            }
            this->unlock ();
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

static DDS::Boolean
copy_to_sequence (
    DDS::Object_ptr element,
    void *arg)
{
    DDS::Condition_ptr condition =
        dynamic_cast<DDS::Condition_ptr>(element);
    assert (condition != NULL);
    DDS::ConditionSeq *attached_conditions =
        reinterpret_cast<DDS::ConditionSeq *>(arg);
    assert (attached_conditions != NULL);
    DDS::ULong length;

    length = attached_conditions->length ();
    attached_conditions->length (length + 1);
    (*attached_conditions)[length] = DDS::Condition::_duplicate (condition);
    return TRUE;
}

DDS::ReturnCode_t
DDS::WaitSet::get_conditions(
    DDS::ConditionSeq &attached_conditions)
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        attached_conditions.length(conditions->getNrElements()+guards->getNrElements());
        attached_conditions.length(0);
        (void)conditions->walk(copy_to_sequence, &attached_conditions);
        guards->walk(copy_to_sequence, &attached_conditions);
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::WaitSet::trigger (
    DDS::Condition_ptr cond)
{
    DDS::ReturnCode_t result;
    u_result uResult;

    assert (cond != NULL);

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uResult = u_waitsetNotify (uWaitset, cond);
        result = uResultToReturnCode (uResult);
        CPP_REPORT(result, "Could not trigger WaitSet.");
    }

    return result;
}

os_int32
DDS::WaitSet::getDomainId()
{
    int domainId = -1;

    if (this->check() == DDS::RETCODE_OK) {
        domainId = u_waitsetGetDomainId(this->uWaitset);
    }

    return domainId;
}
