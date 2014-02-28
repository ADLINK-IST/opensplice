#ifndef OMG_DDS_TOPIC_ANY_TOPIC_DESCRIPTION_HPP_
#define OMG_DDS_TOPIC_ANY_TOPIC_DESCRIPTION_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
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

#include <dds/core/Exception.hpp>
#include <dds/core/ref_traits.hpp>
#include <dds/topic/detail/AnyTopicDescription.hpp>
#include <dds/topic/Topic.hpp>

namespace dds
{
namespace topic
{

class AnyTopicDescription
{
public:
    /**
     * Construct AnyTopicDescription from a TopicDescription.
     */
    template <typename T>
    inline AnyTopicDescription(const dds::topic::TopicDescription<T>& t);

public:

    /**
     * Get the DomainParticipant for the AnyTopicDescription.
     *
     * @return the DomainParticipant for the AnyTopicDescription
     */
    const dds::domain::DomainParticipant& domain_participant() const;

    /**
     * Get the Topic name for the AnyTopicDescription.
     *
     * @return the Topic name
     */
    const std::string& name() const;

    /**
     * Get the type name for the AnyTopicDescription.
     *
     * @return the type name
     */
    const std::string& type_name() const;

protected:
    inline AnyTopicDescription(detail::TDHolderBase* holder);

public:
    /**
     * Swap two AnyTopicDescription.
     */
    inline AnyTopicDescription& swap(AnyTopicDescription& rhs);

    /**
     * Assign a typed Topic to a AnyTopicDescription.
     */
    template <typename T>
    AnyTopicDescription& operator =(const dds::topic::Topic<T>& rhs);

    /**
     * Assign AnyTopicDescription to another AnyTopicDescription.
     */
    inline AnyTopicDescription& operator =(AnyTopicDescription rhs);

public:
    /**
     * Get a typed TopicDescription from an
     * AnyTopicDescription.
     *
     * @return the typed DataReader
     */
    template <typename T>
    const dds::topic::TopicDescription<T>& get();

public:
    /**
     * Operator overload to get.
     */
    const detail::TDHolderBase* operator->() const;

    /**
     * Operator overload to get.
     */
    detail::TDHolderBase* operator->();

protected:
    dds::core::smart_ptr_traits<detail::TDHolderBase>::ref_type holder_;
};

}
}

#endif /* OMG_DDS_TOPIC_ANY_TOPIC_DESCRIPTION_HPP_ */
