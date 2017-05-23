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

package org.omg.dds.core.policy;

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;


/**
 * Filter that allows a {@link org.omg.dds.sub.DataReader} to specify that it is
 * interested only in (potentially) a subset of the values of the data. The
 * filter states that the DataReader does not want to receive more than one
 * value each minimumSeparation, regardless of how fast the changes occur. It is
 * inconsistent for a DataReader to have a minimumSeparation longer than the
 * result of its {@link org.omg.dds.core.policy.Deadline#getPeriod()}. By
 * default, minimumSeparation = 0, indicating that the DataReader is potentially
 * interested in all values.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.sub.DataReader}
 *<p>
 * <b>RxO:</b> N/A
 *<p>
 * <b>Changeable:</b> Yes
 *<p>
 * The TIME_BASED_FILTER applies to each instance separately, that is, the
 * constraint is that the DataReader does not want to see more than one sample
 * of each instance per minumumSeparation period.
 * <p>
 * This setting allows a DataReader to further decouple itself from the
 * {@link org.omg.dds.pub.DataWriter} objects. It can be used to protect
 * applications that are running on a heterogeneous network where some nodes are
 * capable of generating data much faster than others can consume it. It also
 * accommodates the fact that for fast-changing data different subscribers may
 * have different requirements as to how frequently they need to be notified of
 * the most current values.
 * <p>
 * The setting of a TIME_BASED_FILTER, that is, the selection of a
 * minimumSeparation with a value greater than zero is compatible with all
 * settings of the HISTORY and RELIABILITY QoS. The TIME_BASED_FILTER specifies
 * the samples that are of interest to the DataReader. The HISTORY and
 * RELIABILITY QoS affect the behavior of the middleware with respect to the
 * samples that have been determined to be of interest to the DataReader, that
 * is, they apply after the TIME_BASED_FILTER has been applied.
 * <p>
 * In the case where the reliability QoS kind is RELIABLE then in steady state,
 * defined as the situation where the DataWriter does not write new samples for
 * a period "long" compared to the minimumSeparation, the system should
 * guarantee delivery the last sample to the DataReader.
 * <p>
 * The setting of the TIME_BASED_FILTER minimumSeparation must be consistent
 * with the DEADLINE period. For these two QoS policies to be consistent they
 * must verify that "period &gt;= minimumSeparation." An attempt to set these
 * policies in an inconsistent manner when an entity is created via a
 * {@link org.omg.dds.core.Entity#setQos(org.omg.dds.core.EntityQos)} operation
 * will cause the operation to fail.
 *
 * @see Deadline
 * @see History
 * @see Reliability
 */
public interface TimeBasedFilter extends QosPolicy.ForDataReader
{
    /**
    * @return  the minimum separation duration.
    */
    public Duration getMinimumSeparation();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param minimumSeparation Specifies a duration which is the minimum period between received samples to be passed through the filter.
     *                          The default value is 0, meaning that all samples are accepted.
     *
     * @return  a new TimeBasedFilter policy
     */
    public TimeBasedFilter withMinimumSeparation(
            Duration minimumSeparation);

    /**
     * Copy this policy and override the value of the property.
     * @param minimumSeparation Specifies a long minimumSeparation which is the minimum period between received samples to be passed
     *                          through the filter. The default value is 0, meaning that all samples are accepted.
     * @param unit              The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     *
     * @return  a new TimeBasedFilter policy
     */
    public TimeBasedFilter withMinimumSeparation(
            long minimumSeparation,
            TimeUnit unit);
}
