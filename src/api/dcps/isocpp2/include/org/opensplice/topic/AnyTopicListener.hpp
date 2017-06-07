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


#ifndef ORG_OPENSPLICE_TOPIC_ANY_TOPIC_LISTENER_HPP_
#define ORG_OPENSPLICE_TOPIC_ANY_TOPIC_LISTENER_HPP_


#include <dds/topic/AnyTopicListener.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{

class OMG_DDS_API AnyTopicListener : public virtual dds::topic::AnyTopicListener
{
public:
    virtual ~AnyTopicListener() { }

public:
    virtual void on_all_data_disposed(
        dds::topic::AnyTopic& topic,
        const org::opensplice::core::status::AllDataDisposedTopicStatus& status) = 0;
};


class OMG_DDS_API NoOpAnyTopicListener : public virtual AnyTopicListener
{
public:
    virtual ~NoOpAnyTopicListener() { }

public:
    virtual void on_all_data_disposed(
        dds::topic::AnyTopic&,
        const org::opensplice::core::status::AllDataDisposedTopicStatus&) { }
};

}
}
}

#endif /* ORG_OPENSPLICE_TOPIC_ANY_TOPIC_LISTENER_HPP_ */
