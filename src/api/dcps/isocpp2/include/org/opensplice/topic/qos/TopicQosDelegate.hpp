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

#ifndef ORG_OPENSPLICE_TOPIC_QOS_TOPIC_QOS_DELEGATE_HPP_
#define ORG_OPENSPLICE_TOPIC_QOS_TOPIC_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <dds/core/policy/CorePolicy.hpp>

#include "u_types.h"

struct _DDS_NamedTopicQos;

namespace org
{
namespace opensplice
{
namespace topic
{
namespace qos
{

class OMG_DDS_API TopicQosDelegate
{
public:
    TopicQosDelegate();
    TopicQosDelegate(const TopicQosDelegate& other);

    ~TopicQosDelegate();

    void policy(const dds::core::policy::TopicData&          topic_data);
    void policy(const dds::core::policy::Durability&         durability);
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    void policy(const dds::core::policy::DurabilityService&  durability_service);
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    void policy(const dds::core::policy::Deadline&           deadline);
    void policy(const dds::core::policy::LatencyBudget&      budget);
    void policy(const dds::core::policy::Liveliness&         liveliness);
    void policy(const dds::core::policy::Reliability&        reliability);
    void policy(const dds::core::policy::DestinationOrder&   order);
    void policy(const dds::core::policy::History&            history);
    void policy(const dds::core::policy::ResourceLimits&     resources);
    void policy(const dds::core::policy::TransportPriority&  priority);
    void policy(const dds::core::policy::Lifespan&           lifespan);
    void policy(const dds::core::policy::Ownership&          ownership);

    template <typename POLICY> const POLICY& policy() const;
    template <typename POLICY> POLICY& policy();

    /* The returned userlayer QoS has to be freed. */
    u_topicQos u_qos() const;
    void u_qos(const u_topicQos qos);

    void named_qos(const struct _DDS_NamedTopicQos &qos);

    void check() const;

    bool force_merge(void) const;
    void force_merge(bool force);

    bool operator ==(const TopicQosDelegate& other) const;
    TopicQosDelegate& operator =(const TopicQosDelegate& other);

private:
    void defaults();

    dds::core::policy::TopicData              topic_data_;
    dds::core::policy::Durability             durability_;
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    dds::core::policy::DurabilityService      durability_service_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    dds::core::policy::Deadline               deadline_;
    dds::core::policy::LatencyBudget          budget_;
    dds::core::policy::Liveliness             liveliness_;
    dds::core::policy::Reliability            reliability_;
    dds::core::policy::DestinationOrder       order_;
    dds::core::policy::History                history_;
    dds::core::policy::ResourceLimits         resources_;
    dds::core::policy::TransportPriority      priority_;
    dds::core::policy::Lifespan               lifespan_;
    dds::core::policy::Ownership              ownership_;

    bool merge_;
};



//==============================================================================


template<> inline const dds::core::policy::TopicData&
TopicQosDelegate::policy<dds::core::policy::TopicData>() const
{
    return topic_data_;
}
template<> inline dds::core::policy::TopicData&
TopicQosDelegate::policy<dds::core::policy::TopicData>()
{
    return topic_data_;
}


template<> inline const dds::core::policy::Durability&
TopicQosDelegate::policy<dds::core::policy::Durability>() const
{
    return durability_;
}
template<> inline dds::core::policy::Durability&
TopicQosDelegate::policy<dds::core::policy::Durability>()
{
    return durability_;
}


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
template<> inline const dds::core::policy::DurabilityService&
TopicQosDelegate::policy<dds::core::policy::DurabilityService>() const
{
    return durability_service_;
}
template<> inline dds::core::policy::DurabilityService&
TopicQosDelegate::policy<dds::core::policy::DurabilityService>()
{
    return durability_service_;
}
#endif  // OMG_DDS_PERSISTENCE_SUPPORT


template<> inline const dds::core::policy::Deadline&
TopicQosDelegate::policy<dds::core::policy::Deadline>() const
{
    return deadline_;
}
template<> inline dds::core::policy::Deadline&
TopicQosDelegate::policy<dds::core::policy::Deadline>()
{
    return deadline_;
}


template<> inline const dds::core::policy::LatencyBudget&
TopicQosDelegate::policy<dds::core::policy::LatencyBudget>() const
{
    return budget_;
}
template<> inline dds::core::policy::LatencyBudget&
TopicQosDelegate::policy<dds::core::policy::LatencyBudget>()
{
    return budget_;
}


template<> inline const dds::core::policy::Liveliness&
TopicQosDelegate::policy<dds::core::policy::Liveliness>() const
{
    return liveliness_;
}
template<> inline dds::core::policy::Liveliness&
TopicQosDelegate::policy<dds::core::policy::Liveliness>()
{
    return liveliness_;
}


template<> inline const dds::core::policy::Reliability&
TopicQosDelegate::policy<dds::core::policy::Reliability>() const
{
    return reliability_;
}
template<> inline dds::core::policy::Reliability&
TopicQosDelegate::policy<dds::core::policy::Reliability>()
{
    return reliability_;
}


template<> inline const dds::core::policy::DestinationOrder&
TopicQosDelegate::policy<dds::core::policy::DestinationOrder>() const
{
    return order_;
}
template<> inline dds::core::policy::DestinationOrder&
TopicQosDelegate::policy<dds::core::policy::DestinationOrder>()
{
    return order_;
}


template<> inline const dds::core::policy::History&
TopicQosDelegate::policy<dds::core::policy::History>() const
{
    return history_;
}
template<> inline dds::core::policy::History&
TopicQosDelegate::policy<dds::core::policy::History>()
{
    return history_;
}


template<> inline const dds::core::policy::ResourceLimits&
TopicQosDelegate::policy<dds::core::policy::ResourceLimits>() const
{
    return resources_;
}
template<> inline dds::core::policy::ResourceLimits&
TopicQosDelegate::policy<dds::core::policy::ResourceLimits>()
{
    return resources_;
}


template<> inline const dds::core::policy::TransportPriority&
TopicQosDelegate::policy<dds::core::policy::TransportPriority>() const
{
    return priority_;
}
template<> inline dds::core::policy::TransportPriority&
TopicQosDelegate::policy<dds::core::policy::TransportPriority>()
{
    return priority_;
}


template<> inline const dds::core::policy::Lifespan&
TopicQosDelegate::policy<dds::core::policy::Lifespan>() const
{
    return lifespan_;
}
template<> inline dds::core::policy::Lifespan&
TopicQosDelegate::policy<dds::core::policy::Lifespan>()
{
    return lifespan_;
}


template<> inline const  dds::core::policy::Ownership&
TopicQosDelegate::policy<dds::core::policy::Ownership>() const
{
    return ownership_;
}
template<> inline dds::core::policy::Ownership&
TopicQosDelegate::policy<dds::core::policy::Ownership>()
{
    return ownership_;
}

}
}
}
}

#endif /* ORG_OPENSPLICE_TOPIC_QOS_TOPIC_QOS_DELEGATE_HPP_ */
