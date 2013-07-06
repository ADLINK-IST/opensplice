#ifndef OMG_TDDS_CORE_QOS_ENTITY_QOS_HPP_
#define OMG_TDDS_CORE_QOS_ENTITY_QOS_HPP_

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

#include <dds/core/Value.hpp>


namespace dds {
  namespace core {
    template <typename DELEGATE>
    class TEntityQos;
  }
}

/* Acts as a container for Qos policies allowing to set and retrieve all
 * the policies of an entity as a unit.
 */
template <typename DELEGATE>
class dds::core::TEntityQos : public dds::core::Value<DELEGATE> {
public:
  TEntityQos();

  TEntityQos(const TEntityQos& other);

  template <typename T>
  TEntityQos(const TEntityQos<T>& qos);

public:
  ~TEntityQos();

public:
  /**
   * Generic method for setting a policy applicable to this QoS object.
   * Available policies depends on the actual instantiation of the template
   * class, which might be DomainParticipantQos, TopicQos, PublisherQos, etc.
   *
   * @param p the policy to be set for this QoS instance.
   */
  template <typename POLICY>
  TEntityQos& policy(const POLICY& p);

  /**
   * Generic method for obtaining the value set for a specific policy
   * belonging to this QoS instance.
   *
   * @return
   */
  template <typename POLICY>
  const POLICY& policy() const;

  /**
   * Generic method for obtaining the value set for a specific policy
   * belonging to this QoS instance.
   *
   * @return
   */
  template <typename POLICY>
  POLICY& policy();

  template <typename POLICY>
  TEntityQos& operator << (const POLICY& p);

  template <typename POLICY>
  const TEntityQos& operator >> (POLICY& p) const;

  template <typename T>
  TEntityQos<DELEGATE>& operator = (const TEntityQos<T>& other);

};
/* namespace tdds / namespace core / namespace qos */

#endif /* OMG_TDDS_CORE_QOS_ENTITY_QOS_HPP_ */
