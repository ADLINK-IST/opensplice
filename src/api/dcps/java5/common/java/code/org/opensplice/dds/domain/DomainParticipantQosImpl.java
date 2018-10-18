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
package org.opensplice.dds.domain;

import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.QosPolicy.ForDomainParticipant;
import org.opensplice.dds.core.EntityQosImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.EntityFactoryImpl;
import org.opensplice.dds.core.policy.PolicyConverter;
import org.opensplice.dds.core.policy.SchedulingImpl;
import org.opensplice.dds.core.policy.UserDataImpl;
import org.opensplice.dds.core.policy.Scheduling.ListenerScheduling;
import org.opensplice.dds.core.policy.Scheduling.WatchdogScheduling;
import org.opensplice.dds.domain.DomainParticipantQos;
import org.omg.dds.core.policy.UserData;

public class DomainParticipantQosImpl extends
EntityQosImpl<ForDomainParticipant> implements DomainParticipantQos {
    private static final long serialVersionUID = -1811553017861487660L;

    public DomainParticipantQosImpl(OsplServiceEnvironment environment) {
        super(environment);
    }

    public DomainParticipantQosImpl(OsplServiceEnvironment environment,
            ForDomainParticipant... domainParticipants) {
        super(environment, domainParticipants);

    }

    private DomainParticipantQosImpl(DomainParticipantQosImpl source,
            ForDomainParticipant... policy) {
        super(source.environment, source.policies.values());
        setupPolicies(policy);
    }

    @Override
    protected void setupMissingPolicies() {
        synchronized (this.policies) {
            if (!this.policies.containsKey(EntityFactory.class)) {
                this.policies.put(EntityFactory.class, new EntityFactoryImpl(
                        this.environment));
            }
            if (!this.policies.containsKey(UserData.class)) {
                this.policies.put(UserData.class,
                        new UserDataImpl(this.environment));
            }
            if (!this.policies.containsKey(ListenerScheduling.class)) {
                this.policies.put(ListenerScheduling.class, new SchedulingImpl(
                        this.environment));
            }
            if (!this.policies.containsKey(WatchdogScheduling.class)) {
                this.policies.put(WatchdogScheduling.class, new SchedulingImpl(
                        this.environment));
            }
        }
    }

    @Override
    public UserData getUserData() {
        synchronized (this.policies) {
            return (UserData) this.policies.get(UserData.class);
        }
    }

    @Override
    public EntityFactory getEntityFactory() {
        synchronized (this.policies) {
            return (EntityFactory) this.policies.get(EntityFactory.class);
        }
    }

    @Override
    public ListenerScheduling getListenerScheduling() {
        synchronized (this.policies) {
            return (ListenerScheduling) this.policies.get(ListenerScheduling.class);
        }
    }

    @Override
    public WatchdogScheduling getWatchdogScheduling() {
        synchronized (this.policies) {
            return (WatchdogScheduling) this.policies.get(WatchdogScheduling.class);
        }
    }

    @Override
    public DomainParticipantQos withPolicy(QosPolicy.ForDomainParticipant policy) {
        return this.withPolicies(policy);
    }

    @Override
    public DomainParticipantQos withPolicies(
            QosPolicy.ForDomainParticipant... policy) {
        synchronized (this.policies) {
            return new DomainParticipantQosImpl(this, policy);
        }
    }

    public static DomainParticipantQosImpl convert(OsplServiceEnvironment env,
            DDS.DomainParticipantQos oldQos) {

        if (oldQos == null) {
            throw new IllegalArgumentExceptionImpl(env,
                    "oldQos parameter is null.");
        }

        DomainParticipantQosImpl qos = new DomainParticipantQosImpl(env);

        qos.put(EntityFactory.class,
                PolicyConverter.convert(env, oldQos.entity_factory));
        qos.put(UserData.class, PolicyConverter.convert(env, oldQos.user_data));
        qos.put(ListenerScheduling.class,
                PolicyConverter.convert(env, oldQos.listener_scheduling));
        qos.put(WatchdogScheduling.class,
                PolicyConverter.convert(env, oldQos.watchdog_scheduling));

        return qos;
    }

    public DDS.DomainParticipantQos convert() {
        DDS.DomainParticipantQos old = new DDS.DomainParticipantQos();

        synchronized (this.policies) {
            old.entity_factory = PolicyConverter.convert(this.environment,
                    ((EntityFactory) this.policies.get(EntityFactory.class)));
            old.user_data = PolicyConverter.convert(this.environment,
                    ((UserData) this.policies.get(UserData.class)));
            old.listener_scheduling = PolicyConverter.convert(this.environment,
                    (ListenerScheduling) this.policies
                    .get(ListenerScheduling.class));
            old.watchdog_scheduling = PolicyConverter.convert(this.environment,
                    (WatchdogScheduling) this.policies
                    .get(WatchdogScheduling.class));
        }
        return old;
    }
}
