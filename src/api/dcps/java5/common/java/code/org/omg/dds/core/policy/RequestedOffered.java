/* Copyright 2011, Object Management Group, Inc.
 * Copyright 2011, PrismTech, Inc.
 * Copyright 2011, Real-Time Innovations, Inc.
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
 * This interface is implemented by QoS policies that enforce a Request/Offer
 * contract between publications and subscriptions -- that is, for which the
 * level of service offered by the publication is greater than or equal to
 * that requested by the subscription.
 * <p>
 * Details of this contract are provided in each of the implementing types.
 *
 * @param   <SELF>      The QoS policy interface that extends this interface.
 */
public interface RequestedOffered<SELF> extends Comparable<SELF>
{
    /**
     * Use the object returned by this method to evaluate the Request/Offer
     * relationship with another instance of this policy. If this policy is
     * <em>offered</em> by a {@link org.omg.dds.pub.DataWriter} (or
     * {@link org.omg.dds.pub.Publisher}), any other policy evaluated as less
     * than or equal to it by the {@link Comparable} may be compatibly
     * <em>requested</em> by a {@link org.omg.dds.sub.DataReader} (or
     * {@link org.omg.dds.sub.Subscriber}). Similarly, if this policy is
     * <em>requested</em>, any other policy that is greater than or equal to
     * it may be compatibly <em>offered</em>.
     *
     * @return  a {@link Comparable} object capable of evaluating the
     *          Request/Offer relationship between two instances of the QoS
     *          policy identified by the type parameter <code>SELF</code>.
     */
    public Comparable<SELF> requestedOfferedContract();
}
