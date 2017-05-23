#ifndef OMG_DDS_PUB_ANY_DATA_WRITER_LISTENER_HPP_
#define OMG_DDS_PUB_ANY_DATA_WRITER_LISTENER_HPP_

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

#include <dds/pub/AnyDataWriter.hpp>

namespace dds
{
namespace pub
{
class AnyDataWriterListener;
class NoOpAnyDataWriterListener;
}
}

class OMG_DDS_API dds::pub::AnyDataWriterListener
{
public:
    virtual ~AnyDataWriterListener();

public:
    /**
     * Status information:
     * total_count
     *
     *   Meaning:
     *
     *       Total cumulative number of offered deadline periods elapsed during
     *        which a DataWriter failed to provide data. Missed deadlines
     *        accumulate; that is, each deadline period the total_count will be
     *        incremented by one.
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
     * @param writer the AnyDataWriter the Listener is applied to
     * @param status the OfferedDeadlineMissedStatus status
     */
    virtual void on_offered_deadline_missed(dds::pub::AnyDataWriter& writer,
                                            const ::dds::core::status::OfferedDeadlineMissedStatus& status) = 0;

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
     *       A list containing, for each policy, the total number of times that the
     *       concerned DataWriter discovered a DataReader for the same Topic
     *       with a requested QoS that is incompatible with that offered by the
     *       DataWriter.
     *
     *
     * @param writer the AnyDataWriter the Listener is applied to
     * @param status the OfferedIncompatibleQosStatus status
     */
    virtual void on_offered_incompatible_qos(dds::pub::AnyDataWriter& writer,
            const ::dds::core::status::OfferedIncompatibleQosStatus& status) = 0;

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
    * @param writer the AnyDataWriter the Listener is applied to
    * @param status the LivelinessLostStatus status
    */
    virtual void on_liveliness_lost(dds::pub::AnyDataWriter& writer,
                                    const ::dds::core::status::LivelinessLostStatus& status) = 0;

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
     * @param writer the AnyDataWriter the Listener is applied to
     * @param status the LivelinessLostStatus status
     */
    virtual void on_publication_matched(dds::pub::AnyDataWriter& writer,
                                        const ::dds::core::status::PublicationMatchedStatus& status) = 0;

};

class OMG_DDS_API dds::pub::NoOpAnyDataWriterListener : public virtual dds::pub::AnyDataWriterListener
{
public:
    virtual ~NoOpAnyDataWriterListener();

public:
    virtual void on_offered_deadline_missed(dds::pub::AnyDataWriter& writer,
                                            const ::dds::core::status::OfferedDeadlineMissedStatus& status);

    virtual void on_offered_incompatible_qos(dds::pub::AnyDataWriter& writer,
            const ::dds::core::status::OfferedIncompatibleQosStatus& status);

    virtual void on_liveliness_lost(dds::pub::AnyDataWriter& writer,
                                    const ::dds::core::status::LivelinessLostStatus& status);

    virtual void on_publication_matched(dds::pub::AnyDataWriter& writer,
                                        const ::dds::core::status::PublicationMatchedStatus& status);

};


#endif /* OMG_DDS_PUB_ANY_DATA_WRITER_LISTENER_HPP_ */
