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
import org.omg.dds.core.policy.QosPolicy.ForTopic;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.policy.TopicData;
import org.omg.dds.core.policy.TransportPriority;
import org.omg.dds.core.policy.TypeConsistencyEnforcement;
import org.omg.dds.core.policy.TypeConsistencyEnforcement.Kind;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.core.EntityQosImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.DataRepresentationImpl;
import org.opensplice.dds.core.policy.DeadlineImpl;
import org.opensplice.dds.core.policy.DestinationOrderImpl;
import org.opensplice.dds.core.policy.DurabilityImpl;
import org.opensplice.dds.core.policy.DurabilityServiceImpl;
import org.opensplice.dds.core.policy.HistoryImpl;
import org.opensplice.dds.core.policy.LatencyBudgetImpl;
import org.opensplice.dds.core.policy.LifespanImpl;
import org.opensplice.dds.core.policy.LivelinessImpl;
import org.opensplice.dds.core.policy.OwnershipImpl;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.ReliabilityImpl;
import org.opensplice.dds.core.policy.ResourceLimitsImpl;
import org.opensplice.dds.core.policy.TopicDataImpl;
import org.opensplice.dds.core.policy.TransportPriorityImpl;
import org.opensplice.dds.core.policy.TypeConsistencyEnforcementImpl;

public class TopicQosImpl extends EntityQosImpl<ForTopic> implements TopicQos {
    private static final long serialVersionUID = -1401988185243587729L;
    private final TypeConsistencyEnforcement typeConsistencyEnforcement;

    public TopicQosImpl(OsplServiceEnvironment environment,
            TypeConsistencyEnforcement typeConsistencyEnforcement,
            ForTopic... policies) {
        super(environment, policies);
        this.typeConsistencyEnforcement = typeConsistencyEnforcement;
    }

    public TopicQosImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.typeConsistencyEnforcement = new TypeConsistencyEnforcementImpl(
                environment, Kind.EXACT_TYPE_TYPE_CONSISTENCY);
    }

    private TopicQosImpl(TopicQosImpl source, ForTopic... policy) {
        super(source.environment, source.policies.values());
        this.typeConsistencyEnforcement = source.typeConsistencyEnforcement;
        setupPolicies(policy);
    }

    @Override
    public TopicData getTopicData() {
        return (TopicData) this.policies.get(TopicData.class);
    }

    @Override
    public Durability getDurability() {
        return (Durability) this.policies.get(Durability.class);
    }

    @Override
    public DurabilityService getDurabilityService() {
        return (DurabilityService) this.policies.get(DurabilityService.class);
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
    public Ownership getOwnership() {
        return (Ownership) this.policies.get(Ownership.class);
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
    protected void setupMissingPolicies() {
        this.policies.putIfAbsent(TopicData.class, new TopicDataImpl(this.environment));
        this.policies.putIfAbsent(Durability.class, new DurabilityImpl(this.environment));
        this.policies.putIfAbsent(DurabilityService.class, new DurabilityServiceImpl(this.environment));
        this.policies.putIfAbsent(Deadline.class, new DeadlineImpl(this.environment));
        this.policies.putIfAbsent(LatencyBudget.class, new LatencyBudgetImpl(this.environment));
        this.policies.putIfAbsent(Liveliness.class, new LivelinessImpl(this.environment));
        this.policies.putIfAbsent(Reliability.class, new ReliabilityImpl(this.environment));
        this.policies.putIfAbsent(DestinationOrder.class, new DestinationOrderImpl(this.environment));
        this.policies.putIfAbsent(History.class, new HistoryImpl(this.environment));
        this.policies.putIfAbsent(ResourceLimits.class, new ResourceLimitsImpl(this.environment));
        this.policies.putIfAbsent(TransportPriority.class, new TransportPriorityImpl(this.environment));
        this.policies.putIfAbsent(Lifespan.class, new LifespanImpl(this.environment));
        this.policies.putIfAbsent(Ownership.class, new OwnershipImpl(this.environment));
        this.policies.putIfAbsent(DataRepresentation.class, new DataRepresentationImpl(this.environment));
    }

    @Override
    public TopicQos withPolicy(QosPolicy.ForTopic policy) {
        return this.withPolicies(policy);
    }

    @Override
    public TopicQos withPolicies(QosPolicy.ForTopic... policy) {
        return new TopicQosImpl(this, policy);
    }

    public static TopicQosImpl convert(OsplServiceEnvironment env,
            DDS.TopicQos oldQos) {

        if (oldQos == null) {
            throw new IllegalArgumentExceptionImpl(env,
                    "oldQos parameter is null.");
        }

        TopicQosImpl qos = new TopicQosImpl(env);

        qos.put(Deadline.class, PolicyConverter.convert(env, oldQos.deadline));
        qos.put(DestinationOrder.class,
                PolicyConverter.convert(env, oldQos.destination_order));
        qos.put(Durability.class,
                PolicyConverter.convert(env, oldQos.durability));
        qos.put(DurabilityService.class,
                PolicyConverter.convert(env, oldQos.durability_service));
        qos.put(History.class, PolicyConverter.convert(env, oldQos.history));
        qos.put(LatencyBudget.class,
                PolicyConverter.convert(env, oldQos.latency_budget));
        qos.put(Lifespan.class, PolicyConverter.convert(env, oldQos.lifespan));
        qos.put(Liveliness.class,
                PolicyConverter.convert(env, oldQos.liveliness));
        qos.put(Ownership.class, PolicyConverter.convert(env, oldQos.ownership));
        qos.put(Reliability.class,
                PolicyConverter.convert(env, oldQos.reliability));
        qos.put(ResourceLimits.class,
                PolicyConverter.convert(env, oldQos.resource_limits));
        qos.put(TopicData.class,
                PolicyConverter.convert(env, oldQos.topic_data));
        qos.put(TransportPriority.class,
                PolicyConverter.convert(env, oldQos.transport_priority));

        return qos;
    }

    public DDS.TopicQos convert() {
        DDS.TopicQos old = new DDS.TopicQos();

        synchronized (this.policies) {
            old.deadline = PolicyConverter.convert(this.environment,
                    ((Deadline) this.policies
                    .get(Deadline.class)));
            old.destination_order = PolicyConverter
.convert(this.environment,
                    ((DestinationOrder) this.policies
                            .get(DestinationOrder.class)));
            old.durability = PolicyConverter
.convert(this.environment,
                    ((Durability) this.policies.get(Durability.class)));
            old.durability_service = PolicyConverter
.convert(this.environment,
                    ((DurabilityService) this.policies
                            .get(DurabilityService.class)));
            old.history = PolicyConverter.convert(this.environment,
                    ((History) this.policies
                    .get(History.class)));
            old.latency_budget = PolicyConverter
.convert(this.environment,
                    ((LatencyBudget) this.policies
                            .get(LatencyBudget.class)));
            old.lifespan = PolicyConverter.convert(this.environment,
                    ((Lifespan) this.policies
                    .get(Lifespan.class)));
            old.liveliness = PolicyConverter
.convert(this.environment,
                    ((Liveliness) this.policies.get(Liveliness.class)));
            old.ownership = PolicyConverter.convert(this.environment,
                    ((Ownership) this.policies
                    .get(Ownership.class)));
            old.reliability = PolicyConverter
.convert(this.environment,
                    ((Reliability) this.policies
                            .get(Reliability.class)));
            old.resource_limits = PolicyConverter
.convert(this.environment,
                    ((ResourceLimits) this.policies
                            .get(ResourceLimits.class)));
            old.topic_data = PolicyConverter.convert(this.environment,
                    ((TopicData) this.policies
                    .get(TopicData.class)));
            old.transport_priority = PolicyConverter
.convert(this.environment,
                    ((TransportPriority) this.policies
                            .get(TransportPriority.class)));
        }

        return old;
    }

}
