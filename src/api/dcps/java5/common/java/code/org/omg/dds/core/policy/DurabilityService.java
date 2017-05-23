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
 * Specifies the configuration of the durability service. That is, the
 * service that implements the {@link Durability.Kind} of
 * {@link Durability.Kind#TRANSIENT} and
 * {@link Durability.Kind#PERSISTENT}.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.pub.DataWriter}
 *<p>
 * <b>RxO:</b> No
 *<p>
 * <b>Changeable:</b> No
 *<p>
 * This policy is used to configure the {@link org.omg.dds.core.policy.History} and the
 * {@link org.omg.dds.core.policy.ResourceLimits} used by the fictitious {@link org.omg.dds.sub.DataReader}
 * and {@link org.omg.dds.pub.DataWriter} used by the "persistence service." The "persistence
 * service" is the one responsible for implementing
 * {@link Durability.Kind#TRANSIENT} and
 * {@link Durability.Kind#PERSISTENT}.
 *
 *
 *
 * @see Durability
 */
public interface DurabilityService
extends QosPolicy.ForTopic, QosPolicy.ForDataWriter
{
    public Duration getServiceCleanupDelay();

    /**
     * @return the historyKind
     */
    public History.Kind getHistoryKind();

    /**
     * @return the historyDepth
     */
    public int getHistoryDepth();

    /**
     * @return the maxSamples
     */
    public int getMaxSamples();

    /**
     * @return the maxInstances
     */
    public int getMaxInstances();

    /**
     * @return the maxSamplesPerInstance
     */
    public int getMaxSamplesPerInstance();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param serviceCleanupDelay    A duration on how long the durability service must wait before it is allowed to remove the
     *                               information on the transient or persistent topic data-instances as a result of incoming
     *                               dispose messages.
     *
     * @return  a new DurabilityService policy
     */
    public DurabilityService withServiceCleanupDelay(
            Duration serviceCleanupDelay);

    /**
     * Copy this policy and override the value of the property.
     * @param serviceCleanupDelay    A long on how long the durability service must wait before it is allowed to remove the
     *                               information on the transient or persistent topic data-instances as a result of incoming
     *                               dispose messages.
     *
     * @param unit                   The TimeUnit which the serviceCleanupDelay long describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @return  a new DurabilityService policy
     */
    public DurabilityService withServiceCleanupDelay(
            long serviceCleanupDelay,
            TimeUnit unit);

    /**
     * Copy this policy and override the value of the property.
     * @param historyKind            Specifies the type of history, which may be KEEP_LAST or KEEP_ALL,
     *                               the durability service must apply for the transient or persistent topic data-instances.
     *
     * @return  a new DurabilityService policy
     */
    public DurabilityService withHistoryKind(
            History.Kind historyKind);

    /**
     * Copy this policy and override the value of the property.
     * @param historyDepth            Specifies the number of samples of each instance of data (identified by its key) that is managed
     *                                by the durability service for the transient or persistent topic data-instances. If history_kind
     *                                is KEEP_LAST, history_depth must be smaller than or equal to max_samples_per_instance
     *                                for this QosPolicy to be consistent.
     *
     * @return  a new DurabilityService policy
     */
    public DurabilityService withHistoryDepth(int historyDepth);

    /**
     * Copy this policy and override the value of the property.
     * @param maxSamples              Specifies the maximum number of data samples for all instances the durability service will manage
     *                                for the transient or persistent topic data-instances.
     *
     * @return  a new DurabilityService policy
     */
    public DurabilityService withMaxSamples(int maxSamples);

    /**
     * Copy this policy and override the value of the property.
     * @param maxInstances            Specifies the maximum number of instances the durability service - manage for the transient or
     *                                persistent topic data-instances.
     *
     * @return  a new DurabilityService policy
     */
    public DurabilityService withMaxInstances(int maxInstances);

    /**
     * Copy this policy and override the value of the property.
     * @param maxSamplesPerInstance   Specifies the maximum number of samples of any single instance the durability service will manage
     *                                for the transient or persistent topic data-instances. If history_kind is KEEP_LAST,
     *                                max_samples_per_instance must be greater than or equal to history_depth for this QosPolicy to be
     *                                consistent.
     *
     * @return  a new DurabilityService policy
     */
    public DurabilityService withMaxSamplesPerInstance(
            int maxSamplesPerInstance);
}
