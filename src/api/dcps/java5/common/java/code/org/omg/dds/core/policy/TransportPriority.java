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
 * This policy is a hint to the infrastructure as to how to set the priority
 * of the underlying transport used to send the data. The default value is
 * zero.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.pub.DataWriter}
 *<p>
 * <b>RxO:</b> N/A
 *<p>
 * <b>Changeable:</b> Yes
 *<p>
 * The purpose of this QoS is to allow the application to take advantage of
 * transports capable of sending messages with different priorities.
 * <p>
 * This policy is considered a hint. The policy depends on the ability of the
 * underlying transports to set a priority on the messages they send. Any
 * value within the range of a 32-bit signed integer may be chosen; higher
 * values indicate higher priority. However, any further interpretation of
 * this policy is specific to a particular transport and a particular
 * implementation of the Service. For example, a particular transport is
 * permitted to treat a range of priority values as equivalent to one
 * another. It is expected that during transport configuration the
 * application would provide a mapping between the values of the
 * TRANSPORT_PRIORITY set on DataWriter and the values meaningful to each
 * transport. This mapping would then be used by the infrastructure when
 * propagating the data written by the DataWriter.
 */
public interface TransportPriority
extends QosPolicy.ForTopic, QosPolicy.ForDataWriter
{
    /**
     * @return the value
     */
    public int getValue();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param value             Specifies the priority with which the Data Distribution System can
                                handle the data produced by the DataWriter.
     *
     * @return  a new TransportPriority policy
     */
    public TransportPriority withValue(int value);
}
