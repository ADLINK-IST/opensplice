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


#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_COPY_HPP_
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_COPY_HPP_

#include "dds/core/macros.hpp"

namespace org
{
namespace opensplice
{
namespace topic
{

OMG_DDS_API u_bool
__ParticipantBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::ParticipantBuiltinTopicData *from,
    struct v_participantInfo *to);

OMG_DDS_API u_bool
__TopicBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::TopicBuiltinTopicData *from,
    struct v_topicInfo *to);

OMG_DDS_API u_bool
__PublicationBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::PublicationBuiltinTopicData *from,
    v_publicationInfo *to);

OMG_DDS_API u_bool
__SubscriptionBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::SubscriptionBuiltinTopicData *from,
    struct v_subscriptionInfo *to);

OMG_DDS_API u_bool
__CMParticipantBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMParticipantBuiltinTopicData *from,
    struct v_participantCMInfo *to);

OMG_DDS_API u_bool
__CMPublisherBuiltinTopicData__copyIn(
    c_base base,
    const  org::opensplice::topic::CMPublisherBuiltinTopicData *from,
    struct v_publisherCMInfo  *to);

OMG_DDS_API u_bool
__CMSubscriberBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMSubscriberBuiltinTopicData *from,
    struct v_subscriberCMInfo *to);

OMG_DDS_API u_bool
__CMDataWriterBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMDataWriterBuiltinTopicData *from,
    struct v_dataWriterCMInfo *to);

OMG_DDS_API u_bool
__CMDataReaderBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMDataReaderBuiltinTopicData *from,
    struct v_dataReaderCMInfo *to);

OMG_DDS_API u_bool
__TypeBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::TypeBuiltinTopicData *from,
    struct v_typeInfo *to);

OMG_DDS_API void
__ParticipantBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__TopicBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__PublicationBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__SubscriptionBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__CMParticipantBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__CMPublisherBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__CMSubscriberBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__CMDataWriterBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__CMDataReaderBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);

OMG_DDS_API void
__TypeBuiltinTopicData__copyOut(
    const void *_from,
    void *_to);
}
}
}


#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_COPY_HPP_ */
