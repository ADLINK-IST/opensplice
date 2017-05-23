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

#ifndef ORG_OPENSPLICE_CORE_POLICY_QOS_POLICY_COUNT_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_QOS_POLICY_COUNT_DELEGATE_HPP_

#include <dds/core/types.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{

class QosPolicyCountDelegate
{
public:
    QosPolicyCountDelegate(dds::core::policy::QosPolicyId id, uint32_t count)
        : policy_id_(id),
          count_(count)
    { }

    QosPolicyCountDelegate(const QosPolicyCountDelegate& other)
        : policy_id_(other.policy_id()),
          count_(other.count())
    { }

public:
    dds::core::policy::QosPolicyId policy_id() const
    {
        return policy_id_;
    }
    void policy_id(dds::core::policy::QosPolicyId id)
    {
        policy_id_ = id;
    }

    uint32_t count() const
    {
        return count_;
    }
    void count(uint32_t c)
    {
        count_ = c;
    }

    bool operator ==(const QosPolicyCountDelegate& other) const
    {
        return other.policy_id_ == policy_id_ &&
               other.count_ == count_;
    }


private:
    dds::core::policy::QosPolicyId policy_id_;
    uint32_t count_;
};

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_POLICY_QOS_POLICY_COUNT_DELEGATE_HPP_ */
