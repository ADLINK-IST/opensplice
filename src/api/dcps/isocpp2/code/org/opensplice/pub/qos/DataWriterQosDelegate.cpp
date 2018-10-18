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

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/pub/qos/DataWriterQosDelegate.hpp>

#include "u_writerQos.h"

#include "dds_namedQosTypesSplType.h"

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{

DataWriterQosDelegate::DataWriterQosDelegate()
{
    this->defaults();
}

DataWriterQosDelegate::DataWriterQosDelegate(
    const DataWriterQosDelegate& other)
    : user_data_(other.user_data_),
      durability_(other.durability_),
      deadline_(other.deadline_),
      budget_(other.budget_),
      liveliness_(other.liveliness_),
      reliability_(other.reliability_),
      order_(other.order_),
      history_(other.history_),
      resources_(other.resources_),
      priority_(other.priority_),
      lifespan_(other.lifespan_),
      ownership_(other.ownership_),
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
      strength_(other.strength_),
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
      lifecycle_(other.lifecycle_)
{
}

DataWriterQosDelegate::DataWriterQosDelegate(
    const org::opensplice::topic::qos::TopicQosDelegate& tqos)
{
    /* First, set the defaults to be sure that all policies have a proper
     * value. Also the ones that will not be set by the TopicQos. */
    this->defaults();
    /* Now, merge the topic policies. */
    *this = tqos;
}

DataWriterQosDelegate::~DataWriterQosDelegate()
{
}

void
DataWriterQosDelegate::policy(const dds::core::policy::UserData& user_data)
{
    user_data.delegate().check();
    user_data_ = user_data;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::Durability& durability)
{
    durability.delegate().check();
    durability_ = durability;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::Deadline& deadline)
{
    deadline.delegate().check();
    deadline_ = deadline;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::LatencyBudget&  budget)
{
    budget.delegate().check();
    budget_ = budget;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::Liveliness& liveliness)
{
    liveliness.delegate().check();
    liveliness_ = liveliness;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::Reliability& reliability)
{
    reliability.delegate().check();
    reliability_ = reliability;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::DestinationOrder& order)
{
    order.delegate().check();
    order_ = order;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::History& history)
{
    history.delegate().check();
    history_ = history;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::ResourceLimits& resources)
{
    resources.delegate().check();
    resources_ = resources;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::TransportPriority& priority)
{
    priority.delegate().check();
    priority_ = priority;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::Lifespan& lifespan)
{
    lifespan.delegate().check();
    lifespan_ = lifespan;
}

void
DataWriterQosDelegate::policy(const dds::core::policy::Ownership& ownership)
{
    ownership.delegate().check();
    ownership_ = ownership;
}

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
void
DataWriterQosDelegate::policy(const dds::core::policy::OwnershipStrength& strength)
{
    strength.delegate().check();
    strength_ = strength;
}
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

void
DataWriterQosDelegate::policy(const dds::core::policy::WriterDataLifecycle& lifecycle)
{
    lifecycle.delegate().check();
    lifecycle_ = lifecycle;
}

u_writerQos
DataWriterQosDelegate::u_qos() const
{
    u_writerQos qos = u_writerQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->userData    = user_data_   .delegate().v_policyI();
    qos->durability  = durability_  .delegate().v_policyI();
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
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    qos->strength    = strength_    .delegate().v_policyI();
#else
    qos->strength = 0;
#endif  // OMG_DDS_OWNERSHIP_SUPPORT
    qos->lifecycle = lifecycle_.delegate().v_policyI();
    return qos;
}

void
DataWriterQosDelegate::u_qos(const u_writerQos qos)
{
    assert(qos);
    user_data_   .delegate().v_policyI(qos->userData   );
    durability_  .delegate().v_policyI(qos->durability );
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
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    strength_    .delegate().v_policyI(qos->strength   );
#endif  // OMG_DDS_OWNERSHIP_SUPPORT
    lifecycle_   .delegate().v_policyI(qos->lifecycle  );
}

void
DataWriterQosDelegate::named_qos(const struct _DDS_NamedDataWriterQos &qos)
{
    /* We only need the QoS part of the named QoS. */
    const struct _DDS_DataWriterQos *q = &qos.datawriter_qos;
    /* The idl policies are aligned the same as the kernel/builtin representation.
     * So, cast and use the kernel policy translations (or builtin when available). */
    user_data_   .delegate().v_policy((v_builtinUserDataPolicy&)(q->user_data)            );
    durability_  .delegate().v_policy((v_durabilityPolicy&)     (q->durability)           );
    deadline_    .delegate().v_policy((v_deadlinePolicy&)       (q->deadline)             );
    budget_      .delegate().v_policy((v_latencyPolicy&)        (q->latency_budget)       );
    liveliness_  .delegate().v_policy((v_livelinessPolicy&)     (q->liveliness)           );
    reliability_ .delegate().v_policy((v_reliabilityPolicy&)    (q->reliability)          );
    order_       .delegate().v_policy((v_orderbyPolicy&)        (q->destination_order)    );
    history_     .delegate().v_policy((v_historyPolicy&)        (q->history)              );
    resources_   .delegate().v_policy((v_resourcePolicy&)       (q->resource_limits)      );
    priority_    .delegate().v_policy((v_transportPolicy&)      (q->transport_priority)   );
    lifespan_    .delegate().v_policy((v_lifespanPolicy&)       (q->lifespan)             );
    ownership_   .delegate().v_policy((v_ownershipPolicy&)      (q->ownership)            );
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    strength_    .delegate().v_policy((v_strengthPolicy&)       (q->ownership_strength)   );
#endif  // OMG_DDS_OWNERSHIP_SUPPORT
    lifecycle_   .delegate().v_policy((v_writerLifecyclePolicy&)(q->writer_data_lifecycle));
}

void
DataWriterQosDelegate::check() const
{
    /* Policies are checked when set.
     * But consistency between policies needs to be checked. */
    history_.delegate().check_against(resources_.delegate());
}

bool
DataWriterQosDelegate::operator ==(const DataWriterQosDelegate& other) const
{
    return other.user_data_   == user_data_   &&
           other.durability_  == durability_  &&
           other.deadline_    == deadline_    &&
           other.budget_      == budget_      &&
           other.liveliness_  == liveliness_  &&
           other.reliability_ == reliability_ &&
           other.order_       == order_       &&
           other.history_     == history_     &&
           other.resources_   == resources_   &&
           other.priority_    ==  priority_   &&
           other.lifespan_    == lifespan_    &&
           other.ownership_   == ownership_   &&
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
           other.strength_    == strength_    &&
#endif
           other.lifecycle_   == lifecycle_;
}

DataWriterQosDelegate&
DataWriterQosDelegate::operator =(const DataWriterQosDelegate& other)
{
    user_data_   = other.user_data_;
    durability_  = other.durability_;
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
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    strength_    = other.strength_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
    lifecycle_   = other.lifecycle_;
    return *this;
}

DataWriterQosDelegate&
DataWriterQosDelegate::operator =(const org::opensplice::topic::qos::TopicQosDelegate& tqos)
{
    durability_  = tqos.policy<dds::core::policy::Durability>();
    deadline_    = tqos.policy<dds::core::policy::Deadline>();
    budget_      = tqos.policy<dds::core::policy::LatencyBudget>();
    liveliness_  = tqos.policy<dds::core::policy::Liveliness>();
    reliability_ = tqos.policy<dds::core::policy::Reliability>();
    order_       = tqos.policy<dds::core::policy::DestinationOrder>();
    history_     = tqos.policy<dds::core::policy::History>();
    resources_   = tqos.policy<dds::core::policy::ResourceLimits>();
    priority_    = tqos.policy<dds::core::policy::TransportPriority>();
    lifespan_    = tqos.policy<dds::core::policy::Lifespan>();
    ownership_   = tqos.policy<dds::core::policy::Ownership>();
    return *this;
}

void
DataWriterQosDelegate::defaults()
{
    /* Get default QoS from userlayer and copy result. */
    u_writerQos qos = u_writerQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
    qos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    qos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    this->u_qos(qos);
    u_writerQosFree(qos);
}

}
}
}
}
