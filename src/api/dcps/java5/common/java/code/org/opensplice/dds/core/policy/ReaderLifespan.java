/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;
import org.omg.dds.core.policy.QosPolicy;

/**
 * This {@link org.omg.dds.core.policy.QosPolicy} is similar to the
 * {@link org.omg.dds.core.policy.Lifespan} (applicable to
 * {@link org.omg.dds.topic.Topic} and {@link org.omg.dds.pub.DataWriter}), but
 * limited to the DataReader on which the policy is applied. The data is
 * automatically removed from the DataReader if it has not been taken yet after
 * the lifespan duration expires. The duration of the ReaderLifespan is added to
 * the insertion time of the data in the DataReader to determine the expiry
 * time.
 * <p>
 * When both the ReaderLifespan and a DataWriter Lifespan are applied to the
 * same data, only the earliest expiry time is taken into account. By default,
 * the ReaderLifespan is not used. The duration is set to
 * {@link org.omg.dds.core.Duration#infiniteDuration(org.omg.dds.core.ServiceEnvironment)}
 * <p>
 * This policy is applicable to a DataReader only, and is mutable even when the
 * DataReader is already enabled. If modified, the new setting will only be
 * applied to samples that are received after the modification took place.
 *
 * @see org.omg.dds.core.policy.Lifespan
 */
public interface ReaderLifespan extends QosPolicy.ForDataReader {
    /**
     * @return The duration of this ReaderLifespan.
     */
    public Duration getDuration();

    /**
     * Copy this policy and override the value of the property.
     * @param duration          The duration after which data loses validity and is removed
     * @return a new ReaderLifespan policy
     */
    public ReaderLifespan withDuration(Duration duration);

    /**
     * Copy this policy and override the value of the property.
     * @param duration          The duration after which data loses validity and is removed
     * @param unit              The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return a new ReaderLifespan policy
     */
    public ReaderLifespan withDuration(long duration, TimeUnit unit);
}
