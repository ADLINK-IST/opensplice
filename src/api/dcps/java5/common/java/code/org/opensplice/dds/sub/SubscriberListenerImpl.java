/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
package org.opensplice.dds.sub;

import java.io.Serializable;

import org.omg.dds.sub.SubscriberListener;
import org.opensplice.dds.core.Listener;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.event.DataAvailableEventImpl;
import org.opensplice.dds.core.event.DataOnReadersEventImpl;
import org.opensplice.dds.core.event.LivelinessChangedEventImpl;
import org.opensplice.dds.core.event.RequestedDeadlineMissedEventImpl;
import org.opensplice.dds.core.event.RequestedIncompatibleQosEventImpl;
import org.opensplice.dds.core.event.SampleLostEventImpl;
import org.opensplice.dds.core.event.SampleRejectedEventImpl;
import org.opensplice.dds.core.event.SubscriptionMatchedEventImpl;
import org.opensplice.dds.core.status.DataAvailableStatusImpl;
import org.opensplice.dds.core.status.DataOnReadersStatusImpl;
import org.opensplice.dds.core.status.StatusConverter;

import DDS.DataReader;
import DDS.LivelinessChangedStatus;
import DDS.RequestedDeadlineMissedStatus;
import DDS.RequestedIncompatibleQosStatus;
import DDS.SampleLostStatus;
import DDS.SampleRejectedStatus;
import DDS.Subscriber;
import DDS.SubscriptionMatchedStatus;

public class SubscriberListenerImpl extends Listener<SubscriberListener>
        implements DDS.SubscriberListener, Serializable {
    private static final long serialVersionUID = -1147185392577428150L;
    private final transient SubscriberImpl subscriber;

    public SubscriberListenerImpl(OsplServiceEnvironment environment,
            SubscriberImpl subscriber, SubscriberListener listener) {
        this(environment, subscriber, listener, false);
    }
    public SubscriberListenerImpl(OsplServiceEnvironment environment,
            SubscriberImpl subscriber, SubscriberListener listener,
            boolean waitUntilInitialised) {
        super(environment, listener, waitUntilInitialised);
        this.subscriber = subscriber;
    }

    @Override
    public void on_data_on_readers(Subscriber subs) {
        this.waitUntilInitialised();
        this.listener.onDataOnReaders(new DataOnReadersEventImpl(
                this.environment, this.subscriber, new DataOnReadersStatusImpl(
                        this.environment)));
    }

    @Override
    public void on_requested_deadline_missed(DataReader reader,
            RequestedDeadlineMissedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onRequestedDeadlineMissed(new RequestedDeadlineMissedEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_sample_rejected(DataReader reader,
            SampleRejectedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onSampleRejected(new SampleRejectedEventImpl<Object>(
                    this.environment, found, StatusConverter.convert(
                            this.environment, status)));
        }
    }

    @Override
    public void on_liveliness_changed(DataReader reader,
            LivelinessChangedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onLivelinessChanged(new LivelinessChangedEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_data_available(DataReader reader) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onDataAvailable(new DataAvailableEventImpl<Object>(
                    this.environment, found, new DataAvailableStatusImpl(
                            this.environment)));
        }
    }

    @Override
    public void on_requested_incompatible_qos(DataReader arg0,
            RequestedIncompatibleQosStatus arg1) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(arg0);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onRequestedIncompatibleQos(new RequestedIncompatibleQosEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, arg1)));
        }
    }

    @Override
    public void on_subscription_matched(DataReader reader,
            SubscriptionMatchedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onSubscriptionMatched(new SubscriptionMatchedEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_sample_lost(DataReader reader, SampleLostStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.subscriber
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onSampleLost(new SampleLostEventImpl<Object>(
                    this.environment, found, StatusConverter.convert(
                            this.environment, status)));
        }
    }

}
