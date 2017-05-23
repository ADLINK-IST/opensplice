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

package org.omg.dds.core.event;

import org.omg.dds.sub.DataReader;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.ResourceLimits;
import org.omg.dds.core.status.SampleRejectedStatus;

/**
 * A (received) sample has been rejected.
 *
 * When the {@link ResourceLimits} Qos is active a SampleReject event can occur when the
 * History determined by {@link History} QoS of the DataReader is full.
 *
 * @param <TYPE>    The data type of the source {@link org.omg.dds.sub.DataReader}.
 *
 * @see SampleRejectedStatus
 */
public abstract class SampleRejectedEvent<TYPE>
extends StatusChangedEvent<DataReader<TYPE>>
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = -7006367152383302636L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    public abstract SampleRejectedStatus getStatus();


    // --- Object Life Cycle: ------------------------------------------------

    protected SampleRejectedEvent(DataReader<TYPE> source) {
        super(source);
    }


    // --- From Object: ------------------------------------------------------

    @Override
    public abstract SampleRejectedEvent<TYPE> clone();
}
