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


/**
 * @file
 */

#include <org/opensplice/core/cond/WaitSetDelegate.hpp>

#include <dds/core/cond/WaitSet.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/TimeUtils.hpp>

#include <u_object.h>
#include <u_observable.h>
#include <u_waitset.h>

#include <algorithm>


org::opensplice::core::cond::WaitSetDelegate::WaitSetDelegate()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    u_waitset uWaitset = u_waitsetNew2 ();
    if (uWaitset == NULL) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "u_waitsetNew2 failed");
    }

    this->userHandle = u_object(uWaitset);
}

org::opensplice::core::cond::WaitSetDelegate::~WaitSetDelegate()
{
    if (!this->closed) {
        try {
            this->close();
        } catch (...) {
            /* Empty: the exception throw should have already traced an error. */
        }
    }
}

void
org::opensplice::core::cond::WaitSetDelegate::init(ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
}

void
org::opensplice::core::cond::WaitSetDelegate::close()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    std::vector<dds::core::cond::Condition> clist;
    clist.reserve(this->conditions_.size());

    for (ConditionIterator it = this->conditions_.begin(); it != this->conditions_.end(); ++it) {
        clist.push_back(it->second);
    }

    for (std::vector<dds::core::cond::Condition>::iterator it = clist.begin(); it != clist.end(); ++it) {
        it->delegate()->remove_waitset(this);
    }

    UserObjectDelegate::close();

    scopedLock.unlock();
}

int
org::opensplice::core::cond::WaitSetDelegate::wait_action (
    c_voidp userData,
    c_voidp arg)
{
    os_boolean proceed = OS_TRUE;
    ConditionDelegate *pCondition;
    WaitActionArg *wsargs = reinterpret_cast<org::opensplice::core::cond::WaitSetDelegate::WaitActionArg *>(arg);

    if (userData == NULL) {
        std::vector<org::opensplice::core::cond::ConditionDelegate *>::iterator it;
        for (it = wsargs->guards.begin(); it != wsargs->guards.end(); ++it) {
            if ((*it)->trigger_value()) {
                ConditionDelegate::ref_type ref =
                        OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<ConditionDelegate>((*it)->get_strong_ref());
                dds::core::cond::Condition c(ref);
                wsargs->active_conditions.push_back(c);
                proceed = OS_FALSE;
            }
        }
    } else {
        pCondition = reinterpret_cast<ConditionDelegate *>(userData);
        assert(pCondition);
        ConditionDelegate::ref_type ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<ConditionDelegate>(pCondition->get_strong_ref());
        dds::core::cond::Condition c(ref);
        wsargs->active_conditions.push_back(c);
        proceed = OS_FALSE;
    }
    return proceed;
}

org::opensplice::core::cond::WaitSetDelegate::ConditionSeq&
org::opensplice::core::cond::WaitSetDelegate::wait(
    ConditionSeq& triggered,
    const dds::core::Duration& timeout)
{
    bool infTimeout = timeout == dds::core::Duration::infinite();

    os_duration uTimeout = org::opensplice::core::timeUtils::convertDuration(timeout);
    os_timeW uTimeEnd = OS_TIMEW_INVALID;

    if (!OS_DURATION_ISINFINITE(uTimeout)) {
        uTimeEnd = os_timeWAdd(os_timeWGet(), uTimeout);
    }

    triggered.clear();
    bool ready = false;
    while (!ready) {
        org::opensplice::core::ScopedObjectLock scopedLock(*this);
        std::vector<org::opensplice::core::cond::ConditionDelegate *> guards(this->guards_);

        WaitActionArg wsargs = {guards, triggered};
        scopedLock.unlock();

        u_result uResult = u_waitsetWaitAction2(u_waitset(this->userHandle), (u_waitsetAction2)wait_action, &wsargs, uTimeout);
        if (uResult == U_RESULT_OK) {
            if (triggered.size() > 0) {
                ready = true;
            } else {
                if (!OS_DURATION_ISINFINITE(uTimeout)) {
                    os_timeW uTimeNow = os_timeWGet();
                    if (os_timeWCompare(uTimeNow, uTimeEnd) == OS_LESS) {
                        uTimeout = os_timeWDiff(uTimeEnd, uTimeNow);
                    } else {
                        ready = true;
                    }
                }
            }
        } else if (uResult == U_RESULT_TIMEOUT) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_TIMEOUT_ERROR, "dds::core::cond::WaitSet::wait() times out.");
        } else {
            ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_waitsetWaitAction2 failed.");
        }
    }

    return triggered;
}

void
org::opensplice::core::cond::WaitSetDelegate::dispatch(
    const dds::core::Duration& timeout)
{
    ConditionSeq triggered;
    wait(triggered, timeout);
    for (ConditionSeq::iterator it = triggered.begin(); it != triggered.end(); ++it) {
        it->dispatch();
    }
}

void org::opensplice::core::cond::WaitSetDelegate::attach_condition(
    const dds::core::cond::Condition& cond)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    cond.delegate()->add_waitset(cond, this);
    scopedLock.unlock();
}

bool
org::opensplice::core::cond::WaitSetDelegate::detach_condition(
    org::opensplice::core::cond::ConditionDelegate *cond)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    bool removed = cond->remove_waitset(this);
    scopedLock.unlock();

    return removed;
}

void
org::opensplice::core::cond::WaitSetDelegate::add_condition_locked(
    const dds::core::cond::Condition& cond)
{
    u_observable uCondition = cond.delegate()->get_user_condition();
    u_result uResult = u_waitsetAttach(u_waitset(this->userHandle), uCondition, cond.delegate().get());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_waitsetAttach failed.");
    ConditionEntry entry(cond.delegate().get(), cond);
    this->conditions_.insert(entry);
    this->set_domain_id(u_waitsetGetDomainId(u_waitset(this->userHandle)));
}

void
org::opensplice::core::cond::WaitSetDelegate::add_guardCondition_locked(
    const dds::core::cond::Condition& cond)
{
    // Trigger waitset to update the guard condition list
    u_result uResult = u_waitsetNotify(u_waitset(this->userHandle), NULL);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_waitsetNotify failed.");

    ConditionEntry entry(cond.delegate().get(), cond);
    this->conditions_.insert(entry);
    this->guards_.push_back(entry.first);
}

void
org::opensplice::core::cond::WaitSetDelegate::remove_condition_locked(
    org::opensplice::core::cond::ConditionDelegate *cond)
{
    u_observable uCondition = cond->get_user_condition_unlocked();
    u_result uResult = u_waitsetDetach_s(u_waitset(this->userHandle), uCondition);
    if (uResult != U_RESULT_ALREADY_DELETED) {
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_waitsetDetach failed.");
    }
    ConditionIterator it = this->conditions_.find(cond);
    assert(it != this->conditions_.end());
    this->conditions_.erase(it);
    this->set_domain_id(u_waitsetGetDomainId(u_waitset(this->userHandle)));
}

void
org::opensplice::core::cond::WaitSetDelegate::remove_guardCondition_locked(
    org::opensplice::core::cond::ConditionDelegate *cond)
{
    ConditionIterator it = this->conditions_.find(cond);
    assert(it != this->conditions_.end());
    std::vector<ConditionDelegate *>::iterator gi = this->guards_.begin();
    while (gi != this->guards_.end() && *gi != cond) {
        ++gi;
    }
    if (gi != this->guards_.end()) {
        this->guards_.erase(gi);
    }
    this->conditions_.erase(it);

    // Trigger waitset to update the guard condition list
    u_result uResult = u_waitsetNotify(u_waitset(this->userHandle), NULL);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_waitsetNotify failed.");
}

org::opensplice::core::cond::WaitSetDelegate::ConditionSeq&
org::opensplice::core::cond::WaitSetDelegate::conditions(
    ConditionSeq& conds) const
{
    conds.clear();

    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    ConstConditionIterator it;

    for (it = this->conditions_.begin(); it != this->conditions_.end(); ++it) {
        conds.push_back(it->second);
    }

    scopedLock.unlock();

    return conds;
}

void
org::opensplice::core::cond::WaitSetDelegate::trigger(
        org::opensplice::core::cond::ConditionDelegate *cond)
{
    this->check();

    u_result uResult = u_waitsetNotify (u_waitset(this->userHandle), cond);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_waitsetNotify failed.");
}

dds::core::cond::TWaitSet<org::opensplice::core::cond::WaitSetDelegate>
org::opensplice::core::cond::WaitSetDelegate::wrapper()
{
    WaitSetDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<WaitSetDelegate>(this->get_strong_ref());
    dds::core::cond::WaitSet ws(ref);
    return ws;
}
