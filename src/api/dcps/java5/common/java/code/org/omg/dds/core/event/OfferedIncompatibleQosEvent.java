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
import org.omg.dds.core.status.OfferedIncompatibleQosStatus;

/**
 * A {@link org.omg.dds.core.policy.QosPolicy} value was incompatible with what was requested.
 *
 * @param <TYPE>    The data type of the source {@link org.omg.dds.pub.DataWriter}.
 * 
 * @see OfferedIncompatibleQosStatus
 * @see RequestedIncompatibleQosEvent
 */
public abstract class OfferedIncompatibleQosEvent<TYPE>
extends StatusChangedEvent<DataWriter<TYPE>>
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 33668708964334599L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    public abstract OfferedIncompatibleQosStatus getStatus();


    // --- Object Life Cycle: ------------------------------------------------

    protected OfferedIncompatibleQosEvent(DataWriter<TYPE> source) {
        super(source);
    }


    // --- From Object: ------------------------------------------------------

    @Override
    public abstract OfferedIncompatibleQosEvent<TYPE> clone();
}
