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

package org.omg.dds.domain;

import java.util.Collection;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.status.Status;


/**
 * The sole purpose of this class is to allow the creation and destruction of
 * {@link org.omg.dds.domain.DomainParticipant} objects. DomainParticipantFactory itself has no
 * factory. It is a pre-existing per-{@link org.omg.dds.core.ServiceEnvironment} singleton
 * object that can be accessed by means of the
 * {@link #getInstance(ServiceEnvironment)} operation.
 *
 * <p>
 * <b><i>Example</i></b>
 * <pre>
 * <code>
 * // Set "serviceClassName" property to OpenSplice implementation
 * System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
 *                 "org.opensplice.dds.core.OsplServiceEnvironment");
 * // Instantiate a DDS ServiceEnvironment
 * ServiceEnvironment env = ServiceEnvironment.createInstance(ApplicationClassName.class.getClassLoader());
 * // Get the DomainParticipantFactory
 * DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);
 * </code>
 * </pre>
 */
public abstract class DomainParticipantFactory implements DDSObject
{
    // -----------------------------------------------------------------------
    // Singleton Access
    // -----------------------------------------------------------------------

    /**
     * This operation returns the per-ServiceEnvironment
     * DomainParticipantFactory singleton. The operation is idempotent, that
     * is, it can be called multiple times without side effects, and each
     * time it will return a DomainParticipantFactory instance that is equal
     * to the previous results.
     *
     * @param env       Identifies the Service instance to which the
     *                  object will belong.
     */
    public static DomainParticipantFactory getInstance(ServiceEnvironment env)
    {
        return env.getSPI().getParticipantFactory();
    }



    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    /**
     * Create a new participant in the domain with ID 0 having default QoS
     * and no listener.
     *
     * @see     #createParticipant(int)
     * @see     #createParticipant(int, DomainParticipantQos, DomainParticipantListener, Collection)
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    public abstract DomainParticipant createParticipant();

    /**
     * This operation creates a new DomainParticipant object. The
     * DomainParticipant signifies that the calling application intends to
     * join the domain identified by the domainId argument.
     * @param domainId  The ID of the Domain to which the DomainParticipant is joined.
     *                  This should be the ID as specified in the configuration file.
     *                  This will also be applicable for the lookupParticipant and
     *                  getDomainid operations.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #createParticipant()
     * @see     #createParticipant(int, DomainParticipantQos, DomainParticipantListener, Collection)
     * @return a new DomainParticipant object
     */
    public abstract DomainParticipant createParticipant(
            int domainId);

    /**
     * This operation creates a new DomainParticipant object with the specified
     * QoS, listener, and statuses. The DomainParticipant signifies that the
     * calling application intends to join the domain identified by the domainId
     * argument.
     *
     * <p>
     * For each communication status, the StatusChangedFlag flag is initially set to false.
     * It becomes true whenever that communication status changes. For each communication status
     * activated in the mask, the associated {@link org.omg.dds.domain.DomainParticipantListener} operation is
     * invoked and the communication status is reset to false, as the listener implicitly accesses the
     * status which is passed as a parameter to that operation. The status is reset prior to calling
     * the listener, so if the application calls the get&lt;status_name&gt;Status from inside the listener
     * it will see the status already reset. The following statuses are applicable to the DomainParticipantListener:
     * <ul>
     * <li>OfferedDeadlineMissedStatus</li>
     * <li>OfferedIncompatibleQosStatus</li>
     * <li>LivelinessLostStatus</li>
     * <li>PublicationMatchedStatus</li>
     * <li>RequestedDeadlineMissedStatus</li>
     * <li>RequestedIncompatibleQosStatus</li>
     * <li>SampleRejectedStatus</li>
     * <li>LivelinessChangedStatus</li>
     * <li>DataAvailableStatus</li>
     * <li>SubscriptionMatchedStatus</li>
     * <li>SampleLostStatus</li>
     * <li>InconsistentTopicStatus</li>
     * </ul>
     * @param domainId  The ID of the Domain to which the DomainParticipant is joined.
     *                  This should be the ID as specified in the configuration file.
     *                  This will also be applicable for the lookupParticipant and
     *                  getDomainid operations.
     * @param   qos     The desired QoS policies. If the specified QoS
     *                  policies are not consistent, the operation will
     *                  fail and no Subscriber will be created.
     * @param   listener
     *                  The listener to be attached.
     * @param   statuses
     *                  Of which status changes the listener should be notified. A
     *                  null collection signifies all status changes.
     *
     * @throws org.omg.dds.core.InconsistentPolicyException
     *             if the specified QoS policies are not consistent.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see #createParticipant()
     * @see #createParticipant(int)
     * @return a new DomainParticipant object
     */
    public abstract DomainParticipant createParticipant(
            int domainId,
            DomainParticipantQos qos,
            DomainParticipantListener listener,
            Collection<Class<? extends Status>> statuses);

    /**
     * This operation creates a new DomainParticipant object with the specified
     * QoS and the specified listener. The DomainParticipant signifies that the
     * calling application intends to join the domain identified by the domainId
     * argument.
     *
     * <p>
     * For each communication status, the StatusChangedFlag flag is initially set to false.
     * It becomes true whenever that communication status changes. For each communication status
     * activated in the mask, the associated {@link org.omg.dds.domain.DomainParticipantListener} operation is
     * invoked and the communication status is reset to false, as the listener implicitly accesses the
     * status which is passed as a parameter to that operation. The status is reset prior to calling
     * the listener, so if the application calls the get&lt;status_name&gt;Status from inside the listener
     * it will see the status already reset. The following statuses are applicable to the DomainParticipantListener:
     * <ul>
     * <li>OfferedDeadlineMissedStatus</li>
     * <li>OfferedIncompatibleQosStatus</li>
     * <li>LivelinessLostStatus</li>
     * <li>PublicationMatchedStatus</li>
     * <li>RequestedDeadlineMissedStatus</li>
     * <li>RequestedIncompatibleQosStatus</li>
     * <li>SampleRejectedStatus</li>
     * <li>LivelinessChangedStatus</li>
     * <li>DataAvailableStatus</li>
     * <li>SubscriptionMatchedStatus</li>
     * <li>SampleLostStatus</li>
     * <li>InconsistentTopicStatus</li>
     * </ul>
     * @param domainId  The ID of the Domain to which the DomainParticipant is joined.
     *                  This should be the ID as specified in the configuration file.
     *                  This will also be applicable for the lookupParticipant and
     *                  getDomainid operations.
     * @param   qos     The desired QoS policies. If the specified QoS
     *                  policies are not consistent, the operation will
     *                  fail and no Subscriber will be created.
     * @param   listener
     *                  The listener to be attached.
     * @param   statuses
     *                  Of which status changes the listener should be notified. A
     *                  null collection signifies all status changes.
     *
     * @throws org.omg.dds.core.InconsistentPolicyException
     *             if the specified QoS policies are not consistent.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see #createParticipant()
     * @see #createParticipant(int)
     * @return a new DomainParticipant object
     */
    public abstract DomainParticipant createParticipant(
            int domainId,
            DomainParticipantQos qos,
            DomainParticipantListener listener,
            Class<? extends Status>... statuses);

    /**
     * This operation retrieves a previously created DomainParticipant
     * belonging to specified domain ID. If no such DomainParticipant exists,
     * the operation will return null.
     * <p>
     * If multiple DomainParticipant entities belonging to that domain ID
     * exist, then the operation will return one of them. It is not specified
     * which one.
     * @param domainId  the ID of the Domain for which a joining DomainParticipant
     *                  should be retrieved. This should be the ID as specified in the configuration file.
     * @return a DomainParticipant object
     */
    public abstract DomainParticipant lookupParticipant(int domainId);

    /**
     * This operation returns the value of the DomainParticipantFactory QoS
     * policies.
     * @return a DomainParticipantFactoryQos object
     * @see     #setQos(DomainParticipantFactoryQos)
     */
    public abstract DomainParticipantFactoryQos getQos();

    /**
     * This operation sets the value of the DomainParticipantFactory QoS
     * policies. These policies control the behavior of the object, a factory
     * for entities.
     * <p>
     * Note that despite having QoS, the DomainParticipantFactory is not an
     * {@link org.omg.dds.core.Entity}.
     * @param qos   Must contain the new set of QosPolicy settings for the DomainParticipantFactory.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *             if the resulting policies are not self consistent; in that
     *             case, the operation will have no effect.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see #getQos()
     */
    public abstract void setQos(DomainParticipantFactoryQos qos);

    /**
     * This operation retrieves the default value of the DomainParticipant
     * QoS, that is, the QoS policies which will be used for newly created
     * {@link org.omg.dds.domain.DomainParticipant} entities in the case where the QoS policies
     * are defaulted in the {@link #createParticipant()} operation.
     * <p>
     * The values retrieved will match the set of values specified on the
     * last successful call to
     * {@link #setDefaultParticipantQos(DomainParticipantQos)}, or else, if
     * the call was never made, the default values identified by the DDS
     * specification.
     * @return the default DomainParticipantQos object
     * @see     #setDefaultParticipantQos(DomainParticipantQos)
     */
    public abstract DomainParticipantQos getDefaultParticipantQos();

    /**
     * This operation sets a default value of the DomainParticipant QoS policies
     * which will be used for newly created
     * {@link org.omg.dds.domain.DomainParticipant} entities in the case where
     * the QoS policies are defaulted in the {@link #createParticipant()}
     * operation.
     * @param qos    An object of the DomainParticipantQos class, which contains the new default
     *               DomainParticipantQos for the newly created DomainParticipants.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *             if the resulting policies are not self consistent; in that
     *             case, the operation will have no effect.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see #getDefaultParticipantQos()
     */
    public abstract void setDefaultParticipantQos(DomainParticipantQos qos);

}
