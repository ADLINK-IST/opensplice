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

import java.util.Collection;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.DomainEntity;
import org.omg.dds.core.Duration;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

/**
 * A Publisher is the object responsible for the actual dissemination of
 * publications.
 * <p>
 * The Publisher acts on the behalf of one or several
 * {@link org.omg.dds.pub.DataWriter} objects that belong to it. When it is
 * informed of a change to the data associated with one of its DataWriter
 * objects, it decides when it is appropriate to actually send the data-update
 * message. In making this decision, it considers any extra information that
 * goes with the data (time stamp, writer, etc.) as well as the QoS of the
 * Publisher and the DataWriter.
 * <p>
 * All operations except for
 * {@link org.omg.dds.core.Entity#setQos(org.omg.dds.core.EntityQos)},
 * {@link org.omg.dds.core.Entity#getQos()},
 * {@link org.omg.dds.core.Entity#setListener(java.util.EventListener)},
 * {@link org.omg.dds.core.Entity#getListener()},
 * {@link org.omg.dds.core.Entity#enable()},
 * {@link org.omg.dds.core.Entity#getStatusCondition()},
 * {@link #createDataWriter(Topic)}, and
 * {@link org.omg.dds.pub.Publisher#close()} may fail with the exception
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
 * // Create a Publisher using default Qos and no Listener.
 * Publisher myPub = participant.createPublisher();
 * </code>
 * </pre>
 */
public interface Publisher
extends DomainEntity<PublisherListener, PublisherQos>
{
    // --- Create (any) DataWriter: ------------------------------------------

    /**
     * This operation creates a DataWriter. The returned DataWriter will be
     * attached and belongs to the Publisher.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataWriter is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataWriter QoS by means of
     *         {@link org.omg.dds.pub.Publisher#getDefaultDataWriterQos()}.</li>
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired -- see
     *         {@link #copyFromTopicQos(DataWriterQos, TopicQos)}.
     *         </li>
     *     <li>Use the resulting QoS policies to construct the DataWriter.
     *         </li>
     * </ul>
     *
     * The {@link org.omg.dds.topic.Topic} passed to this operation must have been created from
     * the same {@link org.omg.dds.domain.DomainParticipant} that was used to create this
     * Publisher. If the Topic was created from a different
     * DomainParticipant, the operation will fail.
     * @param topic A reference to the topic for which the DataWriter is created.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #createDataWriter(Topic, DataWriterQos, DataWriterListener, Collection)
     */
    public <TYPE> DataWriter<TYPE> createDataWriter(
            Topic<TYPE> topic);

    /**
     * This operation creates a DataWriter. The returned DataWriter will be
     * attached and belongs to the Publisher.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataWriter is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataWriter QoS by means of
     *         {@link org.omg.dds.pub.Publisher#getDefaultDataWriterQos()}.</li>
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired -- see
     *         {@link #copyFromTopicQos(DataWriterQos, TopicQos)}.
     *         </li>
     *     <li>Use the resulting QoS policies to construct the DataWriter.
     *         </li>
     * </ul>
     * <p>
     * For each communication status, the StatusChangedFlag flag is initially set to false.
     * It becomes true whenever that communication status changes. For each communication status
     * activated in the mask, the associated {@link org.omg.dds.pub.DataWriterListener} operation is
     * invoked and the communication status is reset to false, as the listener implicitly accesses the
     * status which is passed as a parameter to that operation. The status is reset prior to calling
     * the listener, so if the application calls the get&lt;status_name&gt;Status from inside the listener
     * it will see the status already reset. The following statuses are applicable to the DataWriterListener:
     * <ul>
     * <li>OfferedDeadlineMissedStatus</li>
     * <li>OfferedIncompatibleQosStatus</li>
     * <li>LivelinessLostStatus</li>
     * <li>PublicationMatchedStatus</li>
     * </ul>
     * <p>
     * Status Propagation:<p>
     * In case a communication status is not activated in the statuses of the DataWriterListener,
     * the PublisherListener of the containing Publisher is invoked (if attached and activated for the status that occurred).
     * This allows the application to set a default behaviour in the PublisherListener of the containing Publisher and a DataWriter
     * specific behaviour when needed. In case the communication status is not activated in the statuses of the PublisherListener as well,
     * the communication status will be propagated to the DomainParticipantListener of the containing DomainParticipant. In case the
     * DomainParticipantListener is also not attached or the communication status is not activated in its mask, the application is
     * not notified of the change.
     * <p>
     * The {@link org.omg.dds.topic.Topic} passed to this operation must have been created from
     * the same {@link org.omg.dds.domain.DomainParticipant} that was used to create this
     * Publisher. If the Topic was created from a different
     * DomainParticipant, the operation will fail.
     * @return A DataWriter object.
     * @param topic     A reference to the topic for which the DataWriter is created.
     * @param qos       The DataWriterQos for the new DataWriter. In case these settings
     *                  are not self consistent, no DataWriter is created.
     * @param listener  A reference to the DataWriterListener instance which will be attached to the new DataWriter.
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
     *                  The corresponding Publisher has been closed.
     * @see     #createDataWriter(Topic)
     */
    public <TYPE> DataWriter<TYPE> createDataWriter(
            Topic<TYPE> topic,
            DataWriterQos qos,
            DataWriterListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses);

    /**
     * This operation creates a DataWriter. The returned DataWriter will be
     * attached and belongs to the Publisher.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataWriter is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataWriter QoS by means of
     *         {@link org.omg.dds.pub.Publisher#getDefaultDataWriterQos()}.</li>
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired -- see
     *         {@link #copyFromTopicQos(DataWriterQos, TopicQos)}.
     *         </li>
     *     <li>Use the resulting QoS policies to construct the DataWriter.
     *         </li>
     * </ul>
     * <p>
     * For each communication status, the StatusChangedFlag flag is initially set to false.
     * It becomes true whenever that communication status changes. For each communication status
     * activated in the mask, the associated {@link org.omg.dds.pub.DataWriterListener} operation is
     * invoked and the communication status is reset to false, as the listener implicitly accesses the
     * status which is passed as a parameter to that operation. The status is reset prior to calling
     * the listener, so if the application calls the get&lt;status_name&gt;Status from inside the listener
     * it will see the status already reset. The following statuses are applicable to the DataWriterListener:
     * <ul>
     * <li>OfferedDeadlineMissedStatus</li>
     * <li>OfferedIncompatibleQosStatus</li>
     * <li>LivelinessLostStatus</li>
     * <li>PublicationMatchedStatus</li>
     * </ul>
     * <p>
     * Status Propagation:<p>
     * In case a communication status is not activated in the statuses of the DataWriterListener,
     * the PublisherListener of the containing Publisher is invoked (if attached and activated for the status that occurred).
     * This allows the application to set a default behaviour in the PublisherListener of the containing Publisher and a DataWriter
     * specific behaviour when needed. In case the communication status is not activated in the statuses of the PublisherListener as well,
     * the communication status will be propagated to the DomainParticipantListener of the containing DomainParticipant. In case the
     * DomainParticipantListener is also not attached or the communication status is not activated in its mask, the application is
     * not notified of the change.
     * <p>
     * The {@link org.omg.dds.topic.Topic} passed to this operation must have been created from
     * the same {@link org.omg.dds.domain.DomainParticipant} that was used to create this
     * Publisher. If the Topic was created from a different
     * DomainParticipant, the operation will fail.
     * @return A DataWriter object.
     * @param topic     A reference to the topic for which the DataWriter is created.
     * @param qos       The DataWriterQos for the new DataWriter. In case these settings
     *                  are not self consistent, no DataWriter is created.
     * @param listener  A reference to the DataWriterListener instance which will be attached to the new DataWriter.
     * @param statuses  An arbitrary number of statuses that the listener should be notified of.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *                  The parameter qos contains conflicting QosPolicy settings.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #createDataWriter(Topic)
     */
    public <TYPE> DataWriter<TYPE> createDataWriter(
            Topic<TYPE> topic,
            DataWriterQos qos,
            DataWriterListener<TYPE> listener,
            Class<? extends Status>... statuses);

    /**
     * This operation creates a DataWriter. The returned DataWriter will be
     * attached and belongs to the Publisher.
     * <p>
     * Note that a common application pattern to construct the QoS for the
     * DataWriter is to:
     *
     * <ul>
     *     <li>Retrieve the QoS policies on the associated {@link org.omg.dds.topic.Topic} by
     *         means of {@link org.omg.dds.topic.Topic#getQos()}.</li>
     *     <li>Retrieve the default DataWriter QoS by means of
     *         {@link org.omg.dds.pub.Publisher#getDefaultDataWriterQos()}.</li>
     *     <li>Combine those two QoS policies and selectively modify policies
     *         as desired -- see
     *         {@link #copyFromTopicQos(DataWriterQos, TopicQos)}.
     *         </li>
     *     <li>Use the resulting QoS policies to construct the DataWriter.
     *         </li>
     * </ul>
     *
     * The {@link org.omg.dds.topic.Topic} passed to this operation must have been created from
     * the same {@link org.omg.dds.domain.DomainParticipant} that was used to create this
     * Publisher. If the Topic was created from a different
     * DomainParticipant, the operation will fail.
     * @param topic     A reference to the topic for which the DataWriter is created.
     * @param qos       The DataWriterQos for the new DataWriter. In case these settings
     *                  are not self consistent, no DataWriter is created.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @throws org.omg.dds.core.InconsistentPolicyException
     *                  The parameter qos contains conflicting QosPolicy settings.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #createDataWriter(Topic, DataWriterQos, DataWriterListener, Collection)
     */

    public <TYPE> DataWriter<TYPE> createDataWriter(
            Topic<TYPE> topic,
            DataWriterQos qos);

    // --- Lookup operations: ------------------------------------------------

    /**
     * This operation retrieves a previously created {@link org.omg.dds.pub.DataWriter}
     * belonging to the Publisher that is attached to a {@link org.omg.dds.topic.Topic} with a
     * matching name. If no such DataWriter exists, the operation will return
     * null.
     * <p>
     * If multiple DataWriters attached to the Publisher satisfy this
     * condition, then the operation will return one of them. It is not
     * specified which one.
     * @return the found DataWriter
     * @param topicName     The name of the Topic, which is attached to the DataWriter to look for.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #lookupDataWriter(Topic)
     */
    public <TYPE> DataWriter<TYPE> lookupDataWriter(String topicName);

    /**
     * This operation retrieves a previously created {@link org.omg.dds.pub.DataWriter}
     * belonging to the Publisher that is attached to the given
     * {@link org.omg.dds.topic.Topic}. If no such DataWriter exists, the operation will return
     * null.
     * <p>
     * If multiple DataWriters attached to the Publisher satisfy this
     * condition, then the operation will return one of them. It is not
     * specified which one.
     * @return the found DataWriter
     * @param topic     The Topic, which is attached to the DataWriter to look for.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #lookupDataWriter(String)
     */
    public <TYPE> DataWriter<TYPE> lookupDataWriter(Topic<TYPE> topic);

    // --- Other operations: -------------------------------------------------

    /**
     * This operation closes all the entities that were created by means of the
     * "create" operations on the Publisher. That is, it closes all contained
     * {@link org.omg.dds.pub.DataWriter} objects.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the any of the contained entities is in a state where it
     *             cannot be deleted.
     */
    public void closeContainedEntities();

    /**
     * This operation indicates to the Service that the application is about
     * to make multiple modifications using DataWriter objects belonging to
     * the Publisher.
     * <p>
     * It is a hint to the Service so it can optimize its performance by
     * e.g., holding the dissemination of the modifications and then batching
     * them.
     * <p>
     * It is not required that the Service use this hint in any way.
     * <p>
     * The use of this operation must be matched by a corresponding call to
     * {@link #resumePublications()} indicating that the set of modifications
     * has completed. If the Publisher is deleted before
     * {@link #resumePublications()} is called, any suspended updates yet to
     * be published will be discarded.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #resumePublications()
     */
    public void suspendPublications();

    /**
     * This operation indicates to the Service that the application has
     * completed the multiple changes initiated by the previous
     * {@link #suspendPublications()}. This is a hint to the Service that can be
     * used by a Service implementation to e.g., batch all the modifications
     * made since the {@link #suspendPublications()}.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the call to this method does not match a previous call to
     *             {@link #suspendPublications()}.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if it is not possible for sufficient resources to be made
     *             available within the configured max blocking time.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see #suspendPublications()
     */
    public void resumePublications();

    /**
     * This operation requests that the application will begin a 'coherent
     * set' of modifications using {@link org.omg.dds.pub.DataWriter} objects attached to the
     * Publisher. The 'coherent set' will be completed by a matching call to
     * {@link #endCoherentChanges()}.
     * <p>
     * A 'coherent set' is a set of modifications that must be propagated in
     * such a way that they are interpreted at the receivers' side as a
     * consistent set of modifications; that is, the receiver will only be
     * able to access the data after all the modifications in the set are
     * available at the receiver end.
     * <p>
     * A connectivity change may occur in the middle of a set of coherent
     * changes; for example, the set of partitions used by the Publisher or
     * one of its Subscribers may change, a late-joining DataReader may
     * appear on the network, or a communication failure may occur. In the
     * event that such a change prevents an entity from receiving the entire
     * set of coherent changes, that entity must behave as if it had
     * received none of the set.
     * <p>
     * These calls can be nested. In that case, the coherent set terminates
     * only with the last call to {@link #endCoherentChanges()}.
     * <p>
     * The support for 'coherent changes' enables a publishing application to
     * change the value of several data instances that could belong to the
     * same or different topics and have those changes be seen 'atomically'
     * by the readers. This is useful in cases where the values are
     * interrelated. For example, if there are two data instances
     * representing the 'altitude' and 'velocity vector' of the same aircraft
     * and both are changed, it may be useful to communicate those values in
     * a way the reader can see both together; otherwise, it may e.g.,
     * erroneously interpret that the aircraft is on a collision course.
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             the Publisher is not able to handle coherent changes because
     *             its PresentationQos has not set coherentAccess to TRUE.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see     #endCoherentChanges()
     */
    public void beginCoherentChanges();

    /**
     * This operation terminates the 'coherent set' initiated by the matching
     * call to {@link #beginCoherentChanges()}.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if there is no matching call to
     *             {@link #beginCoherentChanges()}.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @see #beginCoherentChanges()
     */
    public void endCoherentChanges();

    /**
     * This operation blocks the calling thread until either all data
     * written by the reliable {@link org.omg.dds.pub.DataWriter} entities is acknowledged by
     * all matched reliable {@link org.omg.dds.sub.DataReader} entities, or else the duration
     * specified elapses, whichever happens first.
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
     *                  The corresponding Publisher has been closed.
     */
    public void waitForAcknowledgments(Duration maxWait)
            throws TimeoutException;

    /**
     * This operation blocks the calling thread until either all data
     * written by the reliable {@link org.omg.dds.pub.DataWriter} entities is acknowledged by
     * all matched reliable {@link org.omg.dds.sub.DataReader} entities, or else the duration
     * specified elapses, whichever happens first.
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
     *                  The corresponding Publisher has been closed.
     */
    public void waitForAcknowledgments(long maxWait, TimeUnit unit)
            throws TimeoutException;

    /**
     * This operation retrieves the default value of the DataWriter QoS, that
     * is, the QoS policies which will be used for newly created
     * {@link org.omg.dds.pub.DataWriter} entities in the case where the QoS policies are
     * defaulted in the {@link #createDataWriter(Topic)} operation.
     * <p>
     * The values retrieved will match the set of values specified on the
     * last successful call to
     * {@link #setDefaultDataWriterQos(DataWriterQos)}, or else, if the call
     * was never made, the default values identified by the DDS
     * specification.
     * @return A DataWriterQos
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #setDefaultDataWriterQos(DataWriterQos)
     */
    public DataWriterQos getDefaultDataWriterQos();

    /**
     * This operation sets a default value of the DataWriter QoS policies, which
     * will be used for newly created {@link org.omg.dds.pub.DataWriter}
     * entities in the case where the QoS policies are defaulted in the
     * {@link #createDataWriter(Topic)} operation.
     * @param qos   The new default DataWriterQos
     * @throws org.omg.dds.core.InconsistentPolicyException
     *             if the resulting policies are not self consistent; if they
     *             are not, the operation will have no effect.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see #getDefaultDataWriterQos()
     */
    public void setDefaultDataWriterQos(DataWriterQos qos);

    /**
     * This operation copies the policies in the {@link org.omg.dds.topic.Topic} QoS to the
     * corresponding policies in the {@link org.omg.dds.pub.DataWriter} QoS (replacing values
     * in the DataWriter QoS, if present).
     * <p>
     * This is a "convenience" operation most useful in combination with the
     * operations {@link #getDefaultDataWriterQos()} and
     * {@link org.omg.dds.topic.Topic#getQos()}. The operation can be used to merge the
     * DataWriter default QoS policies with the corresponding ones on the
     * Topic. The resulting QoS can then be used to create a new DataWriter
     * or set its QoS.
     * <p>
     * This operation does not check the resulting DatWriter QoS for
     * consistency. This is because the 'merged' QoS may not be the final
     * one, as the application can still modify some policies prior to
     * applying the policies to the DataWriter.
     *
     * @param dwQos The QoS whose policies are to be overridden. This object
     *              is not modified.
     * @param tQos  The QoS from which the policies are to be taken. This
     *              object is not modified.
     *
     * @return      A copy of dwQos with the applicable policies from tQos
     *              applied to it.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding Publisher has been closed.
     */
    public DataWriterQos copyFromTopicQos(DataWriterQos dwQos, TopicQos tQos);


    // --- From Entity: ------------------------------------------------------
    @Override
    public StatusCondition<Publisher> getStatusCondition();

    @Override
    public DomainParticipant getParent();
}
