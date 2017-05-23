#ifndef OMG_DDS_TOPIC_ANY_TOPIC_LISTENER_HPP_
#define OMG_DDS_TOPIC_ANY_TOPIC_LISTENER_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/topic/AnyTopic.hpp>

namespace dds
{
namespace topic
{

class OMG_DDS_API AnyTopicListener
{
public:
    virtual ~AnyTopicListener();

public:
    virtual void on_inconsistent_topic(
        AnyTopic& topic,
        const dds::core::status::InconsistentTopicStatus& status) = 0;
};


class OMG_DDS_API NoOpAnyTopicListener : public virtual AnyTopicListener
{
public:
    virtual ~NoOpAnyTopicListener();

public:
    virtual void on_inconsistent_topic(
        AnyTopic& topic,
        const dds::core::status::InconsistentTopicStatus& status);
};

}
}

#endif /* OMG_DDS_TOPIC_ANY_TOPIC_LISTENER_HPP_ */
