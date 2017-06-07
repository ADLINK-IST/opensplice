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

import org.omg.dds.pub.DataWriter;
import org.omg.dds.core.status.OfferedDeadlineMissedStatus;

/**
 * The deadline that the {@link org.omg.dds.pub.DataWriter} has committed through its
 * {@link org.omg.dds.core.policy.Deadline} was not respected for a specific instance.
 *
 * @param <TYPE>    The data type of the source {@link org.omg.dds.pub.DataWriter}.
 * 
 * @see OfferedDeadlineMissedStatus
 * @see RequestedDeadlineMissedEvent
 */
public abstract class OfferedDeadlineMissedEvent<TYPE>
extends StatusChangedEvent<DataWriter<TYPE>>
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 7678105392129292520L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    public abstract OfferedDeadlineMissedStatus getStatus();


    // --- Object Life Cycle: ------------------------------------------------

    protected OfferedDeadlineMissedEvent(DataWriter<TYPE> source) {
        super(source);
    }


    // --- From Object: ------------------------------------------------------

    @Override
    public abstract OfferedDeadlineMissedEvent<TYPE> clone();
}
