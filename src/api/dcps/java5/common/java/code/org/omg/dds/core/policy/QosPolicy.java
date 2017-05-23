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

package org.omg.dds.core.policy;

import java.io.Serializable;

import org.omg.dds.core.DDSObject;
import org.omg.dds.type.Extensibility;
import org.omg.dds.type.Nested;


/**
 * This class is the abstract root for all the QoS policies. It provides the
 * basic mechanism for an application to specify quality of service
 * parameters. All concrete QosPolicy classes derive from this root and
 * include a value whose type depends on the concrete QoS policy.
 * <p>
 * The type of a QosPolicy value may be atomic, such as an integer or float,
 * or compound (a structure). Compound types are used whenever multiple
 * parameters must be set coherently to define a consistent value for a
 * QosPolicy.
 * <p>
 * Each {@link org.omg.dds.core.Entity} can be configured with a collection
 * of QosPolicy. However, any Entity cannot support any QosPolicy. For
 * instance, a {@link org.omg.dds.domain.DomainParticipant} supports
 * different QosPolicy than a {@link org.omg.dds.topic.Topic} or a
 * {@link org.omg.dds.pub.Publisher}.
 * <p>
 * QosPolicy can be set when the Entity is created, or modified with the
 * {@link org.omg.dds.core.Entity#setQos(org.omg.dds.core.EntityQos)} method.
 * Each QosPolicy
 * in collection list is treated independently from the others. This approach
 * has the advantage of being very extensible. However, there may be cases
 * where several policies are in conflict. Consistency checking is performed
 * each time the policies are modified via the
 * {@link org.omg.dds.core.Entity#setQos(org.omg.dds.core.EntityQos)}
 * operation.
 * <p>
 * When a policy is changed after being set to a given value, it is not
 * required that the new value be applied instantaneously; the Service is
 * allowed to apply it after a transition phase. In addition, some QosPolicy
 * have "immutable" semantics meaning that they can only be specified either
 * at Entity creation time or else prior to calling the
 * {@link org.omg.dds.core.Entity#enable()} operation on the Entity.
 * <p>
 * Objects of this type are immutable.
 * <p>
 * <b>Properties of QoS Policies</b>
 * <p>
 * In several cases, for communications to occur properly (or efficiently), a
 * QosPolicy on the publisher side must be compatible with a corresponding
 * policy on the subscriber side. For example, if a
 * {@link org.omg.dds.sub.DataReader} requests to receive data reliably while
 * the corresponding {@link org.omg.dds.pub.DataWriter} defines a best-effort
 * policy, communication will not
 * happen as requested. To address this issue and maintain the desirable
 * decoupling of publication and subscription as much as possible, the
 * specification for QosPolicy follows the subscriber-requested,
 * publisher-offered pattern. In this pattern, the subscriber side can
 * specify a "requested" value for a particular QosPolicy. The publisher side
 * specifies an "offered" value for that QosPolicy. The Service will then
 * determine whether the value requested by the subscriber side is compatible
 * with what is offered by the publisher side. If the two policies are
 * compatible, then communication will be established. If the two policies
 * are not compatible, the Service will not establish communications between
 * the two {@link org.omg.dds.core.Entity} objects and will record this fact
 * by means of the
 * {@link org.omg.dds.core.status.OfferedIncompatibleQosStatus} status on the
 * publisher end and
 * {@link org.omg.dds.core.status.RequestedIncompatibleQosStatus} status on
 * the subscriber end. The application can detect this fact by means of a
 * listener (e.g. {@link org.omg.dds.sub.DataReaderListener} or
 * {@link org.omg.dds.pub.DataWriterListener}) or
 * {@link org.omg.dds.core.Condition}s.
 * <p>
 * The QosPolicy objects that need to be set in a compatible manner between
 * the publisher and subscriber ends are indicated by the setting of the
 * "RxO" (Requested/Offered) property:
 *
 * <ul>
 *      <li>An "RxO" setting of "Yes" indicates that the policy can be set
 *          both at the publishing and subscribing ends and the values must
 *          be set in a compatible manner. In this case the compatible values
 *          are explicitly defined.</li>
 *      <li>An "RxO" setting of "No" indicates that the policy can be set
 *          both at the publishing and subscribing ends but the two settings
 *          are independent. That is, all combinations of values are
 *          compatible.</li>
 *      <li>An "RxO" setting of "N/A" indicates that the policy can only be
 *          specified at either the publishing or the subscribing end, but
 *          not at both ends. So compatibility does not apply.</li>
 * </ul>
 *
 * The "changeable" property determines whether the QosPolicy can be changed
 * after the Entity is enabled. In other words, a policy with "changeable"
 * setting of "NO" is considered "immutable" and can only be specified either
 * at {@link org.omg.dds.core.Entity} creation time or else prior to calling
 * the {@link org.omg.dds.core.Entity#enable()} operation.
 */
@Extensibility(Extensibility.Kind.EXTENSIBLE_EXTENSIBILITY)
@Nested
public interface QosPolicy extends Serializable, DDSObject
{
    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.domain.DomainParticipantFactory} objects.
     */
    public static interface ForDomainParticipantFactory extends QosPolicy {
        // empty
    }


    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.domain.DomainParticipant} Entities.
     */
    public static interface ForDomainParticipant extends QosPolicy {
        // empty
    }


    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.pub.Publisher} Entities.
     */
    public static interface ForPublisher extends QosPolicy {
        // empty
    }


    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.sub.Subscriber} Entities.
     */
    public static interface ForSubscriber extends QosPolicy {
        // empty
    }


    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.topic.Topic} Entities.
     */
    public static interface ForTopic extends QosPolicy {
        // empty
    }


    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.pub.DataWriter} Entities.
     */
    public static interface ForDataWriter extends QosPolicy {
        // empty
    }


    /**
     * A QosPolicy interface that implements this marker interface applies
     * to {@link org.omg.dds.sub.DataReader} Entities.
     */
    public static interface ForDataReader extends QosPolicy {
        // empty
    }
}
