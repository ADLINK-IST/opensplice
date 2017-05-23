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

package org.omg.dds.topic;

import org.omg.dds.core.EntityQos;
import org.omg.dds.core.policy.DataRepresentation;
import org.omg.dds.core.policy.Deadline;
import org.omg.dds.core.policy.DestinationOrder;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.DurabilityService;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.LatencyBudget;
import org.omg.dds.core.policy.Lifespan;
import org.omg.dds.core.policy.Liveliness;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.policy.TopicData;
import org.omg.dds.core.policy.TransportPriority;
import org.omg.dds.core.policy.TypeConsistencyEnforcement;


public interface TopicQos extends EntityQos<QosPolicy.ForTopic>
{
    /**
     * @return the topicData QosPolicy
     */
    public TopicData getTopicData();

    /**
     * @return the durability QosPolicy
     */
    public Durability getDurability();

    /**
     * @return the durabilityService QosPolicy
     */
    public DurabilityService getDurabilityService();

    /**
     * @return the deadline QosPolicy
     */
    public Deadline getDeadline();

    /**
     * @return the latencyBudget QosPolicy
     */
    public LatencyBudget getLatencyBudget();

    /**
     * @return the liveliness QosPolicy
     */
    public Liveliness getLiveliness();

    /**
     * @return the reliability QosPolicy
     */
    public Reliability getReliability();

    /**
     * @return the destinationOrder QosPolicy
     */
    public DestinationOrder getDestinationOrder();

    /**
     * @return the history QosPolicy
     */
    public History getHistory();

    /**
     * @return the resourceLimits QosPolicy
     */
    public ResourceLimits getResourceLimits();

    /**
     * @return the transportPriority QosPolicy
     */
    public TransportPriority getTransportPriority();

    /**
     * @return the lifespan QosPolicy
     */
    public Lifespan getLifespan();

    /**
     * @return the ownership QosPolicy
     */
    public Ownership getOwnership();

    public DataRepresentation getRepresentation();

    public TypeConsistencyEnforcement getTypeConsistency();


    // --- Modification: -----------------------------------------------------

    @Override
    public TopicQos withPolicy(QosPolicy.ForTopic policy);

    @Override
    public TopicQos withPolicies(QosPolicy.ForTopic... policy);
}
