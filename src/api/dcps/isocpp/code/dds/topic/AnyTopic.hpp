/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
