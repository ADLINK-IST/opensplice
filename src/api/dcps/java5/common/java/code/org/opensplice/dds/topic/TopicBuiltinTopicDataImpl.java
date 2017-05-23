/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.dds.topic;

import java.util.List;

import org.omg.dds.core.ServiceEnvironment;
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
import org.omg.dds.core.policy.TypeConsistencyEnforcement.Kind;
import org.omg.dds.topic.BuiltinTopicKey;
import org.omg.dds.topic.TopicBuiltinTopicData;
import org.omg.dds.type.typeobject.TypeObject;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.UnsupportedOperationExceptionImpl;
import org.opensplice.dds.core.policy.DataRepresentationImpl;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.TypeConsistencyEnforcementImpl;

public class TopicBuiltinTopicDataImpl implements TopicBuiltinTopicData {
    private static final long serialVersionUID = 4997312302065915561L;
    private final transient OsplServiceEnvironment environment;
    private DDS.TopicBuiltinTopicData old;
    private BuiltinTopicKey key;
    private final TypeConsistencyEnforcement typeConsistency;
    private final DataRepresentation dataRepresentation;

    public TopicBuiltinTopicDataImpl(OsplServiceEnvironment environment,
            DDS.TopicBuiltinTopicData old) {
        this.environment = environment;
        this.old = old;
        this.key = new BuiltinTopicKeyImpl(this.environment, old.key);
        this.dataRepresentation = new DataRepresentationImpl(this.environment,
                DataRepresentation.Id.XCDR_DATA_REPRESENTATION);
        this.typeConsistency = new TypeConsistencyEnforcementImpl(
                this.environment, Kind.EXACT_TYPE_TYPE_CONSISTENCY);
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public BuiltinTopicKey getKey() {
        return this.key;
    }

    @Override
    public String getName() {
        return this.old.name;
    }

    @Override
    public String getTypeName() {
        return this.old.type_name;
    }

    @Override
    public List<String> getEquivalentTypeName() {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "TopicBuiltinTopicData.getEquivalentTypeName() not supported.");
    }

    @Override
    public List<String> getBaseTypeName() {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "TopicBuiltinTopicData.getBaseTypeName() not supported.");
    }

    @Override
    public TypeObject getType() {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "TopicBuiltinTopicData.getType() not supported.");
    }

    @Override
    public Durability getDurability() {
        return PolicyConverter.convert(this.environment, old.durability);
    }

    @Override
    public DurabilityService getDurabilityService() {
        return PolicyConverter
                .convert(this.environment, old.durability_service);
    }

    @Override
    public Deadline getDeadline() {
        return PolicyConverter.convert(this.environment, old.deadline);
    }

    @Override
    public LatencyBudget getLatencyBudget() {
        return PolicyConverter.convert(this.environment, old.latency_budget);
    }

    @Override
    public Liveliness getLiveliness() {
        return PolicyConverter.convert(this.environment, old.liveliness);
    }

    @Override
    public Reliability getReliability() {
        return PolicyConverter.convert(this.environment, old.reliability);
    }

    @Override
    public TransportPriority getTransportPriority() {
        return PolicyConverter
                .convert(this.environment, old.transport_priority);
    }

    @Override
    public Lifespan getLifespan() {
        return PolicyConverter.convert(this.environment, old.lifespan);
    }

    @Override
    public DestinationOrder getDestinationOrder() {
        return PolicyConverter.convert(this.environment, old.destination_order);
    }

    @Override
    public History getHistory() {
        return PolicyConverter.convert(this.environment, old.history);
    }

    @Override
    public ResourceLimits getResourceLimits() {
        return PolicyConverter.convert(this.environment, old.resource_limits);
    }

    @Override
    public Ownership getOwnership() {
        return PolicyConverter.convert(this.environment, old.ownership);
    }

    @Override
    public TopicData getTopicData() {
        return PolicyConverter.convert(this.environment, old.topic_data);
    }

    @Override
    public DataRepresentation getRepresentation() {
        return this.dataRepresentation;
    }

    @Override
    public TypeConsistencyEnforcement getTypeConsistency() {
        return this.typeConsistency;
    }

    @Override
    public void copyFrom(TopicBuiltinTopicData src) {
        if (src == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid TopicBuiltinTopicData (null) provided.");
        }
        try {
            TopicBuiltinTopicDataImpl impl = (TopicBuiltinTopicDataImpl) src;
            this.old = impl.old;
            this.key = new BuiltinTopicKeyImpl(this.environment, this.old.key);
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "TopicBuiltinTopicData.copyFrom() on non-OpenSplice TopicBuiltinTopicData implementation is not supported.");
        }
    }

    @Override
    public TopicBuiltinTopicData clone() {
        return new TopicBuiltinTopicDataImpl(this.environment, this.old);
    }
}
