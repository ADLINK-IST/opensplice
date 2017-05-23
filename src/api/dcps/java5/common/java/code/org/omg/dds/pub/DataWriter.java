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

package org.omg.dds.pub;

import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.DomainEntity;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.Time;
import org.omg.dds.core.status.LivelinessLostStatus;
import org.omg.dds.core.status.OfferedDeadlineMissedStatus;
import org.omg.dds.core.status.OfferedIncompatibleQosStatus;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.topic.SubscriptionBuiltinTopicData;
import org.omg.dds.topic.Topic;


/**
 * DataWriter allows the application to set the value of the data to be
 * published under a given {@link org.omg.dds.topic.Topic}.
 * <p>
 * A DataWriter is attached to exactly one {@link org.omg.dds.pub.Publisher}
 * that acts as a factory for it. A DataWriter is bound to exactly one Topic and
 * therefore to exactly one data type. The Topic must exist prior to the
 * DataWriter's creation.
 * <p>
 * All operations except for the inherited operations
 * {@link #setQos(org.omg.dds.core.EntityQos)}, {@link #getQos()},
 * {@link #setListener(java.util.EventListener)},{@link #getListener()},
 * {@link #enable()}, {@link #getStatusCondition()}, and {@link #close()} may
 * fail with the exception {@link org.omg.dds.core.NotEnabledException}.
 *
 * @param <TYPE>
 *            The concrete type of the data to be published over the the topic.
 *
 *
 * <pre>
 * <b><i>Example</i></b>
 * <code>
 * This example assumes a DomainParticipant named "participant" has been created
 * and a datatype called Foo is present: class Foo { int myKey; String myName; };
 *
 * // Create a topic named "FooTopic" using the default QoS and no Listener.
 * Topic&lt;Foo&gt; fooTopic = participant.createTopic("FooTopic", Foo.class);
 *
 * // Create a Publisher using default Qos and no Listener.
 * Publisher myPub = participant.createPublisher();
 *
 * // Create a DataWriter using the topic Qos and no listener.
 * DataWriterQos dwq  = myPub.copyFromTopicQos(myPub.getDefaultDataWriterQos(), fooTopic.getQos()
 * DataWriter&lt;Foo&gt; fooDW = myPub.createDataWriter(fooTopic,dwq);
 * // Instantiate and initialize a sample of type Foo.
 * Foo myFoo = new Foo(10, "Vortex OpenSplice DDS");
 * // Register this instance in the DataWriter and write the 1st sample.
 * InstanceHandle myHandle = fooDW.registerInstance(myFoo);
 *
 * // write the sample
 * fooDW.write(myFoo, myHandle);
 *
 * //Dispose the instance in all Readers and unregister it.
 * fooDW.dispose(myHandle);
 * fooDW.unregisterInstance(myHandle);
 * </code>
 * </pre>
 */
public interface DataWriter<TYPE>
extends DomainEntity<DataWriterListener<TYPE>, DataWriterQos>
{
    /**
     * Cast this data writer to the given type, or throw an exception if
     * the cast fails.
     *
     * @param <OTHER>   The type of the data published by this writer,
     *                  according to the caller.
     * @return          this data writer
     * @throws          ClassCastException if the cast fails
     */
    public <OTHER> DataWriter<OTHER> cast();

    /**
     * @return  the {@link org.omg.dds.topic.Topic} associated with the DataWriter. This is the
     *          same Topic that was used to create the DataWriter.
     */
    public Topic<TYPE> getTopic();

    /**
     * This operation is intended to be used only if the DataWriter has
     * {@link org.omg.dds.core.policy.Reliability#getKind()} set to
     * {@link org.omg.dds.core.policy.Reliability.Kind#RELIABLE}.
     * Otherwise the operation will return immediately.
     * <p>
     * The operation blocks the calling thread until either all data written
     * by the DataWriter is acknowledged by all matched {@link org.omg.dds.sub.DataReader}
     * entities that have
     * {@link org.omg.dds.core.policy.Reliability#getKind()} set to
     * {@link org.omg.dds.core.policy.Reliability.Kind#RELIABLE},
     * or else the duration
     * specified by the maxWait parameter elapses, whichever happens first.
     * <p>
     * A normal return indicates that all the samples written have been
     * acknowledged by all reliable matched data readers.
     * <p>
     * Be aware that in case the operation succeeds, the data has only been
     * acknowledged by the local infrastructure: it does not mean all remote subscriptions
     * have already received the data. However, delivering the data to remote nodes is then
     * the sole responsibility of the networking service: even when the publishing
     * application would terminate, all data that has not yet been received may be
     * considered 'on-route' and will therefore eventually arrive (unless the networking
     * service itself will crash). In contrast, if a DataWriter would still have data in its local
     * history buffer when it terminates, this data is considered 'lost'.
     *
     *
     * @param maxWait   The maximum duration to block for the waitForAcknowledgments,
     *                  after which the application thread is unblocked. The operation
     *                  Duration.infiniteDuration(env) can be used when the maximum waiting
     *                  time does not need to be bounded.
     * @throws TimeoutException
     *                  if maxWait elapsed before all the data was acknowledged.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     *
     */
    public void waitForAcknowledgments(Duration maxWait)
            throws TimeoutException;

    /**
     * This operation is intended to be used only if the DataWriter has
     * {@link org.omg.dds.core.policy.Reliability#getKind()} set to
     * {@link org.omg.dds.core.policy.Reliability.Kind#RELIABLE}.
     * Otherwise the operation will return immediately.
     * <p>
     * The operation blocks the calling thread until either all data written
     * by the DataWriter is acknowledged by all matched {@link org.omg.dds.sub.DataReader}
     * entities that have
     * {@link org.omg.dds.core.policy.Reliability#getKind()} set to
     * {@link org.omg.dds.core.policy.Reliability.Kind#RELIABLE},
     * or else the duration
     * specified by the maxWait parameter elapses, whichever happens first.
     * <p>
     * A normal return indicates that all the samples written have been
     * acknowledged by all reliable matched data readers.
     * <p>
     * Be aware that in case the operation succeeds, the data has only been
     * acknowledged by the local infrastructure: it does not mean all remote subscriptions
     * have already received the data. However, delivering the data to remote nodes is then
     * the sole responsibility of the networking service: even when the publishing
     * application would terminate, all data that has not yet been received may be
     * considered 'on-route' and will therefore eventually arrive (unless the networking
     * service itself will crash). In contrast, if a DataWriter would still have data in its local
     * history buffer when it terminates, this data is considered 'lost'.
     *
     * @param maxWait   The maximum duration to block for the waitForAcknowledgments,
     *                  after which the application thread is unblocked defined in a long.
     * @param unit      The TimeUnit which the maxWait describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws TimeoutException
     *                  if maxWait elapsed before all the data was acknowledged.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     */
    public void waitForAcknowledgments(long maxWait, TimeUnit unit)
            throws TimeoutException;

    /**
     * This operation obtains the LivelinessLostStatus object of the DataWriter.
     * This object contains the information whether the liveliness (that the DataWriter
     * has committed through its LivelinessQosPolicy) was respected. This means that the
     * status represents whether the DataWriter failed to actively signal its liveliness
     * within the offered liveliness period. If the liveliness is lost, the DataReader
     * objects will consider the DataWriter as no longer "alive". The LivelinessLostStatus
     * can also be monitored using a DataWriterListener or by using the associated StatusCondition.
     *
     * @return a LivelinessLostStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     org.omg.dds.core.status.LivelinessLostStatus
     */
    public LivelinessLostStatus getLivelinessLostStatus();

    /**
     * This operation obtains the OfferedDeadlineMissedStatus object of the DataWriter.
     * This object contains the information whether the deadline (that the DataWriter has
     * committed through its DeadlineQosPolicy) was respected for each instance.
     * The OfferedDeadlineMissedStatus can also be monitored using a DataWriterListener or
     * by using the associated StatusCondition.
     *
     * @return a OfferedDeadlineMissedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     org.omg.dds.core.status.OfferedDeadlineMissedStatus
     */
    public OfferedDeadlineMissedStatus getOfferedDeadlineMissedStatus();

    /**
     * This operation obtains the OfferedIncompatibleQosStatus object of the DataWriter.
     * This object contains the information whether a QosPolicy setting was incompatible
     * with the requested QosPolicy setting. This means that the status represents whether
     * a DataReader object has been discovered by the DataWriter with the same Topic and a
     * requested DataReaderQos that was incompatible with the one offered by the DataWriter.
     * The OfferedIncompatibleQosStatus can also be monitored using a DataWriterListener or
     * by using the associated StatusCondition.
     *
     * @return a OfferedIncompatibleQosStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     org.omg.dds.core.status.OfferedIncompatibleQosStatus
     */
    public OfferedIncompatibleQosStatus getOfferedIncompatibleQosStatus();

    /**
     * This operation obtains the PublicationMatchedStatus object of the DataWriter.
     * This object contains the information whether a new match has been discovered
     * for the current publication, or whether an existing match has ceased to exist.
     * <p>
     * This means that the status represents that either a DataReader object has been
     * discovered by the DataWriter with the same Topic and a compatible Qos, or that a
     * previously discovered DataReader has ceased to be matched to the current DataWriter.
     * A DataReader may cease to match when it gets deleted, when it changes its Qos to
     * a value that is incompatible with the current DataWriter or when either the DataWriter
     * or the DataReader has chosen to put its matching counterpart on its ignore-list using the
     * ignoreSubcription or ignorePublication operations on the DomainParticipant.
     * The operation may fail if the infrastructure does not hold the information necessary
     * to fill in the PublicationMatchedStatus. This is the case when OpenSplice is configured
     * not to maintain discovery information in the Networking Service. (See the description for the
     * NetworkingService/Discovery/enabled property in the Deployment Manual for more information
     * about this subject.) In this case the operation will throw an UnsupportedOperationException.
     * The PublicationMatchedStatus can also be monitored using a DataWriterListener or by using
     * the associated StatusCondition.
     *
     * @return a PublicationMatchedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws  UnsupportedOperationException   if the infrastructure does
     *          not hold the information necessary to fill in the publicationData.
     * @see     org.omg.dds.core.status.PublicationMatchedStatus
     */
    public PublicationMatchedStatus getPublicationMatchedStatus();

    /**
     * This operation manually asserts the liveliness of the DataWriter. This
     * is used in combination with the
     * {@link org.omg.dds.core.policy.Liveliness} to
     * indicate to the Service that the entity remains active.
     * <p>
     * This operation need only be used if
     * {@link org.omg.dds.core.policy.Liveliness#getKind()} is either
     * {@link org.omg.dds.core.policy.Liveliness.Kind#MANUAL_BY_PARTICIPANT}
     * or
     * {@link org.omg.dds.core.policy.Liveliness.Kind#MANUAL_BY_TOPIC}.
     * Otherwise, it has no effect.
     * <p>
     * <b>Note</b> - Writing data via {@link #write(Object)} asserts
     * liveliness on the DataWriter itself and its DomainParticipant.
     * Consequently the use of assertLiveliness is only needed if the
     * application is not writing data regularly.
     *
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     */
    public void assertLiveliness();

    /**
     * This operation retrieves the list of subscriptions currently
     * "associated" with the DataWriter; that is, subscriptions that have a
     * matching {@link org.omg.dds.topic.Topic} and compatible QoS that the application has not
     * indicated should be "ignored" by means of
     * {@link org.omg.dds.domain.DomainParticipant#ignoreSubscription(InstanceHandle)}.
     * <p>
     * The handles returned in the 'subscriptionHandles' list are the ones
     * that are used by the DDS implementation to locally identify the
     * corresponding matched {@link org.omg.dds.sub.DataReader} entities. These handles match
     * the ones that appear in {@link org.omg.dds.sub.Sample#getInstanceHandle()} when
     * reading the "DCPSSubscriptions" built-in topic.
     * <p>
     * Be aware that since an instance handle is an opaque datatype, it does not necessarily
     * mean that the handles obtained from the getMatchedSubscriptions operation have the same
     * value as the ones that appear in the instanceHandle field of the SampleInfo when retrieving
     * the subscription info through corresponding "DCPSSubscriptions" built-in reader. You can't
     * just compare two handles to determine whether they represent the same subscription. If you want
     * to know whether two handles actually do represent the same subscription, use both handles to
     * retrieve their corresponding SubscriptionBuiltinTopicData samples and then compare the key field
     * of both samples.
     * <p>
     * The operation may fail with an UnsupportedOperationException if the infrastructure
     * does not locally maintain the connectivity information.
     *
     * @return  a new collection containing a copy of the information.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws  UnsupportedOperationException   if the infrastructure does
     *          not hold the information necessary to fill in the
     *          subscriptionData.
     * @see     #getMatchedSubscriptionData(InstanceHandle)
     */
    public Set<InstanceHandle> getMatchedSubscriptions();

    /**
     * This operation retrieves information on a subscription that is
     * currently "associated" with the DataWriter; that is, a subscription
     * with a matching {@link org.omg.dds.topic.Topic} and compatible QoS that the application
     * has not indicated should be "ignored" by means of
     * {@link org.omg.dds.domain.DomainParticipant#ignoreSubscription(InstanceHandle)}.
     * <p>
     * The operation {@link #getMatchedSubscriptions()} can be used
     * to find the subscriptions that are currently matched with the
     * DataWriter.
     * <p>
     * The operation may also fail if the infrastructure does not hold the information
     * necessary to fill in the subscription_data. This is the case when OpenSplice is
     * configured not to maintain discovery information in the Networking Service. (See
     * the description for the NetworkingService/Discovery/enabled property in
     * the Deployment Manual for more information about this subject.)  In such cases the
     * operation will throw an UnsupportedOperationException.
     *
     * @param   subscriptionHandle      a handle to the subscription, the
     *          data of which is to be retrieved.
     *
     * @return  a new SubscriptionBuiltinTopicData object containing a copy of the information.
     *
     * @throws  IllegalArgumentException        if subscriptionHandle does
     *          not correspond to a subscription currently associated with
     *          the DataWriter.
     * @throws  UnsupportedOperationException   if the infrastructure does
     *          not hold the information necessary to fill in the
     *          subscriptionData.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #getMatchedSubscriptions()
     */
    public SubscriptionBuiltinTopicData getMatchedSubscriptionData(
            InstanceHandle subscriptionHandle);


    // --- Type-specific interface: ------------------------------------------

    /**
     * This operation informs the Service that the application will be modifying
     * a particular instance. It gives an opportunity to the Service to
     * pre-configure itself to improve performance.
     * <p>
     * It takes as a parameter an instance (to get the key value) and returns a
     * handle that can be used in successive {@link #write(Object)} or
     * {@link #dispose(InstanceHandle)} operations.
     * <p>
     * This operation should be invoked prior to calling any operation that
     * modifies the instance.
     * <p>
     * A nil handle may be returned by the Service if it does not want to
     * allocate any handle for that instance.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for the {@link #write(Object)}.
     * <p>
     * The operation is idempotent. If it is called for an already registered
     * instance, it just returns the already allocated handle. This may be used
     * to lookup and retrieve the handle allocated to a given instance. The
     * explicit use of this operation is optional as the application may call
     * directly the write operation and specify a nil handle to indicate that
     * the 'key' should be examined to identify the instance.
     * <p>
     * <p><b>Blocking</b><p>
     * If the HistoryQosPolicy is set to KEEP_ALL, the registerInstance operation
     * on the DataWriter may block if the modification would cause data to be lost
     * because one of the limits, specified in the ResourceLimitsQosPolicy, to be
     * exceeded. In case the synchronous attribute value of the ReliabilityQosPolicy
     * is set to TRUE for communicating DataWriters and DataReaders then the DataWriter
     * will wait until all synchronous DataReaders have acknowledged the data. Under
     * these circumstances, the maxBlockingTime attribute of the ReliabilityQosPolicy
     * configures the maximum time the registerInstance operation may block (either
     * waiting for space to become available or data to be acknowledged).
     * If maxBlockingTime elapses before the DataWriter is able to store the modification
     * without exceeding the limits and all expected acknowledgments are received, the
     * registerInstance operation will fail and return a nilHandle.
     *
     * @return  A handle to the Instance, which may be used for writing and disposing of.
     *          In case of an error, a nilHandle is returned.
     * @param instanceData  The instance, which the application writes to or disposes of.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances described for
     *             {@link #write(Object)}.
     * @throws TimeoutException
     *             under the same circumstances described for
     *             {@link #write(Object)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #registerInstance(Object, Time)
     * @see #registerInstance(Object, long, TimeUnit)
     * @see #unregisterInstance(InstanceHandle)
     * @see #unregisterInstance(InstanceHandle, Object)
     * @see InstanceHandle#nilHandle(org.omg.dds.core.ServiceEnvironment)
     */
    public InstanceHandle registerInstance(
            TYPE instanceData) throws TimeoutException;

    /**
     * This operation performs the same function as
     * {@link #registerInstance(Object)} and can be used instead in the cases
     * where the application desires to specify the value for the source time
     * stamp. The source time stamp potentially affects the relative order in
     * which readers observe events from multiple writers. For details see
     * {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for the {@link #write(Object)}.
     * @return  A handle to the Instance, which may be used for writing and disposing of.
     *          In case of an error, a nilHandle is returned.
     * @param instanceData      The instance, which the application writes to or disposes of.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances described for
     *             {@link #write(Object)}.
     * @throws TimeoutException
     *             under the same circumstances described for
     *             {@link #write(Object)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #registerInstance(Object)
     * @see #registerInstance(Object, long, TimeUnit)
     * @see #unregisterInstance(InstanceHandle, Object, Time)
     */
    public InstanceHandle registerInstance(
            TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException;

    /**
     * This operation performs the same function as
     * {@link #registerInstance(Object)} and can be used instead in the cases
     * where the application desires to specify the value for the source time
     * stamp. The source time stamp potentially affects the relative order in
     * which readers observe events from multiple writers. For details see
     * {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for the {@link #write(Object)}.
     * @return  A handle to the Instance, which may be used for writing and disposing of.
     *          In case of an error, a nilHandle is returned.
     * @param instanceData      The instance, which the application writes to or disposes of.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances described for
     *             {@link #write(Object)}.
     * @throws TimeoutException
     *             under the same circumstances described for
     *             {@link #write(Object)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #registerInstance(Object)
     * @see #registerInstance(Object, Time)
     * @see #unregisterInstance(InstanceHandle, Object, long, TimeUnit)
     */
    public InstanceHandle registerInstance(
            TYPE instanceData,
            long sourceTimestamp,
            TimeUnit unit) throws TimeoutException;

    /**
     * This operation reverses the action of
     * {@link #registerInstance(Object)}. It should only be called on an
     * instance that is currently registered.
     * <p>
     * The operation should be called just once per instance, regardless of
     * how many times {@link #registerInstance(Object)} was called for that
     * instance.
     * <p>
     * This operation informs the Service that the DataWriter is not
     * intending to modify that data instance any more. This operation also
     * indicates that the Service can locally remove all information
     * regarding that instance. The application should not attempt to use the
     * handle previously allocated to that instance after calling
     * {@link #unregisterInstance(InstanceHandle)}.
     * <p>
     * If handle is any value other than nil, then it must correspond to the
     * value returned by {@link #registerInstance(Object)} when the instance
     * (identified by its key) was registered.
     * <p>
     * If after that, the application wants to modify (write or dispose) the
     * instance, it has to register it again, or else use a nil handle.
     * <p>
     * This operation does not indicate that the instance is deleted (that is
     * the purpose of dispose). The operation just indicates that the
     * DataWriter no longer has 'anything to say' about the instance.
     * DataReader entities that are reading the instance will eventually
     * receive a sample with a {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS}
     * instance state if no other DataWriter entities are writing the
     * instance.
     * <p>
     * This operation can affect the ownership of the data instance (see
     * {@link org.omg.dds.core.policy.Ownership}). If the DataWriter was the exclusive owner
     * of the instance, then calling this method will relinquish that
     * ownership.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under
     * the same circumstances described for the {@link #write(Object)}.
     * @param handle      The handle to the Instance, which has been used for writing and disposing.
     * @throws  IllegalArgumentException    if the handle does not correspond
     *          to an existing instance, and if this situation is detectable
     *          by the Service implementation. If the situation is not
     *          detectable, the behavior is unspecified.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *                  The handle has not been registered with this DataWriter.
     * @see     #unregisterInstance(InstanceHandle, Object)
     * @see     #unregisterInstance(InstanceHandle, Object, Time)
     * @see     #unregisterInstance(InstanceHandle, Object, long, TimeUnit)
     * @see     #registerInstance(Object)
     */
    public void unregisterInstance(
            InstanceHandle handle) throws TimeoutException;

    /**
     * This operation reverses the action of {@link #registerInstance(Object)}.
     * It should only be called on an instance that is currently registered.
     * <p>
     * The operation should be called just once per instance, regardless of how
     * many times {@link #registerInstance(Object)} was called for that
     * instance.
     * <p>
     * This operation informs the Service that the DataWriter is not intending
     * to modify that data instance any more. This operation also indicates that
     * the Service can locally remove all information regarding that instance.
     * The application should not attempt to use the handle previously allocated
     * to that instance after calling
     * {@link #unregisterInstance(InstanceHandle)}.
     * <p>
     * A nil handle can be used for the parameter handle. This indicates that
     * the identity of the instance should be automatically deduced from the
     * instance data (by means of the key).
     * <p>
     * If handle is any value other than nil, then it must correspond to the
     * value returned by {@link #registerInstance(Object)} when the instance
     * (identified by its key) was registered.
     * <p>
     * If after that, the application wants to modify (write or dispose) the
     * instance, it has to register it again, or else use a nil handle.
     * <p>
     * This operation does not indicate that the instance is deleted (that is
     * the purpose of dispose). The operation just indicates that the DataWriter
     * no longer has 'anything to say' about the instance. DataReader entities
     * that are reading the instance will eventually receive a sample with a
     * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS} instance state
     * if no other DataWriter entities are writing the instance.
     * <p>
     * This operation can affect the ownership of the data instance (see
     * {@link org.omg.dds.core.policy.Ownership}). If the DataWriter was the
     * exclusive owner of the instance, then calling this method will relinquish
     * that ownership.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for the {@link #write(Object)}.
     * @param handle            The handle to the Instance, which has been used for writing and disposing.
     * @param instanceData      The instance, which the application writes to or disposes of.
     * @throws IllegalArgumentException
     *             if the handle does not correspond to an existing instance,
     *             and if this situation is detectable by the Service
     *             implementation. If the situation is not detectable, the
     *             behavior is unspecified.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the handle corresponds to an existing instance but does
     *             not correspond to the same instance referred by the
     *             instancData parameter, and if this situation is detectable by
     *             the Service implementation If the situation is not
     *             detectable, the behavior is unspecified.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #unregisterInstance(InstanceHandle)
     * @see #unregisterInstance(InstanceHandle, Object, Time)
     * @see #unregisterInstance(InstanceHandle, Object, long, TimeUnit)
     * @see #registerInstance(Object)
     * @see InstanceHandle#nilHandle(org.omg.dds.core.ServiceEnvironment)
     */
    public void unregisterInstance(
            InstanceHandle handle,
            TYPE instanceData) throws TimeoutException;

    /**
     * This operation performs the same function as
     * {@link #unregisterInstance(InstanceHandle, Object)} and can be used
     * instead in the cases where the application desires to specify the
     * value for the source time stamp. The source time stamp potentially
     * affects the relative order in which readers observe events from
     * multiple writers. For details see {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * The constraints on the values of the handle parameter and the
     * corresponding error behavior are the same specified for the
     * {@link #unregisterInstance(InstanceHandle, Object)} operation.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under
     * the same circumstances described for the {@link #write(Object)}.
     * @param handle            The handle to the Instance, which has been used for writing and disposing.
     * @param instanceData      The instance, which the application writes to or disposes of.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @throws IllegalArgumentException
     *             if the handle does not correspond to an existing instance,
     *             and if this situation is detectable by the Service
     *             implementation. If the situation is not detectable, the
     *             behavior is unspecified.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the handle corresponds to an existing instance but does
     *             not correspond to the same instance referred by the
     *             instancData parameter, and if this situation is detectable by
     *             the Service implementation If the situation is not
     *             detectable, the behavior is unspecified.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see     #unregisterInstance(InstanceHandle)
     * @see     #unregisterInstance(InstanceHandle, Object)
     * @see     #unregisterInstance(InstanceHandle, Object, long, TimeUnit)
     * @see     #registerInstance(Object, Time)
     */
    public void unregisterInstance(
            InstanceHandle handle,
            TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException;

    /**
     * This operation performs the same function as
     * {@link #unregisterInstance(InstanceHandle, Object)} and can be used
     * instead in the cases where the application desires to specify the
     * value for the source time stamp. The source time stamp potentially
     * affects the relative order in which readers observe events from
     * multiple writers. For details see {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * The constraints on the values of the handle parameter and the
     * corresponding error behavior are the same specified for the
     * {@link #unregisterInstance(InstanceHandle, Object)} operation.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under
     * the same circumstances described for the {@link #write(Object)}.
     * @param instanceData      The instance, which the application writes to or disposes of.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws IllegalArgumentException
     *             if the handle does not correspond to an existing instance,
     *             and if this situation is detectable by the Service
     *             implementation. If the situation is not detectable, the
     *             behavior is unspecified.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the handle corresponds to an existing instance but does
     *             not correspond to the same instance referred by the
     *             instancData parameter, and if this situation is detectable by
     *             the Service implementation If the situation is not
     *             detectable, the behavior is unspecified.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see     #unregisterInstance(InstanceHandle)
     * @see     #unregisterInstance(InstanceHandle, Object)
     * @see     #unregisterInstance(InstanceHandle, Object, Time)
     * @see     #registerInstance(Object, long, TimeUnit)
     */
    public void unregisterInstance(
            InstanceHandle handle,
            TYPE instanceData,
            long sourceTimestamp,
            TimeUnit unit) throws TimeoutException;

    /**
     * This operation modifies the value of a data instance. When this operation
     * is used, the Service will automatically supply the value of the source
     * time stamp that is made available to {@link org.omg.dds.sub.DataReader}
     * objects by means of {@link org.omg.dds.sub.Sample#getSourceTimestamp()}.
     * See also {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * As a side effect, this operation asserts liveliness on the DataWriter
     * itself, the {@link org.omg.dds.pub.Publisher} and the
     * {@link org.omg.dds.domain.DomainParticipant}.
     * <p>
     * If {@link org.omg.dds.core.policy.Reliability#getKind()} kind is set to
     * {@link org.omg.dds.core.policy.Reliability.Kind#RELIABLE}, the operation
     * may block if the modification would cause data to be lost or else cause
     * one of the limits specified in
     * {@link org.omg.dds.core.policy.ResourceLimits} to be exceeded. Under
     * these circumstances,
     * {@link org.omg.dds.core.policy.Reliability#getMaxBlockingTime()}
     * configures the maximum time the operation may block waiting for space to
     * become available. If this duration elapses before the DataWriter is able
     * to store the modification without exceeding the limits, the operation
     * will fail with {@link TimeoutException}.
     * <p>
     * Specifically, the DataWriter write operation may block in the following
     * situations (note that the list may not be exhaustive), even if
     * {@link org.omg.dds.core.policy.History#getKind()} is
     * {@link org.omg.dds.core.policy.History.Kind#KEEP_LAST}.
     *
     * <ul>
     * <li>If ({@link org.omg.dds.core.policy.ResourceLimits#getMaxSamples()}
     * &lt; {@link org.omg.dds.core.policy.ResourceLimits#getMaxInstances()} *
     * {@link org.omg.dds.core.policy.History#getDepth()}), then in the
     * situation where the max samples resource limit is exhausted the Service
     * is allowed to discard samples of some other instance as long as at least
     * one sample remains for such an instance. If it is still not possible to
     * make space available to store the modification, the writer is allowed to
     * block.</li>
     * <li>If ({@link org.omg.dds.core.policy.ResourceLimits#getMaxSamples()}
     * &lt; {@link org.omg.dds.core.policy.ResourceLimits#getMaxInstances()}),
     * then the DataWriter may block regardless of the HISTORY depth.</li>
     * </ul>
     * <p>
     * Instead of blocking, the operation is allowed to fail immediately with
     * {@link org.omg.dds.core.OutOfResourcesException} provided the following
     * two conditions are met:
     *
     * <ol>
     * <li>The reason for blocking would be that the RESOURCE_LIMITS are
     * exceeded.</li>
     * <li>The service determines that waiting the max blocking time has no
     * chance of freeing the necessary resources. For example, if the only way
     * to gain the necessary resources would be for the user to unregister an
     * instance.</li>
     * </ol>
     * @param instanceData  The data to be written
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws IllegalArgumentException
     *             if the handle is not a valid sample.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             If the handle has not been registered with this DataWriter
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #write(Object, InstanceHandle)
     * @see #write(Object, InstanceHandle, Time)
     * @see #write(Object, InstanceHandle, long, TimeUnit)
     */
    public void write(
            TYPE instanceData) throws TimeoutException;
    /**
     * This operation performs the same functions as {@link #write(Object)} except that the application
     * provides the value for the parameter sourceTimestamp that is made available to
     * DataReader objects. This timestamp is important for the interpretation of the
     * DestinationOrderQosPolicy.

     * @param instanceData  The data to be written
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws IllegalArgumentException
     *             if the handle is not a valid sample.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             If the handle has not been registered with this DataWriter
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #write(Object, InstanceHandle)
     * @see #write(Object, InstanceHandle, Time)
     * @see #write(Object, InstanceHandle, long, TimeUnit)
     */
    public void write(
            TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException;
    /**
     * This operation performs the same functions as {@link #write(Object)} except that the application
     * provides the value for the parameter sourceTimestamp that is made available to
     * DataReader objects. This timestamp is important for the interpretation of the
     * DestinationOrderQosPolicy.
     * @param instanceData  The data to be written
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws TimeoutException
     *             Either the current action overflowed the available resources
     *             as specified by the combination of the ReliablityQosPolicy,
     *             HistoryQosPolicy and ResourceLimitsQosPolicy, or the current action
     *             was waiting for data delivery acknowledgment by synchronous DataReaders.
     *             This caused blocking of the unregisterInstance operation, which could not
     *             be resolved before maxBlockingTime of the ReliabilityQosPolicy elapsed.
     * @throws IllegalArgumentException
     *             if the handle is not a valid sample.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             If the handle has not been registered with this DataWriter
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #write(Object, InstanceHandle)
     * @see #write(Object, InstanceHandle, Time)
     * @see #write(Object, InstanceHandle, long, TimeUnit)
     */
    public void write(
            TYPE instanceData,
            long sourceTimestamp,
            TimeUnit unit) throws TimeoutException;

    /**
     * This operation modifies the value of a data instance. When this operation
     * is used, the Service will automatically supply the value of the source
     * time stamp that is made available to {@link org.omg.dds.sub.DataReader}
     * objects by means of {@link org.omg.dds.sub.Sample#getSourceTimestamp()}.
     * See also {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * As a side effect, this operation asserts liveliness on the DataWriter
     * itself, the {@link org.omg.dds.pub.Publisher} and the
     * {@link org.omg.dds.domain.DomainParticipant}.
     * <p>
     * A nil handle can be used for the parameter handle. This indicates that
     * the identity of the instance should be automatically deduced from the
     * instanceData (by means of the key). If handle is not nil, then it must
     * correspond to the value returned by {@link #registerInstance(Object)}
     * when the instance (identified by its key) was registered.
     * <p>
     * If {@link org.omg.dds.core.policy.Reliability#getKind()} kind is set to
     * {@link org.omg.dds.core.policy.Reliability.Kind#RELIABLE}, the operation
     * may block if the modification would cause data to be lost or else cause
     * one of the limits specified in
     * {@link org.omg.dds.core.policy.ResourceLimits} to be exceeded. Under
     * these circumstances,
     * {@link org.omg.dds.core.policy.Reliability#getMaxBlockingTime()}
     * configures the maximum time the operation may block waiting for space to
     * become available. If this duration elapses before the DataWriter is able
     * to store the modification without exceeding the limits, the operation
     * will fail with {@link TimeoutException}.
     * <p>
     * Specifically, the DataWriter write operation may block in the following
     * situations (note that the list may not be exhaustive), even if
     * {@link org.omg.dds.core.policy.History#getKind()} is
     * {@link org.omg.dds.core.policy.History.Kind#KEEP_LAST}.
     *
     * <ul>
     * <li>If ({@link org.omg.dds.core.policy.ResourceLimits#getMaxSamples()}
     * &lt; {@link org.omg.dds.core.policy.ResourceLimits#getMaxInstances()} *
     * {@link org.omg.dds.core.policy.History#getDepth()}), then in the
     * situation where the max samples resource limit is exhausted the Service
     * is allowed to discard samples of some other instance as long as at least
     * one sample remains for such an instance. If it is still not possible to
     * make space available to store the modification, the writer is allowed to
     * block.</li>
     * <li>If ({@link org.omg.dds.core.policy.ResourceLimits#getMaxSamples()}
     * &lt; {@link org.omg.dds.core.policy.ResourceLimits#getMaxInstances()}),
     * then the DataWriter may block regardless of the HISTORY depth.</li>
     * </ul>
     *
     * Instead of blocking, the operation is allowed to fail immediately with
     * {@link org.omg.dds.core.OutOfResourcesException} provided the following
     * two conditions are met:
     *
     * <ol>
     * <li>The reason for blocking would be that the RESOURCE_LIMITS are
     * exceeded.</li>
     * <li>The service determines that waiting the max blocking time has no
     * chance of freeing the necessary resources. For example, if the only way
     * to gain the necessary resources would be for the user to unregister an
     * instance.</li>
     * </ol>
     *
     * @param instanceData  The data to be written
     * @param handle        the handle to the instance as supplied by registerInstance.
     * @throws IllegalArgumentException
     *             if the handle does not correspond to an existing instance,
     *             and if this situation is detectable by the Service
     *             implementation. If the situation is not detectable, the
     *             behavior is unspecified.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the handle corresponds to an existing instance but does
     *             not correspond to the same instance referred by the
     *             instancData parameter, and if this situation is detectable by
     *             the Service implementation If the situation is not
     *             detectable, the behavior is unspecified.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws TimeoutException
     *             if the configured maximum time elapses and the DataWriter is
     *             still unable to store the new sample without exceeding its
     *             configured resource limits.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #write(Object)
     * @see #write(Object, InstanceHandle, Time)
     * @see #write(Object, InstanceHandle, long, TimeUnit)
     * @see InstanceHandle#nilHandle(org.omg.dds.core.ServiceEnvironment)
     */
    public void write(
            TYPE instanceData,
            InstanceHandle handle) throws TimeoutException;

    /**
     * This operation performs the same function as
     * {@link #write(Object, InstanceHandle)} except that it also provides the
     * value for the source time stamp that is made available to
     * {@link org.omg.dds.sub.DataReader} objects by means of
     * {@link org.omg.dds.sub.Sample#getSourceTimestamp()}. See also
     * {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * The constraints on the values of the handle parameter and the
     * corresponding error behavior are the same specified for
     * {@link #write(Object, InstanceHandle)}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for {@link #write(Object, InstanceHandle)}.
     * @param instanceData  The data to be written
     * @param handle        the handle to the instance as supplied by registerInstance.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @throws IllegalArgumentException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws TimeoutException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #write(Object)
     * @see #write(Object, InstanceHandle)
     * @see #write(Object, InstanceHandle, long, TimeUnit)
     */
    public void write(
            TYPE instanceData,
            InstanceHandle handle,
            Time sourceTimestamp) throws TimeoutException;

    /**
     * This operation performs the same function as
     * {@link #write(Object, InstanceHandle)} except that it also provides the
     * value for the source time stamp that is made available to
     * {@link org.omg.dds.sub.DataReader} objects by means of
     * {@link org.omg.dds.sub.Sample#getSourceTimestamp()}. See also
     * {@link org.omg.dds.core.policy.DestinationOrder}.
     * <p>
     * The constraints on the values of the handle parameter and the
     * corresponding error behavior are the same specified for
     * {@link #write(Object, InstanceHandle)}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for {@link #write(Object, InstanceHandle)}.
     * @param instanceData  The data to be written
     * @param handle        the handle to the instance as supplied by registerInstance.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws IllegalArgumentException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws TimeoutException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #write(Object)
     * @see #write(Object, InstanceHandle)
     * @see #write(Object, InstanceHandle, Time)
     */
    public void write(
            TYPE instanceData,
            InstanceHandle handle,
            long sourceTimestamp,
            TimeUnit unit) throws TimeoutException;

    /**
     * This operation requests the middleware to delete the data (the actual
     * deletion is postponed until there is no more use for that data in the
     * whole system). In general, applications are made aware of the deletion by
     * means of operations on the {@link org.omg.dds.sub.DataReader} objects
     * that already knew that instance. DataReader objects that didn't know the
     * instance will never see it.
     * <p>
     * When this operation is used, the Service will automatically supply the
     * value of the source time stamp that is made available to DataReader
     * objects by means of {@link org.omg.dds.sub.Sample#getSourceTimestamp()}.
     * <p>
     * The constraints on the values of the instanceHandle parameter and the
     * corresponding error behavior are the same specified for
     * {@link #unregisterInstance(InstanceHandle)}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for {@link #write(Object)}.
     *
     * @param instanceHandle    The handle to the instance that needs to be disposed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances as {@link #write(Object)}.
     * @throws TimeoutException
     *             under the same circumstances as {@link #write(Object)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #dispose(InstanceHandle, Object)
     * @see #dispose(InstanceHandle, Object, Time)
     * @see #dispose(InstanceHandle, Object, long, TimeUnit)
     */
    public void dispose(
            InstanceHandle instanceHandle) throws TimeoutException;

    /**
     * This operation requests the middleware to delete the data (the actual
     * deletion is postponed until there is no more use for that data in the
     * whole system). In general, applications are made aware of the deletion by
     * means of operations on the {@link org.omg.dds.sub.DataReader} objects
     * that already knew that instance. DataReader objects that didn't know the
     * instance will never see it.
     * <p>
     * This operation does not modify the value of the instance. The
     * instanceData parameter is passed just for the purposes of identifying the
     * instance.
     * <p>
     * When this operation is used, the Service will automatically supply the
     * value of the source time stamp that is made available to DataReader
     * objects by means of {@link org.omg.dds.sub.Sample#getSourceTimestamp()}.
     * <p>
     * The constraints on the values of the instanceHandle parameter and the
     * corresponding error behavior are the same specified for
     * {@link #unregisterInstance(InstanceHandle, Object)}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for {@link #write(Object, InstanceHandle)}.
     * @param instanceHandle    The handle to the instance that needs to be disposed.
     * @param instanceData      The actual instance to be disposed of.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws TimeoutException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #dispose(InstanceHandle)
     * @see #dispose(InstanceHandle, Object, Time)
     * @see #dispose(InstanceHandle, Object, long, TimeUnit)
     */
    public void dispose(
            InstanceHandle instanceHandle,
            TYPE instanceData) throws TimeoutException;

    /**
     * This operation performs the same functions as
     * {@link #dispose(InstanceHandle, Object)} except that the application
     * provides the value for the source time stamp that is made available to
     * {@link org.omg.dds.sub.DataReader} objects by means of
     * {@link org.omg.dds.sub.Sample#getSourceTimestamp()}.
     * <p>
     * The constraints on the values of the instanceHandle parameter and the
     * corresponding error behavior are the same specified for
     * {@link #dispose(InstanceHandle, Object)}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for {@link #write(Object, InstanceHandle)}.
     * @param instanceHandle    The handle to the instance that needs to be disposed.
     * @param instanceData      The actual instance to be disposed of.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     *
     * @throws IllegalArgumentException
     *             under the same circumstances as
     *             {@link #dispose(InstanceHandle, Object)}.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             under the same circumstances as
     *             {@link #dispose(InstanceHandle, Object)}.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws TimeoutException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #dispose(InstanceHandle)
     * @see #dispose(InstanceHandle, Object)
     * @see #dispose(InstanceHandle, Object, long, TimeUnit)
     */
    public void dispose(
            InstanceHandle instanceHandle,
            TYPE instanceData,
            Time sourceTimestamp) throws TimeoutException;

    /**
     * This operation performs the same functions as
     * {@link #dispose(InstanceHandle, Object)} except that the application
     * provides the value for the source time stamp that is made available to
     * {@link org.omg.dds.sub.DataReader} objects by means of
     * {@link org.omg.dds.sub.Sample#getSourceTimestamp()}.
     * <p>
     * The constraints on the values of the instanceHandle parameter and the
     * corresponding error behavior are the same specified for
     * {@link #dispose(InstanceHandle, Object)}.
     * <p>
     * This operation may block and exit with {@link TimeoutException} under the
     * same circumstances described for {@link #write(Object, InstanceHandle)}.
     * @param instanceHandle    The handle to the instance that needs to be disposed.
     * @param instanceData      The actual instance to be disposed of.
     * @param sourceTimestamp   The timestamp which is provided for the DataReader.
     *                          This timestamp is important for the interpretation of the DestinationOrderQosPolicy.
     * @param unit              The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws IllegalArgumentException
     *             under the same circumstances as
     *             {@link #dispose(InstanceHandle, Object)}.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             under the same circumstances as
     *             {@link #dispose(InstanceHandle, Object)}.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws TimeoutException
     *             under the same circumstances as
     *             {@link #write(Object, InstanceHandle)}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @see #dispose(InstanceHandle)
     * @see #dispose(InstanceHandle, Object)
     * @see #dispose(InstanceHandle, Object, Time)
     */
    public void dispose(
            InstanceHandle instanceHandle,
            TYPE instanceData,
            long sourceTimestamp,
            TimeUnit unit) throws TimeoutException;

    /**
     * This operation can be used to retrieve the instance key that
     * corresponds to an instance handle. The operation will only fill the
     * fields that form the key inside the keyHolder instance.
     *
     * @param   keyHolder       a container, into which this method shall
     *          place its result.
     * @param   handle          a handle indicating the instance whose value
     *          this method should get.
     *
     * @return  keyHolder, if it is non-null, or a new object otherwise.
     *
     * @throws  IllegalArgumentException    if the handle does not correspond
     *          to an existing data object known to the DataWriter. If the
     *          implementation is not able to check invalid handles, then the
     *          result in this situation is unspecified.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *                  this instance is not registered
     * @see     #getKeyValue(InstanceHandle)
     */
    public TYPE getKeyValue(TYPE keyHolder, InstanceHandle handle);

    /**
     * This operation can be used to retrieve the instance key that
     * corresponds to an instance handle. The operation will only fill the
     * fields that form the key inside the keyHolder instance.
     *
     * @param   handle          a handle indicating the instance whose value
     *          this method should get.
     *
     * @return  A new "key holder" object. The contents of the non-key fields
     *          are unspecified.
     *
     * @throws  IllegalArgumentException    if the handle does not correspond
     *          to an existing data object known to the DataWriter. If the
     *          implementation is not able to check invalid handles, then the
     *          result in this situation is unspecified.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataWriter has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *                  this instance is not registered
     * @see     #getKeyValue(Object, InstanceHandle)
     */
    public TYPE getKeyValue(InstanceHandle handle);

    /**
     * This operation takes as a parameter an instance and returns a handle
     * that can be used in subsequent operations that accept an instance
     * handle as an argument. The instance parameter is only used for the
     * purpose of examining the fields that define the key.
     * <p>
     * This operation does not register the instance in question. If the
     * instance has not been previously registered, or if for any other
     * reason the Service is unable to provide an instance handle, the
     * Service will return a nil handle.
     *
     * @param   keyHolder       a sample of the instance whose handle this
     *          method should look up.
     *
     * @return  an immutable handle to the instance.
     *
     */
    public InstanceHandle lookupInstance(TYPE keyHolder);


    // --- From Entity: ------------------------------------------------------
    @Override
    public StatusCondition<DataWriter<TYPE>> getStatusCondition();

    @Override
    public Publisher getParent();
}
