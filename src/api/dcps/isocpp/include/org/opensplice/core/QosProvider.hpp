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

#ifndef ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_
#define ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_

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

class org::opensplice::core::QosProvider
{
public:
    explicit QosProvider(const std::string& uri)
    {
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
    }

    explicit QosProvider(const std::string& uri, const std::string& id)
    {
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
    }

    dds::domain::qos::DomainParticipantQos
    participant_qos()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::domain::qos::DomainParticipantQos();
    }

    dds::domain::qos::DomainParticipantQos
    participant_qos(const std::string& id)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::domain::qos::DomainParticipantQos();
    }

    dds::topic::qos::TopicQos
    topic_qos()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::topic::qos::TopicQos();
    }

    dds::topic::qos::TopicQos
    topic_qos(const std::string& id)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::topic::qos::TopicQos();
    }

    dds::sub::qos::SubscriberQos
    subscriber_qos()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::sub::qos::SubscriberQos();
    }

    dds::sub::qos::SubscriberQos
    subscriber_qos(const std::string& id)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::sub::qos::SubscriberQos();
    }

    dds::sub::qos::DataReaderQos
    datareader_qos()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::sub::qos::DataReaderQos();
    }

    dds::sub::qos::DataReaderQos
    datareader_qos(const std::string& id)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::sub::qos::DataReaderQos();
    }

    dds::pub::qos::PublisherQos
    publisher_qos()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::pub::qos::PublisherQos();
    }

    dds::pub::qos::PublisherQos
    publisher_qos(const std::string& id)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::pub::qos::PublisherQos();
    }

    dds::pub::qos::DataWriterQos
    datawriter_qos()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::pub::qos::DataWriterQos();
    }

    dds::pub::qos::DataWriterQos
    datawriter_qos(const std::string& id)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
        return dds::pub::qos::DataWriterQos();
    }
};

#endif /* ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_ */
