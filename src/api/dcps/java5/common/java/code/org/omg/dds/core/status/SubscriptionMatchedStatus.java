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

package org.omg.dds.core.status;

import org.omg.dds.core.InstanceHandle;


/**
 *
 * This class contains the statistics about the discovered number of {@link org.omg.dds.pub.DataWriter}s that are compatible
 * with the {@link org.omg.dds.sub.DataReader} to which the Status is attached. DataWriter and DataReader are compatible if
 * they use the same {@link org.omg.dds.topic.Topic} and if the QoS requested by the DataReader is compatible with that offered
 * by the DataWriter. A DataWriter will automatically connect to a matching DataReader, but will disconnect when that DataWriter
 * is deleted, when either changes its QoS into an incompatible value, or when either puts its matching counterpart on its ignore-list
 * using the ignoreSubscription or ignorePublication operations on the DomainParticipant.
 *
 * @see org.omg.dds.core.event.SubscriptionMatchedEvent
 * @see PublicationMatchedStatus
 */
public abstract class SubscriptionMatchedStatus extends Status
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 8269669800428084585L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * Total cumulative count the concerned {@link org.omg.dds.sub.DataReader} discovered a
     * "match" with a {@link org.omg.dds.pub.DataWriter}. That is, it found a DataWriter for
     * the same {@link org.omg.dds.topic.Topic} with a requested QoS that is compatible with
     * that offered by the DataReader.
     */
    public abstract int getTotalCount();

    /**
     * The change in totalCount since the last time the listener was called
     * or the status was read.
     */
    public abstract int getTotalCountChange();

    /**
     * The number of {@link org.omg.dds.pub.DataWriter}s currently matched to the concerned
     * {@link org.omg.dds.sub.DataReader}.
     */
    public abstract int getCurrentCount();

    /**
     * The change in currentCount since the last time the listener was called
     * or the status was read.
     */
    public abstract int getCurrentCountChange();

    /**
     * Handle to the last {@link org.omg.dds.pub.DataWriter} that matched the
     * {@link org.omg.dds.sub.DataReader}, causing the status to change.
     */
    public abstract InstanceHandle getLastPublicationHandle();
}
