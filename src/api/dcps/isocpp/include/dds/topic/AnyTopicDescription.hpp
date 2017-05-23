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
#ifndef OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/AnyTopicDescription.hpp>

// Implementation
namespace dds
{
namespace topic
{

template <typename T>
inline AnyTopicDescription::AnyTopicDescription(const dds::topic::TopicDescription<T>& t)
    : holder_(new detail::TDHolder<T>(t)) { }


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

inline AnyTopicDescription::AnyTopicDescription(detail::TDHolderBase* holder)
    : holder_(holder) { }

inline typename AnyTopicDescription::AnyTopicDescription& AnyTopicDescription::swap(AnyTopicDescription& rhs)
{
    holder_.swap(rhs.holder_);
    return *this;
}

template <typename T>
AnyTopicDescription& AnyTopicDescription::operator =(const dds::topic::Topic<T>& rhs)
{
    holder_.reset(new detail::TDHolder<T>(rhs));
    return *this;
}

inline AnyTopicDescription& AnyTopicDescription::operator =(AnyTopicDescription rhs)
{
    return this->swap(rhs);
}

template <typename T>
const dds::topic::TopicDescription<T>& AnyTopicDescription::get()
{
    OMG_DDS_STATIC_ASSERT(::dds::topic::is_topic_type<T>::value == 1);
    detail::TDHolder<T>* h = dynamic_cast<detail::TDHolder<T>* >(holder_.get());
    if(h == 0)
    {
        throw dds::core::InvalidDowncastError("invalid type");
    }
    return h->get();
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

#endif /* OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_ */
