/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef VORTEX_FACE_CONNECTION_CONFIGURATION_HPP_
#define VORTEX_FACE_CONNECTION_CONFIGURATION_HPP_


#include "Vortex_FACE.hpp"
#include "Vortex/FACE/Macros.hpp"


namespace Vortex {
namespace FACE {

class VORTEX_FACE_API ConnectionConfig
{
public:
    typedef Vortex::FACE::smart_ptr_traits< ConnectionConfig >::shared_ptr shared_ptr;

    ConnectionConfig();

    ::FACE::RETURN_CODE_TYPE set(const std::string &tag, const std::string &value);
    ::FACE::RETURN_CODE_TYPE validate();

    std::string getConnectionName();
    std::string getTopicName();
    std::string getTypeName();

    ::FACE::CONNECTION_DIRECTION_TYPE getDirection();
    ::FACE::MESSAGE_TYPE_GUID         getGuid();
    ::FACE::SYSTEM_TIME_TYPE          getRefreshPeriod();

    uint32_t getDomainId();

    dds::domain::qos::DomainParticipantQos  getParticipantQos();
    dds::sub::qos::SubscriberQos            getSubscriberQos();
    dds::pub::qos::PublisherQos             getPublisherQos();
    dds::topic::qos::TopicQos               getTopicQos();
    dds::pub::qos::DataWriterQos            getWriterQos();
    dds::sub::qos::DataReaderQos            getReaderQos();

private:
    long long
    strtoll(const std::string &value, bool &ok);

    std::string connectionName;
    std::string topicName;
    std::string typeName;
    std::string type;
    std::string direction;
    std::string domainId;
    std::string guid;
    std::string refresh;
    std::string qosUri;
    std::string qosProfile;
    std::string qosParticipantId;
    std::string qosSubscriberId;
    std::string qosPublisherId;
    std::string qosTopicId;
    std::string qosReaderId;
    std::string qosWriterId;

    dds::domain::qos::DomainParticipantQos  participantQos;
    dds::sub::qos::SubscriberQos            subscriberQos;
    dds::pub::qos::PublisherQos             publisherQos;
    dds::topic::qos::TopicQos               topicQos;
    dds::pub::qos::DataWriterQos            writerQos;
    dds::sub::qos::DataReaderQos            readerQos;

    bool valid;

    static const size_t TAG_PREFIX_LEN;
};

}; /* namespace FACE */
}; /* namespace Vortex */

#endif /* VORTEX_FACE_CONNECTION_CONFIGURATION_HPP_ */
