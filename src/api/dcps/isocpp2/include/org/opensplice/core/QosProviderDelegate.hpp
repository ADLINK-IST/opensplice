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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_QOSPROVIDERDELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_QOSPROVIDERDELEGATE_HPP_

#include <dds/domain/qos/DomainParticipantQos.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>

#include "c_typebase.h"

C_CLASS(cmn_qosProvider);
C_CLASS(cmn_qosProviderInputAttr);

namespace org
{
namespace opensplice
{
namespace core
{
class QosProviderDelegate;
}
}
}

class OMG_DDS_API org::opensplice::core::QosProviderDelegate
{
public:
    QosProviderDelegate(const std::string& uri, const std::string& id = "");

    ~QosProviderDelegate();

    dds::domain::qos::DomainParticipantQos
    participant_qos(const char* id);

    dds::topic::qos::TopicQos
    topic_qos(const char* id);

    dds::sub::qos::SubscriberQos
    subscriber_qos(const char* id);

    dds::sub::qos::DataReaderQos
    datareader_qos(const char* id);

    dds::pub::qos::PublisherQos
    publisher_qos(const char* id);

    dds::pub::qos::DataWriterQos
    datawriter_qos(const char* id);

private:
    template <typename FROM, typename TO>
    static void named_qos__copyOut(void *from, void *to);

    cmn_qosProvider qosProvider;
    static const C_STRUCT(cmn_qosProviderInputAttr) qosProviderAttr;
};

#endif /* ORG_OPENSPLICE_CORE_QOSPROVIDERDELEGATE_HPP_ */
