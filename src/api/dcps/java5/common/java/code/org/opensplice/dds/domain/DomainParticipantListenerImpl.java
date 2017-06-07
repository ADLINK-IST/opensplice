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
package org.opensplice.dds.domain;

import java.io.Serializable;

import org.omg.dds.domain.DomainParticipantListener;
import org.opensplice.dds.core.Listener;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.event.AllDataDisposedEventImpl;
import org.opensplice.dds.core.event.DataAvailableEventImpl;
import org.opensplice.dds.core.event.DataOnReadersEventImpl;
import org.opensplice.dds.core.event.InconsistentTopicEventImpl;
import org.opensplice.dds.core.event.LivelinessChangedEventImpl;
import org.opensplice.dds.core.event.LivelinessLostEventImpl;
import org.opensplice.dds.core.event.OfferedDeadlineMissedEventImpl;
import org.opensplice.dds.core.event.OfferedIncompatibleQosEventImpl;
import org.opensplice.dds.core.event.PublicationMatchedEventImpl;
import org.opensplice.dds.core.event.RequestedDeadlineMissedEventImpl;
import org.opensplice.dds.core.event.RequestedIncompatibleQosEventImpl;
import org.opensplice.dds.core.event.SampleLostEventImpl;
import org.opensplice.dds.core.event.SampleRejectedEventImpl;
import org.opensplice.dds.core.event.SubscriptionMatchedEventImpl;
import org.opensplice.dds.core.status.DataAvailableStatusImpl;
import org.opensplice.dds.core.status.DataOnReadersStatusImpl;
import org.opensplice.dds.core.status.StatusConverter;

import DDS.AllDataDisposedTopicStatusHolder;
import DDS.DataReader;
import DDS.DataWriter;
import DDS.InconsistentTopicStatus;
import DDS.LivelinessChangedStatus;
import DDS.LivelinessLostStatus;
import DDS.OfferedDeadlineMissedStatus;
import DDS.OfferedIncompatibleQosStatus;
import DDS.PublicationMatchedStatus;
import DDS.RequestedDeadlineMissedStatus;
import DDS.RequestedIncompatibleQosStatus;
import DDS.SampleLostStatus;
import DDS.SampleRejectedStatus;
import DDS.Subscriber;
import DDS.SubscriptionMatchedStatus;
import DDS.Topic;

public class DomainParticipantListenerImpl extends
        Listener<DomainParticipantListener> implements
        DDS.ExtDomainParticipantListener, Serializable {
    private static final long serialVersionUID = -3531755144455417494L;
    private final DomainParticipantImpl participant;
    private final transient org.opensplice.dds.domain.DomainParticipantListener extListener;

    public DomainParticipantListenerImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl participant,
            DomainParticipantListener listener) {
        this(environment, participant, listener, false);
    }

    public DomainParticipantListenerImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl participant,
            DomainParticipantListener listener, boolean waitUntilInitialised) {
        super(environment, listener, waitUntilInitialised);
        this.participant = participant;

        if (listener instanceof org.opensplice.dds.domain.DomainParticipantListener) {
            this.extListener = (org.opensplice.dds.domain.DomainParticipantListener) listener;
        } else {
            this.extListener = null;
        }

    }

    @Override
    public void on_inconsistent_topic(Topic the_topic,
            InconsistentTopicStatus status) {
        try {
            org.omg.dds.topic.Topic<Object> found = participant
                    .getTopic(the_topic);

            if (found != null) {
                this.waitUntilInitialised();
                this.listener
                        .onInconsistentTopic(new InconsistentTopicEventImpl<Object>(
                                this.environment, found, StatusConverter
                                        .convert(this.environment, status)));
            }
        } catch (ClassCastException e) {
        }
    }

    @Override
    public void on_offered_deadline_missed(DataWriter writer,
            OfferedDeadlineMissedStatus status) {

        org.omg.dds.pub.DataWriter<Object> found = this.participant
                .lookupDataWriter(writer);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onOfferedDeadlineMissed(new OfferedDeadlineMissedEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_offered_incompatible_qos(DataWriter writer,
            OfferedIncompatibleQosStatus status) {

        org.omg.dds.pub.DataWriter<Object> found = this.participant
                .lookupDataWriter(writer);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onOfferedIncompatibleQos(new OfferedIncompatibleQosEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_liveliness_lost(DataWriter writer,
            LivelinessLostStatus status) {
        org.omg.dds.pub.DataWriter<Object> found = this.participant
                .lookupDataWriter(writer);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onLivelinessLost(new LivelinessLostEventImpl<Object>(
                    this.environment, found, StatusConverter.convert(
                            this.environment, status)));
        }
    }

    @Override
    public void on_publication_matched(DataWriter writer,
            PublicationMatchedStatus status) {
        org.omg.dds.pub.DataWriter<Object> found = this.participant
                .lookupDataWriter(writer);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onPublicationMatched(new PublicationMatchedEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_data_on_readers(Subscriber subs) {
        org.omg.dds.sub.Subscriber found = this.participant
                .lookupSubscriber(subs);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onDataOnReaders(new DataOnReadersEventImpl(
                    this.environment, found, new DataOnReadersStatusImpl(
                            this.environment)));
        }
    }

    @Override
    public void on_requested_deadline_missed(DataReader reader,
            RequestedDeadlineMissedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.participant
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
    public void on_requested_incompatible_qos(DataReader reader,
            RequestedIncompatibleQosStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.participant
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener
                    .onRequestedIncompatibleQos(new RequestedIncompatibleQosEventImpl<Object>(
                            this.environment, found, StatusConverter.convert(
                                    this.environment, status)));
        }
    }

    @Override
    public void on_sample_rejected(DataReader reader,
            SampleRejectedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.participant
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
        org.omg.dds.sub.DataReader<Object> found = this.participant
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
        org.omg.dds.sub.DataReader<Object> found = this.participant
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onDataAvailable(new DataAvailableEventImpl<Object>(
                    this.environment, found, new DataAvailableStatusImpl(
                            this.environment)));
        }
    }

    @Override
    public void on_subscription_matched(DataReader reader,
            SubscriptionMatchedStatus status) {
        org.omg.dds.sub.DataReader<Object> found = this.participant
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
        org.omg.dds.sub.DataReader<Object> found = this.participant
                .lookupDataReader(reader);

        if (found != null) {
            this.waitUntilInitialised();
            this.listener.onSampleLost(new SampleLostEventImpl<Object>(
                    this.environment, found, StatusConverter.convert(
                            this.environment, status)));
        }
    }

    @Override
    public void on_all_data_disposed(Topic arg0) {
        org.omg.dds.topic.Topic<Object> topic;
        AllDataDisposedTopicStatusHolder holder = new AllDataDisposedTopicStatusHolder();
        int rc = arg0.get_all_data_disposed_topic_status(holder);

        if (rc != DDS.RETCODE_OK.value) {
            return;
        }

        try {
            topic = participant.getTopic(arg0);

            if (this.extListener != null) {
                this.waitUntilInitialised();

                if (holder.value != null) {
                    this.extListener
                            .onAllDataDisposed(new AllDataDisposedEventImpl<Object>(
                                    this.environment, topic, StatusConverter
                                            .convert(this.environment,
                                                    holder.value)));
                } else {
                    this.extListener
                            .onAllDataDisposed(new AllDataDisposedEventImpl<Object>(
                                    this.environment, topic, null));
                }
            }
        } catch (ClassCastException e) {
        }
    }
}
