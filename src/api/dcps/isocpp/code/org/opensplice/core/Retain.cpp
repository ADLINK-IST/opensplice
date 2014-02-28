/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
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