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



/**
 * This policy expresses if the data should "outlive" their writing time.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic},
 * {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.DataWriter}
 * <p>
 * <b>RxO:</b> Yes
 * <p>
 * <b>Changeable:</b> No
 * <p>
 * The decoupling between {@link org.omg.dds.sub.DataReader} and
 * {@link org.omg.dds.pub.DataWriter} offered by the Publish/Subscribe paradigm
 * allows an application to write data even if there are no current readers on
 * the network. Moreover, a DataReader that joins the network after some data
 * has been written could potentially be interested in accessing the most
 * current values of the data as well as potentially some history. This QoS
 * policy controls whether the Service will actually make data available to
 * late-joining readers. Note that although related, this does not strictly
 * control what data the Service will maintain internally. That is, the Service
 * may choose to maintain some data for its own purposes (e.g., flow control)
 * and yet not make it available to late-joining readers if the
 * {@link org.omg.dds.core.policy.Durability} is set to
 * {@link Durability.Kind#VOLATILE}.
 * <p>
 * The value offered is considered compatible with the value requested if and
 * only if the inequality offered kind &gt;= requested kind evaluates to true.
 * For the purposes of this inequality, the values of {@link Durability.Kind}
 * are considered ordered such that VOLATILE &lt; TRANSIENT_LOCAL &lt; TRANSIENT
 * &lt; PERSISTENT.
 * <p>
 * For the purpose of implementing the {@link Durability.Kind} TRANSIENT or
 * PERSISTENT, the service behaves "as if" for each
 * {@link org.omg.dds.topic.Topic} that has TRANSIENT or PERSISTENT DURABILITY
 * kind there was a corresponding "built-in" {@link org.omg.dds.sub.DataReader}
 * and {@link org.omg.dds.pub.DataWriter} configured to have the same DURABILITY
 * kind. In other words, it is "as if" somewhere in the system (possibly on a
 * remote node) there was a "built-in durability
 * DataReader" that subscribed to that Topic and a "built-in durability
 * DataWriter" that published that Topic as needed for the new subscribers that
 * join the system.
 * <p>
 * For each Topic, the built-in fictitious "persistence service" DataReader and
 * DataWriter has its QoS configured from the Topic QoS of the corresponding
 * Topic. In other words, it is "as-if" the service first did
 * {@link org.omg.dds.domain.DomainParticipant#findTopic(String, org.omg.dds.core.Duration)}
 * to access the Topic, and then used the QoS from the Topic to configure the
 * fictitious built-in entities.
 * <p>
 * A consequence of this model is that the transient or persistence service can
 * be configured by means of setting the proper QoS on the Topic.
 * <p>
 * For a given Topic, the usual request/offered semantics apply to the matching
 * between any DataWriter in the system that writes the Topic and the built-in
 * transient/persistent DataReader for that Topic; similarly for the built-in
 * transient/persistent DataWriter for a Topic and any DataReader for the Topic.
 * As a consequence, a DataWriter that has an incompatible QoS with respect to
 * what the Topic specified will not send its data to the transient/persistent
 * service, and a DataReader that has an incompatible QoS with respect to the
 * specified in the Topic will not get data from it.
 * <p>
 * Incompatibilities between local DataReader/DataWriter entities and the
 * corresponding fictitious "built-in transient/persistent entities" cause the
 * {@link org.omg.dds.core.status.RequestedIncompatibleQosStatus}/
 * {@link org.omg.dds.core.status.OfferedIncompatibleQosStatus} to change and
 * the corresponding Listener invocations and/or signaling of
 * {@link org.omg.dds.core.Condition} and {@link org.omg.dds.core.WaitSet}
 * objects as they would with non-fictitious entities.
 * <p>
 * The setting of the serviceCleanupDelay controls when the TRANSIENT or
 * PERSISTENT service is able to remove all information regarding a data
 * instances. Information on a data instances is maintained until the following
 * conditions are met:
 *
 * <ol>
 * <li>the instance has been explicitly disposed (instanceState =
 * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_DISPOSED}),</li>
 * <li>and while in the NOT_ALIVE_DISPOSED state the system detects that there
 * are no more "alive" {@link org.omg.dds.pub.DataWriter} entities writing the
 * instance, that is, all existing writers either unregister the instance (call
 * {@link org.omg.dds.pub.DataWriter#unregisterInstance(org.omg.dds.core.InstanceHandle)}
 * ) or lose their liveliness,</li>
 * <li>and a time interval longer that serviceCleanupDelay has elapsed since the
 * moment the service detected that the previous two conditions were met.</li>
 * </ol>
 *
 * The utility of the serviceCleanupDelay is apparent in the situation where an
 * application disposes an instance and it crashes before it has a chance to
 * complete additional tasks related to the disposition. Upon restart the
 * application may ask for initial data to regain its state and the delay
 * introduced by the serviceCleanupDelay will allow the restarted application to
 * receive the information on the disposed instance and complete the interrupted
 * tasks.
 *
 * @see DurabilityService
 * @see DurabilityService#getServiceCleanupDelay()
 */
public interface Durability
extends QosPolicy.ForTopic,
        QosPolicy.ForDataReader,
        QosPolicy.ForDataWriter,
        RequestedOffered<Durability>
{
    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * @return the kind
     */
    public Kind getKind();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param kind                   Specifies the type of durability from VOLATILE (short life) to PERSISTENT (long life)
     *
     * @return  a new Durability policy
     */
    public Durability withKind(Kind kind);

    /**
     * @return Volatile durability policy.
     */
    public Durability withVolatile();

    /**
     * @return Transient-local durability policy.
     */
    public Durability withTransientLocal();

    /**
     * @return Transient durability policy.
     */
    public Durability withTransient();

    /**
     * @return Persistent durability policy.
     */
    public Durability withPersistent();

    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------

    public enum Kind {
        /**
         * The Service does not need to keep any samples of data instances on
         * behalf of any {@link org.omg.dds.sub.DataReader} that is not known by the
         * {@link org.omg.dds.pub.DataWriter} at the time the instance is written. In other
         * words the Service will only attempt to provide the data to
         * existing subscribers. This is the default kind.
         */
        VOLATILE,

        /**
         * The Service will attempt to keep some samples so that they can be
         * delivered to any potential late-joining {@link org.omg.dds.sub.DataReader}. Which
         * particular samples are kept depends on other QoS such as
         * {@link org.omg.dds.core.policy.History} and {@link org.omg.dds.core.policy.ResourceLimits}.
         *
         * For TRANSIENT_LOCAL, the service is only required to keep the data
         * in the memory of the {@link org.omg.dds.pub.DataWriter} that wrote the data and
         * the data is not required to survive the DataWriter.
         */
        TRANSIENT_LOCAL,

        /**
         * The Service will attempt to keep some samples so that they can be
         * delivered to any potential late-joining {@link org.omg.dds.sub.DataReader}. Which
         * particular samples are kept depends on other QoS such as
         * {@link org.omg.dds.core.policy.History} and {@link org.omg.dds.core.policy.ResourceLimits}.
         *
         * For TRANSIENT, the service is only required to keep the data in
         * memory and not in permanent storage; but the data is not tied to
         * the life cycle of the {@link org.omg.dds.pub.DataWriter} and will, in general,
         * survive it. Support for TRANSIENT kind is optional.
         */
        TRANSIENT,

        /**
         * Data is kept on permanent storage, so that they can outlive a
         * system session. Support for PERSISTENT kind is optional.
         */
        PERSISTENT
    }

}
