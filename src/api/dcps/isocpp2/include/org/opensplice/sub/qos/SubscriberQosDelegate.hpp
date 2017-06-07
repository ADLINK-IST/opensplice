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

#ifndef ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_DELEGATE_HPP_
#define ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_DELEGATE_HPP_

#include <dds/core/policy/CorePolicy.hpp>

#include "u_types.h"

struct _DDS_NamedSubscriberQos;

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

class OMG_DDS_API SubscriberQosDelegate
{
public:
    SubscriberQosDelegate();
    SubscriberQosDelegate(const SubscriberQosDelegate& other);

    ~SubscriberQosDelegate();

    void policy(const dds::core::policy::Presentation& presentation);
    void policy(const dds::core::policy::Partition& partition);
    void policy(const dds::core::policy::GroupData& grout_data);
    void policy(const dds::core::policy::EntityFactory& entity_factory);
    void policy(const org::opensplice::core::policy::Share& share);

    template <typename POLICY> const POLICY& policy() const;
    template <typename POLICY> POLICY& policy();

    /* The returned userlayer QoS has to be freed. */
    u_subscriberQos u_qos() const;
    void u_qos(const u_subscriberQos qos);

    void named_qos(const struct _DDS_NamedSubscriberQos &qos);

    void check() const;

    bool operator ==(const SubscriberQosDelegate& other) const;
    SubscriberQosDelegate& operator =(const SubscriberQosDelegate& other);

private:
    void defaults();

    dds::core::policy::Presentation presentation_;
    dds::core::policy::Partition partition_;
    dds::core::policy::GroupData group_data_;
    dds::core::policy::EntityFactory entity_factory_;
    org::opensplice::core::policy::Share share_;
};



//==============================================================================


template<>
inline const dds::core::policy::Presentation&
SubscriberQosDelegate::policy<dds::core::policy::Presentation>() const
{
    return presentation_;
}
template<>
inline dds::core::policy::Presentation&
SubscriberQosDelegate::policy<dds::core::policy::Presentation>()
{
    return presentation_;
}


template<>
inline const dds::core::policy::Partition&
SubscriberQosDelegate::policy<dds::core::policy::Partition>() const
{
    return partition_;
}
template<>
inline dds::core::policy::Partition&
SubscriberQosDelegate::policy<dds::core::policy::Partition>()
{
    return partition_;
}


template<>
inline const dds::core::policy::GroupData&
SubscriberQosDelegate::policy<dds::core::policy::GroupData>() const
{
    return group_data_;
}
template<>
inline dds::core::policy::GroupData&
SubscriberQosDelegate::policy<dds::core::policy::GroupData>()
{
    return group_data_;
}


template<>
inline const dds::core::policy::EntityFactory&
SubscriberQosDelegate::policy<dds::core::policy::EntityFactory>() const
{
    return entity_factory_;
}
template<>
inline dds::core::policy::EntityFactory&
SubscriberQosDelegate::policy<dds::core::policy::EntityFactory>()
{
    return entity_factory_;
}


template<>
inline const org::opensplice::core::policy::Share&
SubscriberQosDelegate::policy<org::opensplice::core::policy::Share>() const
{
    return share_;
}
template<>
inline org::opensplice::core::policy::Share&
SubscriberQosDelegate::policy<org::opensplice::core::policy::Share>()
{
    return share_;
}

}
}
}
}

#endif /* ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_DELEGATE_HPP_ */
