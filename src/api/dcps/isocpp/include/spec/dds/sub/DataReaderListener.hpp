#ifndef OMG_DDS_SUB_DATA_READER_LISTENER_HPP_
#define OMG_DDS_SUB_DATA_READER_LISTENER_HPP_

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

#include <dds/core/status/Status.hpp>

namespace dds { namespace sub {
  template <typename T>
  class DataReaderListener;

  template <typename T>
  class NoOpDataReaderListener;
} }

template <typename T>
class dds::sub::DataReaderListener {
public:
  typedef typename ::dds::core::smart_ptr_traits<DataReaderListener>::ref_type ref_type;

public:
  virtual ~DataReaderListener() = 0;

public:
  virtual void on_requested_deadline_missed(
      DataReader<T>& the_reader,
      const dds::core::status::RequestedDeadlineMissedStatus& status) = 0;

  virtual void on_requested_incompatible_qos(
      DataReader<T>& the_reader,
      const dds::core::status::RequestedIncompatibleQosStatus& status) = 0;

  virtual void on_sample_rejected(
      DataReader<T>& the_reader,
      const dds::core::status::SampleRejectedStatus& status) = 0;

  virtual void on_liveliness_changed(
      DataReader<T>& the_reader,
      const dds::core::status::LivelinessChangedStatus& status) = 0;

  virtual void on_data_available(DataReader<T>& the_reader) = 0;

  virtual void on_subscription_matched(
      DataReader<T>& the_reader,
      const dds::core::status::SubscriptionMatchedStatus& status) = 0;

  virtual void on_sample_lost(
      DataReader<T>& the_reader,
      const dds::core::status::SampleLostStatus& status) = 0;
};


template <typename T>
class dds::sub::NoOpDataReaderListener : public virtual DataReaderListener<T> {
public:
  typedef typename ::dds::core::smart_ptr_traits<NoOpDataReaderListener>::ref_type ref_type;

public:
  virtual ~NoOpDataReaderListener();

public:
  virtual void on_requested_deadline_missed(
      DataReader<T>& the_reader,
      const dds::core::status::RequestedDeadlineMissedStatus& status);

  virtual void on_requested_incompatible_qos(
      DataReader<T>& the_reader,
      const dds::core::status::RequestedIncompatibleQosStatus& status);

  virtual void on_sample_rejected(
      DataReader<T>& the_reader,
      const dds::core::status::SampleRejectedStatus& status);

  virtual void on_liveliness_changed(
      DataReader<T>& the_reader,
      const dds::core::status::LivelinessChangedStatus& status);

  virtual void on_data_available(DataReader<T>& the_reader);

  virtual void on_subscription_matched(
      DataReader<T>& the_reader,
      const dds::core::status::SubscriptionMatchedStatus& status);

  virtual void on_sample_lost(
      DataReader<T>& the_reader,
      const dds::core::status::SampleLostStatus& status);
};

#endif /* OMG_DDS_SUB_DATA_READER_LISTENER_HPP_ */
