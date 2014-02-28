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

#include <org/opensplice/sub/qos/DataReaderQosImpl.hpp>


org::opensplice::sub::qos::DataReaderQosImpl::DataReaderQosImpl() {  }

org::opensplice::sub::qos::DataReaderQosImpl::DataReaderQosImpl(const org::opensplice::topic::qos::TopicQosImpl& tqos)
    : durability_(tqos.policy<dds::core::policy::Durability>()),

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
      durability_service_(tqos.policy<dds::core::policy::DurabilityService>()),
#endif

      deadline_(tqos.policy<dds::core::policy::Deadline>()),
      budget_(tqos.policy<dds::core::policy::LatencyBudget>()),
      liveliness_(tqos.policy<dds::core::policy::Liveliness>()),
      reliability_(tqos.policy<dds::core::policy::Reliability>()),
      order_(tqos.policy<dds::core::policy::DestinationOrder>()),
      history_(tqos.policy<dds::core::policy::History>()),
      resources_(tqos.policy<dds::core::policy::ResourceLimits>()),
      priority_(tqos.policy<dds::core::policy::TransportPriority>()),
      lifespan_(tqos.policy<dds::core::policy::Lifespan>()),
      ownership_(tqos.policy<dds::core::policy::Ownership>())
{ }

org::opensplice::sub::qos::DataReaderQosImpl::DataReaderQosImpl(
    dds::core::policy::UserData               user_data,
    dds::core::policy::Durability              durability,

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    dds::core::policy::DurabilityService       durability_service,
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    dds::core::policy::Deadline       deadline,
    dds::core::policy::LatencyBudget  budget,
    dds::core::policy::Liveliness     liveliness,
    dds::core::policy::Reliability             reliability,
    dds::core::policy::DestinationOrder        order,
    dds::core::policy::History                 history,
    dds::core::policy::ResourceLimits          resources,
    dds::core::policy::TransportPriority       priority,
    dds::core::policy::Lifespan                lifespan,
    dds::core::policy::Ownership               ownership,
    dds::core::policy::TimeBasedFilter         tfilter,
    dds::core::policy::ReaderDataLifecycle     lifecycle)
    : user_data_(user_data),
      durability_(durability),

#ifdef OMG_DDS_PERSISTENCE_SUPPORT
      durability_service_(durability_service),
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

      deadline_(deadline),
      budget_(budget),
      liveliness_(liveliness),
      reliability_(reliability),
      order_(order),
      history_(history),
      resources_(resources),
      priority_(priority),
      lifespan_(lifespan),
      ownership_(ownership),
      tfilter_(tfilter),
      lifecycle_(lifecycle) {}

org::opensplice::sub::qos::DataReaderQosImpl::~DataReaderQosImpl() { }

void
org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::UserData& user_data)
{
    user_data_ = user_data;
}

void
org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::Durability& durability)
{
    durability_ = durability;
}


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::DurabilityService& durability_service)
{
    durability_service_ = durability_service;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::Deadline& deadline)
{
    deadline_ = deadline;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::LatencyBudget&  budget)
{
    budget_ = budget;
}


void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::Liveliness& liveliness)
{
    liveliness_ = liveliness;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::Reliability& reliability)
{
    reliability_ = reliability;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::DestinationOrder& order)
{
    order_ = order;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::History& history)
{
    history_ = history;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::ResourceLimits& resources)
{
    resources_ = resources;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::TransportPriority& priority)
{
    priority_ = priority;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::Lifespan& lifespan)
{
    lifespan_ = lifespan;
}

void org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::Ownership& ownership)
{
    ownership_ = ownership;
}

void
org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::TimeBasedFilter& tfilter)
{
    tfilter_ = tfilter;
}
void
org::opensplice::sub::qos::DataReaderQosImpl::policy(const dds::core::policy::ReaderDataLifecycle& lifecycle)
{
    lifecycle_ = lifecycle;
}
