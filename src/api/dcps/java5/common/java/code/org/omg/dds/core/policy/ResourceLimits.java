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



/**
 * Specifies the resources that the Service can consume in order to meet the
 * requested QoS.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.topic.Topic}, {@link org.omg.dds.sub.DataReader}, {@link org.omg.dds.pub.DataWriter}
 *<p>
 * <b>RxO:</b> No
 *<p>
 * <b>Changeable:</b> No
 *<p>
 * This policy controls the resources that the Service can use in order to
 * meet the requirements imposed by the application and other QoS settings.
 * <p>
 * If the DataWriter objects are communicating samples faster than they are
 * ultimately taken by the DataReader objects, the middleware will eventually
 * hit against some of the QoS-imposed resource limits. Note that this may
 * occur when just a single DataReader cannot keep up with its corresponding
 * DataWriter. The behavior in this case depends on the setting for the
 * {@link org.omg.dds.core.policy.Reliability}. If reliability is
 * {@link Reliability.Kind#BEST_EFFORT}, then the Service is allowed
 * to drop samples. If the reliability is
 * {@link Reliability.Kind#RELIABLE}, the Service will block the
 * DataWriter or discard the sample at the DataReader in order not to lose
 * existing samples (so that the sample can be resent at a later time).
 * <p>
 * The constant {@link #LENGTH_UNLIMITED} may be used to indicate the absence
 * of a particular limit. For example setting maxSamplesPerInstance to
 * LENGH_UNLIMITED will cause the middleware to not enforce this particular
 * limit.
 * <p>
 * The setting of RESOURCE_LIMITS maxSamples must be consistent with the
 * maxSamplesPerInstance. For these two values to be consistent they must
 * verify that "max_samples &gt;= max_samples_per_instance."
 * <p>
 * The setting of RESOURCE_LIMITS maxSamplesPerInstance must be consistent
 * with the HISTORY depth. For these two QoS to be consistent, they must
 * verify that "depth &lt;= maxSamplesPerInstance."
 * <p>
 * An attempt to set this policy to inconsistent values when an entity is
 * created or via a <code>setQos</code> operation will cause the operation to
 * fail.
 *
 * @see History
 * @see Reliability
 */
public interface ResourceLimits
extends QosPolicy.ForTopic, QosPolicy.ForDataReader, QosPolicy.ForDataWriter
{
    public static final int LENGTH_UNLIMITED = -1;


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
     * @param maxSamples            Specifies the maximum number of data samples for all instances for any single
     *                              DataWriter (or DataReader). By default, unlimited.
     * @return  a new ResourceLimits policy
     */
    public ResourceLimits withMaxSamples(int maxSamples);

    /**
     * Copy this policy and override the value of the property.
     * @param maxInstances            Specifies the maximum number of instances for any single DataWriter (or DataReader).
     *                                By default, unlimited.
     * @return  a new ResourceLimits policy
     */
    public ResourceLimits withMaxInstances(int maxInstances);

    /**
     * Copy this policy and override the value of the property.
     * @param maxSamplesPerInstance Specifies the maximum number of samples of any single instance for any single
     *                              DataWriter (or DataReader). By default, unlimited.
     * @return  a new ResourceLimits policy
     */
    public ResourceLimits withMaxSamplesPerInstance(
            int maxSamplesPerInstance);
}
