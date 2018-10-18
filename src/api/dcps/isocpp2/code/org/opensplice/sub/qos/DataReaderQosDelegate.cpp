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
#include <org/opensplice/sub/qos/DataReaderQosDelegate.hpp>

#include "u_readerQos.h"

#include "dds_namedQosTypesSplType.h"

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

DataReaderQosDelegate::DataReaderQosDelegate()
{
    this->defaults();
}

DataReaderQosDelegate::DataReaderQosDelegate(
    const DataReaderQosDelegate& other)
    : user_data_(other.user_data_),
      durability_(other.durability_),
      deadline_(other.deadline_),
      budget_(other.budget_),
      liveliness_(other.liveliness_),
      reliability_(other.reliability_),
      order_(other.order_),
      history_(other.history_),
      resources_(other.resources_),
      ownership_(other.ownership_),
      tfilter_(other.tfilter_),
      lifecycle_(other.lifecycle_),
      share_(other.share_),
      keys_(other.keys_),
      lifespan_(other.lifespan_)
{
}

DataReaderQosDelegate::DataReaderQosDelegate(
    const org::opensplice::topic::qos::TopicQosDelegate& tqos)
{
    /* First, set the defaults to be sure that all policies have a proper
     * value. Also the ones that will not be set by the TopicQos. */
    this->defaults();
    /* Now, merge the topic policies. */
    *this = tqos;
}

DataReaderQosDelegate::~DataReaderQosDelegate()
{
}

void
DataReaderQosDelegate::policy(const dds::core::policy::UserData& user_data)
{
    user_data.delegate().check();
    user_data_ = user_data;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::Durability& durability)
{
    durability.delegate().check();
    durability_ = durability;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::Deadline& deadline)
{
    deadline.delegate().check();
    deadline_ = deadline;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::LatencyBudget&  budget)
{
    budget.delegate().check();
    budget_ = budget;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::Liveliness& liveliness)
{
    liveliness.delegate().check();
    liveliness_ = liveliness;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::Reliability& reliability)
{
    reliability.delegate().check();
    reliability_ = reliability;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::DestinationOrder& order)
{
    order.delegate().check();
    order_ = order;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::History& history)
{
    history.delegate().check();
    history_ = history;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::ResourceLimits& resources)
{
    resources.delegate().check();
    resources_ = resources;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::Ownership& ownership)
{
    ownership.delegate().check();
    ownership_ = ownership;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::TimeBasedFilter& tfilter)
{
    tfilter.delegate().check();
    tfilter_ = tfilter;
}

void
DataReaderQosDelegate::policy(const dds::core::policy::ReaderDataLifecycle& lifecycle)
{
    lifecycle.delegate().check();
    lifecycle_ = lifecycle;
}

void
DataReaderQosDelegate::policy(const org::opensplice::core::policy::Share& share)
{
    share.delegate().check();
    share_ = share;
}

void
DataReaderQosDelegate::policy(const org::opensplice::core::policy::SubscriptionKey& keys)
{
    keys.delegate().check();
    keys_ = keys;
}

void
DataReaderQosDelegate::policy(const org::opensplice::core::policy::ReaderLifespan& lifespan)
{
    lifespan.delegate().check();
    lifespan_ = lifespan;
}

u_readerQos
DataReaderQosDelegate::u_qos() const
{
    u_readerQos qos = u_readerQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->deadline    = deadline_    .delegate().v_policyI();
    qos->durability  = durability_  .delegate().v_policyI();
    qos->history     = history_     .delegate().v_policyI();
    qos->latency     = budget_      .delegate().v_policyI();
    qos->lifecycle   = lifecycle_   .delegate().v_policyI();
    qos->liveliness  = liveliness_  .delegate().v_policyI();
    qos->orderby     = order_       .delegate().v_policyI();
    qos->ownership   = ownership_   .delegate().v_policyI();
    qos->pacing      = tfilter_     .delegate().v_policyI();
    qos->reliability = reliability_ .delegate().v_policyI();
    qos->resource    = resources_   .delegate().v_policyI();
    qos->userData    = user_data_   .delegate().v_policyI();
    qos->share       = share_       .delegate().v_policyI();
    qos->userKey     = keys_        .delegate().v_policyI();
    qos->lifespan    = lifespan_    .delegate().v_policyI();
    return qos;
}

void
DataReaderQosDelegate::u_qos(const u_readerQos qos)
{
    assert(qos);
    deadline_    .delegate().v_policyI(qos->deadline   );
    durability_  .delegate().v_policyI(qos->durability );
    history_     .delegate().v_policyI(qos->history    );
    budget_      .delegate().v_policyI(qos->latency    );
    lifecycle_   .delegate().v_policyI(qos->lifecycle  );
    liveliness_  .delegate().v_policyI(qos->liveliness );
    order_       .delegate().v_policyI(qos->orderby    );
    ownership_   .delegate().v_policyI(qos->ownership  );
    tfilter_     .delegate().v_policyI(qos->pacing     );
    reliability_ .delegate().v_policyI(qos->reliability);
    resources_   .delegate().v_policyI(qos->resource   );
    user_data_   .delegate().v_policyI(qos->userData   );
    share_       .delegate().v_policyI(qos->share      );
    keys_        .delegate().v_policyI(qos->userKey    );
    lifespan_    .delegate().v_policyI(qos->lifespan   );
}

void
DataReaderQosDelegate::named_qos(const struct _DDS_NamedDataReaderQos &qos)
{
    /* We only need the QoS part of the named QoS. */
    const struct _DDS_DataReaderQos *q = &qos.datareader_qos;
    /* The idl policies are aligned the same as the kernel/builtin representation.
     * So, cast and use the kernel policy translations (or builtin when available).
     * Exception is the keys policy... */
    deadline_    .delegate().v_policy((v_deadlinePolicy&)       (q->deadline)             );
    durability_  .delegate().v_policy((v_durabilityPolicy&)     (q->durability)           );
    history_     .delegate().v_policy((v_historyPolicy&)        (q->history)              );
    budget_      .delegate().v_policy((v_latencyPolicy&)        (q->latency_budget)       );
    lifecycle_   .delegate().v_policy((v_readerLifecyclePolicy&)(q->reader_data_lifecycle));
    liveliness_  .delegate().v_policy((v_livelinessPolicy&)     (q->liveliness)           );
    order_       .delegate().v_policy((v_orderbyPolicy&)        (q->destination_order)    );
    ownership_   .delegate().v_policy((v_ownershipPolicy&)      (q->ownership)            );
    tfilter_     .delegate().v_policy((v_pacingPolicy&)         (q->time_based_filter)    );
    reliability_ .delegate().v_policy((v_reliabilityPolicy&)    (q->reliability)          );
    resources_   .delegate().v_policy((v_resourcePolicy&)       (q->resource_limits)      );
    user_data_   .delegate().v_policy((v_builtinUserDataPolicy&)(q->user_data)            );
    share_       .delegate().v_policy((v_sharePolicy&)          (q->share)                );
    keys_        .delegate().v_policy(                          (q->subscription_keys)    );
    lifespan_    .delegate().v_policy((v_readerLifespanPolicy&) (q->reader_lifespan)      );
}

void
DataReaderQosDelegate::check() const
{
    /* Policies are checked when set.
     * But consistency between policies needs to be checked. */
    history_.delegate().check_against(resources_.delegate());
    deadline_.delegate().check_against(tfilter_.delegate());
}

bool
DataReaderQosDelegate::operator==(const DataReaderQosDelegate& other) const
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
           other.ownership_   == ownership_   &&
           other.tfilter_     == tfilter_     &&
           other.lifecycle_   == lifecycle_   &&
           other.share_       == share_       &&
           other.keys_        == keys_        &&
           other.lifespan_    == lifespan_;
}

DataReaderQosDelegate&
DataReaderQosDelegate::operator =(const DataReaderQosDelegate& other)
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
    ownership_   = other.ownership_;
    tfilter_     = other.tfilter_;
    lifecycle_   = other.lifecycle_;
    share_       = other.share_;
    keys_        = other.keys_;
    lifespan_    = other.lifespan_;
    return *this;
}

DataReaderQosDelegate&
DataReaderQosDelegate::operator =(const org::opensplice::topic::qos::TopicQosDelegate& tqos)
{
    durability_  = tqos.policy<dds::core::policy::Durability>();
    deadline_    = tqos.policy<dds::core::policy::Deadline>();
    budget_      = tqos.policy<dds::core::policy::LatencyBudget>();
    liveliness_  = tqos.policy<dds::core::policy::Liveliness>();
    reliability_ = tqos.policy<dds::core::policy::Reliability>();
    order_       = tqos.policy<dds::core::policy::DestinationOrder>();
    history_     = tqos.policy<dds::core::policy::History>();
    resources_   = tqos.policy<dds::core::policy::ResourceLimits>();
    ownership_   = tqos.policy<dds::core::policy::Ownership>();
    return *this;
}

void
DataReaderQosDelegate::defaults()
{
    /* Get default QoS from userlayer and copy result. */
    u_readerQos qos = u_readerQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
    qos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    this->u_qos(qos);
    u_readerQosFree(qos);
}

}
}
}
}
