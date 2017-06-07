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

#ifndef ORG_OPENSPLICE_CORE_STATUS_CONDITION_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_STATUS_CONDITION_DELEGATE_HPP_

#include <dds/core/Entity.hpp>
#include <dds/core/status/Status.hpp>
#include <org/opensplice/core/cond/ConditionDelegate.hpp>

namespace dds
{
namespace core
{
namespace cond
{
template <typename T> class TStatusCondition;
}
}
}

namespace org
{
namespace opensplice
{
namespace core
{
namespace cond
{

class OMG_DDS_API StatusConditionDelegate : public org::opensplice::core::cond::ConditionDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits<StatusConditionDelegate>::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits<StatusConditionDelegate>::weak_ref_type weak_ref_type;

    StatusConditionDelegate(
            const org::opensplice::core::EntityDelegate *entity,
            u_entity uEntity);

    ~StatusConditionDelegate();

    virtual void close();

    void init(org::opensplice::core::ObjectDelegate::weak_ref_type weak_ref);

    void enabled_statuses(const dds::core::status::StatusMask& status);

    dds::core::status::StatusMask enabled_statuses() const;

    dds::core::Entity& entity();

    virtual bool trigger_value() const;

    dds::core::cond::TStatusCondition<StatusConditionDelegate> wrapper();

private:
    dds::core::Entity myEntity;
    dds::core::status::StatusMask myMask;
};

}
}
}
}

#endif  /* ORG_OPENSPLICE_CORE_STATUS_CONDITION_DELEGATE_HPP_ */
