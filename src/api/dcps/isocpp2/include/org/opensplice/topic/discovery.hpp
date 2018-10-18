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

#ifndef ORG_OPENSPLICE_TOPIC_DISCOVER_HPP_
#define ORG_OPENSPLICE_TOPIC_DISCOVER_HPP_

#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/TopicDescription.hpp>
#include <dds/topic/AnyTopic.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/ContentFilteredTopic.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{


template <typename T, typename DELEGATE>
struct typed_lookup_topic {
    template <typename TOPIC>
    static inline TOPIC discover(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const dds::core::Duration& timeout);

    template <typename TOPIC>
    static inline void discover(
            const dds::domain::DomainParticipant& dp,
            std::vector<TOPIC>& list,
            uint32_t max_size);
};


template <typename T>
struct typed_lookup_topic<T, dds::topic::detail::ContentFilteredTopic<T> > {
    static inline dds::topic::ContentFilteredTopic<T> discover(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const dds::core::Duration& timeout)
    {
        ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "Function not currently supported");
    }

    static inline void discover(
            const dds::domain::DomainParticipant& dp,
            std::vector<dds::topic::ContentFilteredTopic<T> >& list,
            uint32_t max_size)
    {
        ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "Function not currently supported");
    }
};

template <typename T>
struct typed_lookup_topic<T, dds::topic::detail::Topic<T> > {
    static inline dds::topic::Topic<T> discover(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const dds::core::Duration& timeout)
    {
        return dds::topic::detail::Topic<T>::discover_topic(dp, topic_name, timeout);
    }

    static inline void discover(
             const dds::domain::DomainParticipant& dp,
             std::vector<dds::topic::Topic<T> >& list,
             uint32_t max_size)
     {
         dds::topic::detail::Topic<T>::discover_topics(dp, list, max_size);
     }
};



template <typename TOPIC, typename DELEGATE>
struct lookup_topic {
    static inline TOPIC discover(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const dds::core::Duration& timeout)
    {
        return org::opensplice::topic::typed_lookup_topic<typename TOPIC::DataType,
                                                          typename TOPIC::DELEGATE_T>::discover(dp, topic_name, timeout);
    }

    static inline void discover(
            const dds::domain::DomainParticipant& dp,
            std::vector<TOPIC>& list,
            uint32_t max_size)
    {
        org::opensplice::topic::typed_lookup_topic<typename TOPIC::DataType,
                                                   typename TOPIC::DELEGATE_T>::discover(dp, list, max_size);
    }
};

template <>
struct lookup_topic<dds::topic::TopicDescription, org::opensplice::topic::TopicDescriptionDelegate> {
    static inline dds::topic::TopicDescription discover(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const dds::core::Duration& timeout)
    {
        return org::opensplice::topic::AnyTopicDelegate::discover_topic(dp, topic_name, timeout);
    }

    static inline void discover(
            const dds::domain::DomainParticipant& dp,
            std::vector<dds::topic::TopicDescription>& list,
            uint32_t max_size)
    {
        std::vector<dds::topic::AnyTopic> anyTopics;
        org::opensplice::topic::AnyTopicDelegate::discover_topics(dp, anyTopics, max_size);
        for (std::vector<dds::topic::AnyTopic>::iterator it = anyTopics.begin(); it != anyTopics.end(); ++it) {
            list.push_back(*it);
        }
    }
};

template <>
struct lookup_topic<dds::topic::AnyTopic, org::opensplice::topic::AnyTopicDelegate> {
    static inline dds::topic::AnyTopic discover(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const dds::core::Duration& timeout)
    {
        return org::opensplice::topic::AnyTopicDelegate::discover_topic(dp, topic_name, timeout);
    }

    static inline void discover(
            const dds::domain::DomainParticipant& dp,
            std::vector<dds::topic::AnyTopic>& list,
            uint32_t max_size)
    {
        org::opensplice::topic::AnyTopicDelegate::discover_topics(dp, list, max_size);
    }
};



}
}
}


#endif /* ORG_OPENSPLICE_TOPIC_DISCOVER_HPP_ */
