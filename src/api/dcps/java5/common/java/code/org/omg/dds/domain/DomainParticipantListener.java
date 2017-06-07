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

package org.omg.dds.domain;

import org.omg.dds.core.event.InconsistentTopicEvent;
import org.omg.dds.pub.PublisherListener;
import org.omg.dds.sub.SubscriberListener;


/**
 * This is the interface that can be implemented by an application-provided
 * class and then registered with the {@link org.omg.dds.domain.DomainParticipant} such that the
 * application can be notified by the DCPS Service of relevant status changes.
 * <p>
 * The purpose of the DomainParticipantListener is to be the listener of last
 * resort that is notified of all status changes not captured by more specific
 * listeners attached to the {@link org.omg.dds.core.DomainEntity} objects. When a relevant
 * status change occurs, the DCPS Service will first attempt to notify the
 * listener attached to the concerned DomainEntity if one is installed.
 * Otherwise, the DCPS Service will notify the Listener attached to the
 * DomainParticipant.
 * <p>
 * A convenience class {@link org.omg.dds.domain.DomainParticipantAdapter} is offered which has an empty implementation of all
 * DomainParticipantListener callback functions when the application extends from this class only the used callback
 * functions that the user wants to use need to be implemented.
 *
 * <p>
 * <b><i>Example</i></b>
 * <pre>
 * <code>
 * public class MyDomainParticipantListener extends DomainParticipantAdapter
 * {
 *      public void onDataAvailable(DataAvailableEvent&lt;?&gt; status)
 *          // Handle new data here.
 *      }
 *      public void onOfferedIncompatibleQos(OfferedIncompatibleQosEvent&lt;?&gt; status)
 *          // Handle the incompatibility here.
 *      }
 * }
 * //Instantiate a DomainParticipantListener.
 * MyDomainParticipantListener listener = new MyDomainParticipantListener();
 * // Instantiate and add the status masks.
 * Collection&lt;Class&lt;? extends Status&gt;&gt; statuses = new HashSet&lt;Class&lt;? extends Status&gt;&gt;();
 * statuses.add(DataAvailableStatus.class);
 * statuses.add(OfferedIncompatibleQosStatus.class);
 * // Create the DomainParticipant with the listener and the bit-mask and domainId 0.
 * dpf.createDomainParticipant(0, dpf.getDefaultParticipantQos(), listener, statuses);
 * </code>
 * </pre>
 */
public interface DomainParticipantListener
extends PublisherListener, SubscriberListener {
    /**
     * This operation is the external operation (interface, which must be implemented by
     * the application) that is called by the Data Distribution Service when the
     * {@link org.omg.dds.core.status.InconsistentTopicStatus} changes. The implementation may be
     * left empty when this functionality is not needed. This operation will only be called when the
     * relevant TopicListener is installed and enabled for the InconsistentTopicStatus.
     * The InconsistentTopicStatus will change when another Topic exists with the same topic_name but
     * different characteristics. The Data Distribution Service will call the DomainParticipantListener operation
     * with a parameter status, which will contain the object of the class InconsistentTopicStatus.
     *
     * @param status Contains the InconsistentTopicStatus object (this is an input to the application).
     */
    public void onInconsistentTopic(InconsistentTopicEvent<?> status);
}
