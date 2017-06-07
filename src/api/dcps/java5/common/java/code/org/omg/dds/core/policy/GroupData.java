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
 * User data not known by the middleware, but distributed by means of
 * built-in topics. The default value is an empty (zero-sized) sequence.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.pub.Publisher}, {@link org.omg.dds.sub.Subscriber}
 *<p>
 * <b>RxO:</b> No
 *<p>
 * <b>Changeable:</b> Yes
 *<p>
 * The purpose of this QoS is to allow the application to attach additional
 * information to the created {@link org.omg.dds.pub.Publisher} or {@link org.omg.dds.sub.Subscriber}. The
 * value of the GROUP_DATA is available to the application on the
 * {@link org.omg.dds.sub.DataReader} and {@link org.omg.dds.pub.DataWriter} entities and is propagated by
 * means of the built-in topics.
 * <p>
 * This QoS can be used by an application combination with the
 * {@link org.omg.dds.sub.DataReaderListener} and {@link org.omg.dds.pub.DataWriterListener} to implement
 * matching policies similar to those of the {@link org.omg.dds.core.policy.Partition}
 * except the decision can be made based on an application-defined policy.
 */
public interface GroupData
extends QosPolicy.ForPublisher, QosPolicy.ForSubscriber
{
    /**
     * Get a copy of the data.
     */
    public byte[] getValue();


    // --- Modification: -----------------------------------------------------

    /**
     *  copy this policy and override the value of the property.
     *  @param value        A sequence of bytes that holds the application group data. By default, the sequence has length 0.
     *  @param offset       Not used can have any int value.
     *  @param length       Not used can have any int value.
     *
     * @return  a new policy
     */
    public GroupData withValue(byte value[], int offset, int length);
}
