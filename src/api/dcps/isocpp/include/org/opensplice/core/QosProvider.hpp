/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_
#define ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_

#include <org/opensplice/core/config.hpp>

C_CLASS(cmn_qosProvider);
C_CLASS(cmn_qosProviderInputAttr);

namespace org
{
namespace opensplice
{
namespace core
{
class QosProvider;
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::core::QosProvider
{
public:
    explicit QosProvider(const std::string& uri);

    explicit QosProvider(const std::string& uri, const std::string& id);

    ~QosProvider();

    dds::domain::qos::DomainParticipantQos
    participant_qos();

    dds::domain::qos::DomainParticipantQos
    participant_qos(const std::string& id);

    dds::topic::qos::TopicQos
    topic_qos();

    dds::topic::qos::TopicQos
    topic_qos(const std::string& id);

    dds::sub::qos::SubscriberQos
    subscriber_qos();

    dds::sub::qos::SubscriberQos
    subscriber_qos(const std::string& id);

    dds::sub::qos::DataReaderQos
    datareader_qos();

    dds::sub::qos::DataReaderQos
    datareader_qos(const std::string& id);

    dds::pub::qos::PublisherQos
    publisher_qos();

    dds::pub::qos::PublisherQos
    publisher_qos(const std::string& id);

    dds::pub::qos::DataWriterQos
    datawriter_qos();

    dds::pub::qos::DataWriterQos
    datawriter_qos(const std::string& id);

    private:
    cmn_qosProvider qosProvider;

    const C_STRUCT(cmn_qosProviderInputAttr) * getQosProviderInputAttr();
};

#endif /* ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_ */
