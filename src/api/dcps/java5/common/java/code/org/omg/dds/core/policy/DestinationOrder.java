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
 * Controls the criteria used to determine the logical order among changes
 * made by {@link org.omg.dds.pub.Publisher} entities to the same instance of data (i.e.,
 * matching Topic and key). The default kind is
 * {@link DestinationOrder.Kind#BY_RECEPTION_TIMESTAMP}.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.DataWriter}
 *<p>
 * <b>RxO:</b> Yes
 *<p>
 * <b>Changeable:</b> No
 *<p>
 * This policy controls how each subscriber resolves the final value of a
 * data instance that is written by multiple DataWriter objects (which may be
 * associated with different Publisher objects) running on different nodes.
 * <p>
 * The setting {@link Kind#BY_RECEPTION_TIMESTAMP} indicates that, assuming
 * the {@link org.omg.dds.core.policy.Ownership} allows it, the latest received value for
 * the instance should be the one whose value is kept. This is the default value.
 * <p>
 * The setting {@link Kind#BY_SOURCE_TIMESTAMP} indicates that, assuming the
 * {@link org.omg.dds.core.policy.Ownership} allows it, a time stamp placed at the source
 * should be used. This is the only setting that, in the case of concurrent
 * same-strength DataWriter objects updating the same instance, ensures all
 * subscribers will end up with the same final value for the instance.
 * This means that the system needs some time synchronization.
 * <p>
 * The value offered is considered compatible with the value requested if and
 * only if the inequality "offered kind &gt;= requested kind" evaluates to
 * true. For the purposes of this inequality, the values of DESTINATION_ORDER
 * kind are considered ordered such that BY_RECEPTION_TIMESTAMP &lt;
 * BY_SOURCE_TIMESTAMP.
 *
 * @see Ownership
 */
public interface DestinationOrder
extends QosPolicy.ForTopic,
        QosPolicy.ForDataReader,
        QosPolicy.ForDataWriter,
        RequestedOffered<DestinationOrder>
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
     * @param kind                   Specifies the order in which the DataReader stores the data.
     *                               This can be BY_RECEPTION_TIMESTAMP or BY_SOURCE_TIMESTAMP.
     *
     * @return  a new DestinationOrder policy
     */
    public DestinationOrder withKind(Kind kind);

    /**
     * @return A new DestinationPolicy with reception timestamp ordering.
     */
    public DestinationOrder withReceptionTimestamp();

    /**
     * @return A new DestinationPolicy with source timestamp ordering.
     */
    public DestinationOrder withSourceTimestamp();

    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------

    public enum Kind {
        /**
         * Indicates that data is ordered based on the reception time at each
         * {@link org.omg.dds.sub.Subscriber}. Since each subscriber may receive the data at
         * different times there is no guaranteed that the changes will be
         * seen in the same order. Consequently, it is possible for each
         * subscriber to end up with a different final value for the data.
         */
        BY_RECEPTION_TIMESTAMP,

        /**
         * Indicates that data is ordered based on a time stamp placed at the
         * source (by the Service or by the application). In any case this
         * guarantees a consistent final value for the data in all
         * subscribers.
         */
        BY_SOURCE_TIMESTAMP
    }

}
