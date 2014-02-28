#ifndef OMG_DDS_TOPIC_TOPIC_DESCRIPTION_HPP_
#define OMG_DDS_TOPIC_TOPIC_DESCRIPTION_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
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

#include <dds/topic/detail/TopicDescription.hpp>

namespace dds
{
namespace topic
{
template < typename T,
           template <typename Q> class DELEGATE = dds::topic::detail::TopicDescription >
class TopicDescription;
}
}

/**
 * @todo RTF Issue - moved include
 * @note This include was moved here as MSVC appears to ignore any
 * attempt to 're-declare' a template class with a default argument like:
 * dds/topic/detail/AnyTopicDescription.hpp(48): error C2976: 'dds::topic::TopicDescription' : too few template arguments
 * dds/topic/TTopicDescription.hpp(41) : see declaration of 'dds::topic::TopicDescription'
 */
#include <dds/topic/TTopicDescription.hpp>

#endif /* OMG_DDS_TOPIC_TOPIC_DESCRIPTION_HPP_ */
