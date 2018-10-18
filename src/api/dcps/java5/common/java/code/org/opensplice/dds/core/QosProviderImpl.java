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
package org.opensplice.dds.core;

import org.omg.dds.core.QosProvider;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.domain.DomainParticipantFactoryQos;
import org.omg.dds.domain.DomainParticipantQos;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.domain.DomainParticipantQosImpl;
import org.opensplice.dds.pub.DataWriterQosImpl;
import org.opensplice.dds.pub.PublisherQosImpl;
import org.opensplice.dds.sub.DataReaderQosImpl;
import org.opensplice.dds.sub.SubscriberQosImpl;
import org.opensplice.dds.topic.TopicQosImpl;

import DDS.DataReaderQosHolder;
import DDS.DataWriterQosHolder;
import DDS.PublisherQosHolder;
import DDS.SubscriberQosHolder;

public class QosProviderImpl extends QosProvider {
    private final OsplServiceEnvironment environment;
    private DDS.QosProvider old;

    public QosProviderImpl(OsplServiceEnvironment environment, String uri,
            String profile) {
        if (uri == null) {
            throw new IllegalArgumentExceptionImpl(environment, "Invalid uri provided");
        }
        this.environment = environment;
        try {
            this.old = new DDS.QosProvider(uri, profile);
        } catch (NullPointerException npe) {
            Utilities.throwLastErrorException(this.environment);
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public DomainParticipantFactoryQos getDomainParticipantFactoryQos() {
        return this.getDomainParticipantFactoryQos(null);
    }

    @Override
    public DomainParticipantFactoryQos getDomainParticipantFactoryQos(String id) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "QosProvider.getDomainParticipantFactoryQos() not supported.");
    }

    @Override
    public DomainParticipantQos getDomainParticipantQos() {
        return this.getDomainParticipantQos(null);
    }

    @Override
    public DomainParticipantQos getDomainParticipantQos(String id) {
        DDS.DomainParticipantQosHolder holder = new DDS.DomainParticipantQosHolder();
        int rc = old.get_participant_qos(holder, id);
        Utilities.checkReturnCode(rc, this.environment,
                "QosProvider.getDomainParticipantQos() failed.");
        if (rc == DDS.RETCODE_NO_DATA.value) {
            return null;
        }
        return DomainParticipantQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public TopicQos getTopicQos() {
        return this.getTopicQos(null);
    }

    @Override
    public TopicQos getTopicQos(String id) {
        DDS.TopicQosHolder holder = new DDS.TopicQosHolder();
        int rc = old.get_topic_qos(holder, id);
        Utilities.checkReturnCode(rc, this.environment,
                "QosProvider.getTopicQos() failed.");
        if (rc == DDS.RETCODE_NO_DATA.value) {
            return null;
        }
        return TopicQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public SubscriberQos getSubscriberQos() {
        return this.getSubscriberQos(null);
    }

    @Override
    public SubscriberQos getSubscriberQos(String id) {
        SubscriberQosHolder holder = new SubscriberQosHolder();
        int rc = this.old.get_subscriber_qos(holder, id);
        Utilities.checkReturnCode(rc, this.environment,
                "QosProvider.getSubscriberQos() failed.");
        if (rc == DDS.RETCODE_NO_DATA.value) {
            return null;
        }
        return SubscriberQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public PublisherQos getPublisherQos() {
        return this.getPublisherQos(null);
    }

    @Override
    public PublisherQos getPublisherQos(String id) {
        PublisherQosHolder holder = new PublisherQosHolder();
        int rc = this.old.get_publisher_qos(holder, id);
        Utilities.checkReturnCode(rc, this.environment,
                "QosProvider.getPublisherQos() failed.");
        if (rc == DDS.RETCODE_NO_DATA.value) {
            return null;
        }
        return PublisherQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public DataReaderQos getDataReaderQos() {
        return this.getDataReaderQos(null);
    }

    @Override
    public DataReaderQos getDataReaderQos(String id) {
        DataReaderQosHolder holder = new DataReaderQosHolder();
        int rc = this.old.get_datareader_qos(holder, id);
        Utilities.checkReturnCode(rc, this.environment,
                "QosProvider.getDataReaderQos() failed.");
        if (rc == DDS.RETCODE_NO_DATA.value) {
            return null;
        }
        return DataReaderQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public DataWriterQos getDataWriterQos() {
        return this.getDataWriterQos(null);
    }

    @Override
    public DataWriterQos getDataWriterQos(String id) {
        DataWriterQosHolder holder = new DataWriterQosHolder();
        int rc = this.old.get_datawriter_qos(holder, id);
        Utilities.checkReturnCode(rc, this.environment,
                "QosProvider.getDataWriterQos() failed.");
        if (rc == DDS.RETCODE_NO_DATA.value) {
            return null;
        }
        return DataWriterQosImpl.convert(this.environment, holder.value);
    }

}
