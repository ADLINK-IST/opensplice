#ifndef OMG_TDDS_CORE_STATUS_STATUS_HPP_
#define OMG_TDDS_CORE_STATUS_STATUS_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/Value.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/core/policy/QosPolicyCount.hpp>
#include <dds/core/status/State.hpp>

namespace dds { namespace core { namespace status {

  template <typename D>
  class TInconsistentTopicStatus : public dds::core::Value<D> {
  public:
    TInconsistentTopicStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
  };

  template <typename D>
  class TSampleLostStatus : public dds::core::Value<D> {
  public:
    TSampleLostStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
  };


  template <typename D>
  class TSampleRejectedStatus : public dds::core::Value<D> {
  public:
    TSampleRejectedStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    const dds::core::status::SampleRejectedState last_reason() const;
    const dds::core::InstanceHandle last_instance_handle() const;
    };

  template <typename D>
  class TLivelinessLostStatus : public dds::core::Value<D> {
  public:
    TLivelinessLostStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
  };

  template <typename D>
  class TLivelinessChangedStatus : public dds::core::Value<D> {
  public:
    TLivelinessChangedStatus();

  public:
    int32_t alive_count() const;
    int32_t not_alive_count() const;
    int32_t alive_count_change() const;
    int32_t not_alive_count_change() const;
    const dds::core::InstanceHandle last_publication_handle() const;
  };

  template <typename D>
  class TOfferedDeadlineMissedStatus : public dds::core::Value<D> {
  public:
    TOfferedDeadlineMissedStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    const dds::core::InstanceHandle last_instance_handle() const;
  };

  template <typename D>
  class TRequestedDeadlineMissedStatus : public dds::core::Value<D> {
  public:
    TRequestedDeadlineMissedStatus();
  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    const dds::core::InstanceHandle last_instance_handle() const;
  };



  template <typename D>
  class TOfferedIncompatibleQosStatus : public dds::core::Value<D>{
  public:
    TOfferedIncompatibleQosStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    dds::core::policy::QosPolicyId last_policy_id() const;
    const dds::core::policy::QosPolicyCountSeq policies() const;

    const dds::core::policy::QosPolicyCountSeq&
    policies(dds::core::policy::QosPolicyCountSeq& dst) const;
  };

  template <typename D>
  class TRequestedIncompatibleQosStatus : public dds::core::Value<D> {
  public:
    TRequestedIncompatibleQosStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    dds::core::policy::QosPolicyId last_policy_id() const;
    const dds::core::policy::QosPolicyCountSeq policies() const;
    const dds::core::policy::QosPolicyCountSeq&
    policies(dds::core::policy::QosPolicyCountSeq& dst) const;
  };

  template <typename D>
  class TPublicationMatchedStatus : public dds::core::Value<D> {
  public:
    TPublicationMatchedStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    int32_t current_count() const;
    int32_t current_count_change() const;
    const dds::core::InstanceHandle last_subscription_handle() const;
  };

  template <typename D>
  class TSubscriptionMatchedStatus : public dds::core::Value<D> {
  public:
    TSubscriptionMatchedStatus();

  public:
    int32_t total_count() const;
    int32_t total_count_change() const;
    int32_t current_count() const;
    int32_t current_count_change() const;
    const dds::core::InstanceHandle last_publication_handle() const;
  };

} } }/* namespace tdds::core::status */

#endif /* OMG_TDDS_CORE_STATUS_STATUS_HPP_ */
