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

import java.io.Serializable;
import java.util.List;

import org.omg.dds.core.DDSObject;
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
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.policy.TopicData;
import org.omg.dds.core.policy.TransportPriority;
import org.omg.dds.core.policy.TypeConsistencyEnforcement;
import org.omg.dds.type.Extensibility;
import org.omg.dds.type.ID;
import org.omg.dds.type.Key;
import org.omg.dds.type.Optional;
import org.omg.dds.type.typeobject.TypeObject;

/**
 * The DCPSTopic topic communicates the existence of topics by means of the
 * TopicBuiltinTopicData datatype. Each TopicBuiltinTopicData sample in
 * a Domain represents a Topic in that Domain: a new TopicBuiltinTopicData
 * instance is created when a newly-added Topic is enabled. However, the instance is
 * not disposed when a Topic is deleted by its participant because a topic lifecycle
 * is tied to the lifecycle of a Domain, not to the lifecycle of an individual
 * participant. An updated TopicBuiltinTopicData sample is written each time a
 * Topic modifies one or more of its QosPolicy values.
 */
@Extensibility(Extensibility.Kind.MUTABLE_EXTENSIBILITY)
public interface TopicBuiltinTopicData
extends Cloneable, Serializable, DDSObject
{
    @ID(0x005A) @Key
    public BuiltinTopicKey getKey();

    /**
     * @return the name
     */
    @ID(0x0005)
    public String getName();

    /**
     * @return the typeName
     */
    @ID(0x0007)
    public String getTypeName();

    @ID(0x0075) @Optional
    public List<String> getEquivalentTypeName();

    @ID(0x0076) @Optional
    public List<String> getBaseTypeName();

    @ID(0x0072) @Optional
    public TypeObject getType();

    /**
     * @return the durability
     */
    @ID(0x001D)
    public Durability getDurability();

    /**
     * @return the durabilityService
     */
    @ID(0x001E)
    public DurabilityService getDurabilityService();

    /**
     * @return the deadline
     */
    @ID(0x0023)
    public Deadline getDeadline();

    /**
     * @return the latencyBudget
     */
    @ID(0x0027)
    public LatencyBudget getLatencyBudget();

    /**
     * @return the liveliness
     */
    @ID(0x001B)
    public Liveliness getLiveliness();

    /**
     * @return the reliability
     */
    @ID(0x001A)
    public Reliability getReliability();

    /**
     * @return the transportPriority
     */
    @ID(0x0049)
    public TransportPriority getTransportPriority();

    /**
     * @return the lifespan
     */
    @ID(0x002B)
    public Lifespan getLifespan();

    /**
     * @return the destinationOrder
     */
    @ID(0x0025)
    public DestinationOrder getDestinationOrder();

    /**
     * @return the history
     */
    @ID(0x0040)
    public History getHistory();

    /**
     * @return the resourceLimits
     */
    @ID(0x0041)
    public ResourceLimits getResourceLimits();

    /**
     * @return the ownership
     */
    @ID(0x001F)
    public Ownership getOwnership();

    /**
     * @return the topicData
     */
    @ID(0x002E)
    public TopicData getTopicData();

    @ID(0x0073)
    public DataRepresentation getRepresentation();

    @ID(0x0074)
    public TypeConsistencyEnforcement getTypeConsistency();


    // -----------------------------------------------------------------------

    /**
     * Overwrite the state of this object with that of the given object.
     */
    public void copyFrom(TopicBuiltinTopicData src);


    // --- From Object: ------------------------------------------------------

    public TopicBuiltinTopicData clone();
}
