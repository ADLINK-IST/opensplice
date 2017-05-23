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
 * Specifies the behavior of the {@link org.omg.dds.sub.DataReader} with regards to the life
 * cycle of the data instances it manages.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.sub.DataReader}
 *<p>
 * <b>RxO:</b> N/A
 *<p>
 * <b>Changeable:</b> Yes
 *<p>
 * This policy controls the behavior of the DataReader with regards to the
 * lifecycle of the data instances it manages, that is, the data instance
 * that have been received and for which the DataReader maintains some
 * internal resources.
 * <p>
 * The DataReader internally maintains the samples that have not been taken
 * by the application, subject to the constraints imposed by other QoS
 * policies such as {@link org.omg.dds.core.policy.History} and
 * {@link org.omg.dds.core.policy.ResourceLimits}.
 * <p>
 * The DataReader also maintains information regarding the identity,
 * {@link org.omg.dds.sub.ViewState} and {@link org.omg.dds.sub.InstanceState} of data instances even after
 * all samples have been 'taken.' This is needed to properly compute the
 * states when future samples arrive.
 * <p>
 * Under normal circumstances the DataReader can only reclaim all resources
 * for instances for which there are no writers and for which all samples
 * have been 'taken.' The last sample the DataReader will have taken for that
 * instance will have an instance state of either
 * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS} or
 * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_DISPOSED} depending on whether the last
 * writer that had ownership of the instance disposed it or not. In the
 * absence of the READER_DATA_LIFECYCLE QoS this behavior could cause
 * problems if the application "forgets" to 'take' those samples. The
 * 'untaken' samples will prevent the DataReader from reclaiming the
 * resources and they would remain in the DataReader indefinitely.
 * <p>
 * The autoPurgeNoWriterSamplesDelay defines the maximum duration for which
 * the DataReader will maintain information regarding an instance once its
 * instance state becomes NOT_ALIVE_NO_WRITERS. After this time elapses, the
 * DataReader will purge all internal information regarding the instance; any
 * untaken samples will also be lost.
 * <p>
 * The autoPurgeDisposedSamplesDelay defines the maximum duration for which
 * the DataReader will maintain samples for an instance once its instance
 * state becomes NOT_ALIVE_DISPOSED. After this time elapses, the DataReader
 * will purge all samples for the instance.
 *
 * @see History
 * @see ResourceLimits
 */
public interface ReaderDataLifecycle extends QosPolicy.ForDataReader
{
    /**
     * @return the duration of the autoPurgeNoWriterSamplesDelay
     */
    public Duration getAutoPurgeNoWriterSamplesDelay();
    /**
     * @return the duration of the autoPurgeDisposedSamplesDelay
     */
    public Duration getAutoPurgeDisposedSamplesDelay();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param autoPurgeNoWriterSamplesDelay     Specifies the duration for which the DataReader will maintain information
     *                                          regarding a data instance for which the instance_state becomes
     *                                          NOT_ALIVE_NO_WRITERS. By default the duration value is
     *                                          DURATION_INFINITE. When the delay time has expired, the data instance is
     *                                          marked so that it can be purged in the next garbage collection sweep.
     *
     * @return  a new ReaderDataLifecycle policy
     */
    public ReaderDataLifecycle withAutoPurgeNoWriterSamplesDelay(
            Duration autoPurgeNoWriterSamplesDelay);

    /**
     * Copy this policy and override the value of the property.
     * @param autoPurgeNoWriterSamplesDelay     Specifies a long autoPurgeNoWriterSamplesDelay for which the DataReader will maintain
     *                                          information regarding a data instance for which the instance_state becomes
     *                                          NOT_ALIVE_NO_WRITERS. By default the duration value is
     *                                          DURATION_INFINITE. When the delay time has expired, the data instance is
     *                                          marked so that it can be purged in the next garbage collection sweep.
     *
     * @param unit                              The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return  a new ReaderDataLifecycle policy
     */
    public ReaderDataLifecycle withAutoPurgeNoWriterSamplesDelay(
            long autoPurgeNoWriterSamplesDelay,
            TimeUnit unit);

    /**
     * Copy this policy and override the value of the property.
     * @param autoPurgeDisposedSamplesDelay     Specifies a duration autoPurgeDisposedSamplesDelay for which the DataReader will maintain
     *                                          information regarding a data instance for which the instance_state becomes NOT_ALIVE_DISPOSED.
     *                                          By default the duration value is DURATION_INFINITE. When the delay time has expired,
     *                                          the data instance is marked so that it can be purged in the next garbage collection sweep.
     * @return  a new ReaderDataLifecycle policy
     */
    public ReaderDataLifecycle withAutoPurgeDisposedSamplesDelay(
            Duration autoPurgeDisposedSamplesDelay);

    /**
     * Copy this policy and override the value of the property.
     * @param autoPurgeDisposedSamplesDelay     Specifies a long autoPurgeDisposedSamplesDelay for which the DataReader will maintain
     *                                          information regarding a data instance for which the instance_state becomes NOT_ALIVE_DISPOSED.
     *                                          By default the duration value is DURATION_INFINITE. When the delay time has expired,
     *                                          the data instance is marked so that it can be purged in the next garbage collection sweep.
     *
     * @param unit                              The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return  a new policy
     */
    public ReaderDataLifecycle withAutoPurgeDisposedSamplesDelay(
            long autoPurgeDisposedSamplesDelay,
            TimeUnit unit);
}
