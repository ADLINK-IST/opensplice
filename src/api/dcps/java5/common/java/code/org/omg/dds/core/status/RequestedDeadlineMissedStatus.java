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
 * The deadline that the {@link org.omg.dds.sub.DataReader} was expecting through its
 * {@link org.omg.dds.core.policy.Deadline} was not respected for a specific instance.
 * Missed deadlines accumulate, that is, each deadline period the totalCount will be
 * incremented by one for each instance for which data was not received.
 *
 * @see org.omg.dds.core.event.RequestedDeadlineMissedEvent
 * @see OfferedDeadlineMissedStatus
 */
public abstract class RequestedDeadlineMissedStatus extends Status
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 4552827018168249989L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * Total cumulative number of missed deadlines detected for any instance
     * read by the {@link org.omg.dds.sub.DataReader}. Missed deadlines accumulate; that is,
     * each deadline period the totalCount will be incremented by one for
     * each instance for which data was not received.
     */
    public abstract int getTotalCount();

    /**
     * The incremental number of deadlines detected since the last time the
     * listener was called or the status was read.
     */
    public abstract int getTotalCountChange();

    /**
     * Handle to the last instance in the {@link org.omg.dds.sub.DataReader} for which a
     * deadline was detected.
     */
    public abstract InstanceHandle getLastInstanceHandle();
}
