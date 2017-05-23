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
#ifndef OSPL_DDS_TOPIC_DISCOVERY_HPP_
#define OSPL_DDS_TOPIC_DISCOVERY_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/discovery.hpp>
#include <org/opensplice/core/exception_helper.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename TOPIC>
TOPIC discover(const dds::domain::DomainParticipant& dp,
               const std::string& name,
               const dds::core::Duration& timeout)
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    return dds::core::null;
}

template <typename TOPICDATA, typename FwdIterator>
uint32_t discover(const dds::domain::DomainParticipant& dp, FwdIterator begin, uint32_t max_size)
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    return 0;
}

template <typename TOPICDATA, typename BinIterator>
uint32_t discover(const dds::domain::DomainParticipant& dp, BinIterator begin)
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    return 0;
}

template <typename FwdIterator>
void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end)
{
    for(FwdIterator i = begin; i < end; i++)
    {
        DDS::ReturnCode_t result = ((dds::domain::DomainParticipant)dp)->dp_->ignore_topic(i->handle());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::ignore_topic"));
    }
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_DISCOVERY_HPP_ */
