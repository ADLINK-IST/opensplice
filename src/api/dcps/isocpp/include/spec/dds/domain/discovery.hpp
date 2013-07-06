#ifndef OMG_DDS_DOMAIN_DISCOVERY_HPP_
#define OMG_DDS_DOMAIN_DISCOVERY_HPP_

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


#include <dds/sub/Subscriber.hpp>


namespace dds { namespace domain {

  /**
   * This function allows one to express the will to ignore the entity
   * represented by the given <code>InstanceHandle</code> for the specific
   * <code>DomainParticipant</code>.
   *
   * @param dp      the <code>DomainParticipant</code> for which the remote
   *                entity will be ignored
   *
   * @param handle  the <code>InstanceHandle</code> of the remote entity that
   *                has to be ignored
   *
   */
  void ignore(const dds::domain::DomainParticipant& dp, const dds::core::InstanceHandle& handle);

  /**
   * This function allows one to express the will to ignore a series of entities
   * whose instance handles are made available via the provided iterators.
   *
   * @param dp      the <code>DomainParticipant</code> for which the remote
   *                entity will be ignored
   *
   * @param begin   the begin iterator for the <code>InstanceHandle</code>
   *                to ignore
   *
   * @param end     the end iterator for the <code>InstanceHandle</code>
   *                to ignore
   *
   */
  template <typename FwdIterator>
  void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end);


} }
#endif /* OMG_DDS_DOMAIN_DISCOVERY_HPP_ */
