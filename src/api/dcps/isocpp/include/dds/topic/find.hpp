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
#ifndef OSPL_DDS_TOPIC_FIND_HPP_
#define OSPL_DDS_TOPIC_FIND_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/find.hpp>
#include <dds/topic/detail/find.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename TOPIC>
TOPIC find(const dds::domain::DomainParticipant& dp, const std::string& topic_name)
{
    return ::dds::topic::detail::find<TOPIC>(dp, topic_name);
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_FIND_HPP_ */
