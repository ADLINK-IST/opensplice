/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderListener;
import org.opensplice.dds.core.Listener;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.event.DataAvailableEventImpl;
import org.opensplice.dds.core.event.LivelinessChangedEventImpl;
import org.opensplice.dds.core.event.RequestedDeadlineMissedEventImpl;
import org.opensplice.dds.core.event.RequestedIncompatibleQosEventImpl;
import org.opensplice.dds.core.event.SampleLostEventImpl;
import org.opensplice.dds.core.event.SampleRejectedEventImpl;
import org.opensplice.dds.core.event.SubscriptionMatchedEventImpl;
import org.opensplice.dds.core.status.DataAvailableStatusImpl;
import org.opensplice.dds.core.status.StatusConverter;

import DDS.LivelinessChangedStatus;
import DDS.RequestedDeadlineMissedStatus;
import DDS.RequestedIncompatibleQosStatus;
import DDS.SampleLostStatus;
import DDS.SampleRejectedStatus;
import DDS.SubscriptionMatchedStatus;

public class DataReaderListenerImpl<TYPE> extends
        Listener<DataReaderListener<TYPE>> implements DDS.DataReaderListener,
        Serializable {
    private static final long serialVersionUID = 6892152554338498323L;
    private final transient DataReader<TYPE> reader;

    public DataReaderListenerImpl(OsplServiceEnvironment environment,
            DataReader<TYPE> reader, DataReaderListener<TYPE> listener) {
        this(environment, reader, listener, false);
    }

    public DataReaderListenerImpl(OsplServiceEnvironment environment,
            DataReader<TYPE> reader, DataReaderListener<TYPE> listener,
            boolean waitUntilInitialised) {
        super(environment, listener, waitUntilInitialised);
        this.reader = reader;
    }

    @Override
    public void on_requested_deadline_missed(DDS.DataReader reader,
            RequestedDeadlineMissedStatus status) {
        this.waitUntilInitialised();
        this.listener
                .onRequestedDeadlineMissed(new RequestedDeadlineMissedEventImpl<TYPE>(
                        this.environment, this.reader, StatusConverter.convert(
                                this.environment, status)));
    }

    @Override
    public void on_sample_rejected(DDS.DataReader reader,
            SampleRejectedStatus status) {
        this.waitUntilInitialised();
        this.listener.onSampleRejected(new SampleRejectedEventImpl<TYPE>(
                this.environment, this.reader, StatusConverter.convert(
                        this.environment, status)));
    }

    @Override
    public void on_liveliness_changed(DDS.DataReader reader,
            LivelinessChangedStatus status) {
        this.waitUntilInitialised();
        this.listener.onLivelinessChanged(new LivelinessChangedEventImpl<TYPE>(
                this.environment, this.reader, StatusConverter.convert(
                        this.environment, status)));
    }

    @Override
    public void on_data_available(DDS.DataReader reader) {
        this.waitUntilInitialised();
        this.listener.onDataAvailable(new DataAvailableEventImpl<TYPE>(
                this.environment, this.reader, new DataAvailableStatusImpl(
                        this.environment)));
    }

    @Override
    public void on_requested_incompatible_qos(DDS.DataReader reader,
            RequestedIncompatibleQosStatus status) {
        this.waitUntilInitialised();
        this.listener
                .onRequestedIncompatibleQos(new RequestedIncompatibleQosEventImpl<TYPE>(
                        this.environment, this.reader, StatusConverter.convert(
                                this.environment, status)));
    }

    @Override
    public void on_subscription_matched(DDS.DataReader reader,
            SubscriptionMatchedStatus status) {
        this.waitUntilInitialised();
        this.listener
                .onSubscriptionMatched(new SubscriptionMatchedEventImpl<TYPE>(
                        this.environment, this.reader, StatusConverter.convert(
                                this.environment, status)));
    }

    @Override
    public void on_sample_lost(DDS.DataReader reader, SampleLostStatus status) {
        this.waitUntilInitialised();
        this.listener.onSampleLost(new SampleLostEventImpl<TYPE>(
                this.environment, this.reader, StatusConverter.convert(
                        this.environment, status)));
    }
}
