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

namespace dds
{
namespace sub
{
template <typename T>
class DataReaderListener;

template <typename T>
class NoOpDataReaderListener;
}
}

/**
 * The Listener provides a generic mechanism (actually a callback function) for the
 * Data Distribution Service to notify the application of relevant asynchronous
 * communication status change events, such as a missed deadline, violation of a
 * Qos policy, etc.
 * The Listener interfaces are designed as an interface at PIM level. In other words,
 * such an interface is part of the application which must implement the interface
 * operations. A user-defined class for these operations can be provided by the
 * application which must extend from the specific Listener class.
 *
 * The Status information can be accessed from within the user provided Listener
 * eg. -
 * ~~~~~~~~~~~~~~~{.cpp}
 * void FooListener::on_requested_deadline_missed(
 * dds::sub::DataReader<Foo>& reader,
 * const dds::core::status::RequestedDeadlineMissedStatus& status)
 * {
 * std::cout << status.total_count() << std::endl;
 * ...
 * }
 * ~~~~~~~~~~~~~~~
 *
 */
template <typename T>
class dds::sub::DataReaderListener
{
public:
    typedef typename ::dds::core::smart_ptr_traits<DataReaderListener>::ref_type ref_type;

public:
    virtual ~DataReaderListener() = 0;

public:
    /**
     * Status information:
     *
     * total_count
     *
     *    Meaning:
     *
     *        Total cumulative number of missed deadlines detected for any instance
     *        read by the DataReader. Missed deadlines accumulate; that is, each
     *        deadline period the total_count will be incremented by one for each
     *        instance for which data was not received.
     *
     * total_count_change
     *
     *    Meaning:
     *
     *        The incremental number of deadlines detected since the last time the
     *        listener was called or the status was read.
     *
     *  last_instance_handle
     *
     *    Meaning:
     *
     *        Handle to the last instance in the DataReader for which a deadline was
     *        detected.
     *
     * @param reader the DataReader the Listener is applied to
     * @param status the RequestedDeadlineMissedStatus status
     */
    virtual void on_requested_deadline_missed(
        DataReader<T>& reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status) = 0;

    /**
     * Status information:
     *
     * total_count
     *
     *    Meaning:
     *
     *        Total cumulative number of missed deadlines detected for any instance
     *        read by the DataReader. Missed deadlines accumulate; that is, each
     *        deadline period the total_count will be incremented by one for each
     *        instance for which data was not received.
     *
     * total_count_change
     *
     *    Meaning:
     *
     *        The change in total_count since the last time the listener was called or
     *        the status was read.
     *
     * last_policy_id
     *
     *    Meaning:
     *
     *        A list containing for each policy the total number of times that the
     *        concerned DataReader discovered a DataWriter for the same Topic
     *        with an offered QoS that is incompatible with that requested by the
     *        DataReader.
     *
     *
     * policies
     *
     *    Meaning:
     *
     *        A list containing for each policy the total number of times that the
     *        concerned DataReader discovered a DataWriter for the same Topic
     *        with an offered QoS that is incompatible with that requested by the
     *        DataReader.
     *
     * @param reader the DataReader the Listener is applied to
     * @param status the RequestedDeadlineMissedStatus status
     */
    virtual void on_requested_incompatible_qos(
        DataReader<T>& reader,
        const dds::core::status::RequestedIncompatibleQosStatus& status) = 0;

    /**
     * Status information:
     * total_count
     *
     *    Meaning:
     *
     *        Total cumulative count of samples rejected by the DataReader.
     *
     * total_count_change
     *
     *    Meaning:
     *
     *        The incremental number of samples rejected since the last time the
     *        listener was called or the status was read.
     *
     * last_reason
     *
     *    Meaning:
     *
     *        Reason for rejecting the last sample rejected. If no samples have been
     *        rejected, the reason is the special value NOT_REJECTED.
     *
     * last_instance_handle
     *
     *    Meaning:
     *
     *        Handle to the instance being updated by the last sample that was
     *        rejected.
     *
     * @param reader the DataReader the Listener is applied to
     * @param status SampleRejectedStatus status
     */
    virtual void on_sample_rejected(
        DataReader<T>& reader,
        const dds::core::status::SampleRejectedStatus& status) = 0;
    /**
     * Status information:
     * alive_count
     *
     *    Meaning:
     *
     *        The total number of currently active DataWriters that write the Topic
     *        read by the DataReader. This count increases when a newly-matched
     *        DataWriter asserts its liveliness for the first time or when a DataWriter
     *        previously considered to be not alive reasserts its liveliness. The count
     *        decreases when a DataWriter considered alive fails to assert its
     *        liveliness and becomes not alive, whether because it was deleted
     *        normally or for some other reason.
     *
     * not_alive_count
     *
     *    Meaning:
     *
     *        The total count of currently DataWriters that write the Topic read by
     *        the DataReader that are no longer asserting their liveliness. This count
     *        increases when a DataWriter considered alive fails to assert its
     *        liveliness and becomes not alive for some reason other than the normal
     *        deletion of that DataWriter. It decreases when a previously not alive
     *        DataWriter either reasserts its liveliness or is deleted normally.
     *
     * alive_count_change
     *
     *    Meaning:
     *
     *        The change in the alive_count since the last time the listener was
     *        called or the status was read.
     *
     * not_alive_count_change
     *
     *    Meaning:
     *
     *        The change in the not_alive_count since the last time the listener was
     *        called or the status was read.
     *
     * last_publication_handle
     *
     *    Meaning:
     *
     *        Handle to the last DataWriter whose change in liveliness caused this
     *        status to change.
     *
     * @param reader the DataReader the Listener is applied to
     * @param status LivelinessChangedStatus status
     */
    virtual void on_liveliness_changed(
        DataReader<T>& reader,
        const dds::core::status::LivelinessChangedStatus& status) = 0;

    /**
     * Status information:
     *
     * on_data_available does not have any matching status and is simply
     * called whenever a new information is availble on the DataReader.
     *
     * @param reader the DataReader the Listener is applied to
     */
    virtual void on_data_available(DataReader<T>& reader) = 0;

    /**
     * Status information:
     *
     * total_count
     *
     *    Meaning:
     *
     *        Total cumulative count the concerned DataReader discovered a
     *        "match" with a DataWriter. That is, it found a DataWriter for the same
     *        Topic with a requested QoS that is compatible with that offered by the
     *        DataReader.
     *
     * total_count_change
     *
     *    Meaning:
     *
     *        The change in total_count since the last time the listener was called or
     *        the status was read.
     *
     * last_publication_handle
     *
     *    Meaning:
     *
     *        Handle to the last DataWriter that matched the DataReader causing the
     *        status to change.
     *
     * current_count
     *
     *    Meaning:
     *        The number of DataWriters currently matched to the concerned
     *        DataReader.
     *
     * current_count_change
     *
     *    Meaning:
     *
     *        The change in current_count since the last time the listener was called
     *        or the status was read.
     *
     *
     * @param reader the DataReader the Listener is applied to
     * @param status SubscripitonMatchedStatus status
     */
    virtual void on_subscription_matched(
        DataReader<T>& reader,
        const dds::core::status::SubscriptionMatchedStatus& status) = 0;

    /**
     * Status information:
     *
     * total_count
     *
     *    Meaning:
     *
     *        Total cumulative count of all samples lost across all instances of data
     *        published under the Topic.
     *
     * total_count_change
     *
     *    Meaning:
     *
     *        The incremental number of samples lost since the last time the listener
     *        was called or the status was read.
     *
     * @param reader the DataReader the Listener is applied to
     * @param status the SampleLostStatus status
     */
    virtual void on_sample_lost(
        DataReader<T>& reader,
        const dds::core::status::SampleLostStatus& status) = 0;
};

/**
 * NoOpDataReaderListener allows the use of a single DataReaderListener
 * or a specific set of DataReaderListeners callback functions.
 *
 * Other PSMs require all callback functions of a Listener be implemented
 * in user code; this can instead be achieved with inheritance
 * of NoOpDataReaderListener.
 *
 * eg. -
 *
 * ~~~~~~~~~~~~~~~{.cpp}
 * class ExampleDataReaderListener :
 * public virtual dds::sub::DataReaderListener<Foo>,
 * public virtual dds::sub::NoOpDataReaderListener<Foo>
 * {
 * ...
 * ~~~~~~~~~~~~~~~
 */
template <typename T>
class dds::sub::NoOpDataReaderListener : public virtual DataReaderListener<T>
{
public:
    typedef typename ::dds::core::smart_ptr_traits<NoOpDataReaderListener>::ref_type ref_type;

public:
    virtual ~NoOpDataReaderListener();

public:
    virtual void on_requested_deadline_missed(
        DataReader<T>& reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status);

    virtual void on_requested_incompatible_qos(
        DataReader<T>& reader,
        const dds::core::status::RequestedIncompatibleQosStatus& status);

    virtual void on_sample_rejected(
        DataReader<T>& reader,
        const dds::core::status::SampleRejectedStatus& status);

    virtual void on_liveliness_changed(
        DataReader<T>& reader,
        const dds::core::status::LivelinessChangedStatus& status);

    virtual void on_data_available(DataReader<T>& reader);

    virtual void on_subscription_matched(
        DataReader<T>& reader,
        const dds::core::status::SubscriptionMatchedStatus& status);

    virtual void on_sample_lost(
        DataReader<T>& reader,
        const dds::core::status::SampleLostStatus& status);
};

#endif /* OMG_DDS_SUB_DATA_READER_LISTENER_HPP_ */
