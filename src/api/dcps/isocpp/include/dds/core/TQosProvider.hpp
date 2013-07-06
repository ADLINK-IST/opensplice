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
#ifndef OSPL_DDS_CORE_TQOSPROVIDER_HPP_
#define OSPL_DDS_CORE_TQOSPROVIDER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/TQosProvider.hpp>

// Implementation
namespace dds
{
namespace core
{

template <typename DELEGATE>
TQosProvider<DELEGATE>::TQosProvider(const std::string& uri, const std::string& profile)
    : Reference<DELEGATE>(new DELEGATE(uri, profile)) { }

template <typename DELEGATE>
TQosProvider<DELEGATE>::TQosProvider(const std::string& uri)
    : Reference<DELEGATE>(new DELEGATE(uri)) { }

template <typename DELEGATE>
dds::domain::qos::DomainParticipantQos
TQosProvider<DELEGATE>::participant_qos()
{
    return this->delegate()->participant_qos();
}

template <typename DELEGATE>
dds::domain::qos::DomainParticipantQos
TQosProvider<DELEGATE>::participant_qos(const std::string& id)
{
    return this->delegate()->participant_qos(id);
}

template <typename DELEGATE>
dds::topic::qos::TopicQos
TQosProvider<DELEGATE>::topic_qos()
{
    return this->delegate()->topic_qos();
}

template <typename DELEGATE>
dds::topic::qos::TopicQos
TQosProvider<DELEGATE>::topic_qos(const std::string& id)
{
    return this->delegate()->topic_qos(id);
}


template <typename DELEGATE>
dds::sub::qos::SubscriberQos
TQosProvider<DELEGATE>::subscriber_qos()
{
    return this->delegate()->subscriber_qos();
}

template <typename DELEGATE>
dds::sub::qos::SubscriberQos
TQosProvider<DELEGATE>::subscriber_qos(const std::string& id)
{
    return this->delegate()->subscriber_qos(id);
}

template <typename DELEGATE>
dds::sub::qos::DataReaderQos
TQosProvider<DELEGATE>::datareader_qos()
{
    return this->delegate()->datareader_qos();
}

template <typename DELEGATE>
dds::sub::qos::DataReaderQos
TQosProvider<DELEGATE>::datareader_qos(const std::string& id)
{
    return this->delegate()->datareader_qos(id);
}

template <typename DELEGATE>
dds::pub::qos::PublisherQos
TQosProvider<DELEGATE>::publisher_qos()
{
    return this->delegate()->publisher_qos();
}

template <typename DELEGATE>
dds::pub::qos::PublisherQos
TQosProvider<DELEGATE>::publisher_qos(const std::string& id)
{
    return this->delegate()->publisher_qos(id);
}

template <typename DELEGATE>
dds::pub::qos::DataWriterQos
TQosProvider<DELEGATE>::datawriter_qos()
{
    return this->delegate()->datawriter_qos();
}

template <typename DELEGATE>
dds::pub::qos::DataWriterQos
TQosProvider<DELEGATE>::datawriter_qos(const std::string& id)
{
    return this->delegate()->datawriter_qos(id);
}
}
}
// End of implementation

#endif /* OSPL_DDS_CORE_TQOSPROVIDER_HPP_ */
