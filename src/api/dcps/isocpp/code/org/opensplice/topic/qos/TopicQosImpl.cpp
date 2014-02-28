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

#include <org/opensplice/topic/qos/TopicQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
namespace qos
{

TopicQosImpl::TopicQosImpl() {  }

TopicQosImpl::TopicQosImpl(
    const dds::core::policy::TopicData&              topic_data,
    const dds::core::policy::Durability&             durability,

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    const dds::core::policy::DurabilityService&      durability_service,
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    const dds::core::policy::Deadline&               deadline,
    const dds::core::policy::LatencyBudget&          budget,
    const dds::core::policy::Liveliness&             liveliness,
    const dds::core::policy::Reliability&            reliability,
    const dds::core::policy::DestinationOrder&       order,
    const dds::core::policy::History&                history,
    const dds::core::policy::ResourceLimits&         resources,
    const dds::core::policy::TransportPriority&      priority,
    const dds::core::policy::Lifespan&               lifespan,
    const dds::core::policy::Ownership&              ownership)
    : topic_data_(topic_data),
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
      ownership_(ownership) {}

TopicQosImpl::~TopicQosImpl() { }


void TopicQosImpl::policy(const dds::core::policy::TopicData& topic_data)
{
    topic_data_ = topic_data;
}

void TopicQosImpl::policy(const dds::core::policy::Durability& durability)
{
    durability_ = durability;
}


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

void TopicQosImpl::policy(const dds::core::policy::DurabilityService& durability_service)
{
    durability_service_ = durability_service;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


void TopicQosImpl::policy(const dds::core::policy::Deadline& deadline)
{
    deadline_ = deadline;
}

void TopicQosImpl::policy(const dds::core::policy::LatencyBudget&  budget)
{
    budget_ = budget;
}


void TopicQosImpl::policy(const dds::core::policy::Liveliness& liveliness)
{
    liveliness_ = liveliness;
}

void TopicQosImpl::policy(const dds::core::policy::Reliability& reliability)
{
    reliability_ = reliability;
}
void TopicQosImpl::policy(const dds::core::policy::DestinationOrder& order)
{
    order_ = order;
}

void TopicQosImpl::policy(const dds::core::policy::History& history)
{
    history_ = history;
}

void TopicQosImpl::policy(const dds::core::policy::ResourceLimits& resources)
{
    resources_ = resources;
}
void TopicQosImpl::policy(const dds::core::policy::TransportPriority& priority)
{
    priority_ = priority;
}
void TopicQosImpl::policy(const dds::core::policy::Lifespan& lifespan)
{
    lifespan_ = lifespan;
}
void TopicQosImpl::policy(const dds::core::policy::Ownership& ownership)
{
    ownership_ = ownership;
}
}
}
}
}
