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

import org.omg.dds.core.event.DataAvailableEvent;
import org.omg.dds.core.event.DataOnReadersEvent;
import org.omg.dds.core.event.LivelinessChangedEvent;
import org.omg.dds.core.event.RequestedDeadlineMissedEvent;
import org.omg.dds.core.event.RequestedIncompatibleQosEvent;
import org.omg.dds.core.event.SampleLostEvent;
import org.omg.dds.core.event.SampleRejectedEvent;
import org.omg.dds.core.event.SubscriptionMatchedEvent;

/**
 * A convenience class SubscriberAdapter is offered which has an empty implementation of all
 * {@link org.omg.dds.sub.SubscriberListener} callback functions when the application extends from this class only the used callback
 * functions that the user wants to use need to be implemented.
 */
public class SubscriberAdapter implements SubscriberListener
{
    @Override
    public void onDataAvailable(DataAvailableEvent<?> status)
    {
        // empty
    }

    @Override
    public void onLivelinessChanged(LivelinessChangedEvent<?> status)
    {
        // empty
    }

    @Override
    public void onRequestedDeadlineMissed(
            RequestedDeadlineMissedEvent<?> status)
    {
        // empty
    }

    @Override
    public void onRequestedIncompatibleQos(
            RequestedIncompatibleQosEvent<?> status)
    {
        // empty
    }

    @Override
    public void onSampleLost(SampleLostEvent<?> status)
    {
        // empty
    }

    @Override
    public void onSampleRejected(SampleRejectedEvent<?> status)
    {
        // empty
    }

    @Override
    public void onSubscriptionMatched(SubscriptionMatchedEvent<?> status)
    {
        // empty
    }

    @Override
    public void onDataOnReaders(DataOnReadersEvent status)
    {
        // empty
    }
}
