#ifndef OMG_DDS_TOPIC_DISCOVER_HPP_
#define OMG_DDS_TOPIC_DISCOVER_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
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



namespace dds { namespace topic {

  /**
   *  Allows one to discover a Topic<T>, AnyTopic, ContentFilteredTopic<T>, etc. by name
   *  This operation usually results in network look-ups
   *  @see dds::topic::find()
   */
  template <typename TOPIC>
  TOPIC discover(const dds::domain::DomainParticipant& dp,
      const std::string& name,
      const dds::core::Duration& timeout = dds::core::Duration::infinite());

  template <typename ANYTOPIC, typename FwdIterator>
  uint32_t discover(const dds::domain::DomainParticipant& dp, FwdIterator begin, uint32_t max_size);

  template <typename ANYTOPIC, typename BinIterator>
  uint32_t discover(const dds::domain::DomainParticipant& dp, BinIterator begin);

  void ignore(const dds::domain::DomainParticipant& dp, const dds::core::InstanceHandle& handle);

  template <typename FwdIterator>
  void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end);

} }

#endif /* OMG_DDS_TOPIC_DISCOVER_HPP_ */
