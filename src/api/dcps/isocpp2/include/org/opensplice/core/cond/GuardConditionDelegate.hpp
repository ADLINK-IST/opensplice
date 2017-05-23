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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_GUARD_CONDITION_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_GUARD_CONDITION_DELEGATE_HPP_

#include <org/opensplice/core/cond/ConditionDelegate.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
namespace cond
{

class OMG_DDS_API GuardConditionDelegate : public org::opensplice::core::cond::ConditionDelegate
{
public:
    GuardConditionDelegate();

    template<typename FUN>
    GuardConditionDelegate(const FUN& functor) :
            org::opensplice::core::cond::ConditionDelegate(functor),
            myTriggerValue(false)
    {
    }

    ~GuardConditionDelegate();

    void init(org::opensplice::core::ObjectDelegate::weak_ref_type weak_ref);

    virtual bool trigger_value() const;

    void trigger_value(bool value);

    virtual void add_waitset(
            const dds::core::cond::TCondition<ConditionDelegate>& cond,
            org::opensplice::core::cond::WaitSetDelegate *waitset);

    virtual bool remove_waitset(
            org::opensplice::core::cond::WaitSetDelegate *waitset);

private:
    bool myTriggerValue;
};

}
}
}
}

#endif  /* ORG_OPENSPLICE_CORE_GUARD_CONDITION_DELEGATE_HPP_ */
