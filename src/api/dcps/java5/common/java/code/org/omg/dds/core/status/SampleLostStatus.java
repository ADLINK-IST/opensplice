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

import org.omg.dds.core.policy.DestinationOrder;
import org.omg.dds.core.policy.Presentation;
import org.omg.dds.core.policy.ResourceLimits;



/**
 * A sample has been lost (never received).
 * The status is independent of the differences in instances, in other words, it includes all
 * samples lost across all instances of data published under the Topic.
 *
 * The following events can lead to a sample lost status:
 * <ul>
 * <li>When the {@link Presentation} QoS is used with coherentAccess set to true and {@link ResourceLimits} Qos is active
 * a sample lost can occur when all resources are consumed by incomplete transactions. In order to prevent deadlocks the
 * the current transaction is dropped which causes a SampleLost event.</li>
 *
 * <li>When the {@link DestinationOrder} QoS is set to BY_SOURCE_TIMESTAMP
 * It can happen that older data is inserted after newer data is already taken by the DataReader.
 * In this case the older data is not inserted into the DataReader and a SampleLost is reported.</li>
 * <li>When the networking service detects a gap between sequence numbers of incoming data it will report a SampleLost event</li>
 * </ul>
 *
 * @see org.omg.dds.core.event.SampleLostEvent
 */
public abstract class SampleLostStatus extends Status
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 6522885693257415947L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * Total cumulative count of all samples lost across all instances of
     * data published under the {@link org.omg.dds.topic.Topic}.
     */
    public abstract int getTotalCount();

    /**
     * The incremental number of samples lost since the last time the
     * listener was called or the status was read.
     */
    public abstract int getTotalCountChange();

}
