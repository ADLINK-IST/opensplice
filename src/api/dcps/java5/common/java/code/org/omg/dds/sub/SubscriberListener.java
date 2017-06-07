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

import java.util.EventListener;

import org.omg.dds.core.event.DataAvailableEvent;
import org.omg.dds.core.event.DataOnReadersEvent;
import org.omg.dds.core.event.LivelinessChangedEvent;
import org.omg.dds.core.event.RequestedDeadlineMissedEvent;
import org.omg.dds.core.event.RequestedIncompatibleQosEvent;
import org.omg.dds.core.event.SampleLostEvent;
import org.omg.dds.core.event.SampleRejectedEvent;
import org.omg.dds.core.event.SubscriptionMatchedEvent;


/**
 * Since a {@link org.omg.dds.sub.Subscriber} is a kind of {@link org.omg.dds.core.Entity}, it has the ability
 * to have an associated listener. In this case, the associated listener must
 * be of concrete type SubscriberListener.
 * <p>
 * A convenience class {@link org.omg.dds.sub.SubscriberAdapter} is offered in Java 5 which has an empty implementation of all
 * listener callback functions when the application extends from this class only the used callback functions that the user wants
 * to use need to be implemented.
 *
 * <p>
 * <b><i>Example</i></b>
 * <pre>
 * <code>
 * public class MySubscriberListener extends SubscriberAdapter
 * {
 *      public void onDataAvailable(DataAvailableEvent&lt;?&gt; status) {
 *          // Handle the incoming data here.
 *      }
 *      public void onRequestedIncompatibleQos(RequestedIncompatibleQosEvent&lt;?&gt; status) {
 *          // Handle the incompatibility here.
 *      }
 * }
 * //Instantiate a SubscriberListener.
 * MySubscriberListener listener = new MySubscriberListener();
 * // Instantiate and add the status masks.
 * Collection&lt;Class&lt;? extends Status&gt;&gt; statuses = new HashSet&lt;Class&lt;? extends Status&gt;&gt;();
 * statuses.add(DataAvailableStatus.class);
 * statuses.add(RequestedIncompatibleQosStatus.class);
 * // Create the Subscriber with the listener and the bit-mask.
 * participant.createSubscriber( participant.getDefaultSubscriberQos(), listener, statuses);
 * </code>
 * </pre>
 */
public interface SubscriberListener extends EventListener {
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the deadline
     * that the DataReader was expecting through its DeadlineQosPolicy was not respected
     * for a specific instance. The implementation may be left empty when this functionality
     * is not needed. This operation will only be called when the relevant SubscriberListener
     * is installed and enabled for the RequestedDeadlineMissedStatus.
     * <p>
     * The Data Distribution Service will provide a reference to the RequestedDeadlineMissedStatus object in the
     * parameter status for use by the application.
     * @param status    Contains the RequestedDeadlineMissedStatus object (this is an input to the application).
     */
    public void onRequestedDeadlineMissed(
            RequestedDeadlineMissedEvent<?> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the
     * RequestedIncompatibleQosStatus changes. The implementation may be left empty when this
     * functionality is not needed. This operation will only be called when the relevant SubscriberListener
     * is installed and enabled for the RequestedIncompatibleQosStatus.
     * <p>
     * The Data Distribution Service will provide a reference to the RequestedIncompatibleQosStatus object
     * in the parameter status, for use by the application.
     * <p>
     * The application can use this operation as a callback function implementing a proper response to the
     * status change. This operation is enabled by setting the RequestedIncompatibleQosStatus in the listener status mask
     * @param status    Contains the RequestedIncompatibleQosStatus object (this is an input to the application).
     */
    public void onRequestedIncompatibleQos(
            RequestedIncompatibleQosEvent<?> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when a (received)
     * sample has been rejected. Samples may be rejected by the DataReader when it runs out of
     * resource_limits to store incoming samples. Usually this means that old samples need to
     * be 'consumed' (for example by 'taking' them instead of 'reading' them) to make room for
     * newly incoming samples.
     * <p>
     * The implementation may be left empty when this functionality is not needed. This operation
     * will only be called when the relevant SubscriberListener is installed and enabled for the
     * SampleRejectedStatus.
     * <p>
     * The Data Distribution Service will provide a reference to the SampleRejectedStatus object in the parameter
     * status for use by the application.
     * @param status     Contains the SampleRejectedStatus object (this is an input to the application).
     */
    public void onSampleRejected(SampleRejectedEvent<?> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the liveliness of
     * one or more DataWriter objects that were writing instances read through this DataReader
     * has changed. In other words, some DataWriter have become "alive" or "not alive".
     * The implementation may be left empty when this functionality is not needed. This operation
     * will only be called when the relevant SubscriberListener is installed and enabled for the
     * LivelinessChangedStatus.
     * <p>
     * The Data Distribution Service will provide a reference to the
     * LivelinessChangedStatus object for use by the application.
     * @param status     Contains the LivelinessChangedStatus object (this is an input to the application).
     */
    public void onLivelinessChanged(LivelinessChangedEvent<?> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when new data is
     * available for this DataReader. The implementation may be left empty when this
     * functionality is not needed. This operation will only be called when the relevant
     * SubscriberListener is installed and enabled for the DataAvailableStatus.
     * <p>
     * The statuses DataOnReadersStatus and DataAvailableStatus will occur together. In case these status changes occur,
     * the Data Distribution Service will look for an attached and activated SubscriberListener
     * or DomainParticipantListener (in that order) for the DataOnReadersStatus. In case the
     * DataOnReadersStatus can not be handled, the Data Distribution Service will look for an
     * attached and activated DataReaderListener, SubscriberListener or DomainParticipantListener
     * for the DataAvailableStatus (in that order).
     * <p>
     * Note that if onDataOnReaders is called, then the Data Distribution Service will not try to
     * call onDataAvailable, however, the application can force a call to the DataReader objects
     * that have data by means of the notifyDatareaders operation.
     *
     * @param status    Contains the DataAvailableStatus object (this is an input to the application).
     */
    public void onDataAvailable(DataAvailableEvent<?> status);
    /**
     * This operation  is called by the Data Distribution Service when a new match
     * has been discovered for the current subscription, or when an existing match
     * has ceased to exist.
     * <p>
     * Usually this means that a new DataWriter that matches the Topic and that has compatible
     * Qos as the current DataReader has either been discovered, or that a previously discovered DataWriter
     * has ceased to be matched to the current DataReader. A DataWriter may cease to
     * match when it gets deleted, when it changes its Qos to a value that is incompatible
     * with the current DataReader or when either the DataReader or the DataWriter
     * has chosen to put its matching counterpart on its ignore-list using the
     * ignoreSubscription or ignorePublication operations.
     * <p>
     * The implementation of this Listener operation may be left empty when this
     * functionality is not needed: it will only be called when the relevant
     * SubscriberListener is installed and enabled for the SubscriptionMatchedStatus
     * @param status    Contains the SubscriptionMatchedStatus object (this is an input to the application).
     */
    public void onSubscriptionMatched(SubscriptionMatchedEvent<?> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when a sample is lost and this
     * is detected.
     * <p>
     * The implementation may be left empty when this functionality is not needed. This operation
     * will only be called when the relevant SubscriberListener is installed and enabled for the
     * SampleLostStatus.
     * <p>
     * The Data Distribution Service will provide a reference to the SampleLostStatus object
     * for use by the application.
     * @param status     Contains the SampleLostStatus object (this is an input to the application).
     */
    public void onSampleLost(SampleLostEvent<?> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when new data is
     * available for this Subscriber. The implementation may be left empty when this
     * functionality is not needed. This operation will only be called when the relevant
     * SubscriberListener is installed and enabled for the DataOnReadersStatus.
     * <p>
     * The statuses DataOnReadersStatus and DataAvailableStatus will occur together.
     * In case these status changes occur, the Data Distribution Service will look for an attached
     * and activated SubscriberListener or DomainParticipantListener (in that order) for the DataOnReadersStatus.
     * In case the DataOnReadersStatus can not be handled, the Data Distribution Service will look for an
     * attached and activated DataReaderListener, SubscriberListener or DomainParticipantListener
     * for the DataAvailableStatus (in that order).
     * <p>
     * Note that if onDataOnReaders is called, then the Data Distribution Service will not try to
     * call onDataAvailable, however, the application can force a call to the DataReader objects
     * that have data by means of the notifyDatareaders operation.
     *
     * @param status    Contains the DataOnReadersStatus object (this is an input to the application).
     */
    public void onDataOnReaders(DataOnReadersEvent status);
}
