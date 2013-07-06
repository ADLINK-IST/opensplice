#ifndef OMG_TDDS_CORE_ENTITY_HPP_
#define OMG_TDDS_CORE_ENTITY_HPP_

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

#include <string>

#include <dds/core/Reference.hpp>
#include <dds/core/status/Status.hpp>

namespace dds { namespace core {
  template <typename DELEGATE>
  class TEntity;
} }

/**
 * This class is the abstract base class for all the DCPS objects that
 * support QoS policies, a listener and a status condition. In the
 * ISO C++ PSM each DDS entity behaves like a polymorphic reference
 * in what it automatically manages its resource and it can be
 * safely assigned up and down the DDS Entity type hierarchy.
 */
template <typename DELEGATE>
class dds::core::TEntity : public dds::core::Reference<DELEGATE> {
public:
  OMG_DDS_REF_TYPE(TEntity, dds::core::Reference, DELEGATE)

  ~TEntity();

public:
  /**
   * This operation enables the Entity. Entity objects can be created
   * either enabled or disabled. This is controlled by the value of
   * the ENTITY_FACTORY Qos policy (Section 7.1.3.20, ENTITY_FACTORY)
   * on the corresponding factory for the Entity.
   * The default setting of ENTITY_FACTORY is such that, by default,
   * it is not necessary to explicitly call enable on newly created
   * entities (see Section 7.1.3.20, ENTITY_FACTORY).
   * The enable operation is idempotent. Calling enable on an already
   * enabled Entity does nor raise exceptions and has no effect.
   *
   * Entities created from a factory that is disabled, are created
   * disabled regardless of the setting of the ENTITY_FACTORY
   * Qos policy. Calling enable on an Entity whose factory is not
   * enabled will fail and return PRECONDITION_NOT_MET.
   * If the ENTITY_FACTORY Qos policy has autoenable_created_entities
   * set to TRUE, the enable operation on the factory will automatically
   * enable all entities created from the factory.
   * The Listeners associated with an entity are not called until
   * the entity is enabled. Conditions associated with an entity that
   * is not enabled are inactive, that is, have a trigger_value==FALSE
     (see Section 7.1.4.4, “Conditions and Wait-sets, on page 131).
   */
  void enable();

  /**
   * This operation retrieves the list of communication statuses in the Entity
   * that are triggered.That is, the list of statuses whose value has
   * changed since the last time the application read the status.
   * The precise definition of the triggered state of communication statuses
   * is given in Section 7.1.4.2, Changes in Status, on page 126.
   * When the entity is first created or if the entity is not enabled,
   * all communication statuses are in the untriggered state so the list
   * returned by the status_changes operation will be empty.
   * The list of statuses returned by the status_changes operation refers
   * to the statuses that are triggered on the Entity itself and does not
   * include statuses that apply to contained entities.
   *
   * @return the status changes
   */
  const dds::core::status::StatusMask status_changes();

  /**
   * This operation returns the InstanceHandle_t that
   * represents the Entity.
   */
  const dds::core::InstanceHandle instance_handle() const;


  /**
   * This method closes the entity and releases all resources associated with
   * DDS, such as threads, sockets, buffers, etc. Any attempt to invoke
   * methods on a closed entity will raise an exception.
   */
  void close();

  /**
   * Indicates that references to this object may go out of scope but that
   * the application expects to look it up again later. Therefore, the
   * Service must consider this object to be still in use and may not
   * close it automatically.
   */
  void retain();
};



#endif /* OMG_TDDS_CORE_ENTITY_HPP_ */
