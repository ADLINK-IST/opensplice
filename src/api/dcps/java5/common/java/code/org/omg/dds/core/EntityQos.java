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

package org.omg.dds.core;

import java.io.Serializable;
import java.util.Map;

import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.type.Extensibility;


/**
 * The Data-Distribution Service (DDS) relies on the use of QoS. A QoS
 * (Quality of Service) is a set of characteristics that controls some aspect
 * of the behavior of the DDS Service. QoS is comprised of individual QoS
 * policies (objects of type deriving from {@link org.omg.dds.core.policy.QosPolicy}).
 * <p>
 * QoS (i.e., a collection of QosPolicy objects) may be associated with all
 * {@link org.omg.dds.core.Entity} objects in the system such as {@link org.omg.dds.topic.Topic},
 * {@link org.omg.dds.pub.DataWriter}, {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.Publisher},
 * {@link org.omg.dds.sub.Subscriber}, and {@link org.omg.dds.domain.DomainParticipant}.
 * <p>
 * Some QosPolicy values may not be consistent with other ones. When a set of
 * QosPolicy is passed ({@link org.omg.dds.core.Entity#setQos(EntityQos)} operations), the set
 * resulting from adding the new policies on top of the previous is checked
 * for consistency. If the resulting QoS is inconsistent, the change of QoS
 * operation fails and the previous values are retained.
 * <p>
 * Objects of this type are immutable.
 */
@Extensibility(Extensibility.Kind.MUTABLE_EXTENSIBILITY)
public interface EntityQos<P extends QosPolicy>
extends Map<Class<? extends P>, P>, Serializable, DDSObject
{
    /**
     * @return  a reference to the corresponding policy in this
     *          <code>EntityQos</code>.
     *
     * @see Map#get(Object)
     */
    public <POLICY extends P> POLICY get(Class<POLICY> id);


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this object and override the value of the given policy.
     *
     * @return  a new object
     *
     * @throws  IllegalArgumentException        if the given policy is not
     *          applicable to the concrete type of this EntityQos.
     *
     * @see     #withPolicies(QosPolicy...)
     */
    public EntityQos<P> withPolicy(P policy);

    /**
     * Copy this object and override the values of the given policies.
     *
     * @return  a new object
     *
     * @throws  IllegalArgumentException        if any given policy is not
     *          applicable to the concrete type of this EntityQos.
     *
     * @see     #withPolicy(QosPolicy)
     */
    public EntityQos<P> withPolicies(P... policy);

    /**
     * Provides an instance of {@link org.omg.dds.core.policy.PolicyFactory}.
     * @return An instance of {@link org.omg.dds.core.policy.PolicyFactory}
     */
    public PolicyFactory getPolicyFactory();
}
