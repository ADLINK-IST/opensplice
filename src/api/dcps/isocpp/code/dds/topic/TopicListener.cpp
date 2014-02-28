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

#ifndef OSPL_DDS_TOPIC_TOPICLISTENER_CPP_
#define OSPL_DDS_TOPIC_TOPICLISTENER_CPP_

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TopicListener.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename T>
NoOpTopicListener<T>::~NoOpTopicListener() { }

template <typename T>
void NoOpTopicListener<T>::on_inconsistent_topic(
    Topic<T>& topic,
    const dds::core::status::InconsistentTopicStatus& status) { }

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TOPICLISTENER_CPP_ */
