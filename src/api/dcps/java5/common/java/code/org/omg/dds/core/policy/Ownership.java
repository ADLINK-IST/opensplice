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
 * This QosPolicy specifies whether a DataWriter exclusively may own an instance. In other words,
 * whether multiple {@link org.omg.dds.pub.DataWriter} objects can write the same instance at the
 * same time. The {@link org.omg.dds.sub.DataReader} objects will only read the modifications on
 * an instance from the DataWriter owning the instance. Exclusive ownership is on an instance-by-instance
 * basis. That is, a Subscriber can receive values written by a lower strength DataWriter as long as
 * they affect instances whose values have not been written or registered by a higher-strength DataWriter.
 * After enabling of the concerning Entity, this QosPolicy cannot be changed any more.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.DataWriter}
 *<p>
 * <b>RxO:</b> Yes
 *<p>
 * <b>Changeable:</b> No
 *<p>
 * Exclusive Ownership
 * The DataWriter with the highest OwnershipStrengthQosPolicy value and being alive (depending on the LivelinessQosPolicy)
 * and which has not violated its DeadlineQosPolicy contract with respect to the instance, will be considered
 * the owner of the instance. Consequently, the ownership can change as a result of:
 * <ul>
 *      <li>a DataWriter in the system with a higher value of the OwnershipStrengthQosPolicy modifies the instance</li>
 *      <li>a change in the OwnershipStrengthQosPolicy value (becomes less) of the DataWriter owning the instance</li>
 *      <li>a change in the liveliness (becomes not alive) of the DataWriter owning the instance</li>
 *      <li>a deadline with respect to the instance that is missed by the DataWriter that owns the instance</li>
 * </ul>
 *
 * Timeline
 * Each DataReader may detect the change of ownership at a different time. In other words, at a particular point in time,
 * the DataReader objects do not have a consistent picture of who owns each instance for that Topic. Outside this grey area
 * in time all DataReader objects will consider the same DataWriter to be the owner. If multiple DataWriter objects with the
 * same OwnershipStrengthQosPolicy modify the same instance, all DataReader objects will make the same choice of the particular
 * DataWriter that is the owner. The DataReader is also notified of this via a status change that is accessible by means of the
 * Listener or Condition mechanisms.
 * <p>
 * Ownership of an Instance
 * DataWriter objects are not aware whether they own a particular instance. There is no error or notification given to a
 * DataWriter that modifies an instance it does not currently own.
 *
 * @see OwnershipStrength
 */
public interface Ownership
extends QosPolicy.ForTopic,
        QosPolicy.ForDataReader,
        QosPolicy.ForDataWriter,
        RequestedOffered<Ownership>
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
     * @param  kind             Specifies whether a DataWriter exclusively owns an instance.
     *
     * @return  a new Ownership policy
     */
    public Ownership withKind(Kind kind);

    /**
     * @return shared Ownership policy
     */
    public Ownership withShared();
    /**
     * @return exclusive Ownership policy
     */
    public Ownership withExclusive();

    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------

    public enum Kind {
        /**
         * Indicates shared ownership for each instance. Multiple writers are
         * allowed to update the same instance and all the updates are made
         * available to the readers. In other words there is no concept of an
         * "owner" for the instances. This is the default behavior.
         */
        SHARED,

        /**
         * Indicates each instance can only be owned by one
         * {@link org.omg.dds.pub.DataWriter}, but the owner of an instance can change
         * dynamically. The selection of the owner is controlled by the
         * setting of the {@link org.omg.dds.core.policy.OwnershipStrength}. The owner is
         * always set to be the highest-strength DataWriter object among the
         * ones currently "active" (as determined by the
         * {@link org.omg.dds.core.policy.Liveliness}).
         */
        EXCLUSIVE
    }

}
