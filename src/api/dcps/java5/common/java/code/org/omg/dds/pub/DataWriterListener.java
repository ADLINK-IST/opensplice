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

import java.util.EventListener;


import org.omg.dds.core.event.LivelinessLostEvent;
import org.omg.dds.core.event.OfferedDeadlineMissedEvent;
import org.omg.dds.core.event.OfferedIncompatibleQosEvent;
import org.omg.dds.core.event.PublicationMatchedEvent;


/**
 * Since a {@link org.omg.dds.pub.DataWriter} is a kind of {@link org.omg.dds.core.Entity}, it has the ability
 * to have a listener associated with it. In this case, the associated
 * listener must be of concrete type DataWriterListener.
 * <p>
 * This interface must be implemented by the application. A user-defined class must be provided by the application
 * which must implement the DataWriterListener interface. All DataWriterListener operations must be implemented
 * in the user-defined class, it is up to the application whether an operation is empty or contains some functionality.
 * <p>
 * A convenience class {@link org.omg.dds.pub.DataWriterAdapter} is offered in Java 5 which has an empty implementation of all
 * listener callback functions when the application extends from this class only the used callback functions that the user wants
 * to use need to be implemented.
 *
 * <p>
 * <b><i>Example</i></b>
 * <pre>
 * <code>
 * public class MyDataWriterListener extends DataWriterAdapter&lt;Foo&gt;
 * {
 *     public void onLivelinessLost(LivelinessLostEvent&lt;Foo&gt; status)
 *          // Handle the lost data here.
 *      }
 *      public void onOfferedIncompatibleQos(OfferedIncompatibleQosEvent&lt;Foo&gt; status)
 *          // Handle the incompatibility here.
 *      }
 * }
 * //Instantiate a DataWriterListener.
 * MyDataWriterListener listener = new MyDataWriterListener();
 * // Instantiate and add the status masks.
 * Collection&lt;Class&lt;? extends Status&gt;&gt; statuses = new HashSet&lt;Class&lt;? extends Status&gt;&gt;();
 * statuses.add(LivelinessLostStatus.class);
 * statuses.add(OfferedIncompatibleQosStatus.class);
 * // Create the DataWriter with the listener and the bit-mask.
 * publisher.createDataWriter(fooTopic, publisher.getDefaultDataWriterQos(), listener, statuses);
 * </code>
 * </pre>
 *
 * @param <TYPE>    The concrete type of the data written by the DataWriter.
 */
public interface DataWriterListener<TYPE> extends EventListener {

    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the
     * {@link org.omg.dds.core.status.OfferedDeadlineMissedStatus} changes. The implementation may be left empty
     * when this functionality is not needed. This operation will only be called when
     * the relevant DataWriterListener is installed and enabled for the offered deadline
     * missed status. The offered deadline missed status will change when the deadline
     * that the DataWriter has committed through its DeadlineQosPolicy was not respected
     * for a specific instance. The Data Distribution Service will call the DataWriterListener
     * operation with a parameter status, which will contain the OfferedDeadlineMissedStatus object.
     *
     * @param status    Contains the OfferedDeadlineMissedStatus object (this is an input to the application).
     */
    public void onOfferedDeadlineMissed(
            OfferedDeadlineMissedEvent<TYPE> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the
     * {@link org.omg.dds.core.status.OfferedIncompatibleQosStatus} changes. The implementation may be left
     * empty when this functionality is not needed. This operation will only be called when
     * the relevant DataWriterListener is installed and enabled for the OfferedIncompatibleQosStatus.
     * The incompatible Qos status will change when a DataReader object has been discovered by the
     * DataWriter with the same Topic and a requested DataReaderQos that was incompatible with the
     * one offered by the DataWriter. The Data Distribution Service will call the DataWriterListener
     * operation with a parameter status, which will contain the OfferedIncompatibleQosStatus object.
     *
     * @param status    Contain the OfferedIncompatibleQosStatus object (this is an input to the application).
     */
    public void onOfferedIncompatibleQos(
            OfferedIncompatibleQosEvent<TYPE> status);
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the
     * {@link org.omg.dds.core.status.LivelinessLostStatus} changes. The implementation may be left empty when this
     * functionality is not needed. This operation will only be called when the relevant
     * DataWriterListener is installed and enabled for the liveliness lost status.
     * The liveliness lost status will change when the liveliness that the DataWriter has
     * committed through its LivelinessQosPolicy was not respected. In other words, the
     * DataWriter failed to actively signal its liveliness within the offered liveliness
     * period. As a result, the DataReader objects will consider the DataWriter as no longer "alive".
     * The Data Distribution Service will call the DataWriterListener operation with
     * a parameter status, which will contain the LivelinessLostStatus object.
     *
     * @param status    Contains the LivelinessLostStatus object (this is an input to the application)
     */
    public void onLivelinessLost(LivelinessLostEvent<TYPE> status);
    /**
     * This operation must be implemented by the application and is called by the Data
     * Distribution Service when a new match has been discovered for the current publication,
     * or when an existing match has ceased to exist. Usually this means that a new DataReader
     * that matches the Topic and that has compatible Qos as the current DataWriter has either
     * been discovered, or that a previously discovered DataReader has ceased to be matched to
     * the current DataWriter. A DataReader may cease to match when it gets deleted, when it
     * changes its Qos to a value that is incompatible with the current DataWriter or when either
     * the DataWriter or the DataReader has chosen to put its matching counterpart on its ignore-list
     * using the ignoreSubcription or ignorePublication operations on the DomainParticipant.
     * The implementation of this Listener operation may be left empty when this functionality is not
     * needed: it will only be called when the relevant DataWriterListener is installed and enabled for
     * the {@link org.omg.dds.core.status.PublicationMatchedStatus}. The Data Distribution Service will
     * provide a reference to the PublicationMatchedStatus object in the parameter status for use by the application.
     *
     * @param status     Contains the PublicationMatchedStatus object (this is an input to the application
     *                   provided by the Data Distribution Service).
     */
    public void onPublicationMatched(PublicationMatchedEvent<TYPE> status);
}
