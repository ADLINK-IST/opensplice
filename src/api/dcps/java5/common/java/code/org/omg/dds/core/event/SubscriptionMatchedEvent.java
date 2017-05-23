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
import org.omg.dds.core.status.SubscriptionMatchedStatus;

/**
 * The {@link org.omg.dds.sub.DataReader} has found a {@link org.omg.dds.pub.DataWriter} that matches the
 * {@link org.omg.dds.topic.Topic} and has compatible QoS, or has ceased to be matched with a
 * DataWriter that was previously considered to be matched.
 *
 * @param <TYPE>    The data type of the source {@link org.omg.dds.sub.DataReader}.
 * 
 * @see SubscriptionMatchedStatus
 * @see PublicationMatchedEvent
 */
public abstract class SubscriptionMatchedEvent<TYPE>
extends StatusChangedEvent<DataReader<TYPE>>
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 285655835365202584L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    public abstract SubscriptionMatchedStatus getStatus();


    // --- Object Life Cycle: ------------------------------------------------

    protected SubscriptionMatchedEvent(DataReader<TYPE> source) {
        super(source);
    }


    // --- From Object: ------------------------------------------------------

    @Override
    public abstract SubscriptionMatchedEvent<TYPE> clone();
}
