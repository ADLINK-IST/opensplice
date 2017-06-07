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
package org.opensplice.dds.type;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
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
import org.opensplice.dds.pub.DataWriterProtobuf;
import org.opensplice.dds.pub.PublisherImpl;
import org.opensplice.dds.sub.AbstractDataReader;
import org.opensplice.dds.sub.DataReaderProtobuf;
import org.opensplice.dds.sub.SubscriberImpl;
import org.opensplice.dds.topic.AbstractTopic;
import org.opensplice.dds.topic.TopicDescriptionExt;
import org.opensplice.dds.topic.TopicProtobuf;

import DDS.TypeSupport;

public abstract class TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE> extends
        AbstractTypeSupport<PROTOBUF_TYPE> {
    protected final Class<PROTOBUF_TYPE> dataType;
    protected final OsplServiceEnvironment environment;
    protected final TypeSupportImpl<DDS_TYPE> ddsTypeSupport;
    private final byte[] metaData;
    private final byte[] typeHash;
    private final byte[] extentions = null;

    protected TypeSupportProtobuf(OsplServiceEnvironment environment,
            Class<PROTOBUF_TYPE> dataType,
            TypeSupportImpl<DDS_TYPE> ddsTypeSupport,
            final byte[] metaData,
            final byte[] metaHash) {
        this.environment = environment;
        this.dataType = dataType;
        this.metaData = metaData;
        this.typeHash = metaHash;
        this.ddsTypeSupport = ddsTypeSupport;

        org.opensplice.dds.dcps.TypeSupportImpl oldTypeSupport = (org.opensplice.dds.dcps.TypeSupportImpl)ddsTypeSupport.getOldTypeSupport();
        oldTypeSupport.set_data_representation_id(DDS.GPB_REPRESENTATION.value);
        oldTypeSupport.set_meta_data(this.metaData);
        oldTypeSupport.set_type_hash(this.typeHash);
    }

    @SuppressWarnings("unchecked")
    public static <SOME_TYPE> org.omg.dds.type.TypeSupport<SOME_TYPE> getInstance(
            OsplServiceEnvironment environment, Class<SOME_TYPE> dataType,
            String registeredName) {
        String typeSupportName = dataType.getName().replaceAll("\\$", "")
                + "TypeSupportProtobuf";

        try {
            Class<?> typeSupportClass = Class.forName(typeSupportName);
            Constructor<?> c = typeSupportClass.getConstructor(
                    OsplServiceEnvironment.class, String.class);

            return (org.omg.dds.type.TypeSupport<SOME_TYPE>) c.newInstance(
                    environment, registeredName);

        } catch (ClassNotFoundException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed (" + typeSupportName
                            + "); " + e.getMessage());
        } catch (InstantiationException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (IllegalAccessException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (IllegalArgumentException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (InvocationTargetException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (NoSuchMethodException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        } catch (SecurityException e) {
            throw new PreconditionNotMetExceptionImpl(environment,
                    "Allocating new TypeSupport failed. " + e.getMessage());
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public TypeSupport getOldTypeSupport() {
        return this.ddsTypeSupport.getOldTypeSupport();
    }

    @Override
    public PROTOBUF_TYPE newData() {
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
    public Class<PROTOBUF_TYPE> getType() {
        return this.dataType;
    }

    @Override
    public String getTypeName() {
        return this.ddsTypeSupport.getTypeName();
    }

    public TypeSupportImpl<DDS_TYPE> getTypeSupportStandard() {
        return this.ddsTypeSupport;
    }

    @Override
    public AbstractTopic<PROTOBUF_TYPE> createTopic(
            DomainParticipantImpl participant, String topicName, TopicQos qos,
            TopicListener<PROTOBUF_TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        return new TopicProtobuf<PROTOBUF_TYPE>(this.environment, participant,
                topicName, this, qos, listener, statuses);
    }

    @Override
    public AbstractDataWriter<PROTOBUF_TYPE> createDataWriter(
            PublisherImpl publisher, AbstractTopic<PROTOBUF_TYPE> topic,
            DataWriterQos qos, DataWriterListener<PROTOBUF_TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        return new DataWriterProtobuf<PROTOBUF_TYPE, DDS_TYPE>(
                this.environment, publisher,
                (TopicProtobuf<PROTOBUF_TYPE>) topic, qos, listener, statuses);
    }

    @Override
    public AbstractDataReader<PROTOBUF_TYPE> createDataReader(
            SubscriberImpl subscriber,
            TopicDescriptionExt<PROTOBUF_TYPE> topicDescription, DataReaderQos qos,
            DataReaderListener<PROTOBUF_TYPE> listener,
            Collection<Class<? extends Status>> statuses){
        return new DataReaderProtobuf<PROTOBUF_TYPE, DDS_TYPE>(
                this.environment, subscriber, topicDescription, qos, listener,
                statuses);
    }

    public byte[] getMetaDescriptor(){
        return this.metaData.clone();
    }

    public byte[] getMetaHash(){
        return this.typeHash.clone();
    }

    public byte[] getExtentions(){
        return this.extentions.clone();
    }

    protected static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                                 + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }

    public abstract PROTOBUF_TYPE ddsToProtobuf(DDS_TYPE ddsData);

    public abstract DDS_TYPE protobufToDds(PROTOBUF_TYPE protobufData);

    public abstract PROTOBUF_TYPE ddsKeyToProtobuf(DDS_TYPE ddsData);
}
