#ifndef ORG_OPENSPLICE_STREAMS_CORE_POLICY_DEFAULTQOS_CPP_
#define ORG_OPENSPLICE_STREAMS_CORE_POLICY_DEFAULTQOS_CPP_
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

#include <org/opensplice/streams/core/policy/DefaultQos.hpp>

namespace org
{
namespace opensplice
{
namespace streams
{
namespace core
{
namespace policy
{

dds::topic::qos::TopicQos default_topic_qos()
{
    dds::topic::qos::TopicQos topicQos;
    topicQos << dds::core::policy::DurabilityService(dds::core::Duration::zero(), dds::core::policy::HistoryKind::KEEP_ALL)
        << dds::core::policy::Reliability::Reliable(dds::core::Duration::infinite())
        << dds::core::policy::DestinationOrder::SourceTimestamp()
        << dds::core::policy::History::KeepAll();

    return topicQos;
}

dds::sub::qos::DataReaderQos default_datareader_qos()
{
    dds::sub::qos::DataReaderQos drQos;
    drQos << dds::core::policy::Reliability::Reliable(dds::core::Duration::infinite())
        << dds::core::policy::DestinationOrder::SourceTimestamp()
        << dds::core::policy::History::KeepAll()
        << dds::core::policy::ResourceLimits(1000);

    dds::core::policy::ReaderDataLifecycle rdl;
    rdl->no_invalid_samples = true;

    drQos << rdl;

    return drQos;
}

dds::pub::qos::DataWriterQos default_datawriter_qos()
{
    dds::pub::qos::DataWriterQos dwQos;
    dwQos << dds::core::policy::Reliability::Reliable(dds::core::Duration(0, 100000000))
        << dds::core::policy::DestinationOrder::SourceTimestamp()
        << dds::core::policy::History::KeepAll()
        << dds::core::policy::ResourceLimits(10)
        << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances();

    return dwQos;
}

}
}
}
}
}

#endif /* ORG_OPENSPLICE_STREAMS_CORE_POLICY_DEFAULTQOS_CPP_ */
