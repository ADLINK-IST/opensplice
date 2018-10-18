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

#ifndef ORG_OPENSPLICE_SUB_QOS_DATA_READER_QOS_DELEGATE_HPP_
#define ORG_OPENSPLICE_SUB_QOS_DATA_READER_QOS_DELEGATE_HPP_

#include <dds/core/detail/conformance.hpp>
#include <org/opensplice/topic/qos/TopicQosDelegate.hpp>

#include "u_types.h"

struct _DDS_NamedDataReaderQos;

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

class OMG_DDS_API DataReaderQosDelegate
{
public:
    DataReaderQosDelegate();
    DataReaderQosDelegate(const DataReaderQosDelegate& other);
    DataReaderQosDelegate(const org::opensplice::topic::qos::TopicQosDelegate& tqos);

    ~DataReaderQosDelegate();

    void policy(const dds::core::policy::UserData&            user_data);
    void policy(const dds::core::policy::Durability&          durability);
    void policy(const dds::core::policy::Deadline&            deadline);
    void policy(const dds::core::policy::LatencyBudget&       budget);
    void policy(const dds::core::policy::Liveliness&          liveliness);
    void policy(const dds::core::policy::Reliability&         reliability);
    void policy(const dds::core::policy::DestinationOrder&    order);
    void policy(const dds::core::policy::History&             history);
    void policy(const dds::core::policy::ResourceLimits&      resources);
    void policy(const dds::core::policy::Ownership&           ownership);
    void policy(const dds::core::policy::TimeBasedFilter&     tfilter);
    void policy(const dds::core::policy::ReaderDataLifecycle& lifecycle);
    void policy(const org::opensplice::core::policy::Share&           share);
    void policy(const org::opensplice::core::policy::SubscriptionKey& keys);
    void policy(const org::opensplice::core::policy::ReaderLifespan&  lifespan);

    template <typename POLICY> const POLICY& policy() const;
    template <typename POLICY> POLICY& policy();

    /* The returned userlayer QoS has to be freed. */
    u_readerQos u_qos() const;
    void u_qos(const u_readerQos qos);

    void named_qos(const struct _DDS_NamedDataReaderQos &qos);

    void check() const;

    bool operator==(const DataReaderQosDelegate& other) const;
    DataReaderQosDelegate& operator =(const DataReaderQosDelegate& other);
    DataReaderQosDelegate& operator =(const org::opensplice::topic::qos::TopicQosDelegate& tqos);

private:
    void defaults();

    dds::core::policy::UserData                user_data_;
    dds::core::policy::Durability              durability_;
    dds::core::policy::Deadline                deadline_;
    dds::core::policy::LatencyBudget           budget_;
    dds::core::policy::Liveliness              liveliness_;
    dds::core::policy::Reliability             reliability_;
    dds::core::policy::DestinationOrder        order_;
    dds::core::policy::History                 history_;
    dds::core::policy::ResourceLimits          resources_;
    dds::core::policy::Ownership               ownership_;
    dds::core::policy::TimeBasedFilter         tfilter_;
    dds::core::policy::ReaderDataLifecycle     lifecycle_;
    org::opensplice::core::policy::Share           share_;
    org::opensplice::core::policy::SubscriptionKey keys_;
    org::opensplice::core::policy::ReaderLifespan  lifespan_;
};



//==============================================================================


template<>
inline const dds::core::policy::Durability&
DataReaderQosDelegate::policy<dds::core::policy::Durability>() const
{
    return durability_;
}
template<>
inline dds::core::policy::Durability&
DataReaderQosDelegate::policy<dds::core::policy::Durability>()
{
    return durability_;
}


template<>
inline const dds::core::policy::UserData&
DataReaderQosDelegate::policy<dds::core::policy::UserData>() const
{
    return user_data_;
}
template<>
inline dds::core::policy::UserData&
DataReaderQosDelegate::policy<dds::core::policy::UserData>()
{
    return user_data_;
}


template<> inline const dds::core::policy::Deadline&
DataReaderQosDelegate::policy<dds::core::policy::Deadline>() const
{
    return deadline_;
}
template<>
inline dds::core::policy::Deadline&
DataReaderQosDelegate::policy<dds::core::policy::Deadline>()
{
    return deadline_;
}


template<> inline const dds::core::policy::LatencyBudget&
DataReaderQosDelegate::policy<dds::core::policy::LatencyBudget>() const
{
    return budget_;
}
template<>
inline dds::core::policy::LatencyBudget&
DataReaderQosDelegate::policy<dds::core::policy::LatencyBudget>()
{
    return budget_;
}


template<> inline const dds::core::policy::Liveliness&
DataReaderQosDelegate::policy<dds::core::policy::Liveliness>() const
{
    return liveliness_;
}
template<>
inline dds::core::policy::Liveliness&
DataReaderQosDelegate::policy<dds::core::policy::Liveliness>()
{
    return liveliness_;
}


template<> inline const dds::core::policy::Reliability&
DataReaderQosDelegate::policy<dds::core::policy::Reliability>() const
{
    return reliability_;
}
template<>
inline dds::core::policy::Reliability&
DataReaderQosDelegate::policy<dds::core::policy::Reliability>()
{
    return reliability_;
}


template<> inline const dds::core::policy::DestinationOrder&
DataReaderQosDelegate::policy<dds::core::policy::DestinationOrder>() const
{
    return order_;
}
template<>
inline dds::core::policy::DestinationOrder&
DataReaderQosDelegate::policy<dds::core::policy::DestinationOrder>()
{
    return order_;
}


template<> inline const dds::core::policy::History&
DataReaderQosDelegate::policy<dds::core::policy::History>() const
{
    return history_;
}
template<>
inline dds::core::policy::History&
DataReaderQosDelegate::policy<dds::core::policy::History>()
{
    return history_;
}


template<> inline const dds::core::policy::ResourceLimits&
DataReaderQosDelegate::policy<dds::core::policy::ResourceLimits>() const
{
    return resources_;
}
template<>
inline dds::core::policy::ResourceLimits&
DataReaderQosDelegate::policy<dds::core::policy::ResourceLimits>()
{
    return resources_;
}


template<> inline const dds::core::policy::Ownership&
DataReaderQosDelegate::policy<dds::core::policy::Ownership>() const
{
    return ownership_;
}
template<>
inline dds::core::policy::Ownership&
DataReaderQosDelegate::policy<dds::core::policy::Ownership>()
{
    return ownership_;
}


template<> inline const dds::core::policy::TimeBasedFilter&
DataReaderQosDelegate::policy<dds::core::policy::TimeBasedFilter>() const
{
    return tfilter_;
}
template<>
inline dds::core::policy::TimeBasedFilter&
DataReaderQosDelegate::policy<dds::core::policy::TimeBasedFilter>()
{
    return tfilter_;
}


template<> inline const dds::core::policy::ReaderDataLifecycle&
DataReaderQosDelegate::policy<dds::core::policy::ReaderDataLifecycle>() const
{
    return lifecycle_;
}
template<>
inline dds::core::policy::ReaderDataLifecycle&
DataReaderQosDelegate::policy<dds::core::policy::ReaderDataLifecycle>()
{
    return lifecycle_;
}


template<>
inline const org::opensplice::core::policy::Share&
DataReaderQosDelegate::policy<org::opensplice::core::policy::Share>() const
{
    return share_;
}
template<>
inline org::opensplice::core::policy::Share&
DataReaderQosDelegate::policy<org::opensplice::core::policy::Share>()
{
    return share_;
}


template<>
inline const org::opensplice::core::policy::SubscriptionKey&
DataReaderQosDelegate::policy<org::opensplice::core::policy::SubscriptionKey>() const
{
    return keys_;
}
template<>
inline org::opensplice::core::policy::SubscriptionKey&
DataReaderQosDelegate::policy<org::opensplice::core::policy::SubscriptionKey>()
{
    return keys_;
}


template<>
inline const org::opensplice::core::policy::ReaderLifespan&
DataReaderQosDelegate::policy<org::opensplice::core::policy::ReaderLifespan>() const
{
    return lifespan_;
}
template<>
inline org::opensplice::core::policy::ReaderLifespan&
DataReaderQosDelegate::policy<org::opensplice::core::policy::ReaderLifespan>()
{
    return lifespan_;
}

}
}
}
}


#endif /* ORG_OPENSPLICE_SUB_QOS_DATA_READER_QOS_DELEGATE_HPP_ */
