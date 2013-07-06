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

namespace dds { namespace topic {

  class AnyTopicDescription {
  public:
    template <typename T>
    inline AnyTopicDescription(const dds::topic::TopicDescription<T>& t);

  public:
    const dds::domain::DomainParticipant& domain_participant() const;

    const std::string& name() const;

    const std::string& type_name() const;

  protected:
    inline AnyTopicDescription(detail::TDHolderBase* holder);

  public:
    inline AnyTopicDescription& swap(AnyTopicDescription& rhs);

    template <typename T>
    AnyTopicDescription& operator =(const dds::topic::Topic<T>& rhs);

    inline AnyTopicDescription& operator =(AnyTopicDescription rhs);

  public:
    template <typename T>
    const dds::topic::TopicDescription<T>& get();

  public:
    const detail::TDHolderBase* operator->() const;

    detail::TDHolderBase* operator->();

  protected:
    dds::core::smart_ptr_traits<detail::TDHolderBase>::ref_type holder_;
  };

}}

#endif /* OMG_DDS_TOPIC_ANY_TOPIC_DESCRIPTION_HPP_ */
