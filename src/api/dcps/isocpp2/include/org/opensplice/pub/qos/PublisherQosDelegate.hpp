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

#ifndef ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_DELEGATE_HPP_
#define ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_DELEGATE_HPP_

#include <dds/core/policy/CorePolicy.hpp>

#include "u_types.h"

struct _DDS_NamedPublisherQos;

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{

class OMG_DDS_API PublisherQosDelegate
{
public:
    PublisherQosDelegate();
    PublisherQosDelegate(const PublisherQosDelegate& other);

    ~PublisherQosDelegate();

    void policy(const dds::core::policy::Presentation& presentation);
    void policy(const dds::core::policy::Partition& partition);
    void policy(const dds::core::policy::GroupData& gdata);
    void policy(const dds::core::policy::EntityFactory& factory_policy);

    template <typename POLICY> const POLICY& policy() const;
    template <typename POLICY> POLICY& policy();

    /* The returned userlayer QoS has to be freed. */
    u_publisherQos u_qos() const;
    void u_qos(const u_publisherQos qos);

    void named_qos(const struct _DDS_NamedPublisherQos &qos);

    void check() const;

    bool operator ==(const PublisherQosDelegate& other) const;
    PublisherQosDelegate& operator =(const PublisherQosDelegate& other);

private:
    void defaults();

    dds::core::policy::Presentation    presentation_;
    dds::core::policy::Partition       partition_;
    dds::core::policy::GroupData       gdata_;
    dds::core::policy::EntityFactory   factory_policy_;
};



//==============================================================================


template<>
inline const dds::core::policy::Presentation&
PublisherQosDelegate::policy<dds::core::policy::Presentation>() const
{
    return presentation_;
}
template<>
inline dds::core::policy::Presentation&
PublisherQosDelegate::policy<dds::core::policy::Presentation>()
{
    return presentation_;
}


template<>
inline const dds::core::policy::Partition&
PublisherQosDelegate::policy<dds::core::policy::Partition>() const
{
    return partition_;
}
template<>
inline dds::core::policy::Partition&
PublisherQosDelegate::policy<dds::core::policy::Partition>()
{
    return partition_;
}


template<>
inline const dds::core::policy::GroupData&
PublisherQosDelegate::policy<dds::core::policy::GroupData>() const
{
    return gdata_;
}
template<>
inline dds::core::policy::GroupData&
PublisherQosDelegate::policy<dds::core::policy::GroupData>()
{
    return gdata_;
}


template<>
inline const dds::core::policy::EntityFactory&
PublisherQosDelegate::policy<dds::core::policy::EntityFactory>() const
{
    return factory_policy_;
}
template<>
inline dds::core::policy::EntityFactory&
PublisherQosDelegate::policy<dds::core::policy::EntityFactory>()
{
    return factory_policy_;
}

}
}
}
}

#endif /* ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_DELEGATE_HPP_ */
