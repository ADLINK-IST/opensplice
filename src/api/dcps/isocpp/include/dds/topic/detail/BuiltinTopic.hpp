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

}
}
}


// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_BUILTINTOPIC_HPP_ */
