#ifndef OMG_DDS_PUB_DATA_WRITER_LISTENER_HPP_
#define OMG_DDS_PUB_DATA_WRITER_LISTENER_HPP_

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


#include <dds/pub/DataWriter.hpp>

namespace dds
{
namespace pub
{

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
 * void FooListener::on_offered_deadline_missed(
 * dds::sub::DataWriter<Foo>& writer,
 * const dds::core::status::OfferedDeadlineMissedStatus& status)
 * {
 * std::cout << status.total_count() << std::endl;
 * ...
 * }
 * ~~~~~~~~~~~~~~~
 *
 */
template <typename T>
class DataWriterListener
{
public:
    virtual ~DataWriterListener() { }

public:
    /**
     * Status information:
     * total_count
     *
     *   Meaning:
     *
     *       Total cumulative number of offered deadline periods elapsed during
     *        which a DataWriter failed to provide data. Missed deadlines
     *        accumulate; that is, at the end of each deadline period the
	  *        total_count will be incremented by one.
     *
     * total_count_change
     *
     *   Meaning:
     *
     *       The change in total_count since the last time the listener was called or
     *       the status was read.
     *
     * last_instance_handle
     *
     *   Meaning:
     *
     *       Handle to the last instance in the DataWriter for which an offered
     *       deadline was missed.
     *
     * @param writer the typed DataWriter the Listener is applied to
     * @param status the OfferedDeadlineMissedStatus the
     */
    virtual void on_offered_deadline_missed(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status) = 0;

    /**
     * Status information:
     * total_count
     *
     *   Meaning:
     *
     *       Total cumulative number of times the concerned DataWriter
     *       discovered a DataReader for the same Topic with a requested QoS that
     *       is incompatible with that offered by the DataWriter.
     *
     * total_count_change
     *
     *   Meaning:
     *
     *       The change in total_count since the last time the listener was called or
     *       the status was read.
     *
     * last_policy_id
     *
     *   Meaning:
     *
     *       The PolicyId of one of the policies that was found to be
     *       incompatible the last time an incompatibility was detected.
     *
     * policies
     *
     *   Meaning:
     *
     *       A list containing for each policy the total number of times that the
     *       concerned DataWriter discovered a DataReader for the same Topic
     *       with a requested QoS that is incompatible with that offered by the
     *       DataWriter.
     *
     *
     * @param writer the typed DataWriter the Listener is applied to
     * @param status the OfferedIncompatibleQosStatus status
     */
    virtual void on_offered_incompatible_qos(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedIncompatibleQosStatus&  status) = 0;

    /**
     * Status information:
     * total_count
     *
     *   Meaning:
     *
     *       Total cumulative number of times that a previously-alive DataWriter
     *       became 'not alive' due to a failure to actively signal its liveliness within
     *       its offered liveliness period. This count does not change when an
     *       already not alive DataWriter simply remains not alive for another
     *       liveliness period.
     *
     * total_count_change
     *
     *   Meaning:
     *
     *       The change in total_count since the last time the listener was called or
     *       the status was read.
     *
     *
     *
     * @param writer the typed DataWriter the Listener is applied to
     * @param status the LivelinessLostStatus status
     */
    virtual void on_liveliness_lost(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::LivelinessLostStatus& status) = 0;

    /**
     * Status information:
     * total_count
     *
     *   Meaning:
     *
     *       Total cumulative count the concerned DataWriter discovered a
     *       "match" with a DataReader. That is, it found a DataReader for the
     *       same Topic with a requested QoS that is compatible with that offered
     *       by the DataWriter.
     *
     * total_count_change
     *
     *   Meaning:
     *
     *       The change in total_count since the last time the listener was called or
     *       the status was read.
     *
     * last_subscription_handle
     *
     *   Meaning:
     *
     *       Handle to the last DataReader that matched the DataWriter causing the
     *       status to change.
     *
     * current_count
     *
     *   Meaning:
     *
     *       The number of DataReaders currently matched to the concerned
     *       DataWriter.
     *
     * current_count_change
     *
     *   Meaning:
     *
     *      The change in current_count since the last time the listener was called
     *      or the status was read.
     *
     *
     * @param writer the typed DataWriter the Listener is applied to
     * @param status the LivelinessLostStatus status
     */
    virtual void on_publication_matched(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::PublicationMatchedStatus& status) = 0;
};

/**
 * NoOpDataWriterListener allows the use of a single DataWriterListener
 * or a specific set of DataWriterListeners callback functions.
 *
 * Other PSMs require all callback functions of a Listener be implemented
 * in user code, this can instead be achieved with inheritance
 * of NoOpDataWriterListener.
 *
 * eg. -
 *
 * ~~~~~~~~~~~~~~~{.cpp}
 * class ExampleDataWriterListener :
 * public virtual dds::sub::DataWriterListener<Foo>,
 * public virtual dds::sub::NoOpDataWriterListener<Foo>
 * {
 * ...
 * ~~~~~~~~~~~~~~~
 */
template <typename T>
class NoOpDataWriterListener : public virtual DataWriterListener<T>
{
public:
    virtual ~NoOpDataWriterListener();

public:
    virtual void
    on_offered_deadline_missed(dds::pub::DataWriter<T>& writer,
                               const dds::core::status::OfferedDeadlineMissedStatus& status);

    virtual void
    on_offered_incompatible_qos(dds::pub::DataWriter<T>& writer,
                                const dds::core::status::OfferedIncompatibleQosStatus&  status);

    virtual void
    on_liveliness_lost(dds::pub::DataWriter<T>& writer,
                       const dds::core::status::LivelinessLostStatus& status);

    virtual void
    on_publication_matched(dds::pub::DataWriter<T>& writer,
                           const dds::core::status::PublicationMatchedStatus& status);
};

}
}

#endif /* OMG_DDS_PUB_DATA_WRITER_LISTENER_HPP_ */
