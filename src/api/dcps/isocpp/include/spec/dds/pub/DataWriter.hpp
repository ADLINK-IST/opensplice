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

namespace dds
{
namespace pub
{
template <typename T,
          template <typename Q> class DELEGATE = dds::pub::detail::DataWriter >
class DataWriter;

template <typename T> class DataWriterListener;
}
}

template <typename T, template <typename Q> class DELEGATE>
class dds::pub::DataWriter : public ::dds::core::TEntity< DELEGATE<T> >
{

public:
    typedef dds::pub::DataWriterListener<T>              Listener;

public:
    OMG_DDS_REF_TYPE(DataWriter, ::dds::core::TEntity, DELEGATE<T>)

public:

    /**
     * Create a DataWriter. The QoS will be set to pub.default_datawriter_qos().
     *
     * @param pub the publisher
     * @param topic the Topic associated with this DataWriter
     * See \ref DCPS_Modules_Publication_DataWriter "DataWriter" for more information
     */
    DataWriter(const dds::pub::Publisher& pub,
               const ::dds::topic::Topic<T>& topic);

    /**
     * Create a DataWriter.
     *
     * @param pub the publisher
     * @param topic the Topic associated with this DataWriter
     * @param qos the DataWriter qos.
     * @param listener the DataWriter listener.
     * @param mask the listener event mask.
     */
    DataWriter(const dds::pub::Publisher& pub,
               const ::dds::topic::Topic<T>& topic,
               const dds::pub::qos::DataWriterQos& qos,
               dds::pub::DataWriterListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

public:
    virtual ~DataWriter();


public:
    //==========================================================================
    //== Write API

    /**
     * Write a sample.
     *
     * @param sample the sample to be written
     */
    void write(const T& sample);

    /**
     * Write a sample with a given timestamp.
     *
     * @param sample the sample to be written
     * @param timestamp the timestamp used for this sample
     */
    void write(const T& sample, const dds::core::Time& timestamp);

    /**
     * Write a sample by providing the instance handle.
     * This is usually the most efficint way of writing a sample.
     *
     * @param sample the sample to be written
     * @param instance the handle representing the instance written
     */
    void write(const T& sample, const ::dds::core::InstanceHandle& instance);

    /**
     * Write a sample, with a time-stamp, by providing the instance handle.
     * This is usually the most efficient way of writing a sample.
     *
     * @param sample the sample to be written
     * @param instance the handle representing the instance written
     * @param timestamp the timestamp to use for this sample
     */
    void write(const T& data,
               const ::dds::core::InstanceHandle& instance,
               const dds::core::Time& timestamp);


    /**
     * Write a topic instance -- a class that encapsulate the
     * sample and its associated instance handle.
     *
     * @param i the instance to write
     */
    void write(const dds::topic::TopicInstance<T>& i);

    /**
     * Write a topic instance with time stamp.
     *
     * @param i the instance to write
     * @param timestamp the timestamp for this sample
     */
    void write(const dds::topic::TopicInstance<T>& i,
               const dds::core::Time& timestamp);

    /**
     * Write a series of samples or TopicInstances (determined by the template
     * specialization).
     *
     * @param begin an iterator pointing to the beginning of a sequence of
     * TopicInstances
     * @param end an iterator pointing to the end of a sequence of
     * TopicInstances
     */
    template <typename FWIterator>
    void write(const FWIterator& begin, const FWIterator& end);

    /**
     * Write a series of samples or TopicInstances (determined by the template
     * specialization) with timestamp.
     *
     * @param begin an iterator pointing to the beginning of a sequence of
     * TopicInstances
     * @param end an iterator pointing to the end of a sequence of
     * TopicInstances
     * @param timestamp the time stamp
     */
    template <typename FWIterator>
    void write(const FWIterator& begin, const FWIterator& end,
               const dds::core::Time& timestamp);

    /**
     * Write a series of samples and their parallel instance handles.
     *
     * @param data_begin an iterator pointing to the beginning of a sequence of
     * samples
     * @param data_end an iterator pointing to the end of a sequence of
     * samples
     * @param handle_begin an iterator pointing to the beginning of a sequence of
     * InstanceHandles
     * @param handle_end an iterator pointing to the end of a sequence of
     * InstanceHandles
     */
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void write(const SamplesFWIterator& data_begin,
               const SamplesFWIterator& data_end,
               const HandlesFWIterator& handle_begin,
               const HandlesFWIterator& handle_end);

    /**
     * Write a series of samples and their parallel instance handles with
     * a timestamp.
     *
     * @param data_begin an iterator pointing to the beginning of a sequence of
     * samples
     * @param data_end an iterator pointing to the end of a sequence of
     * samples
     * @param handle_begin an iterator pointing to the beginning of a sequence of
     * InstanceHandles
     * @param handle_end an iterator pointing to the end of a sequence of
     * InstanceHandles
     * @param timestamp the time stamp
     */
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void write(const SamplesFWIterator& data_begin,
               const SamplesFWIterator& data_end,
               const HandlesFWIterator& handle_begin,
               const HandlesFWIterator& handle_end,
               const dds::core::Time& timestamp);


    /**
     * Write a sample.
     *
     * @param data the sample to be written
     */
    DataWriter& operator << (const T& data);

    /**
     * Write a sample with a given timestamp.
     *
     * @param data a pair consisting of a sample and a time stamp
     */
    DataWriter& operator << (const std::pair<T, dds::core::Time>& data);

    /**
     * Write a sample by providing the instance handle.
     * This is usually the most efficint way of writing a sample.
     *
     * @param data a pair consisting of a sample and an instance handle
     */
    DataWriter& operator << (const std::pair<T, ::dds::core::InstanceHandle>& data);

    DataWriter& operator <<(DataWriter & (*manipulator)(DataWriter&));

    //==========================================================================
    //== Instance Management

    /**
     * Register an instance.
     *
     * @param key the key of the instance to register
     * @return the instance handle
     */
    const ::dds::core::InstanceHandle register_instance(const T& key);

    /**
     * Register an instance with timestamp.
     *
     * @param key the key of the instance to register
     * @param timestamp the timestamp used for registration
     * @return the instance handle
     */
    const ::dds::core::InstanceHandle register_instance(const T& key,
            const dds::core::Time& timestamp);

    /**
     * Unregister an instance.
     *
     * @param i the instance to unregister
     */
    DataWriter& unregister_instance(const ::dds::core::InstanceHandle& i);

    /**
     * Unregister an instance with timestamp.
     *
     * @param i the instance to unregister
     * @param timestamp the timestamp used for registration
     */
    DataWriter& unregister_instance(const ::dds::core::InstanceHandle& i,
                                    const dds::core::Time& timestamp);

    /**
     * Dispose an instance.
     *
     * @param i the instance to dispose
     */
    DataWriter& dispose_instance(const ::dds::core::InstanceHandle& i);

    /**
     * Dispose an instance with a timestamp.
     *
     * @param i the instance to dispose
     * @param timestamp the timestamp
     */
    DataWriter& dispose_instance(const ::dds::core::InstanceHandle& i,
                                 const dds::core::Time& timestamp);

#if OSPL_EXPLICIT_WRITEDISPOSE
    /**
     * Write and dispose a sample.
     *
     * @param sample the sample to be written and disposed
     */
    void writedispose(const T& sample);

    /**
     * Write and dispose a sample with a given timestamp.
     *
     * @param sample the sample to be written and disposed
     * @param timestamp the timestamp used for this sample
     */
    void writedispose(const T& sample, const dds::core::Time& timestamp);

    /**
     * Write and dispose a sample by providing the instance handle.
     * This is usually the most efficient way of writing a sample.
     *
     * @param sample the sample to be written and disposed
     * @param instance the handle representing the instance that is written and disposed
     */
    void writedispose(const T& sample, const ::dds::core::InstanceHandle& instance);

    /**
     * Write and dispose a sample, with a time-stamp, by providing the instance handle.
     * This is usually the most efficient way of writing a sample.
     *
     * @param sample the sample to be written and disposed
     * @param instance the handle representing the instance that is written and disposed
     * @param timestamp the timestamp to use for this sample
     */
    void writedispose(const T& data,
               const ::dds::core::InstanceHandle& instance,
               const dds::core::Time& timestamp);


    /**
     * Write and dispose a series of samples or TopicInstances (determined
     * by the template specialization).
     *
     * @param begin an iterator pointing to the beginning of a sequence of
     * TopicInstances
     * @param end an iterator pointing to the end of a sequence of
     * TopicInstances
     */
    template <typename FWIterator>
    void writedispose(const FWIterator& begin, const FWIterator& end);

    /**
     * Write and dispose a series of samples or TopicInstances (determined
     * by the template specialization) with timestamp.
     *
     * @param begin an iterator pointing to the beginning of a sequence of
     * TopicInstances
     * @param end an iterator pointing to the end of a sequence of
     * TopicInstances
     * @param timestamp the time stamp
     */
    template <typename FWIterator>
    void writedispose(const FWIterator& begin, const FWIterator& end,
               const dds::core::Time& timestamp);

    /**
     * Write and dispose a series of samples and their parallel instance handles.
     *
     * @param data_begin an iterator pointing to the beginning of a sequence of
     * samples
     * @param data_end an iterator pointing to the end of a sequence of
     * samples
     * @param handle_begin an iterator pointing to the beginning of a sequence of
     * InstanceHandles
     * @param handle_end an iterator pointing to the end of a sequence of
     * InstanceHandles
     */
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void writedispose(const SamplesFWIterator& data_begin,
               const SamplesFWIterator& data_end,
               const HandlesFWIterator& handle_begin,
               const HandlesFWIterator& handle_end);

    /**
     * Write and dispose a series of samples and their parallel instance handles with
     * a timestamp.
     *
     * @param data_begin an iterator pointing to the beginning of a sequence of
     * samples
     * @param data_end an iterator pointing to the end of a sequence of
     * samples
     * @param handle_begin an iterator pointing to the beginning of a sequence of
     * InstanceHandles
     * @param handle_end an iterator pointing to the end of a sequence of
     * InstanceHandles
     * @param timestamp the time stamp
     */
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void writedispose(const SamplesFWIterator& data_begin,
               const SamplesFWIterator& data_end,
               const HandlesFWIterator& handle_begin,
               const HandlesFWIterator& handle_end,
               const dds::core::Time& timestamp);
#endif

    /**
     * This operation can be used to retrieve the instance key that corresponds
     * to an instance_handle. The operation will only fill the fields that form
     * the key inside the key_holder instance.
     * This operation may raise a BadParameter exception if the InstanceHandle
     * does not correspond to an existing data-object known to the DataWriter.
     * If the implementation is not able to check invalid handles, then the
     * result in this situation is unspecified.
     *
     * @param i the topic instance
     * @param h the instance handle
     * @return the topic instance
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
     *
     * @param sample the sample
     * @param h the instance handle
     * @return the sample
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
     *
     * @param key the sample
     * @return the instance handle
     */
    dds::core::InstanceHandle lookup_instance(const T& key);

    //==========================================================================
    //== QoS Management

    /**
     * Get the DataWriter QoS.
     *
     * @return the DataWriter QoS
     */
    const ::dds::pub::qos::DataWriterQos& qos() const;

    /**
     * Set the DataWriter QoS.
     *
     * @param qos the new qos for this DataWriter
     */
    void qos(const dds::pub::qos::DataWriterQos& qos);

    /**
     * Set the DataWriter QoS.
     *
     * @param qos the new qos for this DataWriter
     */
    dds::pub::qos::DataWriterQos& operator <<(const ::dds::pub::qos::DataWriterQos& qos);

    /**
     * Get the DataWriter QoS.
     *
     * @param qos will be set to the current qos of this DataWriter
     */
    const DataWriter& operator >> (::dds::pub::qos::DataWriterQos& qos) const;


    //==========================================================================
    //== Entity Navigation

    /**
     * Get the Topic associated with this DataWriter.
     */
    const dds::topic::Topic<T>& topic() const;

    /**
     * Get the Publisher that owns this DataWriter.
     */
    const dds::pub::Publisher& publisher() const;

    //==========================================================================
    //== ACKs

    /**
     * This operation blocks the calling thread until either all data written
     * by the reliable DataWriter entities is acknowledged by all matched
     * reliable DataReader entities, or else the duration specified by the
     * timeout parameter elapses, whichever happens first.
     * A normal return indicates that all the samples written have been
     * acknowledged by all reliable matched data readers. A TimeoutError
     * indicates that timeout elapsed before all the data was acknowledged.
     *
     * @param timeout the time out period
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
     * @param mask the event mask associated with this listener
     */
    void listener(DataWriterListener<T>* listener,
                  const ::dds::core::status::StatusMask& mask);

    /**
     * Return the listener currently associated with this DataWriter.
     *
     * @return the DataWriterListener
     */
    DataWriterListener<T>* listener() const;

    //==========================================================================
    //== Status Management

    /**
     * Get the LivelinessLostStatus.
     *
     * @return the LivelinessLostStatus
     */
    const ::dds::core::status::LivelinessLostStatus liveliness_lost_status();

    /**
     * Get the OfferedDeadlineMissedStatus.
     *
     * @return the OfferedDeadlineMissedStatus
     */
    const ::dds::core::status::OfferedDeadlineMissedStatus offered_deadline_missed_status();

    /**
     * Get the OfferedIncompatibleQosStatus.
     *
     * @return the OfferedIncompatibleQosStatus
     */
    const ::dds::core::status::OfferedIncompatibleQosStatus offered_incompatible_qos_status();

    /**
     * Get the PublicationMatchedStatus.
     *
     * @return the PublicationMatchedStatus
     */
    const ::dds::core::status::PublicationMatchedStatus publication_matched_status();


    //==========================================================================
    //== Liveliness Management

    /**
     * Manually asserts the livelines of this DataWriter.
     */
    void assert_liveliness();

    /**
     * This function closes the entity and releases all resources associated with
     * DDS, such as threads, sockets, buffers, etc. Any attempt to invoke
     * functions on a closed entity will raise an exception.
     */
    virtual void close();

    /**
     * Indicates that references to this object may go out of scope but that
     * the application expects to look it up again later. Therefore, the
     * Service must consider this object to be still in use and may not
     * close it automatically.
     */
    virtual void retain();
};

#endif /* OMG_DDS_PUB_DATA_WRITER_HPP_ */
