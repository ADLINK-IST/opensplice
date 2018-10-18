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


#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_HPP_
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_HPP_


#include <org/opensplice/topic/TBuiltinTopic.hpp>


namespace org
{
namespace opensplice
{
namespace topic
{

typedef TCMParticipantBuiltinTopicData<CMParticipantBuiltinTopicDataDelegate>
CMParticipantBuiltinTopicData;

typedef TCMPublisherBuiltinTopicData<CMPublisherBuiltinTopicDataDelegate>
CMPublisherBuiltinTopicData;

typedef TCMSubscriberBuiltinTopicData<CMSubscriberBuiltinTopicDataDelegate>
CMSubscriberBuiltinTopicData;

typedef TCMDataWriterBuiltinTopicData<CMDataWriterBuiltinTopicDataDelegate>
CMDataWriterBuiltinTopicData;

typedef TCMDataReaderBuiltinTopicData<CMDataReaderBuiltinTopicDataDelegate>
CMDataReaderBuiltinTopicData;

typedef TTypeBuiltinTopicData<TypeBuiltinTopicDataDelegate>
TypeBuiltinTopicData;

}
}
}


#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_HPP_ */
