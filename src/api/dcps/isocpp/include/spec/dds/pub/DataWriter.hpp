#ifndef OMG_DDS_PUB_DATA_WRITER_HPP_
#define OMG_DDS_PUB_DATA_WRITER_HPP_

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

#include <dds/core/InstanceHandle.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/TopicInstance.hpp>
#include <dds/topic/BuiltinTopic.hpp>
#include <dds/pub/Publisher.hpp>

#include <dds/pub/detail/DataWriter.hpp>

namespace dds {
  namespace pub {
    template <typename T,
    template <typename Q> class DELEGATE = dds::pub::detail::DataWriter >
    class DataWriter;

    template <typename T> class DataWriterListener;
  }
}

template <typename T, template <typename Q> class DELEGATE>
class dds::pub::DataWriter : public ::dds::core::TEntity< DELEGATE<T> > {

public:
  typedef dds::pub::DataWriterListener<T>              Listener;

public:
  OMG_DDS_REF_TYPE(DataWriter, ::dds::core::TEntity, DELEGATE<T>)

public:

  /**
   * Create a <code>DataWriter</code>. The QoS will be set to pub.default_datawriter_qos().
   *
   * @param pub the publisher
   * @param topic the <code>Topic</code> associated with this <code>DataWriter</code>
   */
  DataWriter(const dds::pub::Publisher& pub,
      const ::dds::topic::Topic<T>& topic);

  /**
   * Create a <code>DataWriter</code>.
   *
   * @param pub the publisher
   * @param topic the <code>Topic</code> associated with this <code>DataWriter</code>
   * @param qos the <code>DataWriter</code> qos.
   * @param listener the <code>DataWriter</code> listener.
   * @param mask the listener event mask.
   */
  DataWriter(const dds::pub::Publisher& pub,
      const ::dds::topic::Topic<T>& topic,
      const dds::pub::qos::DataWriterQos& qos,
      dds::pub::DataWriterListener<T>* listener = NULL,
      const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

public:
  ~DataWriter();


public:
  //==========================================================================
  //== Write API

  /**
   * Write a sample.
   *
   * @param sample the sample to be written.
   */
  void write(const T& sample);

  /**
   * Write a sample with a given timestamp.
   *
   * @param sample the sample to be written.
   * @param timestamp the timestamp used for this sample.
   */
  void write(const T& sample, const dds::core::Time& timestamp);

  /**
   * Write a sample by providing the instance handle.
   * This is usually the most efficint way of writing a sample.
   *
   * @param sample the sample to be written.
   * @param instance the handle representing the instance written.
   */
  void write(const T& sample, const ::dds::core::InstanceHandle& instance);

  /**
   * Write a sample, with a time-stamp, by providing the instance handle.
   * This is usually the most efficient way of writing a sample.
   *
   * @param sample the sample to be written.
   * @param instance the handle representing the instance written.
   * @param timestamp the timestamp to use for this sample.
   */
  void write(const T& data,
      const ::dds::core::InstanceHandle& instance,
      const dds::core::Time& timestamp);


  /**
   * Write a topic instance -- a class that encapsulate the
   * sample and its associated instance handle.
   *
   * @param i the instance to write.
   */
  void write(const dds::topic::TopicInstance<T>& i);

  /**
   * Write a topic instance with time stamp.
   *
   * @param i the instance to write.
   * @param timestamp the timestamp for this sample.
   */
  void write(const dds::topic::TopicInstance<T>& i,
      const dds::core::Time& timestamp);

  /**
   * Write a series of samples or TopicInstances (determined by the template
   * specialization).
   */
  template <typename FWIterator>
  void write(const FWIterator& begin, const FWIterator& end);

  /**
   * Write a series of samples or TopicInstances (determined by the template
   * specialization) with timestamp.
   */
  template <typename FWIterator>
  void write(const FWIterator& begin, const FWIterator& end,
      const dds::core::Time& timestamp);

  /**
   * Write a series of samples and their parallel instance handles.
   */
  template <typename SamplesFWIterator, typename HandlesFWIterator>
  void write(const SamplesFWIterator& data_begin,
      const SamplesFWIterator& data_end,
      const HandlesFWIterator& handle_begin,
      const HandlesFWIterator& handle_end);

  /**
   * Write a series of samples and their parallel instance handles with
   * a timestamp.
   */
  template <typename SamplesFWIterator, typename HandlesFWIterator>
  void write(const SamplesFWIterator& data_begin,
      const SamplesFWIterator& data_end,
      const HandlesFWIterator& handle_begin,
      const HandlesFWIterator& handle_end,
      const dds::core::Time& timestamp);


  DataWriter& operator << (const T& data);

  DataWriter& operator << (const std::pair<T, dds::core::Time>& data);

  DataWriter& operator << (const std::pair<T, ::dds::core::InstanceHandle>& data);

  DataWriter&
  operator <<(DataWriter& (*manipulator)(DataWriter&));

  //==========================================================================
  //== Instance Management
  /**
   * Register an instance.
   *
   * @param key the key of the instance to register.
   */
  const ::dds::core::InstanceHandle register_instance(const T& key);

  /**
   * Register an instance with timestamp.
   *
   * @param key the key of the instance to register.
   * @param timestamp the timestamp used for registration.
   */
  const ::dds::core::InstanceHandle register_instance(const T& key,
      const dds::core::Time& timestamp);

  /**
   * Unregister an instance.
   *
   * @param i the instance to unregister.
   */
  DataWriter& unregister_instance(const ::dds::core::InstanceHandle& i);

  /**
   * Unregister an instance with timestamp.
   *
   * @param i the instance to unregister.
   * @param timestamp the timestamp used for registration.
   */
  DataWriter& unregister_instance(const ::dds::core::InstanceHandle& i,
      const dds::core::Time& timestamp);

  /**
   * Dispose an instance.
   *
   * @param i the instance to dispose.
   */
  DataWriter& dispose_instance(const ::dds::core::InstanceHandle& i);

  /**
   * Dispose an instance with a timestamp.
   *
   * @param i the instance to dispose.
   * @param timestamp the timestamp.
   */
  DataWriter& dispose_instance(const ::dds::core::InstanceHandle& i,
      const dds::core::Time& timestamp);

  /**
   * This operation can be used to retrieve the instance key that corresponds
   * to an instance_handle. The operation will only fill the fields that form
   * the key inside the key_holder instance.
   * This operation may raise a BadParameter exception if the InstanceHandle
   * does not correspond to an existing data-object known to the DataWriter.
   * If the implementation is not able to check invalid handles, then the
   * result in this situation is unspecified.
   */
  dds::topic::TopicInstance<T>& key_value(dds::topic::TopicInstance<T>& i,
      const ::dds::core::InstanceHandle& h);

  /**
   * This operation can be used to retrieve the instance key that corresponds
   * to an instance_handle. The operation will only fill the fields that form
   * the key inside the key_holder instance.
   * This operation may raise a BadParameter exception if the InstanceHandle
   * does not correspond to an existing data-object known to the DataWriter.
   * If the implementation is not able to check invalid handles, then the
   * result in this situation is unspecified.
   */
  T& key_value(T& sample, const ::dds::core::InstanceHandle& h);

  /**
   * This operation takes as a parameter an instance and returns a handle
   * that can be used in subsequent operations that accept an instance handle
   * as an argument. The instance parameter is only used for the purpose
   * of examining the fields that define the key. This operation does not
   * register the instance in question. If the instance has not been
   * previously registered, or if for any other reason the Service is unable
   * to provide an instance handle, the Service will return a TopicInstance
   * whose handle will be set to the HANDLE_NIL value.
   */
  dds::core::InstanceHandle lookup_instance(const T& key);

  //==========================================================================
  //== QoS Management

  /**
   * Get the <code>DataWriter</code> QoS.
   */
  const ::dds::pub::qos::DataWriterQos& qos() const;

  /**
   * Set the <code>DataWriter</code> QoS.
   *
   * @param the_qos the new qos for this <code>DataWriter</code>.
   */
  void qos(const dds::pub::qos::DataWriterQos& the_qos);

  /**
   * Set the <code>DataWriter</code> QoS.
   *
   * @param the_qos the new qos for this <code>DataWriter</code>.
   */
  DataWriter& operator <<(const ::dds::pub::qos::DataWriterQos& the_qos);


  /**
   * Get the <code>DataWriter</code> QoS.
   *
   * @param the_qos will be set to the current qos of this <code>DataWriter</code>.
   */
  const DataWriter& operator >> (::dds::pub::qos::DataWriterQos& the_qos) const;


  //==========================================================================
  //== Entity Navigation

  /**
   * Get the <code>Topic</code> associated with this <code>DataWriter</code>
   */
  const dds::topic::Topic<T>& topic() const;

  /**
   * Get the <code>Publisher</code> that owns this <code>DataWriter</code>.
   */
  const dds::pub::Publisher& publisher() const;

  //==========================================================================
  //== ACKs

  /**
   * This operation blocks the calling thread until either all data written
   * by the reliable DataWriter entities is acknowledged by all matched
   * reliable DataReader entities, or else the duration specified by the
   * max_wait parameter elapses, whichever happens first.
   * A normal return indicates that all the samples written have been
   * acknowledged by all reliable matched data readers; A TimeoutError
   * indicates that max_wait elapsed before all the data was acknowledged.
   */
  void wait_for_acknowledgments(const dds::core::Duration& timeout);

  //==========================================================================
  //== Listeners Management

  /**
   * By virtue of extending Entity, a DataWriter can be attached to a Listener
   * at creation time or later by using the listener operation.
   * The attached Listener must be a DataWriterListener.
   *
   * @param listener the data writer listener
   * @param mask the event mask associated with this listener.
   */
  void listener(DataWriterListener<T>* the_listener,
      const ::dds::core::status::StatusMask& mask);

  /**
   * Return the listener currently associated with this <code>DataWriter</code>.
   *
   * @return the <code>DataWriterListener</code>.
   */
  DataWriterListener<T>* listener() const;

  //==========================================================================
  //== Status Management

  /**
   * Get the <code>LivelinessLostStatus</code>.
   */
  const ::dds::core::status::LivelinessLostStatus liveliness_lost_status();

  /**
   * Get the <code>OfferedDeadlineMissedStatus</code>.
   */
  const ::dds::core::status::OfferedDeadlineMissedStatus offered_deadlined_missed_status();

  /**
   * Get the <code>OfferedIncompatibleQosStatus</code>.
   */
  const ::dds::core::status::OfferedIncompatibleQosStatus offered_incompatible_qos_status();

  /**
   * Get the <code>PublicationMatchedStatus</code>.
   */
  const ::dds::core::status::PublicationMatchedStatus publication_matched_status();


  //==========================================================================
  //== Liveliness Management

  /**
   * Mannually asserts the livelines of this <code>DataWriter</code>
   */
  void assert_liveliness();
};

#endif /* OMG_DDS_PUB_DATA_WRITER_HPP_ */
