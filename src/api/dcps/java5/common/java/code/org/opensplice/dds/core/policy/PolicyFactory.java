/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.dds.core.policy;

/**
 * OpenSplice-specific extension of
 * {@link org.omg.dds.core.policy.PolicyFactory} that supports an extra set of
 * QosPolicy's.
 * 
 * @see org.opensplice.dds.core.policy.Share
 * @see org.opensplice.dds.core.policy.ReaderLifespan
 * @see org.opensplice.dds.core.policy.Scheduling.WatchdogScheduling
 * @see org.opensplice.dds.core.policy.Scheduling.ListenerScheduling
 * @see org.opensplice.dds.core.policy.SubscriptionKeys
 * @see org.opensplice.dds.core.policy.ViewKeys
 */
public abstract class PolicyFactory extends
        org.omg.dds.core.policy.PolicyFactory {
    /**
     * @return the default Share QosPolicy
     */
    public abstract Share Share();

    /**
     * @return the default ReaderLifespan QosPolicy
     */
    public abstract ReaderLifespan ReaderLifespan();

    /**
     * @return the default WatchdogScheduling QosPolicy
     */
    public abstract Scheduling.WatchdogScheduling WatchDogScheduling();

    /**
     * @return the default ListenerScheduling QosPolicy
     */
    public abstract Scheduling.ListenerScheduling ListenerScheduling();

    /**
     * @return the default SubscriptionKeys QosPolicy
     */
    public abstract SubscriptionKeys SubscriptionKeys();

    /**
     * @return the default ViewKeys QosPolicy
     */
    public abstract ViewKeys ViewKeys();
}
