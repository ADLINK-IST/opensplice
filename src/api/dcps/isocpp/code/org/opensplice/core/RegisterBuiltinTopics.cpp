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

#include <org/opensplice/core/RegisterBuiltinTopics.hpp>
#include <org/opensplice/core/EntityRegistry.hpp>

INSTANTIATE_TYPED_REGISTRIES(::DDS::ParticipantBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::TopicBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::PublicationBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::SubscriptionBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::CMParticipantBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::CMPublisherBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::CMSubscriberBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::CMDataWriterBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::CMDataReaderBuiltinTopicData)
