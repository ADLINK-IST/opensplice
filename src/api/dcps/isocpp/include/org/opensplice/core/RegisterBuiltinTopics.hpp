
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

#ifndef ORG_OPENSPLICE_CORE_REGISTER_BUILTIN_TOPICS_HPP_
#define ORG_OPENSPLICE_CORE_REGISTER_BUILTIN_TOPICS_HPP_

#include <dds/dds.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>

REGISTER_TOPIC_TRAITS(::DDS::ParticipantBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::TopicBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::PublicationBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::SubscriptionBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::CMParticipantBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::CMPublisherBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::CMSubscriberBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::CMDataWriterBuiltinTopicData)

REGISTER_TOPIC_TRAITS(::DDS::CMDataReaderBuiltinTopicData)

#endif /* ORG_OPENSPLICE_CORE_REGISTER_BUILTIN_TOPICS_HPP_ */

