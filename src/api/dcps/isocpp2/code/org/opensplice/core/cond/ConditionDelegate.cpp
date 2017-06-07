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
 */


#include <org/opensplice/core/cond/ConditionDelegate.hpp>
#include <org/opensplice/core/cond/WaitSetDelegate.hpp>
#include <dds/core/cond/WaitSet.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

#include <dds/core/cond/Condition.hpp>

org::opensplice::core::cond::ConditionDelegate::ConditionDelegate() :
        myFunctor(NULL)
{
}

/* The close() operation of Condition will try to remove this Condition from
 * its WaitSets. However, since the WaitSets hold a reference to their Conditions,
 * the destructor can never be invoked for Conditions that are still attached
 * to WaitSets.
 * For that reason we know that if the destructor is invoked, the Condition
 * can no longer be attached to a WaitSet, so we can skip the local close()
 * and immediately proceed the the close() of the UserObjectDelegate parent.
 * If we would try to invoke Condition::close() here, then we would run
 * into a deadlock when we claim the WaitSet lock in case this destructor
 * is invoked by the destructor of the WaitSet, which has the WaitSet already
 * locked before.
 */
org::opensplice::core::cond::ConditionDelegate::~ConditionDelegate()
{
    if (this->myFunctor) {
        delete this->myFunctor;
        this->myFunctor = NULL;
    }
    if (!this->closed) {
        try {
            UserObjectDelegate::close();

        } catch (...) {
            /* Empty: the exception throw should have already traced an error. */
        }
    }
}

void
org::opensplice::core::cond::ConditionDelegate::close()
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->waitSetListMutex);

    std::vector<WaitSetDelegate *> list;

    for (waitsetIterator it = this->waitsets.begin(); it != this->waitsets.end(); ++it) {
        list.push_back(*it);
    }
    scopedLock.unlock();

    for (std::vector<WaitSetDelegate *>::iterator it = list.begin(); it != list.end(); ++it) {
        (*it)->detach_condition(this);
    }

    UserObjectDelegate::close();

    if (this->myFunctor) {
        delete this->myFunctor;
        this->myFunctor = NULL;
    }
}


void
org::opensplice::core::cond::ConditionDelegate::init(ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
}

void
org::opensplice::core::cond::ConditionDelegate::reset_handler()
{
    if (this->myFunctor) {
        delete this->myFunctor;
        this->myFunctor = NULL;
    }
}


void
org::opensplice::core::cond::ConditionDelegate::add_waitset(
        const dds::core::cond::TCondition<ConditionDelegate>& cond,
        org::opensplice::core::cond::WaitSetDelegate *waitset)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->waitSetListMutex);

    bool added = this->waitsets.insert(waitset).second;
    if (added) {
        waitset->add_condition_locked(cond);
    }
}

bool
org::opensplice::core::cond::ConditionDelegate::remove_waitset(
        org::opensplice::core::cond::WaitSetDelegate *waitset)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->waitSetListMutex);

    bool erased = this->waitsets.erase(waitset);
    if (erased) {
        waitset->remove_condition_locked(this);
    } else {
        ISOCPP_THROW_EXCEPTION(ISOCPP_PRECONDITION_NOT_MET_ERROR, "Condition was not attached to WaitSet");
    }

    return erased;
}

void
org::opensplice::core::cond::ConditionDelegate::dispatch()
{
    if (this->trigger_value() && this->myFunctor) {
        dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate> cond = this->wrapper();
        this->myFunctor->dispatch(cond);
    }
}

u_observable
org::opensplice::core::cond::ConditionDelegate::get_user_condition()
{
    return u_observable(this->get_user_handle());
}

u_observable
org::opensplice::core::cond::ConditionDelegate::get_user_condition_unlocked()
{
    return u_observable(this->userHandle);
}

dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate>
org::opensplice::core::cond::ConditionDelegate::wrapper()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    org::opensplice::core::cond::ConditionDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<ConditionDelegate>(this->get_strong_ref());
    dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate> condition(ref);

    return condition;
}
