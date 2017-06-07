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

#ifndef OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/AnyTopicDescription.hpp>

// Implementation
namespace dds
{
namespace topic
{

const dds::domain::DomainParticipant& AnyTopicDescription::domain_participant() const
{
    return holder_->domain_participant();
}

const std::string& AnyTopicDescription::name() const
{
    return holder_->name();
}

const std::string& AnyTopicDescription::type_name() const
{
    return holder_->type_name();
}

const detail::TDHolderBase* AnyTopicDescription::operator->() const
{
    return holder_.get();
}

detail::TDHolderBase* AnyTopicDescription::operator->()
{
    return holder_.get();
}

}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_CPP_ */
