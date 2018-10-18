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
package org.opensplice.dds.core;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;
import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ModifiableTime;
import org.omg.dds.core.QosProvider;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.Time;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.type.TypeSupport;
import org.omg.dds.type.builtin.KeyedBytes;
import org.omg.dds.type.builtin.KeyedString;
import org.omg.dds.type.dynamic.DynamicDataFactory;
import org.omg.dds.type.dynamic.DynamicTypeFactory;
import org.opensplice.dds.core.policy.PolicyFactoryImpl;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.domain.DomainParticipantFactoryImpl;
import org.opensplice.dds.type.TypeSupportImpl;
import org.opensplice.dds.type.TypeSupportProtobuf;

public class OsplServiceEnvironment extends ServiceEnvironment {
    private final Map<String, Object> environment;
    private final OpenSpliceServiceProviderInterface osplSPI;

    public OsplServiceEnvironment(Map<String, Object> environment) {
        this.environment = new ConcurrentHashMap<String, Object>();

        if (environment != null) {
            this.environment.putAll(environment);
        }
        this.osplSPI = new OpenSpliceServiceProviderInterface(this);
    }

    public OsplServiceEnvironment() {
        this.environment = null;
        this.osplSPI = new OpenSpliceServiceProviderInterface(this);
    }

    public Object getEnvironmentValue(String name) {
        Object value;

        if (this.environment != null) {
            value = this.environment.get(name);
        } else {
            value = null;
        }
        return value;
    }

    @Override
    public ServiceProviderInterface getSPI() {
        return this.osplSPI;
    }

    public static class OpenSpliceServiceProviderInterface implements
            ServiceProviderInterface {
        private final OsplServiceEnvironment environment;
        private final DomainParticipantFactoryImpl factory;
        private final PolicyFactoryImpl policyFactory;

        private OpenSpliceServiceProviderInterface(
                OsplServiceEnvironment environment) {
            this.environment = environment;
            this.factory = new DomainParticipantFactoryImpl(this.environment);
            this.policyFactory = new PolicyFactoryImpl(this.environment);
        }

        @Override
        public DomainParticipantFactory getParticipantFactory() {
            return this.factory;
        }

        @Override
        public DynamicTypeFactory getTypeFactory() {
            throw new UnsupportedOperationExceptionImpl(this.environment,
                    "getTypeFactory() not implemented yet.");
        }

        @Override
        public <TYPE> TypeSupport<TYPE> newTypeSupport(Class<TYPE> type,
                String registeredName) {
            TypeSupport<TYPE> result = null;
            Class<?> superType;
            String typeName;

            try {
                typeName = type.getName();
                superType = Class.forName(typeName).getSuperclass();

                if (superType != null) {
                    if ("com.google.protobuf.GeneratedMessage".equals(superType
                            .getName())) {
                        result = TypeSupportProtobuf.getInstance(
                                this.environment, type, registeredName);

                    } else {
                        result = new TypeSupportImpl<TYPE>(this.environment,
                                type, registeredName);
                    }
                } else {
                    throw new PreconditionNotMetExceptionImpl(
                            this.environment,
                            "Allocating new TypeSupport failed. "
                                    + type.getName()
                                    + "' is not a support type for this DDS implementation.");
                }
            } catch (ClassNotFoundException e) {
                throw new PreconditionNotMetExceptionImpl(this.environment,
                        "Allocating new TypeSupport failed. " + e.getMessage());
            }
            return result;
        }

        @Override
        public Duration newDuration(long duration, TimeUnit unit) {
            return new DurationImpl(this.environment, duration, unit);
        }

        @Override
        public Duration infiniteDuration() {
            return new DurationImpl(this.environment,
                    DDS.DURATION_INFINITE_SEC.value,
                    DDS.DURATION_INFINITE_NSEC.value);
        }

        @Override
        public Duration zeroDuration() {
            return new DurationImpl(this.environment,
                    DDS.DURATION_ZERO_SEC.value, DDS.DURATION_ZERO_NSEC.value);
        }

        @Override
        public ModifiableTime newTime(long time, TimeUnit units) {
            return new ModifiableTimeImpl(this.environment, time, units);
        }

        @Override
        public Time invalidTime() {
            return new TimeImpl(this.environment,
                    DDS.TIMESTAMP_INVALID_SEC.value,
                    DDS.TIMESTAMP_INVALID_NSEC.value);
        }

        @Override
        public InstanceHandle nilHandle() {
            return new InstanceHandleImpl(this.environment,
                    DDS.HANDLE_NIL.value);
        }

        @Override
        public GuardCondition newGuardCondition() {
            return new GuardConditionImpl(this.environment);
        }

        @Override
        public WaitSet newWaitSet() {
            return new WaitSetImpl(this.environment);
        }

        @Override
        public Set<Class<? extends Status>> allStatusKinds() {
            return StatusConverter.convertMask(this.environment,
                    DDS.STATUS_MASK_ANY_V1_2.value);
        }

        @Override
        public Set<Class<? extends Status>> noStatusKinds() {
            return StatusConverter.convertMask(this.environment,
                    DDS.STATUS_MASK_NONE.value);
        }

        @Override
        public QosProvider newQosProvider(String uri, String profile) {
            return new QosProviderImpl(this.environment, uri, profile);
        }

        @Override
        public PolicyFactory getPolicyFactory() {
            return this.policyFactory;
        }

        @Override
        public DynamicDataFactory getDynamicDataFactory() {
            throw new UnsupportedOperationExceptionImpl(this.environment,
                    "getDynamicDataFactory() not implemented yet.");
        }

        @Override
        public KeyedString newKeyedString() {
            throw new UnsupportedOperationExceptionImpl(this.environment,
                    "newKeyedString() not implemented yet.");
        }

        @Override
        public KeyedBytes newKeyedBytes() {
            throw new UnsupportedOperationExceptionImpl(this.environment,
                    "newKeyedBytes() not implemented yet.");
        }

    }

}
