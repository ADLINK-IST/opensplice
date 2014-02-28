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

#ifndef ORG_OPENSPLICE_SUB_QOS_DATA_READER_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_QOS_DATA_READER_QOS_IMPL_HPP_

#include <dds/core/detail/conformance.hpp>
#include <dds/core/policy/CorePolicy.hpp>

#include <org/opensplice/topic/qos/TopicQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{
class OSPL_ISOCPP_IMPL_API DataReaderQosImpl;
}
}
}
}

class org::opensplice::sub::qos::DataReaderQosImpl
{
public:
    DataReaderQosImpl();

    DataReaderQosImpl(const org::opensplice::topic::qos::TopicQosImpl& tqos);

    DataReaderQosImpl(::dds::core::policy::UserData               user_data,
                      ::dds::core::policy::Durability              durability,

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
                      ::dds::core::policy::DurabilityService       durability_service,
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

                      ::dds::core::policy::Deadline       deadline,
                      ::dds::core::policy::LatencyBudget  budget,
                      ::dds::core::policy::Liveliness     liveliness,
                      ::dds::core::policy::Reliability             reliability,
                      ::dds::core::policy::DestinationOrder        order,
                      ::dds::core::policy::History                 history,
                      ::dds::core::policy::ResourceLimits          resources,
                      ::dds::core::policy::TransportPriority       priority,
                      ::dds::core::policy::Lifespan                lifespan,
                      ::dds::core::policy::Ownership               ownership,
                      ::dds::core::policy::TimeBasedFilter         tfilter,
                      ::dds::core::policy::ReaderDataLifecycle     lifecycle);

    ~DataReaderQosImpl();


    void policy(const ::dds::core::policy::UserData&            user_data);
    void policy(const ::dds::core::policy::Durability&          durability);

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    void policy(const ::dds::core::policy::DurabilityService&   durability_service);
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    void policy(const ::dds::core::policy::Deadline&            deadline);
    void policy(const ::dds::core::policy::LatencyBudget&       budget);
    void policy(const ::dds::core::policy::Liveliness&          liveliness);
    void policy(const ::dds::core::policy::Reliability&         reliability);
    void policy(const ::dds::core::policy::DestinationOrder&    order);
    void policy(const ::dds::core::policy::History&             history);
    void policy(const ::dds::core::policy::ResourceLimits&      resources);
    void policy(const ::dds::core::policy::TransportPriority&   priority);
    void policy(const ::dds::core::policy::Lifespan&            lifespan);
    void policy(const ::dds::core::policy::Ownership&           ownership);
    void policy(const ::dds::core::policy::TimeBasedFilter&     tfilter);
    void policy(const ::dds::core::policy::ReaderDataLifecycle& lifecycle);

    template <typename P> const P& policy() const;
    template <typename P> P& policy();

    bool operator==(const DataReaderQosImpl& other) const
    {
        return other.user_data_ == user_data_ && other.durability_ == durability_
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
               && other.durability_service_ == durability_service_
#endif // OMG_DDS_PERSISTENCE_SUPPORT
               && other.deadline_ == deadline_ && other.budget_ == budget_
               && other.liveliness_ == liveliness_ && other.reliability_ == reliability_
               && other.order_ == other.order_ && other.history_ == history_
               && other.resources_ == resources_ && other.priority_ == priority_
               && other.lifespan_ == lifespan_ && other.ownership_ == ownership_
               && other.tfilter_ == tfilter_ && other.lifecycle_ == lifecycle_;
    }

private:
    ::dds::core::policy::UserData               user_data_;
    ::dds::core::policy::Durability              durability_;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    ::dds::core::policy::DurabilityService       durability_service_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    ::dds::core::policy::Deadline                deadline_;
    ::dds::core::policy::LatencyBudget           budget_;
    ::dds::core::policy::Liveliness              liveliness_;
    ::dds::core::policy::Reliability             reliability_;
    ::dds::core::policy::DestinationOrder        order_;
    ::dds::core::policy::History                 history_;
    ::dds::core::policy::ResourceLimits          resources_;
    ::dds::core::policy::TransportPriority       priority_;
    ::dds::core::policy::Lifespan                lifespan_;
    ::dds::core::policy::Ownership               ownership_;
    ::dds::core::policy::TimeBasedFilter         tfilter_;
    ::dds::core::policy::ReaderDataLifecycle     lifecycle_;

};

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

template<>
inline const ::dds::core::policy::Durability&
DataReaderQosImpl::policy<dds::core::policy::Durability>() const
{
    return durability_;
}

template<>
inline ::dds::core::policy::Durability&
DataReaderQosImpl::policy<dds::core::policy::Durability>()
{
    return durability_;
}


template<>
inline const ::dds::core::policy::UserData&
DataReaderQosImpl::policy<dds::core::policy::UserData>() const
{
    return user_data_;
}

template<>
inline ::dds::core::policy::UserData&
DataReaderQosImpl::policy<dds::core::policy::UserData>()
{
    return user_data_;
}


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

template<> inline const dds::core::policy::DurabilityService&
DataReaderQosImpl::policy<dds::core::policy::DurabilityService>() const
{
    return durability_service_;
}

template<>
inline dds::core::policy::DurabilityService&
DataReaderQosImpl::policy<dds::core::policy::DurabilityService>()
{
    return durability_service_;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


template<> inline const dds::core::policy::Deadline&
DataReaderQosImpl::policy<dds::core::policy::Deadline>() const
{
    return deadline_;
}

template<>
inline dds::core::policy::Deadline&
DataReaderQosImpl::policy<dds::core::policy::Deadline>()
{
    return deadline_;
}


template<> inline const dds::core::policy::LatencyBudget&
DataReaderQosImpl::policy<dds::core::policy::LatencyBudget>() const
{
    return budget_;
}

template<>
inline dds::core::policy::LatencyBudget&
DataReaderQosImpl::policy<dds::core::policy::LatencyBudget>()
{
    return budget_;
}


template<> inline const dds::core::policy::Liveliness&
DataReaderQosImpl::policy<dds::core::policy::Liveliness>() const
{
    return liveliness_;
}

template<>
inline dds::core::policy::Liveliness&
DataReaderQosImpl::policy<dds::core::policy::Liveliness>()
{
    return liveliness_;
}


template<> inline const dds::core::policy::Reliability&
DataReaderQosImpl::policy<dds::core::policy::Reliability>() const
{
    return reliability_;
}

template<>
inline dds::core::policy::Reliability&
DataReaderQosImpl::policy<dds::core::policy::Reliability>()
{
    return reliability_;
}


template<> inline const dds::core::policy::DestinationOrder&
DataReaderQosImpl::policy<dds::core::policy::DestinationOrder>() const
{
    return order_;
}

template<>
inline dds::core::policy::DestinationOrder&
DataReaderQosImpl::policy<dds::core::policy::DestinationOrder>()
{
    return order_;
}


template<> inline const dds::core::policy::History&
DataReaderQosImpl::policy<dds::core::policy::History>() const
{
    return history_;
}

template<>
inline dds::core::policy::History&
DataReaderQosImpl::policy<dds::core::policy::History>()
{
    return history_;
}


template<> inline const dds::core::policy::ResourceLimits&
DataReaderQosImpl::policy<dds::core::policy::ResourceLimits>() const
{
    return resources_;
}

template<>
inline dds::core::policy::ResourceLimits&
DataReaderQosImpl::policy<dds::core::policy::ResourceLimits>()
{
    return resources_;
}


template<> inline const dds::core::policy::TransportPriority&
DataReaderQosImpl::policy<dds::core::policy::TransportPriority>() const
{
    return priority_;
}

template<>
inline dds::core::policy::TransportPriority&
DataReaderQosImpl::policy<dds::core::policy::TransportPriority>()
{
    return priority_;
}


template<> inline const dds::core::policy::Lifespan&
DataReaderQosImpl::policy<dds::core::policy::Lifespan>() const
{
    return lifespan_;
}

template<>
inline dds::core::policy::Lifespan&
DataReaderQosImpl::policy<dds::core::policy::Lifespan>()
{
    return lifespan_;
}


template<> inline const dds::core::policy::Ownership&
DataReaderQosImpl::policy<dds::core::policy::Ownership>() const
{
    return ownership_;
}

template<>
inline dds::core::policy::Ownership&
DataReaderQosImpl::policy<dds::core::policy::Ownership>()
{
    return ownership_;
}


template<> inline const dds::core::policy::TimeBasedFilter&
DataReaderQosImpl::policy<dds::core::policy::TimeBasedFilter>() const
{
    return tfilter_;
}

template<>
inline dds::core::policy::TimeBasedFilter&
DataReaderQosImpl::policy<dds::core::policy::TimeBasedFilter>()
{
    return tfilter_;
}


template<> inline const dds::core::policy::ReaderDataLifecycle&
DataReaderQosImpl::policy<dds::core::policy::ReaderDataLifecycle>() const
{
    return lifecycle_;
}

template<>
inline dds::core::policy::ReaderDataLifecycle&
DataReaderQosImpl::policy<dds::core::policy::ReaderDataLifecycle>()
{
    return lifecycle_;
}

}
}
}
}


#endif /* ORG_OPENSPLICE_SUB_QOS_DATA_READER_QOS_IMPL_HPP_ */
