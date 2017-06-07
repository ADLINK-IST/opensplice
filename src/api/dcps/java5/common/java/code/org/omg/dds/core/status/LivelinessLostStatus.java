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



/**
 * The liveliness that the {@link org.omg.dds.pub.DataWriter} has committed through its
 * {@link org.omg.dds.core.policy.Liveliness} was not respected; thus {@link org.omg.dds.sub.DataReader}
 * entities will consider the DataWriter as no longer "active."
 *
 * @see org.omg.dds.core.event.LivelinessLostEvent
 * @see LivelinessChangedStatus
 * @see SubscriptionMatchedStatus
 */
public abstract class LivelinessLostStatus extends Status
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = -8294734757039670162L;



    // -----------------------------------------------------------------------
    // Methods
    // -----------------------------------------------------------------------

    /**
     * Total cumulative number of times that a previously-alive
     * {@link org.omg.dds.pub.DataWriter} became not alive due to a failure to actively
     * signal its liveliness within its offered liveliness period. This count
     * does not change when an already not alive DataWriter simply remains
     * not alive for another liveliness period.
     */
    public abstract int getTotalCount();

    /**
     * The change in totalCount since the last time the listener was called
     * or the status was read.
     */
    public abstract int getTotalCountChange();

}
