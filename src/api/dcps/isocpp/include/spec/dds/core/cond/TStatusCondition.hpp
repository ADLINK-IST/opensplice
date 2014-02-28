#ifndef OMG_DDS_CORE_T_STATUS_CONDITION_HPP_
#define OMG_DDS_CORE_T_STATUS_CONDITION_HPP_

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

#include <dds/core/status/State.hpp>
#include <dds/core/cond/TCondition.hpp>
#include <dds/core/cond/detail/StatusCondition.hpp>
#include <dds/core/Entity.hpp>

namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE>
class TStatusCondition;
}
}
}

/**
 * A StatusCondition object is a specific Condition that is
 * associated with each Entity.
 * The trigger_value of the StatusCondition depends on the communication
 * status of that entity (e.g., arrival of data, loss of information, etc.),
 * filtered by the set of enabled_statuses on the StatusCondition.
 * The enabled_statuses and its relation to Listener and WaitSet is detailed
 * in Trigger State of the StatusCondition.
 */
template <typename DELEGATE>
class dds::core::cond::TStatusCondition : public dds::core::cond::TCondition<DELEGATE>
{
public:
    TStatusCondition(const dds::core::null_type&);

    /**
     * Create a StatusCondition (Section 7.1.2.1.9,
     * StatusCondition Class) associated with an Entity. The
     * Condition can then be added to a WaitSet (Section 7.1.2.1.6,
     * WaitSet Class) so that the application can wait for specific
     * status changes that affect the Entity.
     *
     * @param e The Entity to associate with the StatusCondition.
     */
    TStatusCondition(const dds::core::Entity& e);

    /**
     * Create a StatusCondition (Section 7.1.2.1.9,
     * StatusCondition Class) associated with an Entity. The
     * Condition can then be added to a WaitSet (Section 7.1.2.1.6,
     * WaitSet Class) so that the application can wait for specific
     * status changes that affect the Entity. The supplied functor
     * will be called if the StatusCondition is triggered when dispatch
     * is called on a WaitSet the StatusCondition is attached to.
     *
     * @param e The Entity to associate with the StatusCondition.
     * @param functor The functor to be called when the StatusCondition triggers.
     */
    template <typename FUN>
    TStatusCondition(const dds::core::Entity& e, const FUN& functor);

public:
    /**
     * This operation defines the list of communication statuses that
     * are taken into account to determine the trigger_value of the
     * StatusCondition. This operation may change the trigger_value of
     * the StatusCondition.
     * WaitSet objects' behavior depends on the changes of the trigger_value
     * of their attached Conditions. Therefore, any WaitSet to which the
     * StatusCondition is attached is potentially affected by this operation.
     * If this function is not invoked, the default list of enabled statuses
     * includes all the statuses.
     *
     * @param status the enabled statuses
     */
    void
    enabled_statuses(const ::dds::core::status::StatusMask& status) const;

    /**
     * This operation retrieves the list of communication statuses that are
     * taken into account to determine the trigger_value of the
     * StatusCondition. This operation returns the statuses that were
     * explicitly set on the last call to set enabled_statuses, or, if
     * set enabled_statuses was never called, the default list
     * (see Section 7.1.2.1.9.1).
     *
     * @return The list of communication statuses that are
     * taken into account to determine the trigger_value of the
     * StatusCondition.
     */
    const ::dds::core::status::StatusMask enabled_statuses() const;

    /**
     * This operation returns the Entity assocated with the StatusCondition
     *
     * @return The Entity associated with the StatusCondition.
     */
    const dds::core::Entity& entity() const;
};

#endif  /* OMG_DDS_CORE_T_STATUS_CONDITION_HPP_ */
