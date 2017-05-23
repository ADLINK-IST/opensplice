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
