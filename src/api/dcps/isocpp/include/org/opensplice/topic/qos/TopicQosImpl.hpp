/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_TOPIC_QOS_TOPIC_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_TOPIC_QOS_TOPIC_QOS_IMPL_HPP_

#include <dds/core/detail/conformance.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
namespace qos
{
class OSPL_ISOCPP_IMPL_API TopicQosImpl;
}
}
}
}

class org::opensplice::topic::qos::TopicQosImpl
{
public:
    TopicQosImpl();

    TopicQosImpl(const dds::core::policy::TopicData&            topic_data,
                 const dds::core::policy::Durability&           durability,

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
                 const dds::core::policy::DurabilityService&    durability_service,
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

                 const dds::core::policy::Deadline&             deadline,
                 const dds::core::policy::LatencyBudget&        budget,
                 const dds::core::policy::Liveliness&           liveliness,
                 const dds::core::policy::Reliability&          reliability,
                 const dds::core::policy::DestinationOrder&     order,
                 const dds::core::policy::History&              history,
                 const dds::core::policy::ResourceLimits&       resources,
                 const dds::core::policy::TransportPriority&    priority,
                 const dds::core::policy::Lifespan&             lifespan,
                 const dds::core::policy::Ownership&            ownership);

    ~TopicQosImpl();

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
    bool operator ==(const TopicQosImpl& other) const
    {
        return other.topic_data_ == topic_data_ &&
               other.durability_ == durability_ &&
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
               other.durability_service_ == durability_service_ &&
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
               other.deadline_ == deadline_ &&
               other.budget_ == budget_ &&
               other.liveliness_ == liveliness_ &&
               other.reliability_ == reliability_ &&
               other.order_ == order_ &&
               other.history_ == history_ &&
               other.resources_ == resources_ &&
               other.priority_ == priority_ &&
               other.lifespan_ == lifespan_ &&
               other.ownership_ == ownership_;
    }
private:
    dds::core::policy::TopicData               topic_data_;
    dds::core::policy::Durability              durability_;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    dds::core::policy::DurabilityService       durability_service_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    dds::core::policy::Deadline       deadline_;
    dds::core::policy::LatencyBudget  budget_;
    dds::core::policy::Liveliness     liveliness_;
    dds::core::policy::Reliability             reliability_;
    dds::core::policy::DestinationOrder        order_;
    dds::core::policy::History                 history_;
    dds::core::policy::ResourceLimits          resources_;
    dds::core::policy::TransportPriority       priority_;
    dds::core::policy::Lifespan                lifespan_;
    dds::core::policy::Ownership               ownership_;
};

namespace org
{
namespace opensplice
{
namespace topic
{
namespace qos
{
template<> inline const dds::core::policy::TopicData&
TopicQosImpl::policy<dds::core::policy::TopicData>() const
{
    return topic_data_;
}

template<> inline const dds::core::policy::Durability&
TopicQosImpl::policy<dds::core::policy::Durability>() const
{
    return durability_;
}


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

template<> inline const dds::core::policy::DurabilityService&
TopicQosImpl::policy<dds::core::policy::DurabilityService>() const
{
    return durability_service_;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


template<> inline const dds::core::policy::Deadline&
TopicQosImpl::policy<dds::core::policy::Deadline>() const
{
    return deadline_;
}

template<> inline const dds::core::policy::LatencyBudget&
TopicQosImpl::policy<dds::core::policy::LatencyBudget>() const
{
    return budget_;
}

template<> inline const dds::core::policy::Liveliness&
TopicQosImpl::policy<dds::core::policy::Liveliness>() const
{
    return liveliness_;
}

template<> inline const dds::core::policy::Reliability&
TopicQosImpl::policy<dds::core::policy::Reliability>() const
{
    return reliability_;
}


template<> inline const dds::core::policy::DestinationOrder&
TopicQosImpl::policy<dds::core::policy::DestinationOrder>() const
{
    return order_;
}

template<> inline const dds::core::policy::History&
TopicQosImpl::policy<dds::core::policy::History>() const
{
    return history_;
}


template<> inline const dds::core::policy::ResourceLimits&
TopicQosImpl::policy<dds::core::policy::ResourceLimits>() const
{
    return resources_;
}


template<> inline const dds::core::policy::TransportPriority&
TopicQosImpl::policy<dds::core::policy::TransportPriority>() const
{
    return priority_;
}


template<> inline const dds::core::policy::Lifespan&
TopicQosImpl::policy<dds::core::policy::Lifespan>() const
{
    return lifespan_;
}

template<> inline const  dds::core::policy::Ownership&
TopicQosImpl::policy<dds::core::policy::Ownership>() const
{
    return ownership_;
}
}
}
}
}

#endif /* ORG_OPENSPLICE_TOPIC_QOS_TOPIC_QOS_IMPL_HPP_ */
