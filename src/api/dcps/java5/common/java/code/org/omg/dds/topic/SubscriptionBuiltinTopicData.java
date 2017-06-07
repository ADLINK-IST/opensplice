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
import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.LatencyBudget;
import org.omg.dds.core.policy.Liveliness;
import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.TimeBasedFilter;
import org.omg.dds.core.policy.TopicData;
import org.omg.dds.core.policy.TypeConsistencyEnforcement;
import org.omg.dds.core.policy.UserData;
import org.omg.dds.type.Extensibility;
import org.omg.dds.type.ID;
import org.omg.dds.type.Key;
import org.omg.dds.type.Optional;
import org.omg.dds.type.typeobject.TypeObject;

/**
 * The DCPSSubscription topic communicates the existence of datareaders by
 * means of the SubscriptionBuiltinTopicData datatype. Each
 * SubscriptionBuiltinTopicData sample in a Domain represents a datareader
 * in that Domain: a new SubscriptionBuiltinTopicData instance is created
 * when a newly-added DataReader is enabled, and it is disposed when that
 * DataReader is deleted. An updated SubscriptionBuiltinTopicData sample is
 * written each time the DataReader (or the Subscriber to which it belongs)
 * modifies a QosPolicy that applies to the entities connected to it.
 */
@Extensibility(Extensibility.Kind.MUTABLE_EXTENSIBILITY)
public interface SubscriptionBuiltinTopicData
extends Cloneable, Serializable, DDSObject
{
    @ID(0x005A) @Key
    public BuiltinTopicKey getKey();

    /**
     * @return the participantKey
     */
    @ID(0x0050)
    public BuiltinTopicKey getParticipantKey();

    /**
     * @return the topicName
     */
    @ID(0x0005)
    public String getTopicName();

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
     * @return the ownership
     */
    @ID(0x001F)
    public Ownership getOwnership();

    /**
     * @return the destinationOrder
     */
    @ID(0x0025)
    public DestinationOrder getDestinationOrder();

    /**
     * @return the userData
     */
    @ID(0x002C)
    public UserData getUserData();

    /**
     * @return the timeBasedFilter
     */
    @ID(0x0004)
    public TimeBasedFilter getTimeBasedFilter();

    /**
     * @return the presentation
     */
    @ID(0x0021)
    public Presentation getPresentation();

    /**
     * @return the partition
     */
    @ID(0x0029)
    public Partition getPartition();

    /**
     * @return the topicData
     */
    @ID(0x002E)
    public TopicData getTopicData();

    /**
     * @return the groupData
     */
    @ID(0x002D)
    public GroupData getGroupData();

    @ID(0x0073)
    public DataRepresentation getRepresentation();

    @ID(0x0074)
    public TypeConsistencyEnforcement getTypeConsistency();


    // -----------------------------------------------------------------------

    /**
     * Overwrite the state of this object with that of the given object.
     */
    public void copyFrom(SubscriptionBuiltinTopicData src);


    // --- From Object: ------------------------------------------------------

    public SubscriptionBuiltinTopicData clone();
}
