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

#include <org/opensplice/pub/qos/DataWriterQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{

DataWriterQosImpl::DataWriterQosImpl() : strength_(0), lifecycle_(true)
{  }

DataWriterQosImpl::DataWriterQosImpl(const org::opensplice::topic::qos::TopicQosImpl& tqos)
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
      ownership_(tqos.policy<dds::core::policy::Ownership>()),
      strength_(0)
{ }

DataWriterQosImpl::DataWriterQosImpl(
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

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    dds::core::policy::OwnershipStrength       strength,
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

    dds::core::policy::WriterDataLifecycle     lifecycle)
    : user_data_(user_data),
      durability_(durability),

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
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
      strength_(strength),
      lifecycle_(lifecycle) {}

DataWriterQosImpl::~DataWriterQosImpl() { }

void DataWriterQosImpl::policy(const dds::core::policy::UserData& user_data)
{
    user_data_ = user_data;
}

void DataWriterQosImpl::policy(const dds::core::policy::Durability& durability)
{
    durability_ = durability;
}


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

void DataWriterQosImpl::policy(const dds::core::policy::DurabilityService& durability_service)
{
    durability_service_ = durability_service;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


void DataWriterQosImpl::policy(const dds::core::policy::Deadline& deadline)
{
    deadline_ = deadline;
}

void DataWriterQosImpl::policy(const dds::core::policy::LatencyBudget&  budget)
{
    budget_ = budget;
}


void DataWriterQosImpl::policy(const dds::core::policy::Liveliness& liveliness)
{
    liveliness_ = liveliness;
}

void DataWriterQosImpl::policy(const dds::core::policy::Reliability& reliability)
{
    reliability_ = reliability;
}
void DataWriterQosImpl::policy(const dds::core::policy::DestinationOrder& order)
{
    order_ = order;
}

void DataWriterQosImpl::policy(const dds::core::policy::History& history)
{
    history_ = history;
}

void DataWriterQosImpl::policy(const dds::core::policy::ResourceLimits& resources)
{
    resources_ = resources;
}
void DataWriterQosImpl::policy(const dds::core::policy::TransportPriority& priority)
{
    priority_ = priority;
}
void DataWriterQosImpl::policy(const dds::core::policy::Lifespan& lifespan)
{
    lifespan_ = lifespan;
}
void DataWriterQosImpl::policy(const dds::core::policy::Ownership& ownership)
{
    ownership_ = ownership;
}


#ifdef  OMG_DDS_OWNERSHIP_SUPPORT

void
DataWriterQosImpl::policy(const dds::core::policy::OwnershipStrength& strength)
{
    strength_ = strength;
}

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


void
DataWriterQosImpl::policy(const dds::core::policy::WriterDataLifecycle& lifecycle)
{
    lifecycle_ = lifecycle;
}
}
}
}
}
