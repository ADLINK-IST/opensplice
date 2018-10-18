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
#ifndef OSPL_DDS_TOPIC_TTOPIC_HPP_
#define OSPL_DDS_TOPIC_TTOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TTopic.hpp>
#include <dds/topic/AnyTopic.hpp>
#include <org/opensplice/core/Retain.hpp>

// Implementation

namespace dds
{
namespace topic
{

class AnyTopic;

template <typename T, template <typename Q> class DELEGATE>
Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
                          const std::string& topic_name)
    : dds::topic::TopicDescription<T, DELEGATE>(new DELEGATE<T>(dp,
            topic_name,
            topic_type_name<T>::value(),
            dp.default_topic_qos(),
            NULL,
            dds::core::status::StatusMask::none()))
{
    std::stringstream ss;
    ss << topic_name;
    ss << dp.domain_id();
    org::opensplice::core::EntityRegistry<std::string, Topic<T, DELEGATE> >::insert(ss.str(), *this);
}

template <typename T, template <typename Q> class DELEGATE>
Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
                          const std::string& topic_name,
                          const std::string& type_name)
    : dds::topic::TopicDescription<T, DELEGATE>(new DELEGATE<T>(dp,
            topic_name,
            type_name,
            dp.default_topic_qos(),
            NULL,
            dds::core::status::StatusMask::none()))
{
    std::stringstream ss;
    ss << topic_name;
    ss << dp.domain_id();
    org::opensplice::core::EntityRegistry<std::string, Topic<T, DELEGATE> >::insert(ss.str(), *this);
}

template <typename T, template <typename Q> class DELEGATE>
Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
                          const std::string& topic_name,
                          const dds::topic::qos::TopicQos& qos,
                          dds::topic::TopicListener<T>* listener,
                          const dds::core::status::StatusMask& mask)
    : dds::topic::TopicDescription<T, DELEGATE>(new DELEGATE<T>(dp,
            topic_name,
            topic_type_name<T>::value(),
            qos,
            listener,
            mask))
{
    if(listener != NULL)
    {
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : TopicListener is not currently supported")));
    }
    std::stringstream ss;
    ss << topic_name;
    ss << dp.domain_id();
    org::opensplice::core::EntityRegistry<std::string, Topic<T, DELEGATE> >::insert(ss.str(), *this);
}

template <typename T, template <typename Q> class DELEGATE>
Topic<T, DELEGATE>::Topic(const dds::domain::DomainParticipant& dp,
                          const std::string& topic_name,
                          const std::string& type_name,
                          const dds::topic::qos::TopicQos& qos,
                          dds::topic::TopicListener<T>* listener,
                          const dds::core::status::StatusMask& mask)
    : dds::topic::TopicDescription<T, DELEGATE>(new DELEGATE<T>(dp,
            topic_name,
            type_name,
            qos,
            listener,
            mask))
{
    if(listener != NULL)
    {
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : TopicListener is not currently supported")));
    }
    std::stringstream ss;
    ss << topic_name;
    ss << dp.domain_id();
    org::opensplice::core::EntityRegistry<std::string, Topic<T, DELEGATE> >::insert(ss.str(), *this);
}

template <typename T, template <typename Q> class DELEGATE>
Topic<T, DELEGATE>::~Topic() {}

/** @internal  @todo Relates to OMG_DDS_X_TYPE_DYNAMIC_TYPE_SUPPORT OSPL-1736 no implementation */
template <typename T, template <typename Q> class DELEGATE>
void Topic<T, DELEGATE>::listener(Listener* listener,
                                  const ::dds::core::status::StatusMask& event_mask)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
}

/** @internal @todo Relates to OMG_DDS_X_TYPE_DYNAMIC_TYPE_SUPPORT OSPL-1736 no implementation */
template <typename T, template <typename Q> class DELEGATE>
typename Topic<T, DELEGATE>::Listener* Topic<T, DELEGATE>::listener() const
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
    return this->::dds::core::Reference<DELEGATE<T> >::delegate()->listener();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::topic::qos::TopicQos& Topic<T, DELEGATE>::qos() const
{
    return this->::dds::core::Reference<DELEGATE<T> >::delegate()->qos();
}

template <typename T, template <typename Q> class DELEGATE>
void Topic<T, DELEGATE>::qos(const dds::topic::qos::TopicQos& qos)
{
    this->::dds::core::Reference<DELEGATE<T> >::delegate()->qos(qos);
}

template <typename T, template <typename Q> class DELEGATE>
dds::topic::qos::TopicQos& Topic<T, DELEGATE>::operator << (const dds::topic::qos::TopicQos& qos)
{
    this->qos(qos);
    return (dds::topic::qos::TopicQos&)this->qos();
}

template <typename T, template <typename Q> class DELEGATE>
const Topic<T, DELEGATE>& Topic<T, DELEGATE>::operator >> (dds::topic::qos::TopicQos& qos) const
{
    qos = this->qos();
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::status::InconsistentTopicStatus&
Topic<T, DELEGATE>::inconsistent_topic_status() const
{
    return this->delegate()->inconsistent_topic_status();
}


}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TTOPIC_HPP_ */
