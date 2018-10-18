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
 */


#include <org/opensplice/sub/cond/ReadConditionDelegate.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/ReportUtils.hpp>



org::opensplice::sub::cond::ReadConditionDelegate::ReadConditionDelegate(
    const dds::sub::AnyDataReader& dr,
    const dds::sub::status::DataState& state_filter) :
        QueryDelegate(dr, state_filter)
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
org::opensplice::sub::cond::ReadConditionDelegate::~ReadConditionDelegate()
{
    if (!this->closed) {
        try {
            QueryDelegate::deinit();
        } catch (...) {
            /* Empty: the exception throw should have already traced an error. */
        }
    }
}

void
org::opensplice::sub::cond::ReadConditionDelegate::init(
    ObjectDelegate::weak_ref_type weak_ref)
{
    QueryDelegate::init(weak_ref);
}

void
org::opensplice::sub::cond::ReadConditionDelegate::close()
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);

    QueryDelegate::deinit();

    ConditionDelegate::close();
}

unsigned char
org::opensplice::sub::cond::ReadConditionDelegate::always_true(
        void *object,
        void *arg)
{
    return true;
}

bool
org::opensplice::sub::cond::ReadConditionDelegate::trigger_value() const
{
    bool result = (bool) u_queryTest(u_query(((ReadConditionDelegate *)this)->get_user_condition()), always_true, NULL);

    return result;
}

bool
org::opensplice::sub::cond::ReadConditionDelegate::modify_state_filter(
    dds::sub::status::DataState& s)
{
    return this->state_filter_equal(s);
}

u_observable
org::opensplice::sub::cond::ReadConditionDelegate::get_user_condition()
{
    return QueryDelegate::get_user_condition();
}
