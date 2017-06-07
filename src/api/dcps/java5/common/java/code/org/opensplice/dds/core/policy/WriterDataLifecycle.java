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

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;

/**
 * This QosPolicy provides OpenSplice-specific extensions to the
 * {@link org.omg.dds.core.policy.WriterDataLifecycle}. Next to all attributes
 * in the original QosPolicy, it also provides:
 * <ul>
 * <li><b>autoPurgeSuspendedSamplesDelay</b> - specifies the duration after
 * which the {@link org.omg.dds.pub.DataWriter} will automatically remove a
 * sample from its history during periods in which its Publisher is suspended.
 * This duration is calculated based on the source timestamp of the written
 * sample. By default the duration value is set to infinite and therefore no
 * automatic purging of samples occurs. See
 * {@link org.omg.dds.pub.Publisher#suspendPublications()} for more information
 * on suspended publications
 * <li><b>autoUnregisterInstanceDelay</b> - specifies the Duration after which
 * the DataWriter will automatically unregister an instance after the
 * application wrote a sample for it and no further action is performed on the
 * same instance by this DataWriter afterwards. This means that when the
 * application writes a new sample for this instance, the duration is
 * recalculated from that action onwards. By default the duration value is
 * infinite and therefore no automatic unregistration occurs.
 * </ul>
 *
 */
public interface WriterDataLifecycle extends
        org.omg.dds.core.policy.WriterDataLifecycle {
    /**
     * @return the autoPurgeSuspendedSamplesDelay Duration
     */
    public Duration getAutoPurgeSuspendedSamplesDelay();

    /**
     * @return the autoUnregisterInstanceDelay Duration
     */
    public Duration getAutoUnregisterInstanceDelay();

    /**
     * Copy this policy and override the value of the property.
     * @param duration     Specifies the duration after which the DataWriter will automatically remove a sample from its
     *                     history during periods in which its Publisher is suspended. This duration is calculated based
     *                     on the source timestamp of the written sample. By default the duration value is set to
     *                     DURATION_INFINITE and therefore no automatic purging of samples occurs.
     * @return a new WriterDataLifecycle policy
     */
    public WriterDataLifecycle withAutoPurgeSuspendedSamplesDelay(
            Duration duration);

    /**
     * Copy this policy and override the value of the property.
     * @param duration     Specifies the duration after which the DataWriter will automatically remove a sample from its
     *                     history during periods in which its Publisher is suspended. This duration is calculated based
     *                     on the source timestamp of the written sample. By default the duration value is set to
     *                     DURATION_INFINITE and therefore no automatic purging of samples occurs.
     *
     * @param unit         The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return a new policy
     */
    public WriterDataLifecycle withAutoPurgeSuspendedSamplesDelay(
            long duration, TimeUnit unit);

    /**
     * Copy this policy and override the value of the property.
     * @param duration     Specifies the duration after which the DataWriter will automatically unregister an instance
     *                     after the application wrote a sample for it and no further action is performed on the same
     *                     instance by this DataWriter afterwards. This means that when the application writes a new
     *                     sample for this instance, the duration is recalculated from that action onwards.
     *                     By default the duration value is DURATION_INFINITE and therefore no automatic unregistration occurs.
     * @return a new WriterDataLifecycle policy
     */
    public WriterDataLifecycle withAutoUnregisterInstanceDelay(Duration duration);

    /**
     * Copy this policy and override the value of the property.
     * @param duration     Specifies the duration after which the DataWriter will automatically unregister an instance
     *                     after the application wrote a sample for it and no further action is performed on the same
     *                     instance by this DataWriter afterwards. This means that when the application writes a new
     *                     sample for this instance, the duration is recalculated from that action onwards.
     *                     By default the duration value is DURATION_INFINITE and therefore no automatic unregistration occurs.
     * @param unit         The TimeUnit which the period describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return a new WriterDataLifecycle policy
     */
    public WriterDataLifecycle withAutoUnregisterInstanceDelay(long duration,
            TimeUnit unit);
}
