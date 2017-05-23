#ifndef OMG_DDS_CORE_QOS_TPROVIDER_HPP_
#define OMG_DDS_CORE_QOS_TPROVIDER_HPP_


/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
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

#include <dds/core/Reference.hpp>

#include <dds/domain/qos/DomainParticipantQos.hpp>

#include <dds/topic/qos/TopicQos.hpp>

#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>

#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>

namespace dds
{
namespace core
{
template <typename DELEGATE>
class TQosProvider;
}
}

template <typename DELEGATE>
class dds::core::TQosProvider : public dds::core::Reference<DELEGATE>
{
public:
    /**
     * Create a QosProvider fetching QoS configuration from the specified URI.
     * For instance:
     * <pre><code>
           QosProvider xml_file_provider("file:///somewhere/on/disk/qos-config.xml");
        </code></pre>

     * The URI determines the how the Qos configuration is fetched and the
     * format in which it is represented. This specification requires compliant
     * implementations to support at least one file-based configuration using
     * the XML syntax defined as part of the DDS for CCM specification (formal/12.02.01).
     *
     * @param uri URI of QosProvider xml file
     * @param profile the profile name within the xml file
     */
    explicit TQosProvider(const std::string& uri, const std::string& profile);

    /**
     * Create a QosProvider fetching QoS configuration from the specified URI
     * @param uri URI of the QosProvider xml file
     */
    explicit TQosProvider(const std::string& uri);

    /**
     * @return DomainParticipantQos from the given URI (and profile)
     */
    dds::domain::qos::DomainParticipantQos participant_qos();

    /**
     * @param id the identifier of a specific Domain Participant
     * @return DomainParticipantQos from the given URI (and profile) using the id
     */
    dds::domain::qos::DomainParticipantQos participant_qos(const std::string& id);

    /**
     * @return TopicQos from the given URI (and profile)
     */
    dds::topic::qos::TopicQos topic_qos();

    /**
     * @param id the identifier of a specific Topic
     * @return TopicQos from the given URI (and profile) using the id
     */
    dds::topic::qos::TopicQos topic_qos(const std::string& id);

    /**
     * @return SubscriberQos from the given URI (and profile)
     */
    dds::sub::qos::SubscriberQos subscriber_qos();

    /**
     * @param id the identifier of a specific Subscriber
     * @return SubscriberQos from the given URI (and profile) using the id
     */
    dds::sub::qos::SubscriberQos subscriber_qos(const std::string& id);

    /**
     * @return DataReadertQos from the given URI (and profile)
     */
    dds::sub::qos::DataReaderQos datareader_qos();

    /**
     * @param id the identifier of a specific DataReader
     * @return DataReaderQos from the given URI (and profile) using the id
     */
    dds::sub::qos::DataReaderQos datareader_qos(const std::string& id);

    /**
     * @return PublisherQos from the given URI (and profile)
     */
    dds::pub::qos::PublisherQos publisher_qos();

    /**
     * @param id the identifier of a specific Publisher
     * @return PublisherQos from the given URI (and profile) using the id
     */
    dds::pub::qos::PublisherQos publisher_qos(const std::string& id);

    /**
     * @return DataWriterQos from the given URI (and profile)
     */
    dds::pub::qos::DataWriterQos datawriter_qos();

    /**
     * @param id the identifier of a specific DataWriter
     * @return DataWriterQos from the given URI (and profile) using the id
     */
    dds::pub::qos::DataWriterQos datawriter_qos(const std::string& id);
};

#endif /* OMG_DDS_CORE_QOS_TPROVIDER_HPP_ */
