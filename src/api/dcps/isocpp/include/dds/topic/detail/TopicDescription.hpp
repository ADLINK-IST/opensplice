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
#ifndef OSPL_DDS_TOPIC_DETAIL_TOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_DETAIL_TOPICDESCRIPTION_HPP_

/**
 * @file
 */

// Implementation

#include <string>

#include <org/opensplice/core/EntityDelegate.hpp>
#include <dds/domain/DomainParticipant.hpp>

namespace dds
{
namespace topic
{
namespace detail
{
template <typename T>
class TopicDescription;
}
}
}

template <typename T>
class dds::topic::detail::TopicDescription : public org::opensplice::core::EntityDelegate
{


public:

    TopicDescription(const dds::domain::DomainParticipant& dp,
                     const std::string& name,
                     const std::string& type_name)
        : dp_(dp),
          name_(name),
          type_name_(type_name)
    { }

public:

    /**
     *  @internal Get the name used to create the TopicDescription.
     */
    const std::string& name() const
    {
        return name_;
    }

    /**
     *  @internal The type_name used to create the TopicDescription.
     */
    const std::string& type_name() const
    {
        return type_name_;
    }

    /**
     *  @internal This operation returns the DomainParticipant to which the
     * TopicDescription belongs.
     */
    const dds::domain::DomainParticipant& domain_participant() const
    {
        return dp_;
    }

protected:
    dds::domain::DomainParticipant dp_;
    std::string name_;
    std::string type_name_;
};

// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_TOPICDESCRIPTION_HPP_ */
