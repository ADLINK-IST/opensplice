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

#include <org/opensplice/core/Retain.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/AnyDataWriter.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/AnyDataReader.hpp>
#include <dds/topic/AnyTopic.hpp>
#include <vector>
#include <algorithm>

namespace org
{
namespace opensplice
{
namespace core
{

/* DomainParticipant */

static std::vector<dds::domain::DomainParticipant> domainParticipantRetain;

template <>
OSPL_ISOCPP_IMPL_API void
retain_add<dds::domain::DomainParticipant>(dds::domain::DomainParticipant& e)
{
    domainParticipantRetain.push_back(e);
}

template <>
OSPL_ISOCPP_IMPL_API void
retain_remove<dds::domain::DomainParticipant>(dds::domain::DomainParticipant& e)
{
    domainParticipantRetain.erase(std::remove(
                                      domainParticipantRetain.begin(), domainParticipantRetain.end(), e), domainParticipantRetain.end());
}

/* Publisher */

static std::vector<dds::pub::Publisher> publisherRetain;

template <>
OSPL_ISOCPP_IMPL_API void
retain_add<dds::pub::Publisher>(dds::pub::Publisher& e)
{
    publisherRetain.push_back(e);
}

template <>
OSPL_ISOCPP_IMPL_API void
retain_remove<dds::pub::Publisher>(dds::pub::Publisher& e)
{
    publisherRetain.erase(std::remove(
                              publisherRetain.begin(), publisherRetain.end(), e), publisherRetain.end());
}

/* DataWriter */

static std::vector<dds::pub::AnyDataWriter> dataWriterRetain;

template <>
OSPL_ISOCPP_IMPL_API void
retain_add<dds::pub::AnyDataWriter>(dds::pub::AnyDataWriter& e)
{
    dataWriterRetain.push_back(e);
}

template <>
OSPL_ISOCPP_IMPL_API void
retain_remove<dds::pub::AnyDataWriter>(dds::pub::AnyDataWriter& e)
{
    dataWriterRetain.erase(std::remove(
                               dataWriterRetain.begin(), dataWriterRetain.end(), e), dataWriterRetain.end());
}

/* Subscriber */

static std::vector<dds::sub::Subscriber> subscriberRetain;

template <>
OSPL_ISOCPP_IMPL_API void
retain_add<dds::sub::Subscriber>(dds::sub::Subscriber& e)
{
    subscriberRetain.push_back(e);
}

template <>
OSPL_ISOCPP_IMPL_API void
retain_remove<dds::sub::Subscriber>(dds::sub::Subscriber& e)
{
    subscriberRetain.erase(std::remove(
                               subscriberRetain.begin(), subscriberRetain.end(), e), subscriberRetain.end());
}

/* DataReader */

static std::vector<dds::sub::AnyDataReader> dataReaderRetain;

template <>
OSPL_ISOCPP_IMPL_API void
retain_add<dds::sub::AnyDataReader>(dds::sub::AnyDataReader& e)
{
    dataReaderRetain.push_back(e);
}

template <>
OSPL_ISOCPP_IMPL_API void
retain_remove<dds::sub::AnyDataReader>(dds::sub::AnyDataReader& e)
{
    dataReaderRetain.erase(std::remove(
                               dataReaderRetain.begin(), dataReaderRetain.end(), e), dataReaderRetain.end());
}

/* Topic */

static std::vector<dds::topic::AnyTopic> topicRetain;

template <>
OSPL_ISOCPP_IMPL_API void
retain_add<dds::topic::AnyTopic>(dds::topic::AnyTopic& e)
{
    topicRetain.push_back(e);
}

template <>
OSPL_ISOCPP_IMPL_API void
retain_remove<dds::topic::AnyTopic>(dds::topic::AnyTopic& e)
{
    topicRetain.erase(std::remove(
                          topicRetain.begin(), topicRetain.end(), e), topicRetain.end());
}

}
}
}