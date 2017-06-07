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

package org.omg.dds.topic;

import java.util.EventListener;


import org.omg.dds.core.event.InconsistentTopicEvent;


/**
 * Since {@link org.omg.dds.topic.Topic} is a kind of {@link org.omg.dds.core.Entity}, it has the ability to
 * have an associated listener. In this case, the associated listener must be
 * of concrete type TopicListener.
 *
 * <p>
 * <b><i>Example</i></b>
 * <pre>
 * <code>
 * public class MyTopicListener extends TopicAdapter&lt;Foo&gt;
 * {
 *     public void onInconsistentTopic(InconsistentTopicEvent&lt;Foo&gt; status)
 *          // Handle the inconsistency here.
 *     }
 * }
 * //Instantiate a TopicListener.
 * MyTopicListener listener = new MyTopicListener();
 * // Instantiate and add the status masks.
 * Collection&lt;Class&lt;? extends Status&gt;&gt; statuses = new HashSet&lt;Class&lt;? extends Status&gt;&gt;();
 * statuses.add(InconsistentTopicStatus.class);
 * // Create the Topic with the listener and the bit-mask.
 * participant.createTopic("fooTopic", Foo.class, participant.getDefaultTopicQos(), listener, statuses);
 * </code>
 * </pre>
 * @param <TYPE>    The concrete type of the data published and/or subscribed
 *                  by the readers and writers that use to topic.
 */
public interface TopicListener<TYPE> extends EventListener {
    /**
     * This operation is the external operation (interface, which must be implemented by the application)
     * that is called by the Data Distribution Service when the {@link org.omg.dds.core.status.InconsistentTopicStatus} changes.
     * The implementation may be left empty when this functionality is not needed. This operation will only be called when
     * the relevant TopicListener is installed and enabled for the InconsistentTopicStatus. The InconsistentTopicStatus will
     * change when another Topic exists with the same topicName but different characteristics. The Data Distribution Service will
     * call the TopicListener operation with a parameter status, which will contain the InconsistentTopicStatus struct.
     * @param status     Contains the InconsistentTopicStatus object (this is an input to the application).
     */
    public void onInconsistentTopic(InconsistentTopicEvent<TYPE> status);
}
