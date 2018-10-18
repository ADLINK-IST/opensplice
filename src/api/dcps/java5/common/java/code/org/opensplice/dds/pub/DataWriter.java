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
package org.opensplice.dds.pub;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.Time;

/**
 * OpenSplice-specific extension of {@link org.omg.dds.pub.DataWriter} with
 * support for writing atomic writing of a sample and disposing the
 * corresponding instance.
 *
 * @param <TYPE>
 *            The concrete type of the data to be published over the the topic.
 */
public interface DataWriter<TYPE> extends org.omg.dds.pub.DataWriter<TYPE> {
    /**
     * This operation requests the Data Distribution Service to modify the
     * instance and mark it disposed. Copies of the instance and its
     * corresponding samples, which are stored in every connected DataReader
     * and, dependent on the QoSPolicy settings, also in the Transient and
     * Persistent stores, will be modified and marked as disposed by setting the
     * InstanceState to NOT_ALIVE_DISPOSED.
     * <p>
     * When this operation is used, the Data Distribution Service will
     * automatically supply the value of the sourceTimestamp that is made
     * available to connected DataReader objects. This timestamp is important
     * for the interpretation of the DestinationOrder QosPolicy.
     * <p>
     * As a side effect, this operation asserts liveliness on the DataWriter
     * itself and on the containing DomainParticipant.
     * <p>
     * <b>Effects on DataReaders</b><br>
     * Actual deletion of the instance administration in a connected DataReader
     * will be postponed until the following conditions have been met:
     * <ul>
     * <li>the instance must be unregistered (either implicitly or explicitly)
     * by all connected DataWriters that have previously registered it.
     * <ul>
     * <li>A DataWriter can register an instance explicitly by using one of the
     * special registerInstance operations
     * <li>A DataWriter can register an instance implicitly by using the special
     * nil InstanceHandle in one of the other DataWriter operations.
     * <li>A DataWriter can unregister an instance explicitly by using one of
     * the special unregisterInstance methods
     * <li>A DataWriter will unregister all its contained instances implicitly
     * when it is deleted.
     * <li>When a DataReader detects a loss of liveliness in one of its
     * connected DataWriters, it will consider all instances registered by that
     * DataWriter as being implicitly unregistered.
     * </ul>
     * <li><b>and</b> the application must have consumed all samples belonging
     * to the instance, either implicitly or explicitly.
     * <ul>
     * <li>An application can consume samples explicitly by invoking the take
     * operation, or one of its variants, on its DataReaders.
     * <li>The DataReader can consume disposed samples implicitly when the
     * autopurgeDisposedSamplesDelay of the ReaderDataLifecycle QosPolicy has
     * expired.
     * </ul>
     * </ul>
     * The DataReader may also remove instances that haven't been disposed
     * first: this happens when the autopurgeNowriterSamplesDelay of the
     * ReaderDataLifecycle QosPolicy has expired after the instance is
     * considered unregistered by all connected DataWriters ( i . e. when it has
     * a InstanceState of NOT_ALIVE_NO_WRITERS). See also
     * {@link org.omg.dds.core.policy.ReaderDataLifecycle}
     * <p>
     * <b>Effects on Transient/Persistent Stores</b><br>
     * Actual deletion of the instance administration in the connected Transient
     * and Persistent stores will be postponed until the following conditions
     * have been met:
     * <ul>
     * <li>the instance must be unregistered (either implicitly or explicitly)
     * by all connected DataWriters that have previously registered it. (See
     * above.)
     * <li>and the period of time specified by the service_cleanup_delay
     * attribute in the DurabilityServiceQosPolicy on the Topic must have
     * elapsed after the instance is considered unregistered by all connected
     * DataWriters.
     * </ul>
     * See also {@link org.omg.dds.core.policy.DurabilityService}
     * <p>
     * <b>Instance Handle</b><br>
     * The nil handle value (see
     * {@link org.omg.dds.core.InstanceHandle#nilHandle(org.omg.dds.core.ServiceEnvironment)}
     * can be used for the parameter handle. This indicates the identity of the
     * instance is automatically deduced from the instance_data (by means of the
     * key). If handle is any value other than nil, it must correspond to the
     * value that was returned by either the register_instance operation or the
     * register_instance_w_timestamp operation, when the instance (identified by
     * its key) was registered. If there is no correspondence, the result of the
     * operation is unspecified. The sample that is passed as instance_data will
     * actually be delivered to the connected DataReaders, but will immediately
     * be marked for deletion.
     * <p>
     * <b>Blocking</b><br>
     * If the {@link org.omg.dds.core.policy.History} QosPolicy is set to
     * KEEP_ALL, the writeDispose operation on the DataWriter may block if the
     * modification would cause data to be lost because one of the limits,
     * specified in the {@link org.omg.dds.core.policy.ResourceLimits}
     * QosPolicy, to be exceeded. In case the synchronous attribute value of the
     * {@link org.omg.dds.core.policy.Reliability} QosPolicy is set to true for
     * communicating DataWriters and DataReaders then the DataWriter will wait
     * until all synchronous DataReaders have acknowledged the data. Under these
     * circumstances, the maxBlockingTime attribute of the Reliability QosPolicy
     * configures the maximum time the writeDispose operation may block (either
     * waiting for space to become available or data to be acknowledged). If
     * maxBlockingTime elapses before the DataWriter is able to store the
     * modification without exceeding the limits and all expected
     * acknowledgements are received, the writeDispose operation will fail and
     * throws a TimeoutException.
     * <p>
     * <b>Sample Validation</b><br>
     * Before the sample is accepted by the DataWriter, it is validated against
     * the restrictions imposed by the IDL to Java language mapping, where:
     * <ul>
     * <li>a string (bounded or unbounded) may not be null. (Use "" for an empty
     * string instead)
     * <li>the length of a bounded string may not exceed the limit specified in
     * IDL
     * <li>the length of a bounded sequence may not exceed the limit specified
     * in IDL
     * <li>the length of an array must exactly match the size specified in IDL
     * </ul>
     * If any of these restrictions is violated, the operation will fail and
     * throw a IllegalArgumentException. More specific information about the
     * context of this error will be written to the error log.
     *
     * @param instanceData
     *            the data to write
     * @throws TimeoutException
     *             if the configured maximum time elapses and the DataWriter is
     *             still unable to store the new sample without exceeding its
     *             configured resource limits.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #writeDispose(Object, Time)
     * @see #writeDispose(Object, long, TimeUnit)
     * @see #writeDispose(Object, InstanceHandle)
     * @see #writeDispose(Object, InstanceHandle, Time)
     * @see #writeDispose(Object, InstanceHandle, long, TimeUnit)
     */
    public void writeDispose(TYPE instanceData) throws TimeoutException;

    /**
     * @see #writeDispose(Object)
     * @param instanceData      The data to write
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     */
    public void writeDispose(TYPE instanceData, Time sourceTimestamp)
            throws TimeoutException;

    /**
     * @see #writeDispose(Object)
     * @param instanceData      The data to write
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     */
    public void writeDispose(TYPE instanceData, long sourceTimestamp,
            TimeUnit unit) throws TimeoutException;

    /**
     * @see #writeDispose(Object)
     * @param instanceData      The data to write
     * @param handle            The handle to the instance that needs to be disposed.
     */
    public void writeDispose(TYPE instanceData, InstanceHandle handle)
            throws TimeoutException;

    /**
     * @see #writeDispose(Object)
     * @param instanceData      The data to write
     * @param handle            The handle to the instance that needs to be disposed.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     */
    public void writeDispose(TYPE instanceData, InstanceHandle handle,
            Time sourceTimestamp) throws TimeoutException;

    /**
     * @see #writeDispose(Object)
     * @param instanceData      The data to write
     * @param handle            The handle to the instance that needs to be disposed.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     */
    public void writeDispose(TYPE instanceData, InstanceHandle handle,
            long sourceTimestamp, TimeUnit unit) throws TimeoutException;
}
