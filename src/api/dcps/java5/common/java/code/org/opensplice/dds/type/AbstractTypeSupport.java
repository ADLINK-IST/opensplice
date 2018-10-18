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
package org.opensplice.dds.type;

import java.util.Collection;

import org.omg.dds.core.status.Status;
import org.omg.dds.pub.DataWriterListener;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.sub.DataReaderListener;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.topic.TopicListener;
import org.omg.dds.topic.TopicQos;
import org.omg.dds.type.TypeSupport;
import org.opensplice.dds.domain.DomainParticipantImpl;
import org.opensplice.dds.pub.AbstractDataWriter;
import org.opensplice.dds.pub.PublisherImpl;
import org.opensplice.dds.sub.AbstractDataReader;
import org.opensplice.dds.sub.SubscriberImpl;
import org.opensplice.dds.topic.AbstractTopic;
import org.opensplice.dds.topic.TopicDescriptionExt;

public abstract class AbstractTypeSupport<TYPE> extends TypeSupport<TYPE> {
    public abstract DDS.TypeSupport getOldTypeSupport();

    public abstract AbstractDataWriter<TYPE> createDataWriter(
            PublisherImpl publisher, AbstractTopic<TYPE> topic,
            DataWriterQos qos,
            DataWriterListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses);

    public abstract AbstractDataReader<TYPE> createDataReader(
            SubscriberImpl subscriber,
            TopicDescriptionExt<TYPE> topicDescription, DataReaderQos qos,
            DataReaderListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses);

    public abstract AbstractTopic<TYPE> createTopic(
            DomainParticipantImpl participant, String topicName, TopicQos qos,
            TopicListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses);
}
