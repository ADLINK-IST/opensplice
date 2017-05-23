#ifndef ORG_OPENSPLICE_STREAMS_TOPIC_TOPICTRAITS_HPP_
#define ORG_OPENSPLICE_STREAMS_TOPIC_TOPICTRAITS_HPP_
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

namespace org
{
namespace opensplice
{
namespace streams
{
namespace topic
{

template <typename Topic>
struct stream_topic { };

}
}
}
}

/**
 * @internal Maps Topic type struct specializations through to
 * their corresponding stream topics
 * @param TOPIC The CCPP topic type.
 */
#define REGISTER_STREAM_TOPIC_TRAITS(TOPIC)                    \
    namespace org { namespace opensplice { namespace streams { namespace topic {        \
    template<> struct stream_topic<TOPIC> {                \
        typedef TOPIC##StreamSample type; \
    };                                    \
    } } } }

#endif /* ORG_OPENSPLICE_STREAMS_TOPIC_TOPICTRAITS_HPP_ */
