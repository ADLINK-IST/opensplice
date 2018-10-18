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

#ifndef ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_IMPL_HPP_

#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{
class PublisherQosImpl;
}
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::pub::qos::PublisherQosImpl
{
public:
    PublisherQosImpl();

    PublisherQosImpl(const PublisherQosImpl& other);

    PublisherQosImpl(const dds::core::policy::Presentation& presentation,
                     const dds::core::policy::Partition& partition,
                     const dds::core::policy::GroupData& gdata,
                     const dds::core::policy::EntityFactory& factory_policy);

    template <typename POLICY>
    const POLICY& policy() const;

    template <typename POLICY>
    POLICY& policy();

public:
    void policy(const dds::core::policy::Presentation& presentation);
    void policy(const dds::core::policy::Partition& partition);
    void policy(const dds::core::policy::GroupData& gdata);
    void policy(const dds::core::policy::EntityFactory& factory_policy);
    bool operator ==(const PublisherQosImpl& other) const
    {
        return other.presentation_ == presentation_ &&
               other.partition_ == partition_ &&
               other.gdata_ == gdata_ &&
               other.factory_policy_ == factory_policy_;
    }
private:
    dds::core::policy::Presentation          presentation_;
    dds::core::policy::Partition             partition_;
    dds::core::policy::GroupData             gdata_;
    dds::core::policy::EntityFactory   factory_policy_;
};

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{
template<>
inline const dds::core::policy::Presentation&
PublisherQosImpl::policy<dds::core::policy::Presentation>() const
{
    return presentation_;
}

template<>
inline const dds::core::policy::Partition&
PublisherQosImpl::policy<dds::core::policy::Partition>() const
{
    return partition_;
}

template<>
inline const dds::core::policy::GroupData&
PublisherQosImpl::policy<dds::core::policy::GroupData>() const
{
    return gdata_;
}

template<>
inline const dds::core::policy::EntityFactory&
PublisherQosImpl::policy<dds::core::policy::EntityFactory>() const
{
    return factory_policy_;
}
}
}
}
}

#endif /* ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_IMPL_HPP_ */
