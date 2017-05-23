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
 */

#include <org/opensplice/topic/find.hpp>


namespace org
{
namespace opensplice
{
namespace topic
{



dds::topic::TopicDescription
find_topic_description(
    const dds::domain::DomainParticipant& dp,
    const std::string& topic_name)
{
    dds::topic::TopicDescription t = dds::core::null;

    org::opensplice::core::ObjectDelegate::ref_type entity = dp.delegate()->find_topic(topic_name);
    if (!entity) {
        entity = dp.delegate()->find_cfTopic(topic_name);
    }
    if (entity) {
        dds::topic::TopicDescription::DELEGATE_REF_T ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::topic::TopicDescriptionDelegate>(entity);
        t = dds::topic::TopicDescription(ref);
    }

    return t;
}

dds::topic::AnyTopic
find_any_topic(
    const dds::domain::DomainParticipant& dp,
    const std::string& topic_name)
{
    dds::topic::AnyTopic t = dds::core::null;

    org::opensplice::core::EntityDelegate::ref_type entity = dp.delegate()->find_topic(topic_name);
    if (entity) {
        dds::topic::AnyTopic::DELEGATE_REF_T ref =
                OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<org::opensplice::topic::AnyTopicDelegate>(entity);
        t = dds::topic::AnyTopic(ref);
    }

    return t;
}


}
}
}
