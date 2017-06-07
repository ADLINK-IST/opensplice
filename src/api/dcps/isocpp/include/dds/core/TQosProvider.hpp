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
