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
import org.omg.dds.core.status.LivelinessLostStatus;

/**
 * The liveliness that the {@link org.omg.dds.pub.DataWriter} has committed through its
 * {@link org.omg.dds.core.policy.Liveliness} was not respected; thus {@link org.omg.dds.sub.DataReader}
 * entities will consider the DataWriter as no longer "active."
 *
 * @param <TYPE>    The data type of the source {@link org.omg.dds.pub.DataWriter}.
 * 
 * @see LivelinessLostStatus
 * @see LivelinessChangedEvent
 * @see SubscriptionMatchedEvent
 */
public abstract class LivelinessLostEvent<TYPE>
extends StatusChangedEvent<DataWriter<TYPE>>
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 5161009365346289899L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    public abstract LivelinessLostStatus getStatus();


    // --- Object Life Cycle: ------------------------------------------------

    protected LivelinessLostEvent(DataWriter<TYPE> source) {
        super(source);
    }


    // --- From Object: ------------------------------------------------------

    @Override
    public abstract LivelinessLostEvent<TYPE> clone();
}
