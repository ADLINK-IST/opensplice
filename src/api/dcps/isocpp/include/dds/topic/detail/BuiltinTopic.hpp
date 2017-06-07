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
#ifndef OSPL_DDS_TOPIC_DETAIL_BUILTINTOPIC_HPP_
#define OSPL_DDS_TOPIC_DETAIL_BUILTINTOPIC_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/topic/BuiltinTopicImpl.hpp>
#include <dds/topic/TBuiltinTopic.hpp>

namespace dds
{
namespace topic
{
namespace detail
{

typedef dds::topic::TParticipantBuiltinTopicData<org::opensplice::topic::ParticipantBuiltinTopicDataImpl>
ParticipantBuiltinTopicData;

typedef dds::topic::TTopicBuiltinTopicData<org::opensplice::topic::TopicBuiltinTopicDataImpl>
TopicBuiltinTopicData;

typedef dds::topic::TPublicationBuiltinTopicData<org::opensplice::topic::PublicationBuiltinTopicDataImpl>
PublicationBuiltinTopicData;

typedef dds::topic::TSubscriptionBuiltinTopicData<org::opensplice::topic::SubscriptionBuiltinTopicDataImpl>
SubscriptionBuiltinTopicData;

typedef dds::topic::TCMParticipantBuiltinTopicData<org::opensplice::topic::CMParticipantBuiltinTopicDataImpl>
CMParticipantBuiltinTopicData;

typedef dds::topic::TCMPublisherBuiltinTopicData<org::opensplice::topic::CMPublisherBuiltinTopicDataImpl>
CMPublisherBuiltinTopicData;

typedef dds::topic::TCMSubscriberBuiltinTopicData<org::opensplice::topic::CMSubscriberBuiltinTopicDataImpl>
CMSubscriberBuiltinTopicData;

typedef dds::topic::TCMDataWriterBuiltinTopicData<org::opensplice::topic::CMDataWriterBuiltinTopicDataImpl>
CMDataWriterBuiltinTopicData;

typedef dds::topic::TCMDataReaderBuiltinTopicData<org::opensplice::topic::CMDataReaderBuiltinTopicDataImpl>
CMDataReaderBuiltinTopicData;

}
}
}


// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_BUILTINTOPIC_HPP_ */
