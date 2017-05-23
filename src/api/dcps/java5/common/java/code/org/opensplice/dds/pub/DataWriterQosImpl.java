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
package org.opensplice.dds.pub;

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
import org.omg.dds.core.policy.OwnershipStrength;
import org.omg.dds.core.policy.QosPolicy.ForDataWriter;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.policy.TransportPriority;
import org.omg.dds.core.policy.TypeConsistencyEnforcement;
import org.omg.dds.core.policy.TypeConsistencyEnforcement.Kind;
import org.omg.dds.core.policy.UserData;
import org.omg.dds.core.policy.WriterDataLifecycle;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.core.EntityQosImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.UnsupportedOperationExceptionImpl;
import org.opensplice.dds.core.policy.DataRepresentationImpl;
import org.opensplice.dds.core.policy.DeadlineImpl;
import org.opensplice.dds.core.policy.DestinationOrderImpl;
import org.opensplice.dds.core.policy.DurabilityImpl;
import org.opensplice.dds.core.policy.HistoryImpl;
import org.opensplice.dds.core.policy.LatencyBudgetImpl;
import org.opensplice.dds.core.policy.LifespanImpl;
import org.opensplice.dds.core.policy.LivelinessImpl;
import org.opensplice.dds.core.policy.OwnershipImpl;
import org.opensplice.dds.core.policy.OwnershipStrengthImpl;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.ReliabilityImpl;
import org.opensplice.dds.core.policy.ResourceLimitsImpl;
import org.opensplice.dds.core.policy.TransportPriorityImpl;
import org.opensplice.dds.core.policy.TypeConsistencyEnforcementImpl;
import org.opensplice.dds.core.policy.UserDataImpl;
import org.opensplice.dds.core.policy.WriterDataLifecycleImpl;

public class DataWriterQosImpl extends EntityQosImpl<ForDataWriter> implements
        DataWriterQos {
    private static final long serialVersionUID = -3612002704263154280L;
    private final TypeConsistencyEnforcement typeConsistencyEnforcement;

    public DataWriterQosImpl(OsplServiceEnvironment environment,
            TypeConsistencyEnforcement typeConsistencyEnforcement,
            ForDataWriter... policies) {
        super(environment, policies);
        this.typeConsistencyEnforcement = typeConsistencyEnforcement;

    }

    public DataWriterQosImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.typeConsistencyEnforcement = new TypeConsistencyEnforcementImpl(
                environment, Kind.EXACT_TYPE_TYPE_CONSISTENCY);
    }

    private DataWriterQosImpl(DataWriterQosImpl source, ForDataWriter... policy) {
        super(source.environment, source.policies.values());
        this.typeConsistencyEnforcement = source.typeConsistencyEnforcement;
        setupPolicies(policy);
    }

    @Override
    public Durability getDurability() {
        return (Durability) this.policies.get(Durability.class);
    }

    @Override
    public DurabilityService getDurabilityService() {
        // TODO: Fix Java 5 PSM; DurabilityService is NOT part of DataWriterQos
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "DurabilityService is only part of the TopicQos, not of the DataWriterQos");
    }

    @Override
    public Deadline getDeadline() {
        return (Deadline) this.policies.get(Deadline.class);
    }

    @Override
    public LatencyBudget getLatencyBudget() {
        return (LatencyBudget) this.policies.get(LatencyBudget.class);
    }

    @Override
    public Liveliness getLiveliness() {
        return (Liveliness) this.policies.get(Liveliness.class);
    }

    @Override
    public Reliability getReliability() {
        return (Reliability) this.policies.get(Reliability.class);
    }

    @Override
    public DestinationOrder getDestinationOrder() {
        return (DestinationOrder) this.policies.get(DestinationOrder.class);
    }

    @Override
    public History getHistory() {
        return (History) this.policies.get(History.class);
    }

    @Override
    public ResourceLimits getResourceLimits() {
        return (ResourceLimits) this.policies.get(ResourceLimits.class);
    }

    @Override
    public TransportPriority getTransportPriority() {
        return (TransportPriority) this.policies.get(TransportPriority.class);
    }

    @Override
    public Lifespan getLifespan() {
        return (Lifespan) this.policies.get(Lifespan.class);
    }

    @Override
    public UserData getUserData() {
        return (UserData) this.policies.get(UserData.class);
    }

    @Override
    public Ownership getOwnership() {
        return (Ownership) this.policies.get(Ownership.class);
    }

    @Override
    public OwnershipStrength getOwnershipStrength() {
        return (OwnershipStrength) this.policies.get(OwnershipStrength.class);
    }

    @Override
    public WriterDataLifecycle getWriterDataLifecycle() {
        return (WriterDataLifecycle) this.policies
                .get(WriterDataLifecycle.class);
    }

    @Override
    public DataRepresentation getRepresentation() {
        return (DataRepresentation) this.policies.get(DataRepresentation.class);
    }

    @Override
    public TypeConsistencyEnforcement getTypeConsistency() {
        return this.typeConsistencyEnforcement;
    }

    @Override
    public DataWriterQos withPolicy(ForDataWriter policy) {
        return this.withPolicies(policy);
    }

    @Override
    public DataWriterQos withPolicies(ForDataWriter... policy) {
        return new DataWriterQosImpl(this, policy);
    }

    public void mergeTopicQos(TopicQos topicQos) {
        synchronized (this.policies) {
            this.policies.put(Deadline.class, topicQos.getDeadline());
            this.policies.put(DestinationOrder.class,
                    topicQos.getDestinationOrder());
            this.policies.put(Durability.class, topicQos.getDurability());
            this.policies.put(History.class, topicQos.getHistory());
            this.policies.put(LatencyBudget.class, topicQos.getLatencyBudget());
            this.policies.put(Lifespan.class, topicQos.getLifespan());
            this.policies.put(Liveliness.class, topicQos.getLiveliness());
            this.policies.put(Ownership.class, topicQos.getOwnership());
            this.policies.put(Reliability.class, topicQos.getReliability());
            this.policies.put(DataRepresentation.class,
                    topicQos.getRepresentation());
            this.policies.put(ResourceLimits.class,
                    topicQos.getResourceLimits());
            this.policies.put(TransportPriority.class,
                    topicQos.getTransportPriority());
        }
    }

    @Override
    protected void setupMissingPolicies() {
        if (!this.policies.containsKey(Durability.class)) {
            this.policies.put(Durability.class, new DurabilityImpl(
                    this.environment));
        }
        if (!this.policies.containsKey(Deadline.class)) {
            this.policies.put(Deadline.class,
                    new DeadlineImpl(this.environment));
        }
        if (!this.policies.containsKey(LatencyBudget.class)) {
            this.policies.put(LatencyBudget.class, new LatencyBudgetImpl(
                    this.environment));
        }
        if (!this.policies.containsKey(Liveliness.class)) {
            this.policies.put(Liveliness.class, new LivelinessImpl(
                    this.environment));
        }
        if (!this.policies.containsKey(Reliability.class)) {
            this.policies.put(Reliability.class, new ReliabilityImpl(
                    this.environment).withReliable());
        }
        if (!this.policies.containsKey(DestinationOrder.class)) {
            this.policies.put(DestinationOrder.class, new DestinationOrderImpl(
                    this.environment));
        }
        if (!this.policies.containsKey(History.class)) {
            this.policies.put(History.class, new HistoryImpl(this.environment));
        }
        if (!this.policies.containsKey(ResourceLimits.class)) {
            this.policies.put(ResourceLimits.class, new ResourceLimitsImpl(
                    this.environment));
        }
        if (!this.policies.containsKey(TransportPriority.class)) {
            this.policies.put(TransportPriority.class,
                    new TransportPriorityImpl(this.environment));
        }
        if (!this.policies.containsKey(Lifespan.class)) {
            this.policies.put(Lifespan.class,
                    new LifespanImpl(this.environment));
        }
        if (!this.policies.containsKey(Ownership.class)) {
            this.policies.put(Ownership.class, new OwnershipImpl(
                    this.environment));
        }
        if (!this.policies.containsKey(OwnershipStrength.class)) {
            this.policies.put(OwnershipStrength.class,
                    new OwnershipStrengthImpl(this.environment));
        }
        if (!this.policies.containsKey(DataRepresentation.class)) {
            this.policies.put(DataRepresentation.class,
                    new DataRepresentationImpl(this.environment));
        }
        if (!this.policies.containsKey(UserData.class)) {
            this.policies.put(UserData.class,
                    new UserDataImpl(this.environment));
        }
        if (!this.policies.containsKey(WriterDataLifecycle.class)) {
            this.policies.put(WriterDataLifecycle.class,
                    new WriterDataLifecycleImpl(this.environment));
        }
    }

    public static DataWriterQosImpl convert(OsplServiceEnvironment env,
            DDS.DataWriterQos oldQos) {

        if (oldQos == null) {
            throw new IllegalArgumentExceptionImpl(env,
                    "oldQos parameter is null.");
        }

        DataWriterQosImpl qos = new DataWriterQosImpl(env);

        qos.put(Deadline.class, PolicyConverter.convert(env, oldQos.deadline));
        qos.put(DestinationOrder.class,
                PolicyConverter.convert(env, oldQos.destination_order));
        qos.put(Durability.class,
                PolicyConverter.convert(env, oldQos.durability));
        qos.put(History.class, PolicyConverter.convert(env, oldQos.history));
        qos.put(LatencyBudget.class,
                PolicyConverter.convert(env, oldQos.latency_budget));
        qos.put(Lifespan.class, PolicyConverter.convert(env, oldQos.lifespan));
        qos.put(Liveliness.class,
                PolicyConverter.convert(env, oldQos.liveliness));
        qos.put(Ownership.class, PolicyConverter.convert(env, oldQos.ownership));
        qos.put(OwnershipStrength.class,
                PolicyConverter.convert(env, oldQos.ownership_strength));
        qos.put(Reliability.class,
                PolicyConverter.convert(env, oldQos.reliability));
        qos.put(ResourceLimits.class,
                PolicyConverter.convert(env, oldQos.resource_limits));
        qos.put(TransportPriority.class,
                PolicyConverter.convert(env, oldQos.transport_priority));
        qos.put(UserData.class, PolicyConverter.convert(env, oldQos.user_data));
        qos.put(WriterDataLifecycle.class,
                PolicyConverter.convert(env, oldQos.writer_data_lifecycle));

        return qos;
    }

    public DDS.DataWriterQos convert() {
        DDS.DataWriterQos old = new DDS.DataWriterQos();

        synchronized (this.policies) {
            old.deadline = PolicyConverter.convert(this.environment,
                    ((Deadline) this.policies.get(Deadline.class)));
            old.destination_order = PolicyConverter.convert(this.environment,
                    ((DestinationOrder) this.policies
                            .get(DestinationOrder.class)));
            old.durability = PolicyConverter.convert(this.environment,
                    ((Durability) this.policies.get(Durability.class)));
            old.history = PolicyConverter.convert(this.environment,
                    ((History) this.policies.get(History.class)));
            old.latency_budget = PolicyConverter.convert(this.environment,
                    ((LatencyBudget) this.policies.get(LatencyBudget.class)));
            old.lifespan = PolicyConverter.convert(this.environment,
                    ((Lifespan) this.policies.get(Lifespan.class)));
            old.liveliness = PolicyConverter.convert(this.environment,
                    ((Liveliness) this.policies.get(Liveliness.class)));
            old.ownership = PolicyConverter.convert(this.environment,
                    ((Ownership) this.policies.get(Ownership.class)));
            old.ownership_strength = PolicyConverter.convert(this.environment,
                    ((OwnershipStrength) this.policies
                            .get(OwnershipStrength.class)));
            old.reliability = PolicyConverter.convert(this.environment,
                    ((Reliability) this.policies.get(Reliability.class)));
            old.resource_limits = PolicyConverter.convert(this.environment,
                    ((ResourceLimits) this.policies.get(ResourceLimits.class)));
            old.transport_priority = PolicyConverter.convert(this.environment,
                    ((TransportPriority) this.policies
                            .get(TransportPriority.class)));
            old.user_data = PolicyConverter.convert(this.environment,
                    ((UserData) this.policies
                    .get(UserData.class)));
            old.writer_data_lifecycle = PolicyConverter.convert(
                    this.environment, ((WriterDataLifecycle) this.policies
                            .get(WriterDataLifecycle.class)));
        }

        return old;
    }

}
