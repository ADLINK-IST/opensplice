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

package org.omg.dds.sub;

import java.util.Collection;
import java.util.Set;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.DomainEntity;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.topic.TopicDescription;
import org.omg.dds.topic.TopicQos;

/**
 * A Subscriber is the object responsible for the actual reception of the data
 * resulting from its subscriptions.
 * <p>
 * A Subscriber acts on the behalf of one or several
 * {@link org.omg.dds.sub.DataReader} objects that are related to it. When it
 * receives data (from the other parts of the system), it builds the list of
 * concerned DataReader objects, and then indicates to the application that data
 * is available, through its listener or by enabling related conditions. The
 * application can access the list of concerned DataReader objects through the
 * operation {@link #getDataReaders()} and then access the data available
 * through operations on the DataReaders.
 * <p>
 * All operations except for the inherited operations
 * {@link org.omg.dds.core.Entity#setQos(org.omg.dds.core.EntityQos)},
 * {@link org.omg.dds.core.Entity#getQos()},
 * {@link org.omg.dds.core.Entity#setListener(java.util.EventListener)},
 * {@link org.omg.dds.core.Entity#getListener()},
 * {@link org.omg.dds.core.Entity#enable()},
 * {@link org.omg.dds.core.Entity#getStatusCondition()}, and
 * {@link #createDataReader(TopicDescription)} may fail with the exception
 * {@link org.omg.dds.core.NotEnabledException}.
 * <p>
 * <pre>
 * <b><i>Example</i></b>
 * <code>
 * This example assumes a DomainParticipant named "participant" has been created
 * and a datatype called Foo is present: class Foo { int myKey; String myName; };
 *
 * // Create a topic named "FooTopic" using the default QoS and no Listener.
 * Topic&lt;Foo&gt; fooTopic = participant.createTopic("FooTopic", Foo.class);
 *
 * // Create a Subscriber using default Qos and no Listener.
 * Subscriber mySub = participant.createSubscriber();
 * </code>
 * </pre>
 */
public interface Subscriber
extends DomainEntity<SubscriberListener, SubscriberQos>
{
    // --- Create (any) DataReader: ------------------------------------------

    /**
     * This operation creates a {@link org.omg.dds.sub.DataReader}. The returned DataReader
     * will be attached and belong to the Subscriber.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataReader is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataReader QoS by means of
     *         {@link #getDefaultDataReaderQos()}.
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired.</li>
     *     <li>Use the resulting QoS policies to construct the DataReader.
     * </ul>
     *
     * The {@link org.omg.dds.topic.TopicDescription} passed to this operation must have been
     * created from the same {@link org.omg.dds.domain.DomainParticipant} that was used to
     * create this Subscriber. If the TopicDescription was created from a
     * different DomainParticipant, the operation will fail.
     * @param topic A reference to the topic for which the DataReader is created.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see     #createDataReader(TopicDescription, DataReaderQos, DataReaderListener, Collection)
     */
    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic);

    /**
     * This operation creates a {@link org.omg.dds.sub.DataReader}. The returned DataReader
     * will be attached and belong to the Subscriber.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataReader is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataReader QoS by means of
     *         {@link #getDefaultDataReaderQos()}.
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired.</li>
     *     <li>Use the resulting QoS policies to construct the DataReader.
     * </ul>
     * For each communication status, the StatusChangedFlag flag is initially set to false.
     * It becomes true whenever that communication status changes. For each communication status
     * activated in the mask, the associated {@link org.omg.dds.sub.DataReaderListener} operation is
     * invoked and the communication status is reset to false, as the listener implicitly accesses the
     * status which is passed as a parameter to that operation. The status is reset prior to calling
     * the listener, so if the application calls the get&lt;status_name&gt;Status from inside the listener
     * it will see the status already reset. The following statuses are applicable to the DataReaderListener:
     * <ul>
     * <li>RequestedDeadlineMissedStatus</li>
     * <li>RequestedIncompatibleQosStatus</li>
     * <li>SampleRejectedStatus</li>
     * <li>LivelinessChangedStatus</li>
     * <li>DataAvailableStatus</li>
     * <li>SubscriptionMatchedStatus</li>
     * <li>SampleLostStatus</li>
     * </ul>
     * <p>
     * Status Propagation:<p>
     * In case a communication status is not activated in the statuses of the DataReaderListener,
     * the PublisherListener of the containing Subscriber is invoked (if attached and activated for the status that occurred).
     * This allows the application to set a default behaviour in the SubscriberListener of the containing Subscriber and a DataReader
     * specific behaviour when needed. In case the communication status is not activated in the statuses of the SubscriberListener as well,
     * the communication status will be propagated to the DomainParticipantListener of the containing DomainParticipant. In case the
     * DomainParticipantListener is also not attached or the communication status is not activated in its mask, the application is
     * not notified of the change.
     * <p>
     * The {@link org.omg.dds.topic.TopicDescription} passed to this operation must have been
     * created from the same {@link org.omg.dds.domain.DomainParticipant} that was used to
     * create this Subscriber. If the TopicDescription was created from a
     * different DomainParticipant, the operation will fail.
     *
     * @return A DataReader object.
     * @param topic     A reference to the topic for which the DataReader is created.
     * @param qos       The DataReaderQos for the new DataReader. In case these settings
     *                  are not self consistent, no DataReader is created.
     * @param listener  A reference to the DataReaderListener instance which will be attached to the new DataReader.
     * @param statuses  A collection of statuses the listener should be notified of.
     *                  A null collection signifies all status changes.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *                  The parameter qos contains conflicting QosPolicy settings.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see     #createDataReader(TopicDescription)
     */
    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic,
            DataReaderQos qos,
            DataReaderListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses);

    /**
     * This operation creates a {@link org.omg.dds.sub.DataReader}. The returned DataReader
     * will be attached and belong to the Subscriber.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataReader is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataReader QoS by means of
     *         {@link #getDefaultDataReaderQos()}.
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired.</li>
     *     <li>Use the resulting QoS policies to construct the DataReader.
     * </ul>
     * For each communication status, the StatusChangedFlag flag is initially set to false.
     * It becomes true whenever that communication status changes. For each communication status
     * activated in the mask, the associated {@link org.omg.dds.sub.DataReaderListener} operation is
     * invoked and the communication status is reset to false, as the listener implicitly accesses the
     * status which is passed as a parameter to that operation. The status is reset prior to calling
     * the listener, so if the application calls the get&lt;status_name&gt;Status from inside the listener
     * it will see the status already reset. The following statuses are applicable to the DataReaderListener:
     * <ul>
     * <li>RequestedDeadlineMissedStatus</li>
     * <li>RequestedIncompatibleQosStatus</li>
     * <li>SampleRejectedStatus</li>
     * <li>LivelinessChangedStatus</li>
     * <li>DataAvailableStatus</li>
     * <li>SubscriptionMatchedStatus</li>
     * <li>SampleLostStatus</li>
     * </ul>
     * <p>
     * Status Propagation:<p>
     * In case a communication status is not activated in the statuses of the DataReaderListener,
     * the PublisherListener of the containing Subscriber is invoked (if attached and activated for the status that occurred).
     * This allows the application to set a default behaviour in the SubscriberListener of the containing Subscriber and a DataReader
     * specific behaviour when needed. In case the communication status is not activated in the statuses of the SubscriberListener as well,
     * the communication status will be propagated to the DomainParticipantListener of the containing DomainParticipant. In case the
     * DomainParticipantListener is also not attached or the communication status is not activated in its mask, the application is
     * not notified of the change.
     * <p>
     * The {@link org.omg.dds.topic.TopicDescription} passed to this operation must have been
     * created from the same {@link org.omg.dds.domain.DomainParticipant} that was used to
     * create this Subscriber. If the TopicDescription was created from a
     * different DomainParticipant, the operation will fail.
     *
     * @return A DataReader object.
     * @param topic     A reference to the topic for which the DataReader is created.
     * @param qos       The DataReaderQos for the new DataReader. In case these settings
     *                  are not self consistent, no DataReader is created.
     * @param listener  A reference to the DataReaderListener instance which will be attached to the new DataReader.
     * @param statuses  An arbitrary number of statuses that the listener should be notified of.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *                  The parameter qos contains conflicting QosPolicy settings.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     *
     * @see     #createDataReader(TopicDescription)
     */
    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic,
            DataReaderQos qos,
            DataReaderListener<TYPE> listener,
            Class<? extends Status>... statuses);

    /**
     * This operation creates a {@link org.omg.dds.sub.DataReader}. The returned DataReader
     * will be attached and belong to the Subscriber.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataReader is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataReader QoS by means of
     *         {@link #getDefaultDataReaderQos()}.
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired.</li>
     *     <li>Use the resulting QoS policies to construct the DataReader.
     * </ul>
     * @param topic The {@link org.omg.dds.topic.TopicDescription} created from the same
     * {@link org.omg.dds.domain.DomainParticipant} that was used to create this Subscriber.
     * @param qos An instance of (@link org.omg.dds.sub.DataReaderQos} or null.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *                  The parameter qos contains conflicting QosPolicy settings.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     *
     * @see #createDataReader(TopicDescription)
     */

    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic,
            DataReaderQos qos);

    // --- Lookup operations: ------------------------------------------------

    /**
     * This operation retrieves a previously-created DataReader belonging to
     * the Subscriber that is attached to a {@link org.omg.dds.topic.Topic} with a matching
     * topicName. If no such DataReader exists, the operation will return
     * null.
     * <p>
     * If multiple DataReaders attached to the Subscriber satisfy this
     * condition, then the operation will return one of them. It is not
     * specified which one.
     * <p>
     * The use of this operation on the built-in Subscriber allows access to
     * the built-in DataReader entities for the built-in topics.
     * @return the found DataReader
     * @param topicName     The name of the Topic, which is attached to the DataReader to look for.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see     DomainParticipant#getBuiltinSubscriber()
     */
    public <TYPE> DataReader<TYPE> lookupDataReader(String topicName);

    /**
     * This operation retrieves a previously-created DataReader belonging to
     * the Subscriber that is attached to the given {@link org.omg.dds.topic.TopicDescription}.
     * If no such DataReader exists, the operation will return null.
     * <p>
     * If multiple DataReaders attached to the Subscriber satisfy this
     * condition, then the operation will return one of them. It is not
     * specified which one.
     * <p>
     * The use of this operation on the built-in Subscriber allows access to
     * the built-in DataReader entities for the built-in topics.
     * @return the found DataReader
     * @param topicName     The topic description of the Topic, which is attached to the DataReader to look for.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see     DomainParticipant#getBuiltinSubscriber()
     */
    public <TYPE> DataReader<TYPE> lookupDataReader(
            TopicDescription<TYPE> topicName);


    // --- Other operations: -------------------------------------------------

    /**
     * This operation closes all the entities that were created by means of the
     * "create" operations on the Subscriber. That is, it closes all contained
     * {@link org.omg.dds.sub.DataReader} objects. This pattern is applied
     * recursively. In this manner the operation on the Subscriber will end up
     * closing all the entities recursively contained in the Subscriber, that is
     * also the {@link org.omg.dds.sub.QueryCondition} and
     * {@link org.omg.dds.sub.ReadCondition} objects belonging to the contained
     * DataReaders.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if any of the contained entities is in a state where it
     *             cannot be closed. This will occur, for example, if a
     *             contained DataReader cannot be closed because the application
     *             has called a {@link org.omg.dds.sub.DataReader#read()} or
     *             {@link org.omg.dds.sub.DataReader#take()} operation and has
     *             not called the corresponding {@link Sample.Iterator#close()}
     *             operation to return the loaned samples.
     */
    public void closeContainedEntities();

    /**
     * This operation is equivalent to calling
     * {@link #getDataReaders(DataState)} with any sample state (
     * {@link Subscriber.DataState#withAnySampleState()}), any view state (
     * {@link Subscriber.DataState#withAnyViewState()}), and any instance state
     * ({@link Subscriber.DataState#withAnyInstanceState()}).
     *
     * @return A collection of DataReaders that belong to this Subscriber.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the Subscriber has
     *             {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *             set to
     *             {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP}
     *             and this operation is not invoked inside a
     *             {@link #beginAccess()}/{@link #endAccess()} block.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see #getDataReaders(DataState)
     * @see #beginAccess()
     * @see #endAccess()
     * @see org.omg.dds.core.policy.Presentation
     */
    public Collection<DataReader<?>> getDataReaders();

    /**
     * This operation allows the application to access the
     * {@link org.omg.dds.sub.DataReader} objects that contain samples with the
     * specified sample states, view states, and instance states.
     * <p>
     * If the {@link org.omg.dds.core.policy.Presentation} of the Subscriber has
     * {@link org.omg.dds.core.policy.Presentation#getAccessScope()} set to
     * {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP}, this
     * operation should only be invoked inside a {@link #beginAccess()}/
     * {@link #endAccess()} block. Otherwise it will fail with
     * {@link org.omg.dds.core.PreconditionNotMetException}.
     * <p>
     * Depending on the setting of the
     * {@link org.omg.dds.core.policy.Presentation}, the returned collection of
     * DataReader objects may be a 'set' containing each DataReader at most once
     * in no specified order, or a 'list' containing each DataReader one or more
     * times in a specific order. (This refers to the semantics of the
     * collection; the concrete type of the collection may or may not implement
     * {@link Set} or {@link java.util.List}).
     *
     * <ol>
     * <li>If {@link org.omg.dds.core.policy.Presentation#getAccessScope()} is
     * {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#INSTANCE} or
     * {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#TOPIC}, the
     * returned collection is a 'set'.</li>
     * <li>If {@link org.omg.dds.core.policy.Presentation#getAccessScope()} is
     * {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP} and
     * {@link org.omg.dds.core.policy.Presentation#isOrderedAccess()} is true,
     * then the returned collection is a 'list'.</li>
     * </ol>
     *
     * This difference is due to the fact that, in the second situation it is
     * required to access samples belonging to different DataReader objects in a
     * particular order. In this case, the application should process each
     * DataReader in the same order it appears in the 'list' and read or take
     * exactly one sample from each DataReader.
     *
     * @param dataState
     *            a DataReader will only be placed into the readers collection
     *            if it has data available with one of these sample states, view
     *            states, and instance states.
     *
     * @return A collection of DataReaders that contain samples with the
     *         specified sample states.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the Subscriber has
     *             {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *             set to
     *             {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP}
     *             and this operation is not invoked inside a
     *             {@link #beginAccess()}/{@link #endAccess()} block.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see #getDataReaders()
     * @see #beginAccess()
     * @see #endAccess()
     * @see org.omg.dds.core.policy.Presentation
     */
    public Collection<DataReader<?>> getDataReaders(DataState dataState);

    /**
     * This operation invokes the operation
     * {@link org.omg.dds.sub.DataReaderListener#onDataAvailable(org.omg.dds.core.event.DataAvailableEvent)}
     * on the DataReaderListener objects attached to contained DataReader
     * entities with a {@link org.omg.dds.core.event.DataAvailableEvent} that is considered
     * changed.
     * <p>
     * This operation is typically invoked from
     * {@link org.omg.dds.sub.SubscriberListener#onDataOnReaders(org.omg.dds.core.event.DataOnReadersEvent)}.
     * That way the SubscriberListener can delegate to the DataReaderListener
     * objects the handling of the data.
     */
    public void notifyDataReaders();

    /**
     * This operation indicates that the application is about to access the
     * data samples in any of the {@link org.omg.dds.sub.DataReader} objects attached to the
     * Subscriber.
     * <p>
     * The application is required to use this operation only if the
     * {@link org.omg.dds.core.policy.Presentation} of the
     * Subscriber has
     * {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     * equal to
     * {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP}.
     * <p>
     * In the aforementioned case, the operation must be called prior to
     * calling any of the sample-accessing operations, namely:
     * {@link #getDataReaders()}, {@link org.omg.dds.sub.DataReader#read()},
     * {@link org.omg.dds.sub.DataReader#take()}, or their overloads. Otherwise the
     * sample-accessing operations will fail with
     * {@link org.omg.dds.core.PreconditionNotMetException}. Once the application has
     * finished accessing the data samples it must call {@link #endAccess()}.
     * <p>
     * It is not required for the application to call {@link #beginAccess()}/
     * {@link #endAccess()} if the
     * {@link org.omg.dds.core.policy.Presentation} has the
     * access scope set to something other than 'GROUP'. Calling these
     * methods in this case is not considered an error and has no effect.
     * <p>
     * The calls to {@link #beginAccess()}/{@link #endAccess()} may be
     * nested. In that case, the application must call {@link #endAccess()}
     * as many times as it called beginAccess.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *                  When the PresentationQos has not set AccessScope to GROUP
     * @see     #endAccess()
     * @see     org.omg.dds.core.policy.Presentation
     */
    public void beginAccess();

    /**
     * Indicates that the application has finished accessing the data samples in
     * {@link org.omg.dds.sub.DataReader} objects managed by the Subscriber.
     * <p>
     * This operation must be used to 'close' a corresponding
     * {@link #beginAccess()}.
     * <p>
     * After calling endAccess the application should no longer access any of
     * the Sample (including corresponding data) elements returned from the
     * sample-accessing operations.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if this call does not close a previous call to
     *             {@link #beginAccess()}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @see #beginAccess()
     */
    public void endAccess();

    /**
     * This operation retrieves the default value of the DataReader QoS, that
     * is, the QoS policies which will be used for newly created
     * {@link org.omg.dds.sub.DataReader} entities in the case where the QoS policies are
     * defaulted in the {@link #createDataReader(TopicDescription)}
     * operation.
     * <p>
     * The values retrieved will match the set of values specified on the
     * last successful call to
     * {@link #setDefaultDataReaderQos(DataReaderQos)}, or else, if the call
     * was never made, the default values identified by the DDS
     * specification.
     * @return the default DataReaderQos
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #setDefaultDataReaderQos(DataReaderQos)
     */
    public DataReaderQos getDefaultDataReaderQos();

    /**
     * This operation sets a default value of the DataReader QoS policies, which
     * will be used for newly created {@link org.omg.dds.sub.DataReader}
     * entities in the case where the QoS policies are defaulted in the
     * {@link #createDataReader(TopicDescription)} operation.
     * @param qos   The new default DataReaderQos
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *             if the resulting policies are not self consistent. In this
     *             case, this operation will have no effect.
     *
     * @see #getDefaultDataReaderQos()
     */
    public void setDefaultDataReaderQos(DataReaderQos qos);

    /**
     * This operation copies the policies in the {@link org.omg.dds.topic.TopicQos} to the
     * corresponding policies in the {@link org.omg.dds.sub.DataReaderQos} (replacing values
     * in the latter, if present).
     * <p>
     * This is a "convenience" operation most useful in combination with the
     * operations {@link #getDefaultDataReaderQos()} and
     * {@link org.omg.dds.topic.Topic#getQos()}. The operation can be used to merge the
     * DataReader default QoS policies with the corresponding ones on the
     * Topic. The resulting QoS can then be used to create a new DataReader
     * or set its QoS.
     * <p>
     * This operation does not check the resulting QoS for consistency. This
     * is because the 'merged' QoS may not be the final one, as the
     * application can still modify some policies prior to applying the
     * policies to the DataReader.
     *
     * @param   drQos   The QoS, the policies of which will be overridden.
     *                  This object is not modified.
     * @param   tQos    The source for the new policies to be copied. This
     *                  object is not modified.
     *
     * @return          A copy of drQos with the applicable policies from
     *                  tQos applied to it.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Subscriber has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    public DataReaderQos copyFromTopicQos(DataReaderQos drQos, TopicQos tQos);


    // --- From Entity: ------------------------------------------------------
    @Override
    public StatusCondition<Subscriber> getStatusCondition();

    @Override
    public DomainParticipant getParent();


    // --- DataState: ------------------------------------------------------

    /**
     * Create and return a new modifiable {@link DataState} object. This
     * object will be initialized with no sample states, no instance states,
     * and no view states.
     * <p>
     * This method shall never return null.
     *
     * @return  a new {@link DataState} object.
     */
    public DataState createDataState();


    /**
     * A DataState encapsulates sets of sample states, view states, and
     * instance states as a convenience.
     * <p>
     * Instances of DataState may be unmodifiable, in which case methods
     * that would change them shall throw
     * {@link UnsupportedOperationException}.
     */
    public static interface DataState extends DDSObject, Cloneable {
        // --- Accessors: ----------------------------------------------------

        /**
         * Get the current set of sample states. The resulting unmodifiable
         * collection may be empty, but it shall never be null.
         *
         * @return      the current set of sample states.
         */
        public Set<SampleState> getSampleStates();

        /**
         * Get the current set of view states. The resulting unmodifiable
         * collection may be empty, but it shall never be null.
         *
         * @return      the current set of view states.
         */
        public Set<ViewState> getViewStates();

        /**
         * Get the current set of instance states. The resulting unmodifiable
         * collection may be empty, but it shall never be null.
         *
         * @return      the current set of instance states.
         */
        public Set<InstanceState> getInstanceStates();


        // --- Mutators: -----------------------------------------------------

        /**
         * Add the given {@link org.omg.dds.sub.SampleState} to this DataState.
         *
         * @param state
         *            the state to add.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState with(SampleState state);

        /**
         * Add the given {@link org.omg.dds.sub.ViewState} to this DataState.
         *
         * @param state
         *            the state to add.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState with(ViewState state);

        /**
         * Add the given {@link org.omg.dds.sub.InstanceState} to this
         * DataState.
         *
         * @param state
         *            the state to add.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState with(InstanceState state);

        /**
         * Add all {@link org.omg.dds.sub.SampleState} values to this DataState.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState withAnySampleState();

        /**
         * Add all {@link org.omg.dds.sub.ViewState} values to this DataState.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState withAnyViewState();

        /**
         * Add all {@link org.omg.dds.sub.InstanceState} values to this
         * DataState.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState withAnyInstanceState();

        /**
         * Add {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_DISPOSED} and
         * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS} to this
         * DataState.
         *
         * @return this
         *
         * @throws UnsupportedOperationException
         *             if this DataState is unmodifiable.
         */
        public DataState withNotAliveInstanceStates();


        // --- From Object: --------------------------------------------------

        public DataState clone();

        public boolean equals(Object other);

        public int hashCode();
    }
}
