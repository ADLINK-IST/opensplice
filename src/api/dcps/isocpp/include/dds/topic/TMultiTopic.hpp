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
#ifndef OSPL_DDS_TOPIC_TMULTITOPIC_HPP_
#define OSPL_DDS_TOPIC_TMULTITOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TMultiTopic.hpp>

// Implementation

namespace dds
{
namespace topic
{

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT

template <typename T, template <typename Q> class DELEGATE>
template <typename FWDIterator>
MultiTopic<T, DELEGATE>::MultiTopic(const dds::domain::DomainParticipant& dp,
                                    const std::string& name,
                                    const std::string expression,
                                    const FWDIterator& params_begin,
                                    const FWDIterator& params_end)
    : dds::topic::TopicDescription<T, DELEGATE>(
        new DELEGATE<T>(dp, name, expression, params_begin, params_end))
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
}

template <typename T, template <typename Q> class DELEGATE>
MultiTopic<T, DELEGATE>::~MultiTopic() { }

/** @internal @todo no implementation. Known Issue - no OMG_DDS_MULTI_TOPIC_SUPPORT */
template <typename T, template <typename Q> class DELEGATE>
const std::string MultiTopic<T, DELEGATE>::expression() const
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
    return NULL;
}

/** @internal @todo no implementation. Known Issue - no OMG_DDS_MULTI_TOPIC_SUPPORT */
template <typename T, template <typename Q> class DELEGATE>
void MultiTopic<T, DELEGATE>::expression_parameters(const FWDIterator& params_begin, const FWDIterator& params_end)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
}

/** @internal @todo no implementation. Known Issue - no OMG_DDS_MULTI_TOPIC_SUPPORT */
template <typename T, template <typename Q> class DELEGATE>
dds::core::StringSeq void MultiTopic<T, DELEGATE>::expression_parameters() const
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
}


#endif  // OMG_DDS_MULTI_TOPIC_SUPPORT

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TMULTITOPIC_HPP_ */
