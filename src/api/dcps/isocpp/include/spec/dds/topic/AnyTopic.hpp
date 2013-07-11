#ifndef OMG_DDS_TOPIC_ANY_TOPIC_HPP_
#define OMG_DDS_TOPIC_ANY_TOPIC_HPP_

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
#include <dds/topic/detail/AnyTopic.hpp>
#include <dds/topic/TopicDescription.hpp>

namespace dds {
  namespace topic {
    class AnyTopic;

    /**
     * Extracts a typed <code>Topic</code> from an
     * <code>AnyTopic</code>.
     */
    template <typename T>
    Topic<T> get(const AnyTopic& at);
  }
}
class OMG_DDS_API dds::topic::AnyTopic {
public:
  template <typename T>
  AnyTopic(const dds::topic::Topic<T>& t);


public:
  const dds::domain::DomainParticipant& domain_participant() const;

  const dds::core::status::InconsistentTopicStatus& inconsistent_topic_status();

  const dds::topic::qos::TopicQos& qos() const;

  void qos(const dds::topic::qos::TopicQos& q);

public:
  template <typename T>
  Topic<T> get();
public:
  inline const detail::THolderBase* operator->() const;

  detail::THolderBase* operator->();

private:
  dds::core::smart_ptr_traits<detail::THolderBase>::ref_type holder_;
};


#endif /* OMG_DDS_TOPIC_ANY_TOPIC_HPP_ */
