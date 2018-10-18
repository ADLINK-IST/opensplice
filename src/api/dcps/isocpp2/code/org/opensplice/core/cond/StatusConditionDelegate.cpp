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

#include <org/opensplice/core/cond/StatusConditionDelegate.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/status/StatusUtils.hpp>

#include <dds/core/cond/StatusCondition.hpp>

#include "u_statusCondition.h"

org::opensplice::core::cond::StatusConditionDelegate::StatusConditionDelegate(
        const org::opensplice::core::EntityDelegate *entity,
        u_entity uEntity) :
                org::opensplice::core::cond::ConditionDelegate(),
                myEntity(OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::core::EntityDelegate>(entity->get_strong_ref())),
                myMask(dds::core::status::StatusMask::all())
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    this->userHandle = u_object(u_statusConditionNew(uEntity));
    if (!this->userHandle) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Failed to create StatusCondition");
    }
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
org::opensplice::core::cond::StatusConditionDelegate::~StatusConditionDelegate()
{
}

void
org::opensplice::core::cond::StatusConditionDelegate::close()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    org::opensplice::core::cond::ConditionDelegate::close();
}

void
org::opensplice::core::cond::StatusConditionDelegate::init(
        org::opensplice::core::ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
}

void
org::opensplice::core::cond::StatusConditionDelegate::enabled_statuses(
        const dds::core::status::StatusMask& status)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    v_eventMask vMask = org::opensplice::core::utils::vEventMaskFromStatusMask(status);
    u_result uResult = u_statusCondition_set_mask(u_statusCondition(this->userHandle), vMask);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not apply suggested mask to StatusCondition.");
    myMask = status;
}

dds::core::status::StatusMask
org::opensplice::core::cond::StatusConditionDelegate::enabled_statuses() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    dds::core::status::StatusMask resultMask(this->myMask);

    return resultMask;
}

dds::core::Entity&
org::opensplice::core::cond::StatusConditionDelegate::entity()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    dds::core::Entity& resultEntity(this->myEntity);

    return resultEntity;
}

bool
org::opensplice::core::cond::StatusConditionDelegate::trigger_value() const
{
    c_ulong triggerValue = 0;

    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_statusCondition_get_triggerValue(u_statusCondition(this->userHandle), &triggerValue);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not obtain triggerValue for StatusCondition.");

    return (triggerValue > 0);
}

dds::core::cond::TStatusCondition<org::opensplice::core::cond::StatusConditionDelegate>
org::opensplice::core::cond::StatusConditionDelegate::wrapper()
{
    org::opensplice::core::cond::StatusConditionDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<StatusConditionDelegate>(this->get_strong_ref());
    dds::core::cond::TStatusCondition<StatusConditionDelegate> statusCondition(ref);

    return statusCondition;
}
