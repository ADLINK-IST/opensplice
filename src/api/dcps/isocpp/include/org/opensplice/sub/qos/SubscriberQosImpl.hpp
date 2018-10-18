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

#ifndef ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_IMPL_HPP_

#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

class OSPL_ISOCPP_IMPL_API SubscriberQosImpl
{
public:
    SubscriberQosImpl();

    SubscriberQosImpl(
        dds::core::policy::Presentation presentation,
        dds::core::policy::Partition partition,
        dds::core::policy::GroupData group_data,
        dds::core::policy::EntityFactory entity_factory);

    ~SubscriberQosImpl();

    void policy(const dds::core::policy::Presentation& presentation);
    void policy(const dds::core::policy::Partition& partition);
    void policy(const dds::core::policy::GroupData& grout_data);
    void policy(const dds::core::policy::EntityFactory& entity_factory);

    template <typename P> const P& policy() const;
    template <typename P> P& policy();
    bool operator ==(const SubscriberQosImpl& other) const
    {
        return other.presentation_ == presentation_ &&
               other.partition_ == partition_ &&
               other.group_data_ == group_data_ &&
               other.entity_factory_ == entity_factory_;
    }
private:
    dds::core::policy::Presentation presentation_;
    dds::core::policy::Partition partition_;
    dds::core::policy::GroupData group_data_;
    dds::core::policy::EntityFactory entity_factory_;
};

template<>
inline const ::dds::core::policy::Presentation&
org::opensplice::sub::qos::SubscriberQosImpl::policy<dds::core::policy::Presentation>() const
{
    return presentation_;
}

template<>
inline const ::dds::core::policy::Partition&
SubscriberQosImpl::policy<dds::core::policy::Partition>() const
{
    return partition_;
}

template<>
inline const ::dds::core::policy::GroupData&
SubscriberQosImpl::policy<dds::core::policy::GroupData>() const
{
    return group_data_;
}

template<>
inline const ::dds::core::policy::EntityFactory&
SubscriberQosImpl::policy<dds::core::policy::EntityFactory>() const
{
    return entity_factory_;
}

}
}
}
}

#endif /* ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_IMPL_HPP_ */
