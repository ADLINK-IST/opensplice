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

package org.omg.dds.core;

import java.util.Collection;
import java.util.Set;

import org.omg.dds.core.status.Status;


/**
 * A StatusCondition object is an immutable object that specifies Condition
 * that is associated with each {@link org.omg.dds.core.Entity}.
 * The triggerValue of the StatusCondition depends on the communication status
 * of that entity (e.g., arrival of data, loss of information, etc.),
 * "filtered" by the set of enabledStatuses on the StatusCondition.
 * <p>
 * The triggerValue of a StatusCondition is the Boolean OR of the
 * ChangedStatusFlag of all the communication statuses to which it is
 * sensitive. That is, triggerValue == false only if all the values of the
 * ChangedStatusFlags are false.
 * <p>
 * The sensitivity of the StatusCondition to a particular communication
 * status is controlled by the list of enabledStatuses set on the condition
 * by means of {@link #setEnabledStatuses(Collection)}.
 * <p>
 * Valid statuses are:
 * <ul>
 * <li>{@link org.omg.dds.core.status.DataAvailableStatus}</li>
 * <li>{@link org.omg.dds.core.status.DataOnReadersStatus}</li>
 * <li>{@link org.omg.dds.core.status.InconsistentTopicStatus}</li>
 * <li>{@link org.omg.dds.core.status.LivelinessChangedStatus}</li>
 * <li>{@link org.omg.dds.core.status.LivelinessLostStatus}</li>
 * <li>{@link org.omg.dds.core.status.OfferedDeadlineMissedStatus}</li>
 * <li>{@link org.omg.dds.core.status.OfferedIncompatibleQosStatus}</li>
 * <li>{@link org.omg.dds.core.status.PublicationMatchedStatus}</li>
 * <li>{@link org.omg.dds.core.status.RequestedDeadlineMissedStatus}</li>
 * <li>{@link org.omg.dds.core.status.RequestedIncompatibleQosStatus}</li>
 * <li>{@link org.omg.dds.core.status.SampleLostStatus}</li>
 * <li>{@link org.omg.dds.core.status.SampleRejectedStatus}</li>
 * <li>{@link org.omg.dds.core.status.SubscriptionMatchedStatus}</li>
 * </ul>
 *
 * @param <ENTITY>      The type of the entity with which this condition is
 *                      associated.
 */
public interface StatusCondition<ENTITY extends Entity<?, ?>>
extends Condition {
    /**
     * This operation retrieves the list of communication statuses that are
     * taken into account to determine the triggerValue of the
     * StatusCondition. This operation returns the statuses that were
     * explicitly set on the last call to
     * {@link #setEnabledStatuses(Collection)} or, if it was never called,
     * the default list.
     *
     * @return  a new Set of enabled statuses.
     *
     * @see     #setEnabledStatuses(Collection)
     */
    public Set<Class<? extends Status>> getEnabledStatuses();

    /**
     * This operation accepts a variable number of statuses
     * that are encapsulated in a new StatusCondition object
     * <p>
     * If this function is not invoked, the default list of enabled statuses
     * includes all the statuses.
     *
     * @param statuses  For which status changes the condition should trigger.
     *                  An empty parameter list signifies all status changes.
     *
     * @see     #setEnabledStatuses(Collection)
     */

    public void setEnabledStatuses(
    		Class<? extends Status>... statuses);

    /**
     * This operation accepts a collection of statuses
     * that are encapsulated in a new StatusCondition object
     * <p>
     * If this function is not invoked, the default list of enabled statuses
     * includes all the statuses.
     *
     * @param statuses  For which status changes the condition should trigger.
     *                  A null collection signifies all status changes.
     *
     * @see     #setEnabledStatuses(Collection)
     */
        public void setEnabledStatuses(
                Collection<Class<? extends Status>> statuses);

    /**
     * @return  the Entity associated with the StatusCondition. Note that
     *          there is exactly one Entity associated with each
     *          StatusCondition.
     */
    public ENTITY getParent();
}
