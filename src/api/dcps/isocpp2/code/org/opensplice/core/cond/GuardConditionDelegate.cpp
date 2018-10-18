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

#include <org/opensplice/core/cond/GuardConditionDelegate.hpp>
#include <org/opensplice/core/cond/WaitSetDelegate.hpp>

org::opensplice::core::cond::GuardConditionDelegate::GuardConditionDelegate() :
        org::opensplice::core::cond::ConditionDelegate(),
        myTriggerValue(false)
{
}

org::opensplice::core::cond::GuardConditionDelegate::~GuardConditionDelegate()
{
}

void
org::opensplice::core::cond::GuardConditionDelegate::init(
        org::opensplice::core::ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
}

bool
org::opensplice::core::cond::GuardConditionDelegate::trigger_value() const
{
    return this->myTriggerValue;
}

void
org::opensplice::core::cond::GuardConditionDelegate::trigger_value(bool value)
{
    org::opensplice::core::ScopedMutexLock scopedWsListLock(this->waitSetListMutex);

    this->myTriggerValue = value;
    if (value) {
        for (waitsetIterator it = this->waitsets.begin(); it != this->waitsets.end(); ++it)
        {
            (*it)->trigger(this);
        }
    }
    scopedWsListLock.unlock();
}

void
org::opensplice::core::cond::GuardConditionDelegate::add_waitset(
        const dds::core::cond::Condition& cond,
        org::opensplice::core::cond::WaitSetDelegate *waitset)
{
    org::opensplice::core::ScopedMutexLock scopedWsListLock(this->waitSetListMutex);

    bool added = this->waitsets.insert(waitset).second;
    if (added) {
        waitset->add_guardCondition_locked(cond);
    }
    scopedWsListLock.unlock();
}

bool
org::opensplice::core::cond::GuardConditionDelegate::remove_waitset(
        org::opensplice::core::cond::WaitSetDelegate *waitset)
{
    org::opensplice::core::ScopedMutexLock scopedWsListLock(this->waitSetListMutex);

    bool erased = this->waitsets.erase(waitset);
    if (erased) {
        waitset->remove_guardCondition_locked(this);
    } else {
        ISOCPP_THROW_EXCEPTION(ISOCPP_PRECONDITION_NOT_MET_ERROR, "Condition was not attached to WaitSet");
    }
    scopedWsListLock.unlock();

    return erased;
}


