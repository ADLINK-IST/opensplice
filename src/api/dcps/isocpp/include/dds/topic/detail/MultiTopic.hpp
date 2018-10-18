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
#ifndef OSPL_DDS_TOPIC_DETAIL_MULTITOPIC_HPP_
#define OSPL_DDS_TOPIC_DETAIL_MULTITOPIC_HPP_

/**
 * @file
 */

// Implementation

#include <string>
#include <vector>

#include <dds/core/types.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/detail/TopicDescription.hpp>
#include <dds/sub/Query.hpp>

namespace dds
{
namespace topic
{
namespace detail
{

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT

class MultiTopic  : public dds::topic::detail::TopicDescription<T>
{
public:
    MultiTopic(const dds::domain::DomainParticipant& dp,
               const std::string& name,
               const dds::core::Query& query)
        : dds::topic::detail::TopicDescription<T>(dp, name, topic_type_name<T>::value()),
          query_(query) { }

    virtual ~MultiTopic() { }

public:
    const dds::core::Query& query()
    {
        return query_;
    }

    void expression_parameters(const dds::core::StringSeq& params)
    {
        query_.parameters(params.begin(), params.end());
    }

private:
    std::string              subscription_expression_;
    std::vector<std::string> params_;
    dds::core::Query query_;
};

#endif  // OMG_DDS_MULTI_TOPIC_SUPPORT

}
}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_MULTITOPIC_HPP_ */
