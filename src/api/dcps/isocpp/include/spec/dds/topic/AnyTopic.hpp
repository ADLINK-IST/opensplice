#ifndef OMG_DDS_TOPIC_ANY_TOPIC_HPP_
#define OMG_DDS_TOPIC_ANY_TOPIC_HPP_

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

#include <dds/core/Exception.hpp>
#include <dds/core/ref_traits.hpp>
#include <dds/topic/detail/AnyTopic.hpp>
#include <dds/topic/TopicDescription.hpp>

namespace dds
{
namespace topic
{
class AnyTopic;

/**
 * Extracts a typed Topic from an AnyTopic.
 *
 * @param at the AnyTopic
 * @return the typed Topic
 */
template <typename T>
Topic<T> get(const AnyTopic& at);
}
}
class OMG_DDS_API dds::topic::AnyTopic
{
public:
    /**
     * Create an AnyTopic instance from a Topic.
     *
     * @param t the Topic
     */
    template <typename T>
    AnyTopic(const dds::topic::Topic<T>& t);

public:
    /**
     * Gets the DomainParticipant associated with the contained Topic.
     *
     * @param the DomainParticipant
     */
    const dds::domain::DomainParticipant& domain_participant() const;

    /**
     * This function allows the application to retrieve the
     * inconsistent_topic_status of the contained Topic.
     *
     * Each DomainEntity has a set of relevant communication statuses.
     * A change of status causes the corresponding Listener to be invoked
     * and can also be monitored by means of the associated StatusCondition.
     */
    const dds::core::status::InconsistentTopicStatus& inconsistent_topic_status();

    /**
     * Gets the QoS of the contained Topic.
     *
     * @return the QoS
     */
    const dds::topic::qos::TopicQos& qos() const;

    /**
     * Sets the QoS of the contained Topic.
     *
     * @param q the QoS
     */
    void qos(const dds::topic::qos::TopicQos& q);

public:
    /**
     * Extracts a typed Topic from an AnyTopic.
     *
     * @param at the AnyTopic
     * @return the typed Topic
     */
    template <typename T>
    Topic<T> get();

    /**
     * Checks if two AnyTopics contain the same Topic.
     *
     * @param other the AnyTopic to compare with
     */
    bool operator==(const dds::topic::AnyTopic& other) const
    {
        return holder_.get()->get_dds_topic() == other.holder_.get()->get_dds_topic();
    }
public:
    inline const detail::THolderBase* operator->() const;

    detail::THolderBase* operator->();

private:
    dds::core::smart_ptr_traits<detail::THolderBase>::ref_type holder_;
};


#endif /* OMG_DDS_TOPIC_ANY_TOPIC_HPP_ */
