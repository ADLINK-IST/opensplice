/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
package org.opensplice.dds.sub;

import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.QosPolicy.ForSubscriber;
import org.omg.dds.sub.SubscriberQos;
import org.opensplice.dds.core.EntityQosImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.EntityFactoryImpl;
import org.opensplice.dds.core.policy.GroupDataImpl;
import org.opensplice.dds.core.policy.PartitionImpl;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.PresentationImpl;
import org.opensplice.dds.core.policy.Share;

public class SubscriberQosImpl extends EntityQosImpl<ForSubscriber> implements
        org.opensplice.dds.sub.SubscriberQos {
    private static final long serialVersionUID = 5350093533137522289L;

    public SubscriberQosImpl(OsplServiceEnvironment environment,
            ForSubscriber... policies) {
        super(environment, policies);
    }

    public SubscriberQosImpl(OsplServiceEnvironment environment) {
        super(environment);
    }

    private SubscriberQosImpl(SubscriberQosImpl source, ForSubscriber... policy) {
        super(source.environment, source.policies.values());
        setupPolicies(policy);
    }

    @Override
    protected void setupMissingPolicies() {
        synchronized(this.policies) {
            if (!this.policies.containsKey(Presentation.class)) {
                this.policies.put(Presentation.class, new PresentationImpl(
                        this.environment));
            }
            if (!this.policies.containsKey(Partition.class)) {
                this.policies.put(Partition.class, new PartitionImpl(
                        this.environment));
            }
            if (!this.policies.containsKey(GroupData.class)) {
                this.policies.put(GroupData.class, new GroupDataImpl(
                        this.environment));
            }
            if (!this.policies.containsKey(EntityFactory.class)) {
                this.policies.put(EntityFactory.class, new EntityFactoryImpl(
                        this.environment));
            }
        }
    }

    @Override
    public Presentation getPresentation() {
        synchronized(this.policies) {
            return (Presentation) this.policies.get(Presentation.class);
        }
    }

    @Override
    public Partition getPartition() {
        synchronized(this.policies) {
            return (Partition) this.policies.get(Partition.class);
        }
    }

    @Override
    public GroupData getGroupData() {
        synchronized(this.policies) {
            return (GroupData) this.policies.get(GroupData.class);
        }
    }

    @Override
    public EntityFactory getEntityFactory() {
        synchronized(this.policies) {
            return (EntityFactory) this.policies.get(EntityFactory.class);
        }
    }

    @Override
    public Share getShare() {
        synchronized(this.policies) {
            return (Share) this.policies.get(Share.class);
        }
    }

    @Override
    public SubscriberQos withPolicy(ForSubscriber policy) {
        return this.withPolicies(policy);
    }

    @Override
    public SubscriberQos withPolicies(ForSubscriber... policy) {
        synchronized (this.policies) {
            return new SubscriberQosImpl(this, policy);
        }
    }

    public static SubscriberQosImpl convert(OsplServiceEnvironment env,
            DDS.SubscriberQos oldQos) {
        if (oldQos == null) {
            throw new IllegalArgumentExceptionImpl(env,
                    "oldQos parameter is null.");
        }

        SubscriberQosImpl qos = new SubscriberQosImpl(env);

        qos.put(EntityFactory.class,
                PolicyConverter.convert(env, oldQos.entity_factory));
        qos.put(GroupData.class,
                PolicyConverter.convert(env, oldQos.group_data));
        qos.put(Partition.class, PolicyConverter.convert(env, oldQos.partition));
        qos.put(Presentation.class,
                PolicyConverter.convert(env, oldQos.presentation));

        Share share = PolicyConverter.convert(env, oldQos.share);

        if (share != null) {
            qos.put(Share.class, share);
        }

        return qos;
    }

    public DDS.SubscriberQos convert() {
        DDS.SubscriberQos old = new DDS.SubscriberQos();

        synchronized (this.policies) {
            old.entity_factory = PolicyConverter
.convert(this.environment,
                    ((EntityFactory) this.policies
                            .get(EntityFactory.class)));
            old.group_data = PolicyConverter.convert(this.environment,
                    ((GroupData) this.policies
                    .get(GroupData.class)));

            old.partition = PolicyConverter.convert(this.environment,
                    ((Partition) this.policies
                    .get(Partition.class)));
            old.presentation = PolicyConverter
.convert(this.environment,
                    ((Presentation) this.policies
                            .get(Presentation.class)));
            old.share = PolicyConverter.convert(this.environment,
                    ((Share) this.policies
                    .get(Share.class)));
        }
        return old;
    }

}
