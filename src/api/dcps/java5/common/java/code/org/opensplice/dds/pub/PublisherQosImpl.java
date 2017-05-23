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

import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.QosPolicy.ForPublisher;
import org.omg.dds.pub.PublisherQos;
import org.opensplice.dds.core.EntityQosImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.EntityFactoryImpl;
import org.opensplice.dds.core.policy.GroupDataImpl;
import org.opensplice.dds.core.policy.PartitionImpl;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.PresentationImpl;

public class PublisherQosImpl extends EntityQosImpl<ForPublisher> implements
        PublisherQos {

    private static final long serialVersionUID = 3160319098450969471L;

    public PublisherQosImpl(OsplServiceEnvironment environment,
            ForPublisher... policies) {
        super(environment, policies);
    }

    public PublisherQosImpl(OsplServiceEnvironment environment) {
        super(environment);
    }

    private PublisherQosImpl(PublisherQosImpl source,
            ForPublisher... policy) {
        super(source.environment, source.policies.values());
        setupPolicies(policy);
    }

    @Override
    protected void setupMissingPolicies() {
        if (!this.containsKey(Presentation.class)) {
            this.put(Presentation.class, new PresentationImpl(this.environment));
        }
        if (!this.containsKey(Partition.class)) {
            this.put(Partition.class, new PartitionImpl(this.environment));
        }
        if (!this.containsKey(GroupData.class)) {
            this.put(GroupData.class, new GroupDataImpl(this.environment));
        }
        if (!this.containsKey(EntityFactory.class)) {
            this.put(EntityFactory.class, new EntityFactoryImpl(
                    this.environment));
        }
    }

    @Override
    public Presentation getPresentation() {
        return this.get(Presentation.class);
    }

    @Override
    public Partition getPartition() {
        return this.get(Partition.class);
    }

    @Override
    public GroupData getGroupData() {
        return this.get(GroupData.class);
    }

    @Override
    public EntityFactory getEntityFactory() {
        return this.get(EntityFactory.class);
    }

    @Override
    public PublisherQos withPolicy(ForPublisher policy) {
        return this.withPolicies(policy);
    }

    @Override
    public PublisherQos withPolicies(ForPublisher... policy) {
        return new PublisherQosImpl(this, policy);
    }

    public static PublisherQosImpl convert(OsplServiceEnvironment env,
            DDS.PublisherQos oldQos) {
        if (oldQos == null) {
            throw new IllegalArgumentExceptionImpl(env,
                    "oldQos parameter is null.");
        }

        PublisherQosImpl qos = new PublisherQosImpl(env);

        qos.put(EntityFactory.class,
                PolicyConverter.convert(env, oldQos.entity_factory));
        qos.put(GroupData.class,
                PolicyConverter.convert(env, oldQos.group_data));
        qos.put(Partition.class, PolicyConverter.convert(env, oldQos.partition));
        qos.put(Presentation.class,
                PolicyConverter.convert(env, oldQos.presentation));

        return qos;
    }

    public DDS.PublisherQos convert() {
        DDS.PublisherQos old = new DDS.PublisherQos();

        synchronized (this.policies) {
            old.entity_factory = PolicyConverter.convert(this.environment,
                    ((EntityFactory) this.policies.get(EntityFactory.class)));
            old.group_data = PolicyConverter.convert(this.environment,
                    ((GroupData) this.policies.get(GroupData.class)));

            old.partition = PolicyConverter.convert(this.environment,
                    ((Partition) this.policies.get(Partition.class)));
            old.presentation = PolicyConverter.convert(this.environment,
                    ((Presentation) this.policies.get(Presentation.class)));
        }
        return old;
    }
}
