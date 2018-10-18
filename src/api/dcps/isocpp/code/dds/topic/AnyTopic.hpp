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

#ifndef OSPL_DDS_TOPIC_ANYTOPIC_CPP_
#define OSPL_DDS_TOPIC_ANYTOPIC_CPP_

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/AnyTopic.hpp>

// Implementation
namespace dds
{
namespace topic
{

const dds::domain::DomainParticipant& AnyTopic::domain_participant() const
{
    return holder_->domain_participant();
}

const dds::core::status::InconsistentTopicStatus& AnyTopic::inconsistent_topic_status()
{
    return holder_->inconsistent_topic_status();
}

const dds::topic::qos::TopicQos& AnyTopic::qos() const
{
    return holder_->qos();
}

void AnyTopic::qos(const dds::topic::qos::TopicQos& q)
{
    holder_->qos(q);
}
detail::THolderBase* AnyTopic::operator->()
{
    return holder_.get();
}
}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_ANYTOPIC_CPP_ */
