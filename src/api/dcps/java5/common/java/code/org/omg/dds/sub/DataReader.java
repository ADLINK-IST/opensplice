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

import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.DomainEntity;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.LivelinessChangedStatus;
import org.omg.dds.core.status.RequestedDeadlineMissedStatus;
import org.omg.dds.core.status.RequestedIncompatibleQosStatus;
import org.omg.dds.core.status.SampleLostStatus;
import org.omg.dds.core.status.SampleRejectedStatus;
import org.omg.dds.core.status.SubscriptionMatchedStatus;
import org.omg.dds.topic.PublicationBuiltinTopicData;
import org.omg.dds.topic.TopicDescription;


/**
 * A DataReader allows the application (1) to declare the data it wishes to
 * receive (i.e., make a subscription) and (2) to access the data received by
 * the attached {@link org.omg.dds.sub.Subscriber}.
 * <p>
 * A DataReader refers to exactly one {@link org.omg.dds.topic.TopicDescription}
 * (either a {@link org.omg.dds.topic.Topic}, a
 * {@link org.omg.dds.topic.ContentFilteredTopic}, or a
 * {@link org.omg.dds.topic.MultiTopic}) that identifies the data to be read.
 * The subscription has a unique resulting type. The data reader may give access
 * to several instances of the resulting type, which can be distinguished from
 * each other by their keys.
 * <p>
 * All operations except for the inherited operations
 * {@link #setQos(org.omg.dds.core.EntityQos)}, {@link #getQos()},
 * {@link #setListener(java.util.EventListener)}, {@link #getListener()},
 * {@link #enable()}, {@link #getStatusCondition()}, and {@link #close()} may
 * fail with the exception {@link org.omg.dds.core.NotEnabledException}.
 * <p>
 * All sample-accessing operations, namely all variants of {@link #read()} or
 * {@link #take()}, may fail with the exception
 * {@link org.omg.dds.core.PreconditionNotMetException}.
 * <p>
 * <b>Access to the Data</b>
 * <p>
 * Data is made available to the application by the following operations on
 * DataReader objects: {@link #read()}, {@link #take()}, and the other methods
 * beginning with those prefixes. The general semantics of the "read"
 * operations is that the application only gets access to the corresponding
 * data; the data remains the middleware's responsibility and can be read again.
 * The semantics of the "take" operations is that the application takes full
 * responsibility for the data; that data will no longer be accessible to the
 * DataReader. Consequently, it is possible for a DataReader to access the same
 * sample multiple times but only if all previous accesses were read operations.
 * <p>
 * Each of these operations returns an ordered collection of
 * {@link org.omg.dds.sub.Sample}s (data values and associated
 * meta-information). Each data value represents an atom of data information
 * (i.e., a value for one instance). This collection may contain samples related
 * to the same or different instances (identified by the key). Multiple samples
 * can refer to the same instance if the settings of the
 * {@link org.omg.dds.core.policy.History} allow for it.
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
 *
 * // Create a DataReader using the topic Qos and no listener.
 * DataReaderQos drq  = mySub.copyFromTopicQos(mySub.getDefaultDataReaderQos(), fooTopic.getQos()
 * DataReader&lt;Foo&gt; fooDr = mySub.createDataReader(fooTopic,drq);
 *
 * // There are multiple ways of reader/taking data one way is through a pre allocated collection another is
 * // by means of an Iterator. Below are both shown in an example
 *
 * // Read samples and store them in a pre allocated collection
 * List&lt;Sample&lt;Foo&gt;&gt; samples = new ArrayList&lt;Sample&lt;Foo&gt;&gt;();
 * fooDr.read(samples);
 * // Process each Sample and print its name and production time.
 * for (Sample&lt;Foo&gt; sample : samples) {
 *     Foo foo = sample.getData();
 *     if (foo != null) { //Check if the sample is valid.
 *          Time t = sample.getSourceTimestamp();
 *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
 *     }
 * }
 *
 * // Take samples using an Iterator
 * Iterator&lt;Sample&lt;Foo&gt;&gt; samples = fooDr.take();
 * // Process each Sample and print its name and production time.
 * while (samples.hasNext()) {
 *     Sample sample = samples.next();
 *     Foo foo = sample.getData();
 *     if (foo != null) { //Check if the sample is valid.
 *          Time t = sample.getSourceTimestamp();
 *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
 *     }
 * }
 * </code>
 * </pre>
 * @param <TYPE>
 *            The concrete type of the data to be read.
 */
public interface DataReader<TYPE>
extends DomainEntity<DataReaderListener<TYPE>, DataReaderQos>
{
    /**
     * Cast this data reader to the given type, or throw an exception if
     * the cast fails.
     *
     * @param <OTHER>   The type of the data subscribed to by this reader,
     *                  according to the caller.
     * @return          this data reader
     * @throws          ClassCastException if the cast fails
     */
    public <OTHER> DataReader<OTHER> cast();

    /**
     * This operation creates a ReadCondition. The returned ReadCondition
     * will be attached and belong to the DataReader.
     * <p>
     * State Masks<p>
     * The result of the ReadCondition depends on the selection of samples
     * determined by three {@link org.omg.dds.sub.Subscriber.DataState} masks:
     * <ul>
     * <li>{@link org.omg.dds.sub.SampleState} is the mask, which selects only those samples with the desired
     *     sample states READ, NOT_READ or both.</li>
     * <li>{@link org.omg.dds.sub.ViewState} is the mask, which selects only those samples with the desired view states
     *     NEW, NOT_NEW or both</li>
     * <li>{@link org.omg.dds.sub.InstanceState} is the mask, which selects only those samples with the desired instance states
     *     ALIVE_INSTANCE, NOT_ALIVE_DISPOSED, NOT_ALIVE_NO_WRITERS or a combination of these.</li>
     * </ul>
     * <pre>
     * Example:
     * <code>
     * This example assumes a Subscriber called "subscriber" and DataReader named "fooDr" has been created
     * and a datatype called Foo is present: class Foo { int age; String name; };
     * // Create a ReadCondition with a DataState with a sample state of not read, new view state and alive instance state.
     * DataState ds = subscriber.createDataState();
     * ds = ds.with(SampleState.NOT_READ)
     *        .with(ViewState.NEW)
     *        .with(InstanceState.ALIVE);
     * ReadCondition&lt;Foo&gt; readCond = fooDr.createReadCondition(ds);
     * </code>
     * </pre>
     * @return a new ReadCondition
     * @param   states  The returned condition will only trigger on samples
     *          with one of these sample states, view states, and
     *          instance states.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    public ReadCondition<TYPE> createReadCondition(
            Subscriber.DataState states);

    /**
     * This operation creates a QueryCondition. The returned QueryCondition
     * will be attached and belong to the DataReader. It will trigger on any
     * sample state, view state, or instance state.
     * <p>
     * The selection of the content is done using the queryExpression with
     * parameters queryParameters.
     * <ul>
     * <li>The queryExpression attribute is a string that specifies the criteria
     *     to select the data samples of interest. It is similar to the WHERE
     *     part of an SQL clause.</li>
     * <li>The queryParameters attribute is a sequence of strings that give
     *     values to the "parameters" (i.e., "%n" tokens) in the queryExpression.
     *     The number of supplied parameters must fit with the requested values
     *     in the queryExpression (i.e., the number of "%n" tokens).</li>
     * </ul>
     * <pre>
     * Example:
     * <code>
     * This example assumes a Subscriber called "subscriber" and DataReader named "fooDr" has been created
     * and a datatype called Foo is present: class Foo { int age; String name; };
     * // Create a QueryCondition
     * // And apply a condition that only samples with age &gt; 0 and name equals BILL are shown.
     * String expr = "age &gt; %0 AND name = %1");
     * List&lt;String&gt; params = new ArrayList&lt;String&gt;();
     * params.add("0");
     * params.add("BILL");
     * QueryCondition&lt;Foo&gt; queryCond = fooDr.createQueryCondition(expr,params);
     * </code>
     * </pre>
     * @return a new QueryCondition
     * @param   queryExpression The returned condition will only trigger on
     *          samples that pass this content-based filter expression.
     * @param   queryParameters A set of parameter values for the
     *          queryExpression.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #createQueryCondition(org.omg.dds.sub.Subscriber.DataState, String, List)
     */
    public QueryCondition<TYPE> createQueryCondition(
            String queryExpression,
            List<String> queryParameters);

    /**
     * This operation creates a QueryCondition. The returned QueryCondition
     * will be attached and belong to the DataReader. It will trigger on any
     * sample state, view state, or instance state.
     * <p>
     * The selection of the content is done using the queryExpression with
     * parameters queryParameters.
     * <ul>
     * <li>The queryExpression attribute is a string that specifies the criteria
     *     to select the data samples of interest. It is similar to the WHERE
     *     part of an SQL clause.</li>
     * <li>The queryParameters attribute is a sequence of strings that give
     *     values to the "parameters" (i.e., "%n" tokens) in the queryExpression.
     *     The number of supplied parameters must fit with the requested values
     *     in the queryExpression (i.e., the number of "%n" tokens).</li>
     * </ul>
     * <pre>
     * Example:
     * <code>
     * This example assumes a Subscriber called "subscriber" and DataReader named "fooDr" has been created
     * and a datatype called Foo is present: class Foo { int age; String name; };
     * // Create a QueryCondition
     * // And apply a condition that only samples with age &gt; 0 and name equals BILL are shown.
     * String expr = "age &gt; %0 AND name = %1");
     * QueryCondition&lt;Foo&gt; queryCond = fooDr.createQueryCondition(expr,"0","BILL");
     * </code>
     * </pre>
     * @return a new QueryCondition
     * @param   queryExpression The returned condition will only trigger on
     *          samples that pass this content-based filter expression.
     * @param   queryParameters A set of parameter values for the
     *          queryExpression.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #createQueryCondition(org.omg.dds.sub.Subscriber.DataState, String, List)
     */
    public QueryCondition<TYPE> createQueryCondition(
            String queryExpression,
            String... queryParameters);

    /**
     * This operation creates a QueryCondition. The returned QueryCondition
     * will be attached and belong to the DataReader.
     * <p>
     * The selection of the content is done using the queryExpression with
     * parameters queryParameters.
     * <ul>
     * <li>The queryExpression attribute is a string that specifies the criteria
     *     to select the data samples of interest. It is similar to the WHERE
     *     part of an SQL clause.</li>
     * <li>The queryParameters attribute is a sequence of strings that give
     *     values to the "parameters" (i.e., "%n" tokens) in the queryExpression.
     *     The number of supplied parameters must fit with the requested values
     *     in the queryExpression (i.e., the number of "%n" tokens).</li>
     * </ul>
     * <p>
     * State Masks<p>
     * The result of the QueryCondition also depends on the selection of samples
     * determined by three {@link org.omg.dds.sub.Subscriber.DataState} masks:
     * <ul>
     * <li>{@link org.omg.dds.sub.SampleState} is the mask, which selects only those samples with the desired
     *     sample states READ, NOT_READ or both.</li>
     * <li>{@link org.omg.dds.sub.ViewState} is the mask, which selects only those samples with the desired view states
     *     NEW, NOT_NEW or both</li>
     * <li>{@link org.omg.dds.sub.InstanceState} is the mask, which selects only those samples with the desired instance states
     *     ALIVE_INSTANCE, NOT_ALIVE_DISPOSED, NOT_ALIVE_NO_WRITERS or a combination of these.</li>
     * </ul>
     * <pre>
     * Example:
     * <code>
     * This example assumes a Subscriber called "subscriber" and DataReader named "fooDr" has been created
     * and a datatype called Foo is present: class Foo { int age; String name; };
     * // Create a QueryCondition with a DataState with a sample state of not read, new view state and alive instance state.
     * // And apply a condition that only samples with age &gt; 0 and name equals BILL are shown.
     * DataState ds = subscriber.createDataState();
     * ds = ds.with(SampleState.NOT_READ)
     *        .with(ViewState.NEW)
     *        .with(InstanceState.ALIVE);
     * String expr = "age &gt; %0 AND name = %1");
     * List&lt;String&gt; params = new ArrayList&lt;String&gt;();
     * params.add("0");
     * params.add("BILL");
     * QueryCondition&lt;Foo&gt; queryCond = fooDr.createQueryCondition(ds,expr,params);
     * </code>
     * </pre>
     * @return a new QueryCondition
     * @param   states  The returned condition will only trigger on samples
     *          with one of these sample states, view states, and instance
     *          states.
     * @param   queryExpression The returned condition will only trigger on
     *          samples that pass this content-based filter expression.
     * @param   queryParameters A set of parameter values for the
     *          queryExpression.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #createQueryCondition(String, List)
     */
    public QueryCondition<TYPE> createQueryCondition(
            Subscriber.DataState states,
            String queryExpression,
            List<String> queryParameters);

    /**
     * This operation creates a QueryCondition. The returned QueryCondition
     * will be attached and belong to the DataReader.
     * <p>
     * The selection of the content is done using the queryExpression with
     * parameters queryParameters.
     * <ul>
     * <li>The queryExpression attribute is a string that specifies the criteria
     *     to select the data samples of interest. It is similar to the WHERE
     *     part of an SQL clause.</li>
     * <li>The queryParameters attribute is a sequence of strings that give
     *     values to the "parameters" (i.e., "%n" tokens) in the queryExpression.
     *     The number of supplied parameters must fit with the requested values
     *     in the queryExpression (i.e., the number of "%n" tokens).</li>
     * </ul>
     * <p>
     * State Masks<p>
     * The result of the QueryCondition also depends on the selection of samples
     * determined by three {@link org.omg.dds.sub.Subscriber.DataState} masks:
     * <ul>
     * <li>{@link org.omg.dds.sub.SampleState} is the mask, which selects only those samples with the desired
     *     sample states READ, NOT_READ or both.</li>
     * <li>{@link org.omg.dds.sub.ViewState} is the mask, which selects only those samples with the desired view states
     *     NEW, NOT_NEW or both</li>
     * <li>{@link org.omg.dds.sub.InstanceState} is the mask, which selects only those samples with the desired instance states
     *     ALIVE_INSTANCE, NOT_ALIVE_DISPOSED, NOT_ALIVE_NO_WRITERS or a combination of these.</li>
     * </ul>
     * <pre>
     * Example:
     * <code>
     * This example assumes a Subscriber called "subscriber" and DataReader named "fooDr" has been created
     * and a datatype called Foo is present: class Foo { int age; String name; };
     * // Create a QueryCondition with a DataState with a sample state of not read, new view state and alive instance state.
     * // And apply a condition that only samples with age &gt; 0 and name equals BILL are shown.
     * DataState ds = subscriber.createDataState();
     * ds = ds.with(SampleState.NOT_READ)
     *        .with(ViewState.NEW)
     *        .with(InstanceState.ALIVE);
     * String expr = "age &gt; %0 AND name = %1");
     * QueryCondition&lt;Foo&gt; queryCond = fooDr.createQueryCondition(ds,expr,"0","BILL");
     * </code>
     * </pre>
     * @return a new QueryCondition
     * @param   states  The returned condition will only trigger on samples
     *          with one of these sample states, view states, and instance
     *          states.
     * @param   queryExpression The returned condition will only trigger on
     *          samples that pass this content-based filter expression.
     * @param   queryParameters A set of parameter values for the
     *          queryExpression.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #createQueryCondition(String, List)
     */
    public QueryCondition<TYPE> createQueryCondition(
            Subscriber.DataState states,
            String queryExpression,
            String... queryParameters);

    /**
     * This operation closes all the entities that were created by means of the
     * "create" operations on the DataReader. That is, it closes all contained
     * ReadCondition and QueryCondition objects.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the any of the contained entities is in a state where it
     *             cannot be closed.
     */
    public void closeContainedEntities();

    /**
     * @return  the TopicDescription associated with the DataReader. This is
     *          the same TopicDescription that was used to create the
     *          DataReader.
     */
    public TopicDescription<TYPE> getTopicDescription();

    /**
     * This operation obtains the SampleRejectedStatus object of the DataReader.
     * This object contains the information whether a received sample has been rejected.
     * Samples may be rejected by the DataReader when it runs out of resourceLimits
     * to store incoming samples. Usually this means that old samples need to be 'consumed'
     * (for example by 'taking' them instead of 'reading' them) to make room for newly
     * incoming samples.
     * <p>
     * The SampleRejectedStatus can also be monitored using a {@link DataReaderListener}
     * or by using the associated StatusCondition.
     *
     * @return the SampleRejectedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     org.omg.dds.core.status.SampleRejectedStatus
     */
    public SampleRejectedStatus getSampleRejectedStatus();

    /**
     * This operation obtains the LivelinessChangedStatus object of the DataReader.
     * This object contains the information whether the liveliness of one or more
     * DataWriter objects that were writing instances read by the DataReader has
     * changed. In other words, some DataWriter have become "alive" or "not alive".
     * <p>
     * The LivelinessChangedStatus can also be monitored using a {@link DataReaderListener}
     * or by using the associated StatusCondition.
     * @return the LivelinessChangedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     org.omg.dds.core.status.LivelinessChangedStatus
     */
    public LivelinessChangedStatus getLivelinessChangedStatus();

    /**
     * This operation obtains the RequestedDeadlineMissedStatus object of the DataReader.
     * This object contains the information whether the deadline that the DataReader was
     * expecting through its DeadlineQosPolicy was not respected for a specific instance.
     * <p>
     * The RequestedDeadlineMissedStatus can also be monitored using a {@link DataReaderListener}
     * or by using the associated StatusCondition.
     * @return the RequestedDeadlineMissedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     org.omg.dds.core.status.RequestedDeadlineMissedStatus
     */
    public RequestedDeadlineMissedStatus getRequestedDeadlineMissedStatus();

    /**
     * This operation obtains the RequestedIncompatibleQosStatus object of the DataReader.
     * This object contains the information whether a QosPolicy setting was incompatible
     * with the offered QosPolicy setting. The Request/Offering mechanism is applicable
     * between the DataWriter and the DataReader. If the QosPolicy settings between DataWriter
     * and DataReader are inconsistent, no communication between them is established.
     * In addition the DataWriter will be informed via a RequestedIncompatibleQos status
     * change and the DataReader will be informed via an OfferedIncompatibleQos status change.
     * <p>
     * The RequestedIncompatibleQosStatus can also be monitored using a {@link DataReaderListener}
     * or by using the associated StatusCondition.
     * @return the RequestedDeadlineMissedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     org.omg.dds.core.status.RequestedIncompatibleQosStatus
     */
    public RequestedIncompatibleQosStatus getRequestedIncompatibleQosStatus();

    /**
     * This operation obtains the SubscriptionMatchedStatus object of the DataReader.
     * This object contains the information whether a new match has been discovered
     * for the current subscription, or whether an existing match has ceased to exist.
     * This means that the status represents that either a DataWriter object has been
     * discovered by the DataReader with the same Topic and a compatible Qos, or that a
     * previously discovered DataWriter has ceased to be matched to the current DataReader.
     * A DataWriter may cease to match when it gets deleted, when it changes its Qos to a
     * value that is incompatible with the current DataReader or when either the DataReader
     * or the DataWriter has chosen to put its matching counterpart on its ignore-list using
     * the ignorePublication or ignoreSubcription operations on the DomainParticipant.
     * The operation may fail if the infrastructure does not hold the information necessary
     * to fill in the SubscriptionMatchedStatus. This is the case when OpenSplice is
     * configured not to maintain discovery information in the Networking Service.
     * (See the description for the NetworkingService/Discovery/enabled property in the
     * Deployment Manual for more information about this subject.) In this case the operation
     * will return an UnsupportedOperationException.
     * <p>
     * The SubscriptionMatchedStatus can also be monitored using a {@link DataReaderListener}
     * or by using the associated StatusCondition.
     * @return the SubscriptionMatchedStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     org.omg.dds.core.status.SubscriptionMatchedStatus
     */
    public SubscriptionMatchedStatus getSubscriptionMatchedStatus();

    /**
     * This operation obtains the SampleLostStatus object of the DataReader.
     * This object contains information whether samples have been lost.
     * This only applies when the ReliabilityQosPolicy is set to RELIABLE.
     * If the ReliabilityQosPolicy is set to BEST_EFFORT the Data Distribution Service
     * will not report the loss of samples.
     * <p>
     * The SampleLostStatus can also be monitored using a {@link DataReaderListener}
     * or by using the associated StatusCondition.
     * @return the SampleLostStatus object
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     org.omg.dds.core.status.SampleLostStatus
     */
    public SampleLostStatus getSampleLostStatus();

    /**
     * This operation is intended only for DataReader entities for which
     * {@link org.omg.dds.core.policy.Durability#getKind()} is not
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE}.
     * <p>
     * As soon as an application enables a non-VOLATILE DataReader it will
     * start receiving both "historical" data, i.e., the data that was
     * written prior to the time the DataReader joined the domain, as well as
     * any new data written by the DataWriter entities. There are situations
     * where the application logic may require the application to wait until
     * all "historical" data is received. This is the purpose of this
     * operation.
     * <p>
     * The operation blocks the calling thread until either all "historical"
     * data is received, or else the duration specified by the maxWait
     * parameter elapses, whichever happens first.
     * @param maxWait   the maximum duration to block for the waitForHistoricalData,
     *                  after which the application thread is unblocked.
     * @throws  TimeoutException        if maxWait elapsed before all the
     *          data was received.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     #waitForHistoricalData(long, TimeUnit)
     */
    public void waitForHistoricalData(Duration maxWait)
            throws TimeoutException;

    /**
     * This operation is intended only for DataReader entities for which
     * {@link org.omg.dds.core.policy.Durability#getKind()} is not
     * {@link org.omg.dds.core.policy.Durability.Kind#VOLATILE}.
     * <p>
     * As soon as an application enables a non-VOLATILE DataReader it will
     * start receiving both "historical" data, i.e., the data that was
     * written prior to the time the DataReader joined the domain, as well as
     * any new data written by the DataWriter entities. There are situations
     * where the application logic may require the application to wait until
     * all "historical" data is received. This is the purpose of this
     * operation.
     * <p>
     * The operation blocks the calling thread until either all "historical"
     * data is received, or else the duration specified by the maxWait
     * parameter elapses, whichever happens first.
     *
     * @param maxWait   The maximum duration to block for the waitForHistoricalData,
     *                  after which the application thread is unblocked.
     * @param unit      The TimeUnit which the sourceTimestamp describes (i.e. TimeUnit.SECONDS or TimeUnit.MILLISECONDS)
     * @throws  TimeoutException        if maxWait elapsed before all the
     *          data was received.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     * @see     #waitForHistoricalData(Duration)
     */
    public void waitForHistoricalData(long maxWait, TimeUnit unit)
            throws TimeoutException;

    /**
     * This operation retrieves the list of publications currently
     * "associated" with the DataReader; that is, publications that have a
     * matching {@link org.omg.dds.topic.Topic} and compatible QoS that the application has not
     * indicated should be "ignored" by means of
     * {@link org.omg.dds.domain.DomainParticipant#ignorePublication(InstanceHandle)}.
     * <p>
     * The handles returned in the 'publicationHandles' list are the ones
     * that are used by the DDS implementation to locally identify the
     * corresponding matched DataWriter entities. These handles match the
     * ones that appear in {@link org.omg.dds.sub.Sample#getInstanceHandle()} when reading
     * the "DCPSPublications" built-in topic.
     * <p>
     * Be aware that since an instance handle is an opaque datatype, it does not necessarily
     * mean that the handles obtained from the getMatchedPublications operation have the same
     * value as the ones that appear in the instanceHandle field of the SampleInfo when retrieving
     * the subscription info through corresponding "DCPSPublications" built-in reader. You can't
     * just compare two handles to determine whether they represent the same publication. If you want
     * to know whether two handles actually do represent the same subscription, use both handles to
     * retrieve their corresponding PublicationBuiltinTopicData samples and then compare the key field
     * of both samples.
     * <p>
     * The operation may fail with an UnsupportedOperationException if the infrastructure
     * does not locally maintain the connectivity information.
     *
     * @return  a new collection containing a copy of the information.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws  UnsupportedOperationException   if the infrastructure does
     *          not hold the information necessary to fill in the
     *          publicationnData.
     * @see     #getMatchedPublicationData(InstanceHandle)
     */
    public Set<InstanceHandle> getMatchedPublications();

    /**
     * This operation retrieves information on a publication that is
     * currently "associated" with the DataReader; that is, a publication
     * with a matching {@link org.omg.dds.topic.Topic} and compatible QoS that the application
     * has not indicated should be "ignored" by means of
     * {@link org.omg.dds.domain.DomainParticipant#ignorePublication(InstanceHandle)}.
     * <p>
     * The operation {@link #getMatchedPublications()} can be used
     * to find the publications that are currently matched with the
     * DataReader.
     *
     * @param   publicationHandle       a handle to the publication, the
     *          data of which is to be retrieved.
     *
     * @return  a new object containing a copy of the information.
     *
     * @throws  IllegalArgumentException        if the publicationHandle does
     *          not correspond to a publication currently associated with the
     *          DataReader.
     * @throws  UnsupportedOperationException   if the infrastructure does
     *          not hold the information necessary to fill in the
     *          publicationData.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws  UnsupportedOperationException   if the infrastructure does
     *          not hold the information necessary to fill in the
     *          publicationnData.
     * @see     #getMatchedPublications()
     */
    public PublicationBuiltinTopicData getMatchedPublicationData(
            InstanceHandle publicationHandle);


    // --- Type-specific interface: ------------------------------------------

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #read(Selector)} except that the
     * collection of returned samples is not constrained by any Selector.
     * These samples contain the actual data and meta information.
     * <p>
     * <b><i>Invalid Data</i></b><br>
     * Some elements in the returned sequence may not have valid data: the valid_data
     * field in the SampleInfo indicates whether the corresponding data value contains
     * any meaningful data. If not, the data value is just a 'dummy' sample for which only
     * the keyfields have been assigned. It is used to accompany the SampleInfo that
     * communicates a change in the instanceState of an instance for which there is
     * no 'real' sample available.
     * <p>
     * For example, when an application always 'takes' all available samples of a
     * particular instance, there is no sample available to report the disposal of that
     * instance. In such a case the DataReader will insert a dummy sample into the
     * data values sequence to accompany the SampleInfo element that communicates
     * the disposal of the instance.
     * <p>
     * The act of reading a sample sets its sample_state to SampleState.READ. If
     * the sample belongs to the most recent generation of the instance, it also sets the
     * viewState of the instance to ViewState.NOT_NEW. It does not affect the
     * instanceState of the instance.
     * <p>
     * Example:
     * <pre>
     * <code>
     * // Read samples
     * Iterator&lt;Sample&lt;Foo&gt;&gt; samples = fooDr.read();
     * // Process each Sample and print its name and production time.
     * while (samples.hasNext()) {
     *     Sample sample = samples.next();
     *     Foo foo = sample.getData();
     *     if (foo != null) { //Check if the sample is valid.
     *          Time t = sample.getSourceTimestamp(); // meta information
     *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
     *     }
     * }
     * </code>
     * </pre>
     *
     * @return  a non-null unmodifiable iterator over loaned samples.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #read(Selector)
     * @see     #read(List)
     * @see     #read(List, Selector)
     * @see     #readNextSample(Sample)
     * @see     #take()
     */
    public Sample.Iterator<TYPE> read();

    /**
     * This operation accesses a collection of samples from this DataReader.
     * The returned samples will be limited by the given {@link Selector}. The
     * setting of the {@link org.omg.dds.core.policy.Presentation}
     * may impose further limits on the returned samples.
     *
     * <ol>
     *     <li>If
     *         {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *         is
     *         {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#INSTANCE},
     *         then samples belonging to the same data instance are consecutive.
     *         </li>
     *     <li>If
     *         {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *         is
     *         {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#TOPIC}
     *         and
     *         {@link org.omg.dds.core.policy.Presentation#isOrderedAccess()}
     *         is set to false, then samples belonging to the same data
     *         instance are consecutive.</li>
     *     <li>If
     *         {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *         is
     *         {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#TOPIC}
     *         and
     *         {@link org.omg.dds.core.policy.Presentation#isOrderedAccess()}
     *         is set to true, then samples belonging to the same instance
     *         may or may not be consecutive. This is because to preserve
     *         order it may be necessary to mix samples from different
     *         instances.</li>
     *     <li>If
     *         {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *         is
     *         {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP}
     *         and
     *         {@link org.omg.dds.core.policy.Presentation#isOrderedAccess()}
     *         is set to false, then samples belonging to the same data
     *         instance are consecutive.</li>
     *     <li>If
     *         {@link org.omg.dds.core.policy.Presentation#getAccessScope()}
     *         is
     *         {@link org.omg.dds.core.policy.Presentation.AccessScopeKind#GROUP}
     *         and
     *         {@link org.omg.dds.core.policy.Presentation#isOrderedAccess()}
     *         is set to true, then the returned collection contains at most
     *         one sample. The difference in this case is due to the fact
     *         that it is required that the application is able to read
     *         samples belonging to different DataReader objects in a
     *         specific order.
     *         </li>
     * </ol>
     *
     * In any case, the relative order between the samples of one instance is
     * consistent with the
     * {@link org.omg.dds.core.policy.DestinationOrder}:
     *
     * <ul>
     *     <li>If
     *         {@link org.omg.dds.core.policy.DestinationOrder#getKind()}
     *         is
     *         {@link org.omg.dds.core.policy.DestinationOrder.Kind#BY_RECEPTION_TIMESTAMP},
     *         samples belonging to the same instances will appear in the
     *         relative order in which they were received (FIFO, earlier
     *         samples ahead of the later samples).</li>
     *     <li>If
     *         {@link org.omg.dds.core.policy.DestinationOrder#getKind()}
     *         is
     *         {@link org.omg.dds.core.policy.DestinationOrder.Kind#BY_SOURCE_TIMESTAMP},
     *         samples belonging to the same instances will appear in the
     *         relative order implied by the result of
     *         {@link org.omg.dds.sub.Sample#getSourceTimestamp()} (FIFO, smaller values of
     *         the source time stamp ahead of the larger values).</li>
     * </ul>
     *
     * In addition to the sample data, the read operation also provides
     * sample meta-information ("sample info"). See {@link org.omg.dds.sub.Sample}.
     * <p>
     * The returned samples are "loaned" by the DataReader. The use of this
     * variant allows for zero-copy (assuming the implementation supports it)
     * access to the data and the application will need to "return the loan"
     * to the DataReader using the {@link Sample.Iterator#close()}
     * operation.
     * <p>
     * Some elements in the returned collection may not have valid data. If
     * the instance state in the Sample is
     * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_DISPOSED} or
     * {@link org.omg.dds.sub.InstanceState#NOT_ALIVE_NO_WRITERS}, then the last sample for
     * that instance in the collection, that is, the one with
     * {@link org.omg.dds.sub.Sample#getSampleRank()} == 0, does not contain valid data.
     * Samples that contain no data do not count towards the limits imposed
     * by the {@link org.omg.dds.core.policy.ResourceLimits}.
     * <p>
     * The act of reading a sample sets its sample state to
     * {@link org.omg.dds.sub.SampleState#READ}. If the sample belongs to the most recent
     * generation of the instance, it will also set the view state of the
     * instance to {@link org.omg.dds.sub.ViewState#NOT_NEW}. It will not affect the
     * instance state of the instance.
     * <p>
     * If the DataReader has no samples that meet the constraints, the
     * return value will be a non-null iterator that provides no samples.
    * <p>
     * <b><i>Invalid Data</i></b><br>
     * Some elements in the returned sequence may not have valid data: the valid_data
     * field in the SampleInfo indicates whether the corresponding data value contains
     * any meaningful data. If not, the data value is just a 'dummy' sample for which only
     * the keyfields have been assigned. It is used to accompany the SampleInfo that
     * communicates a change in the instanceState of an instance for which there is
     * no 'real' sample available.
     * <p>
     * For example, when an application always 'takes' all available samples of a
     * particular instance, there is no sample available to report the disposal of that
     * instance. In such a case the DataReader will insert a dummy sample into the
     * data values sequence to accompany the SampleInfo element that communicates
     * the disposal of the instance.
     * <p>
     * The act of reading a sample sets its sample_state to SampleState.READ. If
     * the sample belongs to the most recent generation of the instance, it also sets the
     * viewState of the instance to ViewState.NOT_NEW. It does not affect the
     * instanceState of the instance.
     * <p>
     * Example:
     * <pre>
     * <code>
     * // create a dataState that only reads not read, new and alive samples
     * DataState ds = subscriber.createDataState();
     * ds = ds.with(SampleState.NOT_READ)
     *        .with(ViewState.NEW)
     *        .with(InstanceState.ALIVE);
     * // Read samples through an selector with the set dataState
     * Selector&lt;Foo&gt; query = fooDr.select();
     * query.dataState(ds);
     * Iterator&lt;Sample&lt;Foo&gt;&gt; samples = fooDr.read(query);
     * // Process each Sample and print its name and production time.
     * while (samples.hasNext()) {
     *     Sample sample = samples.next();
     *     Foo foo = sample.getData();
     *     if (foo != null) { //Check if the sample is valid.
     *          Time t = sample.getSourceTimestamp(); // meta information
     *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
     *     }
     * }
     * </code>
     * </pre>
     * @param query a selector to be used on the read
     * @return  a non-null unmodifiable iterator over loaned samples.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #read()
     * @see     #read(List)
     * @see     #read(List, Selector)
     * @see     #readNextSample(Sample)
     * @see     #take(Selector)
     */
    public Sample.Iterator<TYPE> read(Selector<TYPE> query);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #read()} except that the returned
     * samples are no more than #maxSamples.
     *
     * @param maxSamples The maximum number of samples to read
     * @return  a non-null unmodifiable iterator over loaned samples.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #read()
     * @see     #read(Selector)
     * @see     #read(List, Selector)
     * @see     #readNextSample(Sample)
     * @see     #take(List)
     */
    public Sample.Iterator<TYPE> read(int maxSamples);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #read()} except that the returned
     * samples are not "on loan" from the Service; they are deeply copied to
     * the application.
     * <p>
     * If the number of samples read are fewer than the current
     * length of the list, the list will be trimmed to fit the
     * samples read. If list is null, a new list will be allocated
     * and its size may be zero or unbounded depending upon the
     * number of samples read. If there are no samples, the list
     * reference will be non-null and the list will contain zero
     * samples.
     * <p>
     * The read operation will copy the data and meta-information into the
     * elements already inside the given collection, overwriting any samples
     * that might already be present. The use of this variant forces a copy
     * but the application can control where the copy is placed and the
     * application will not need to "return the loan."
     * <pre>
     * <code>
     * // Read samples and store them in a pre allocated collection
     * List&lt;Sample&lt;Foo&gt;&gt; samples = new ArrayList&lt;Sample&lt;Foo&gt;&gt;();
     * fooDr.read(samples);
     * // Process each Sample and print its name and production time.
     * for (Sample&lt;Foo&gt; sample : samples) {
     *     Foo foo = sample.getData();
     *     if (foo != null) { //Check if the sample is valid.
     *          Time t = sample.getSourceTimestamp();
     *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
     *     }
     * }
     * </code>
     * </pre>
     *
     * @param samples   A pre allocated collection to store the samples in
     * @return  <code>samples</code>, for convenience.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #read()
     * @see     #read(Selector)
     * @see     #read(List, Selector)
     * @see     #readNextSample(Sample)
     * @see     #take(List)
     */
    public List<Sample<TYPE>> read(List<Sample<TYPE>> samples);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #read(Selector)} except that the returned
     * samples are not "on loan" from the Service; they are deeply copied to
     * the application.
     * <p>
     * The number of samples are specified as the minimum of
     * {@link Selector#getMaxSamples()} and the length of the list.
     * If the number of samples read are fewer than the current
     * length of the list, the list will be trimmed to fit the
     * samples read. If list is null, a new list will be allocated
     * and its size may be zero or unbounded depending upon the
     * number of samples read. If there are no samples, the list
     * reference will be non-null and the list will contain zero
     * samples.
     * <p>
     * The read operation will copy the data and meta-information into the
     * elements already inside the given collection, overwriting any samples
     * that might already be present. The use of this variant forces a copy
     * but the application can control where the copy is placed and the
     * application will not need to "return the loan."
     * @param samples   A pre allocated collection to store the samples in
     * @param selector  A selector to be used on the read
     * @return  <code>samples</code>, for convenience.
     *
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #read()
     * @see     #read(Selector)
     * @see     #read(List)
     * @see     #readNextSample(Sample)
     * @see     #take(List, Selector)
     */
    public List<Sample<TYPE>> read(
            List<Sample<TYPE>> samples,
            Selector<TYPE> selector);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * The behavior is identical to {@link #read} except for that the samples are removed
     * from the DataReader.
     * <p>
     * <b><i>Invalid Data</i></b><br>
     * Some elements in the returned sequence may not have valid data: the valid_data
     * field in the SampleInfo indicates whether the corresponding data value contains
     * any meaningful data. If not, the data value is just a 'dummy' sample for which only
     * the keyfields have been assigned. It is used to accompany the SampleInfo that
     * communicates a change in the instanceState of an instance for which there is
     * no 'real' sample available.
     * <p>
     * For example, when an application always 'takes' all available samples of a
     * particular instance, there is no sample available to report the disposal of that
     * instance. In such a case the DataReader will insert a dummy sample into the
     * data values sequence to accompany the SampleInfo element that communicates
     * the disposal of the instance.
     * <p>
     * Example:
     * <pre>
     * <code>
     * // Take samples
     * Iterator&lt;Sample&lt;Foo&gt;&gt; samples = fooDr.take();
     * // Process each Sample and print its name and production time.
     * while (samples.hasNext()) {
     *     Sample sample = samples.next();
     *     Foo foo = sample.getData();
     *     if (foo != null) { //Check if the sample is valid.
     *          Time t = sample.getSourceTimestamp(); // meta information
     *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
     *     }
     * }
     * </code>
     * </pre>
     *
     * @return  a non-null unmodifiable iterator over loaned samples.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #take(Selector)
     * @see     #take(List)
     * @see     #take(List, Selector)
     * @see     #takeNextSample(Sample)
     * @see     #read()
     */
    public Sample.Iterator<TYPE> take();

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #take(Selector)} except that the
     * collection of returned samples is not constrained by any Selector.
     * <p>
     * The number of samples accessible via the iterator will not be
     * more than #maxSamples.
     *
     * @return  a non-null unmodifiable iterator over loaned samples.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #take(Selector)
     * @see     #take(List)
     * @see     #take(List, Selector)
     * @see     #takeNextSample(Sample)
     * @see     #read()
     */
    public Sample.Iterator<TYPE> take(int maxSamples);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * The number of samples returned is controlled by the
     * {@link org.omg.dds.core.policy.Presentation} and other
     * factors using the same logic as for {@link #read(Selector)}.
     * <p>
     * The act of taking a sample removes it from the DataReader so it cannot
     * be "read" or "taken" again. If the sample belongs to the most recent
     * generation of the instance, it will also set the view state of the
     * instance to {@link org.omg.dds.sub.ViewState#NOT_NEW}. It will not affect the
     * instance state of the instance.
     * <p>
     * The behavior of the take operation follows the same rules than the
     * read operation regarding the preconditions and postconditions for the
     * arguments and return results. Similar to read, the take operation will
     * "loan" elements to the application; this loan must then be returned by
     * means of {@link Sample.Iterator#close()}. The only difference
     * with read is that, as stated, the sample returned by take will no
     * longer be accessible to successive calls to read or take.
     * <p>
     * If the DataReader has no samples that meet the constraints, the
     * return value will be a non-null iterator that provides no samples.
     * <p>
     * Example:
     * <pre>
     * <code>
     * // create a dataState that only reads not read, new and alive samples
     * DataState ds = subscriber.createDataState();
     * ds = ds.with(SampleState.NOT_READ)
     *        .with(ViewState.NEW)
     *        .with(InstanceState.ALIVE);
     * // Take samples through an selector with the set dataState
     * Selector&lt;Foo&gt; query = fooDr.select();
     * query.dataState(ds);
     * Iterator&lt;Sample&lt;Foo&gt;&gt; samples = fooDr.take(query);
     * // Process each Sample and print its name and production time.
     * while (samples.hasNext()) {
     *     Sample sample = samples.next();
     *     Foo foo = sample.getData();
     *     if (foo != null) { //Check if the sample is valid.
     *          Time t = sample.getSourceTimestamp(); // meta information
     *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
     *     }
     * }
     * </code>
     * </pre>
     * @param query a selector to be used on the read
     * @return  a non-null unmodifiable iterator over loaned samples.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #take()
     * @see     #take(List)
     * @see     #take(List, Selector)
     * @see     #takeNextSample(Sample)
     * @see     #read(Selector)
     */
    public Sample.Iterator<TYPE> take(Selector<TYPE> query);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #take()} except that the returned
     * samples are not "on loan" from the Service; they are deeply copied to
     * the application.
     * <p>
     * The take operation will copy the data and meta-information into the
     * elements already inside the given collection, overwriting any samples
     * that might already be present. The use of this variant forces a copy
     * but the application can control where the copy is placed and the
     * application will not need to "return the loan."
     *
     * <pre>
     * <code>
     * // Take samples and store them in a pre allocated collection
     * List&lt;Sample&lt;Foo&gt;&gt; samples = new ArrayList&lt;Sample&lt;Foo&gt;&gt;();
     * fooDr.take(samples);
     * // Process each Sample and print its name and production time.
     * for (Sample&lt;Foo&gt; sample : samples) {
     *     Foo foo = sample.getData();
     *     if (foo != null) { //Check if the sample is valid.
     *          Time t = sample.getSourceTimestamp();
     *          System.out.println("Name: " + foo.myName + " is produced at " + time.getTime(TimeUnit.SECONDS));
     *     }
     * }
     * </code>
     * </pre>
     * @param samples   A pre allocated collection to store the samples in
     * @return  <code>samples</code>, for convenience.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #take()
     * @see     #take(Selector)
     * @see     #take(List, Selector)
     * @see     #takeNextSample(Sample)
     * @see     #read(List)
     */
    public List<Sample<TYPE>> take(List<Sample<TYPE>> samples);

    /**
     * This operation accesses a collection of samples from this DataReader.
     * It behaves exactly like {@link #take(Selector)} except that the returned
     * samples are not "on loan" from the Service; they are deeply copied to
     * the application.
     * <p>
     * The take operation will copy the data and meta-information into the
     * elements already inside the given collection, overwriting any samples
     * that might already be present. The use of this variant forces a copy
     * but the application can control where the copy is placed and the
     * application will not need to "return the loan."
     *
     * @param samples   A pre allocated collection to store the samples in
     * @param query  A selector to be used on the take
     * @return  <code>samples</code>, for convenience.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #take()
     * @see     #take(Selector)
     * @see     #take(List)
     * @see     #takeNextSample(Sample)
     * @see     #read(List, Selector)
     */
    public List<Sample<TYPE>> take(
            List<Sample<TYPE>> samples,
            Selector<TYPE> query);

    /**
     * This operation copies the next, non-previously accessed sample from
     * this DataReader. The implied order among the samples stored in the
     * DataReader is the same as for {@link #read(List, Selector)}.
     * <p>
     * This operation is semantically equivalent to
     * {@link #read(List, Selector)} where {@link Selector#getMaxSamples()}
     * is 1, {@link Selector#getDataState()} followed by
     * {@link Subscriber.DataState#getSampleStates()} ==
     * {@link org.omg.dds.sub.SampleState#NOT_READ}, {@link Selector#getDataState()} followed by
     * {@link Subscriber.DataState#getViewStates()} contains all view
     * states, and {@link Selector#getDataState()} followed by
     * {@link Subscriber.DataState#getInstanceStates()} contains all
     * instance states.
     * <p>
     * This operation provides a simplified API to "read" samples avoiding
     * the need for the application to manage iterators and specify queries.
     * <p>
     * If there is no unread data in the DataReader, the operation will
     * return false and the provided sample is not modified.
     *
     * @return  true if data was read or false if no data was available.
     * @param sample    A valid sample that will contain the new data.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #read()
     * @see     #read(Selector)
     * @see     #read(List)
     * @see     #read(List, Selector)
     * @see     #takeNextSample(Sample)
     */
    public boolean readNextSample(Sample<TYPE> sample);

    /**
     * This operation copies the next, non-previously accessed sample from
     * this DataReader and "removes" it from the DataReader so it is no
     * longer accessible. This operation is analogous to
     * {@link #readNextSample(Sample)} except for the fact that the sample is
     * "removed" from the DataReader.
     * <p>
     * This operation is semantically equivalent to
     * {@link #take(List, Selector)} where {@link Selector#getMaxSamples()}
     * is 1, {@link Selector#getDataState()} followed by
     * {@link Subscriber.DataState#getSampleStates()} ==
     * {@link org.omg.dds.sub.SampleState#NOT_READ}, {@link Selector#getDataState()} followed by
     * {@link Subscriber.DataState#getViewStates()} contains all view
     * states, and {@link Selector#getDataState()} followed by
     * {@link Subscriber.DataState#getInstanceStates()} contains all
     * instance states.
     * <p>
     * This operation provides a simplified API to "take" samples avoiding
     * the need for the application to manage iterators and specify queries.
     * <p>
     * If there is no unread data in the DataReader, the operation will
     * return false and the provided sample is not modified.
     *
     * @return  true if data was taken or false if no data was available.
     * @param sample    A valid sample that will contain the new data.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @see     #take()
     * @see     #take(Selector)
     * @see     #take(List)
     * @see     #take(List, Selector)
     * @see     #readNextSample(Sample)
     */
    public boolean takeNextSample(Sample<TYPE> sample);

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
     * @return  keyHolder, as a convenience to facilitate chaining.
     *
     * @throws  IllegalArgumentException        if the {@link org.omg.dds.core.InstanceHandle}
     *          does not correspond to an existing data object known to the
     *          DataReader. If the implementation is not able to check
     *          invalid handles, then the result in this situation is
     *          unspecified.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    public TYPE getKeyValue(
            TYPE keyHolder,
            InstanceHandle handle);

    /**
     * This operation can be used to retrieve the instance key that
     * corresponds to an instance handle. The operation will only fill the
     * fields that form the key inside the keyHolder instance.
     *
     * @param   handle          a handle indicating the instance whose value
     *          this method should get.
     *
     * @return  An instance with key fields populated.
     *
     * @throws  IllegalArgumentException        if the {@link org.omg.dds.core.InstanceHandle}
     *          does not correspond to an existing data object known to the
     *          DataReader. If the implementation is not able to check
     *          invalid handles, then the result in this situation is
     *          unspecified.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
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
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DataReader has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    public InstanceHandle lookupInstance(TYPE keyHolder);


    // --- From Entity: ------------------------------------------------------
    @Override
    public StatusCondition<DataReader<TYPE>> getStatusCondition();

    @Override
    public Subscriber getParent();

    /**
     * Provides a {@link Selector} that can be used to refine what {@link #read} or
     * {@link #take} methods return.
     *
     * @return The {@link Selector} object returned by this method
     *         is the default selector. By default it selects
     *         {@link org.omg.dds.core.policy.ResourceLimits#LENGTH_UNLIMITED}
     *         samples.  This is equivalent to calling {@link org.omg.dds.sub.DataReader#read} without
     *         any parameters.
     *
     */
    public Selector<TYPE> select();

    /**
     * Selector class encapsulates different ways of selecting samples from a {@link org.omg.dds.sub.DataReader}.
     * Selector can be used with {@link org.omg.dds.sub.DataReader#read(Selector)} and {@link org.omg.dds.sub.DataReader#take(Selector)}
     * or it can be used stand-alone as it provides {@link #read} and {@link #take} functions.
     * <p>
     * {@link org.omg.dds.sub.DataReader#select} creates a Selector that is bound to the {@link org.omg.dds.sub.DataReader}.
     * <p>
     * A Selector may encapsulate any combination of {@link org.omg.dds.core.InstanceHandle},
     * {@link Subscriber.DataState}, a query filter. It can be used to bound the maximum
     * number of samples retrieved.
     * <p>
     * The DataReader has the select() operation, which can be used to acquire the Selector
     * functionality on the reader implicitly.
     * <pre>
     * <code>
     * // Take a maximum of 3 new samples of a certain instance.
     * samples = reader.select()
     *                     .maxSamples(3)
     *                     .dataState(subscriber.createDataState().withAnyInstanceState().withAnySampleState().withAnyViewState())
     *                     .instance(someValidInstanceHandle)
     *                     .take();
     * </code>
     * </pre>
     * However, this will create and destroy a Selector for every read/take, which is not
     * very performance friendly.
     * <p>
     * The performance can be increase by creating a Selector up front and doing the
     * reading on that Selector directly and re-using it.
     * <pre>
     * <code>
     * // Create a Selector as selective reader up front.
     * Selector&lt;Foo&gt; selector = reader.select();
     * // Configure it to take a maximum of 3 new samples of a certain instance
     * selector.maxSamples(3);
     * selector.dataState(subscriber.createDataState().withAnyInstanceState().withAnySampleState().withAnyViewState());
     * selector.instance(someValidInstanceHandle);
     *
     * // Use the configured Selector to -take- a maximum of 3 new samples of a
     * // certain instance (which it was configured to do).
     * // This can be used in loops for example, reducing the need for creating
     * // implicit Selectors for every take.
     * samples = selector.take();
     * </code>
     * </pre>
     *<pre>
     * <i>Defaults</i>
     * Element         | Default Value
     * --------------- | ----------------
     * dataState       | Any
     * condition       | null
     * maxSamples      | LENGTH_UNLIMITED
     * instance        | nilHandle
     * nextInstance    | false
     * </pre>
     * @param <T>    The concrete type of the data to be read.
     *
     */
    public static interface Selector<T> extends DDSObject {

        // --- Setters ----------------------------------------------

        /**
         * Set InstanceHandle to filter with during the read or take.
         * <p>
         * <i>Example</i><br>
         * <pre>
         * <code>
         * // Read only samples of the given instance.
         * InstanceHandle hdl = someValidInstanceHandle;
         *
         * // Implicit use of Selector
         * samples = reader.select().instance(hdl).read();
         *
         * // Explicit use of Selector
         * Selector&lt;Foo&gt; selector = reader.select();
         * samples = selector.instance(hdl).read();
         * </code>
         * </pre>
         * @param handle the InstanceHandle to read/take for
         * @return a Selector to be able to concatenate Selector settings.
         */
        public Selector<T> instance(InstanceHandle handle);
        /**
         * Set next InstanceHandle to filter with during the read or take.
         * <p>
         * <i>Example</i><br>
         * <pre>
         * <code>
         * // Read all samples, instance by instance by an implicit use of Selector
         * {
         *     // Get sample(s) of first instance
         *     samples = reader.select().nextInstance(true).read();
         *     while (samples.hasNext()) {
         *         // Handle the sample(s) of this instance (just the first one in this case)
         *         Sample sample = samples.next();
         *         samples = reader.select().instance(sample.getInstanceHandle()).nextInstance(true).read();
         *     }
         * }
         * -------------------------------------------------------------------------------------------------
         * // Read all samples, instance by instance by an explicit use of Selector
         * {
         *     // Get sample(s) of first instance
         *     Selector&lt;Foo&gt; selector = reader.select();
         *     selector.nextInstance(true);
         *     samples = selector.read();
         *     while (samples.hasNext()) {
         *         // Handle the sample(s) of this instance (just the first one in this case)
         *         Sample sample = samples.next();
         *         // because we use the same selector we don't need to set the nextInstance flag again.
         *         samples = selector.instance(sample.getInstanceHandle()).read();
         *     }
         * }
         * </code>
         * </pre>
         *
         * @param retrieveNextInstance this boolean indicates if we want to read/take the next instance or not.
         * @return a Selector to be able to concatenate Selector settings.
         */
        public Selector<T> nextInstance(boolean retrieveNextInstance);

        /**
         * Set DataState to filter with during the read or take.
         * <p>
         * <i>Example</i><br>
         * Read only new data.
         * <pre>
         * <code>
         * // DataState to filter only new data
         * DataState newData = subscriber.createDataState().withAnyInstanceState().withAnySampleState().with(ViewState.NEW);
         *
         * // Implicit use of Selector
         * samples = reader.select().dataState(newData).read();
         *
         * // Explicit use of Selector
         * Selector&lt;Foo&gt; selector = reader.select();
         * selector.dataState(newData);
         * samples = selector.read();
         * </code>
         * </pre>
         *
         * @param state the requested DataState of the samples
         * @return a Selector to be able to concatenate Selector settings.
         */
        public Selector<T> dataState(Subscriber.DataState state);

        /**
         * Set the Content to filter with during the read or take.
         * <p>
         * <i>Example</i><br>
         * Read only samples that will be filtered according to the given queryExpression and queryParameters.
         * <pre>
         * <code>
         * // Assume data type has an element called long_1
         * queryExpression = "long_1 &gt; %0 and long_1 &lt; %1";
         * List&lt;String&gt; queryParameters = new ArrayList&lt;String&gt;();
         * queryParameters.add("1");
         * queryParameters.add("7");
         *
         * // Implicit use of Selector
         * samples = reader.select().Content(queryExpression,queryParameters).read();
         *
         * // Explicit use of Selector
         * Selector&lt;Foo&gt; selector = reader.select();
         * selector.Content(queryExpression,queryParameters);
         * samples = selector.read();
         * </code>
         * </pre>
         *
         * @param   queryExpression The returned condition will only trigger on
         *          samples that pass this content-based filter expression.
         * @param   queryParameters A collection of parameter values for the
         *          queryExpression.
         * @return a Selector to be able to concatenate Selector settings.
         */
        public Selector<T> Content(String queryExpression, List<String> queryParameters);

        /**
         * Set the Content to filter with during the read or take.
         * <p>
         * <i>Example</i><br>
         * Read only samples that will be filtered according to the given queryExpression and queryParameters.
         * <pre>
         * <code>
         * // Assume data type has an element called long_1
         * queryExpression = "long_1 &gt; %0 and long_1 &lt; %1";
         *
         * // Implicit use of Selector
         * samples = reader.select().Content(queryExpression,"1","7").read();
         *
         * // Explicit use of Selector
         * Selector&lt;Foo&gt; selector = reader.select();
         * selector.Content(queryExpression,"1","7");
         * samples = selector.read();
         * </code>
         * </pre>
         *
         * @param   queryExpression The returned condition will only trigger on
         *          samples that pass this content-based filter expression.
         * @param   queryParameters A set of parameter values for the
         *          queryExpression.
         * @return a Selector to be able to concatenate Selector settings.
         */
        public Selector<T> Content(String queryExpression, String... queryParameters);
        /**
         * Set maxSamples to limit the number of sample to get during the read or take.
         * <p>
         * <i>Example</i><br>
         * Read a maximum of three samples.
         * <pre>
         * <code>
         * // Implicit use of Selector
         * samples = reader.select().maxSamples(3).read();
         *
         * // Explicit use of Selector
         * Selector&lt;Foo&gt; selector = reader.select();
         * selector.maxSamples(3);
         * samples = selector.read();
         * </code>
         * </pre>
         *
         * @param max maximum number of samples to read/take
         * @return a Selector to be able to concatenate Selector settings.
         */
        public Selector<T> maxSamples(int max);

        // --- Getters ----------------------------------------------

        /**
         * Returns the current instance the selector is working on
         * @return the current instance
         */
        public InstanceHandle getInstance();
        /**
         * Returns if the selector should use the next instance or not
         * @return the retrieveNextInstance value
         */
        public boolean retrieveNextInstance();
        /**
         * Returns the dataState of the selector
         * @return the dataState
         */
        public Subscriber.DataState getDataState();
        /**
         * Returns the query expression of the selector
         * @return the query expression
         */
        public String getQueryExpression();
        /**
         * Returns a collection of the query parameters of the selector
         * @return a collection of the query parameters
         */
        public List<String> getQueryParameters();
        /**
         * Returns the maximum number of samples of the selector
         * @return the maximum number of samples
         */
        public int getMaxSamples();
        /**
         * Returns the ReadCondition of the selector
         * @return the ReadCondition
         */
        public ReadCondition<T> getCondition();

        // --- read/take operations ----------------------------------

        /**
         * This operation works the same as the {@link DataReader#read()}, except that it is performed on this Selector
         * with possible filters set.
         *
         * @return          A samples iterator
         * @throws org.omg.dds.core.DDSException
         *                  An internal error has occurred.
         * @throws org.omg.dds.core.AlreadyClosedException
         *                  The corresponding DataWriter has been closed.
         * @throws org.omg.dds.core.OutOfResourcesException
         *                  The Data Distribution Service ran out of resources to
         *                  complete this operation.
         */
        public Sample.Iterator<T> read();
        /**
         * This operation works the same as the {@link DataReader#read(List)}, except that it is performed on this Selector
         * with possible filters set.
         *
         * @param samples   The collection where the results of the read are stored in
         * @return          A samples collection
         * @throws org.omg.dds.core.DDSException
         *                  An internal error has occurred.
         * @throws org.omg.dds.core.AlreadyClosedException
         *                  The corresponding DataWriter has been closed.
         * @throws org.omg.dds.core.OutOfResourcesException
         *                  The Data Distribution Service ran out of resources to
         *                  complete this operation.
         */
        public List<Sample<T>> read(List<Sample<T>> samples);
        /**
         * This operation works the same as the {@link DataReader#take()}, except that it is performed on this Selector
         * with possible filters set.
         *
         * @return          A samples iterator
         * @throws org.omg.dds.core.DDSException
         *                  An internal error has occurred.
         * @throws org.omg.dds.core.AlreadyClosedException
         *                  The corresponding DataWriter has been closed.
         * @throws org.omg.dds.core.OutOfResourcesException
         *                  The Data Distribution Service ran out of resources to
         *                  complete this operation.
         */
        public Sample.Iterator<T> take();
        /**
         * This operation works the same as the {@link DataReader#take(List)}, except that it is performed on this Selector
         * with possible filters set.
         *
         * @param samples   The collection where the results of the take are stored in
         * @return          A samples collection
         * @throws org.omg.dds.core.DDSException
         *                  An internal error has occurred.
         * @throws org.omg.dds.core.AlreadyClosedException
         *                  The corresponding DataWriter has been closed.
         * @throws org.omg.dds.core.OutOfResourcesException
         *                  The Data Distribution Service ran out of resources to
         *                  complete this operation.
         */
        public List<Sample<T>> take(List<Sample<T>> samples);
    }
}
