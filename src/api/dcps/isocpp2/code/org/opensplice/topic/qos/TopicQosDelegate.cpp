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

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/topic/qos/TopicQosDelegate.hpp>

#include "u_topicQos.h"

#include "dds_namedQosTypesSplType.h"

namespace org
{
namespace opensplice
{
namespace topic
{
namespace qos
{

TopicQosDelegate::TopicQosDelegate()
{
    this->defaults();
}

TopicQosDelegate::TopicQosDelegate(
    const TopicQosDelegate& other)
    : topic_data_(other.topic_data_),
      durability_(other.durability_),
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
      durability_service_(other.durability_service_),
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
      deadline_(other.deadline_),
      budget_(other.budget_),
      liveliness_(other.liveliness_),
      reliability_(other.reliability_),
      order_(other.order_),
      history_(other.history_),
      resources_(other.resources_),
      priority_(other.priority_),
      lifespan_(other.lifespan_),
      ownership_(other.ownership_)
{
}

TopicQosDelegate::~TopicQosDelegate()
{
}

void
TopicQosDelegate::policy(const dds::core::policy::TopicData& topic_data)
{
    topic_data.delegate().check();
    topic_data_ = topic_data;
}

void
TopicQosDelegate::policy(const dds::core::policy::Durability& durability)
{
    durability.delegate().check();
    durability_ = durability;
}

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
void
TopicQosDelegate::policy(const dds::core::policy::DurabilityService& durability_service)
{
    durability_service.delegate().check();
    durability_service_ = durability_service;
}
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

void
TopicQosDelegate::policy(const dds::core::policy::Deadline& deadline)
{
    deadline.delegate().check();
    deadline_ = deadline;
}

void
TopicQosDelegate::policy(const dds::core::policy::LatencyBudget&  budget)
{
    budget.delegate().check();
    budget_ = budget;
}

void
TopicQosDelegate::policy(const dds::core::policy::Liveliness& liveliness)
{
    liveliness.delegate().check();
    liveliness_ = liveliness;
}

void
TopicQosDelegate::policy(const dds::core::policy::Reliability& reliability)
{
    reliability.delegate().check();
    reliability_ = reliability;
}

void
TopicQosDelegate::policy(const dds::core::policy::DestinationOrder& order)
{
    order.delegate().check();
    order_ = order;
}

void
TopicQosDelegate::policy(const dds::core::policy::History& history)
{
    history.delegate().check();
    history_ = history;
}

void
TopicQosDelegate::policy(const dds::core::policy::ResourceLimits& resources)
{
    resources.delegate().check();
    resources_ = resources;
}

void
TopicQosDelegate::policy(const dds::core::policy::TransportPriority& priority)
{
    priority.delegate().check();
    priority_ = priority;
}

void
TopicQosDelegate::policy(const dds::core::policy::Lifespan& lifespan)
{
    lifespan.delegate().check();
    lifespan_ = lifespan;
}

void
TopicQosDelegate::policy(const dds::core::policy::Ownership& ownership)
{
    ownership.delegate().check();
    ownership_ = ownership;
}

u_topicQos
TopicQosDelegate::u_qos() const
{
    u_topicQos qos = u_topicQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->topicData   = topic_data_  .delegate().v_policyI();
    qos->durability  = durability_  .delegate().v_policyI();
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    qos->durabilityService = durability_service_.delegate().v_policyI();
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    qos->deadline    = deadline_    .delegate().v_policyI();
    qos->latency     = budget_      .delegate().v_policyI();
    qos->liveliness  = liveliness_  .delegate().v_policyI();
    qos->reliability = reliability_ .delegate().v_policyI();
    qos->orderby     = order_       .delegate().v_policyI();
    qos->history     = history_     .delegate().v_policyI();
    qos->resource    = resources_   .delegate().v_policyI();
    qos->transport   = priority_    .delegate().v_policyI();
    qos->lifespan    = lifespan_    .delegate().v_policyI();
    qos->ownership   = ownership_   .delegate().v_policyI();
    return qos;
}

void
TopicQosDelegate::u_qos(const u_topicQos qos)
{
    assert(qos);
    topic_data_  .delegate().v_policyI(qos->topicData  );
    durability_  .delegate().v_policyI(qos->durability );
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    durability_service_.delegate().v_policyI(qos->durabilityService);
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    deadline_    .delegate().v_policyI(qos->deadline   );
    budget_      .delegate().v_policyI(qos->latency    );
    liveliness_  .delegate().v_policyI(qos->liveliness );
    reliability_ .delegate().v_policyI(qos->reliability);
    order_       .delegate().v_policyI(qos->orderby    );
    history_     .delegate().v_policyI(qos->history    );
    resources_   .delegate().v_policyI(qos->resource   );
    priority_    .delegate().v_policyI(qos->transport  );
    lifespan_    .delegate().v_policyI(qos->lifespan   );
    ownership_   .delegate().v_policyI(qos->ownership  );
}

void
TopicQosDelegate::named_qos(const struct _DDS_NamedTopicQos &qos)
{
    /* We only need the QoS part of the named QoS. */
    const struct _DDS_TopicQos *q = &qos.topic_qos;
    /* The idl policies are aligned the same as the kernel/builtin representation.
     * So, cast and use the kernel policy translations (or builtin when available). */
    topic_data_  .delegate().v_policy((v_builtinTopicDataPolicy&)(q->topic_data)        );
    durability_  .delegate().v_policy((v_durabilityPolicy&)      (q->durability)        );
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    durability_service_.delegate().v_policy((v_durabilityServicePolicy&)(q->durability_service));
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    deadline_    .delegate().v_policy((v_deadlinePolicy&)        (q->deadline)          );
    budget_      .delegate().v_policy((v_latencyPolicy&)         (q->latency_budget)    );
    liveliness_  .delegate().v_policy((v_livelinessPolicy&)      (q->liveliness)        );
    reliability_ .delegate().v_policy((v_reliabilityPolicy&)     (q->reliability)       );
    order_       .delegate().v_policy((v_orderbyPolicy&)         (q->destination_order) );
    history_     .delegate().v_policy((v_historyPolicy&)         (q->history)           );
    resources_   .delegate().v_policy((v_resourcePolicy&)        (q->resource_limits)   );
    priority_    .delegate().v_policy((v_transportPolicy&)       (q->transport_priority));
    lifespan_    .delegate().v_policy((v_lifespanPolicy&)        (q->lifespan)          );
    ownership_   .delegate().v_policy((v_ownershipPolicy&)       (q->ownership)         );
}

void
TopicQosDelegate::check() const
{
    /* Policies are checked when set.
     * But consistency between policies needs to be checked. */
    history_.delegate().check_against(resources_.delegate());
}

bool
TopicQosDelegate::operator ==(const TopicQosDelegate& other) const
{
    return other.topic_data_  == topic_data_  &&
           other.durability_  == durability_  &&
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
           other.durability_service_ == durability_service_ &&
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
           other.deadline_    == deadline_    &&
           other.budget_      == budget_      &&
           other.liveliness_  == liveliness_  &&
           other.reliability_ == reliability_ &&
           other.order_       == order_       &&
           other.history_     == history_     &&
           other.resources_   == resources_   &&
           other.priority_    == priority_    &&
           other.lifespan_    == lifespan_    &&
           other.ownership_   == ownership_;
}

TopicQosDelegate&
TopicQosDelegate::operator =(const TopicQosDelegate& other)
{
    topic_data_  = other.topic_data_;
    durability_  = other.durability_;
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    durability_service_ = other.durability_service_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    deadline_    = other.deadline_;
    budget_      = other.budget_;
    liveliness_  = other.liveliness_;
    reliability_ = other.reliability_;
    order_       = other.order_;
    history_     = other.history_;
    resources_   = other.resources_;
    priority_    = other.priority_;
    lifespan_    = other.lifespan_;
    ownership_   = other.ownership_;
    return *this;
}

void
TopicQosDelegate::defaults()
{
    /* Get default QoS from userlayer and copy result. */
    u_topicQos qos = u_topicQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
    qos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    this->u_qos(qos);
    u_topicQosFree(qos);
}

}
}
}
}
