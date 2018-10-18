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

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.status.Status;
import org.omg.dds.pub.DataWriterListener;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.sub.DataReaderListener;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.topic.TopicListener;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.PreconditionNotMetExceptionImpl;
import org.opensplice.dds.domain.DomainParticipantImpl;
import org.opensplice.dds.pub.AbstractDataWriter;
import org.opensplice.dds.pub.DataWriterImpl;
import org.opensplice.dds.pub.PublisherImpl;
import org.opensplice.dds.sub.AbstractDataReader;
import org.opensplice.dds.sub.DataReaderImpl;
import org.opensplice.dds.sub.SubscriberImpl;
import org.opensplice.dds.topic.AbstractTopic;
import org.opensplice.dds.topic.TopicDescriptionExt;
import org.opensplice.dds.topic.TopicImpl;

public class TypeSupportImpl<TYPE> extends AbstractTypeSupport<TYPE> {
    private final OsplServiceEnvironment environment;
    private final org.opensplice.dds.dcps.TypeSupportImpl oldTypeSupport;
    private final Class<TYPE> dataType;
    private final String typeName;

    @SuppressWarnings("unchecked")
    public TypeSupportImpl(OsplServiceEnvironment environment,
            Class<TYPE> dataType, String typeName) {
        super();
        this.environment = environment;
        this.dataType = dataType;
        this.typeName = typeName;

        String typeSupportName = dataType.getName() + "TypeSupport";

        try {
            Class<? extends org.opensplice.dds.dcps.TypeSupportImpl> oldTypeSupportClaz;

            oldTypeSupportClaz = (Class<? extends org.opensplice.dds.dcps.TypeSupportImpl>) Class
                    .forName(typeSupportName);
            this.oldTypeSupport = oldTypeSupportClaz.newInstance();
        } catch (ClassCastException e) {
            throw new PreconditionNotMetExceptionImpl(
                    this.environment,
                    "Allocating new TypeSupport failed. "
                            + typeName
                            + "' is not an instance of org.opensplice.dds.dcps.TypeSupportImpl.");
        } catch (InstantiationException e) {
            throw new PreconditionNotMetExceptionImpl(this.environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (IllegalAccessException e) {
            throw new PreconditionNotMetExceptionImpl(this.environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (ClassNotFoundException e) {
            throw new PreconditionNotMetExceptionImpl(this.environment,
                    "Allocating new TypeSupport failed (" + typeSupportName
                            + "); " + e.getMessage());
        }

    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public TYPE newData() {
        try {
            return dataType.newInstance();
        } catch (InstantiationException e) {
            throw new PreconditionNotMetExceptionImpl(this.environment,
                    "Unable to instantiate data; " + e.getMessage());
        } catch (IllegalAccessException e) {
            throw new PreconditionNotMetExceptionImpl(this.environment,
                    "Unable to instantiate data; " + e.getMessage());
        }
    }

    @Override
    public Class<TYPE> getType() {
        return this.dataType;
    }

    @Override
    public String getTypeName() {
        if (this.typeName != null) {
            return this.typeName;
        }
        return oldTypeSupport.get_type_name();
    }

    @Override
    public DDS.TypeSupport getOldTypeSupport() {
        return this.oldTypeSupport;
    }

    @Override
    public AbstractDataWriter<TYPE> createDataWriter(PublisherImpl publisher,
            AbstractTopic<TYPE> topic, DataWriterQos qos,
            DataWriterListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        return new DataWriterImpl<TYPE>(this.environment, publisher,
                (TopicImpl<TYPE>) topic, qos, listener, statuses);
    }

    @Override
    public AbstractDataReader<TYPE> createDataReader(SubscriberImpl subscriber,
            TopicDescriptionExt<TYPE> topicDescription, DataReaderQos qos,
            DataReaderListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        return new DataReaderImpl<TYPE>(this.environment, subscriber,
                topicDescription, qos, listener, statuses);
    }

    @Override
    public AbstractTopic<TYPE> createTopic(DomainParticipantImpl participant,
            String topicName, TopicQos qos, TopicListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        return new TopicImpl<TYPE>(this.environment, participant, topicName,
                this, qos, listener, statuses);
    }

}
