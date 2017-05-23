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
package org.opensplice.dds.pub;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.Duration;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.Status;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterListener;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherListener;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.core.DomainEntityImpl;
import org.opensplice.dds.core.EntityImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.IllegalOperationExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.StatusConditionImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.domain.DomainParticipantImpl;
import org.opensplice.dds.topic.TopicImpl;
import org.opensplice.dds.type.AbstractTypeSupport;

public class PublisherImpl
extends
DomainEntityImpl<DDS.Publisher, DomainParticipantImpl, DDS.DomainParticipant, PublisherQos, PublisherListener, PublisherListenerImpl>
implements Publisher {
    private final ConcurrentHashMap<DDS.DataWriter, DataWriter<?>> writers;

    public PublisherImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl parent, PublisherQos qos,
            PublisherListener listener,
            Collection<Class<? extends Status>> statuses) {
        super(environment, parent, parent.getOld());
        DDS.PublisherQos oldQos;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied PublisherQos is null.");
        }

        try {
            oldQos = ((PublisherQosImpl) qos).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot create Publisher with non-OpenSplice qos");
        }

        if (listener != null) {
            this.listener = new PublisherListenerImpl(this.environment, this,
                    listener, true);
        } else {
            this.listener = null;
        }
        DDS.Publisher old = this.parent.getOld().create_publisher(oldQos,
                this.listener,
                StatusConverter.convertMask(this.environment, statuses));

        if (old == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        this.setOld(old);
        this.writers = new ConcurrentHashMap<DDS.DataWriter, DataWriter<?>>();

        if (this.listener != null) {
            this.listener.setInitialised();
        }
    }

    private void setListener(PublisherListener listener, int mask) {
        PublisherListenerImpl wrapperListener;
        int rc;

        if (listener != null) {
            wrapperListener = new PublisherListenerImpl(this.environment, this,
                    listener);
        } else {
            wrapperListener = null;
        }
        rc = this.getOld().set_listener(wrapperListener, mask);
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.setListener() failed.");

        this.listener = wrapperListener;
    }

    @Override
    public void setListener(PublisherListener listener) {
        this.setListener(listener, StatusConverter.getAnyMask());
    }

    @Override
    public void setListener(PublisherListener listener,
            Collection<Class<? extends Status>> statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public void setListener(PublisherListener listener,
            Class<? extends Status>... statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public PublisherQos getQos() {
        DDS.PublisherQosHolder holder = new DDS.PublisherQosHolder();
        int rc = this.getOld().get_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.getQos() failed.");

        return PublisherQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public void setQos(PublisherQos qos) {
        PublisherQosImpl q;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied PublisherQos is null.");
        }
        try {
            q = (PublisherQosImpl) qos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Setting non-OpenSplice Qos not supported.");
        }
        int rc = this.getOld().set_qos(q.convert());
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.setQos() failed.");

    }

    @Override
    public <TYPE> DataWriter<TYPE> createDataWriter(Topic<TYPE> topic) {
        return this.createDataWriter(topic, this.getDefaultDataWriterQos(),
                null, new HashSet<Class<? extends Status>>());
    }

    @Override
    public <TYPE> DataWriter<TYPE> createDataWriter(Topic<TYPE> topic,
            DataWriterQos qos, DataWriterListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        AbstractDataWriter<TYPE> writer;
        AbstractTypeSupport<TYPE> typeSupport;

        if (topic == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied Topic is null.");
        }
        synchronized(this.writers){
            try {
                typeSupport = (AbstractTypeSupport<TYPE>) topic.getTypeSupport();
                writer = typeSupport.createDataWriter(this,
                        (TopicImpl<TYPE>) topic, qos, listener, statuses);
            } catch (ClassCastException e) {
                throw new IllegalArgumentExceptionImpl(this.environment,
                        "Cannot create DataWriter with non-OpenSplice Topic");
            }
            this.writers.put(writer.getOld(), writer);
        }
        return writer;
    }

    @Override
    public <TYPE> DataWriter<TYPE> createDataWriter(Topic<TYPE> topic,
            DataWriterQos qos, DataWriterListener<TYPE> listener,
            Class<? extends Status>... statuses) {
        return createDataWriter(topic, qos, listener, Arrays.asList(statuses));
    }

    @Override
    public <TYPE> DataWriter<TYPE> createDataWriter(Topic<TYPE> topic,
            DataWriterQos qos) {
        return this.createDataWriter(topic, qos, null,
                new HashSet<Class<? extends Status>>());
    }

    public <TYPE> DataWriter<TYPE> lookupDataWriter(DDS.DataWriter writer) {
        DataWriter<?> dw;

        synchronized (this.writers) {
            dw = this.writers.get(writer);
        }
        if (dw != null) {
            return dw.cast();
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    @Override
    public <TYPE> DataWriter<TYPE> lookupDataWriter(String topicName) {
        if (topicName == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied topicName is null.");
        }
        synchronized (this.writers) {
            for (DataWriter<?> writer : this.writers.values()) {
                if (topicName.equals(writer.getTopic().getName())) {
                    try {
                        return (DataWriter<TYPE>) writer;
                    } catch (ClassCastException e) {
                        throw new IllegalOperationExceptionImpl(
                                this.environment,
                                "Cannot cast DataWriter to desired type.");
                    }
                }
            }
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    @Override
    public <TYPE> DataWriter<TYPE> lookupDataWriter(Topic<TYPE> topic) {
        if (topic == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied Topic is null.");
        }
        synchronized (this.writers) {
            for (DataWriter<?> writer : this.writers.values()) {
                if (topic.equals(writer.getTopic())) {
                    try {
                        return (DataWriter<TYPE>) writer;
                    } catch (ClassCastException e) {
                        throw new IllegalOperationExceptionImpl(
                                this.environment,
                                "Cannot cast DataWriter to desired type.");
                    }
                }
            }
        }
        return null;
    }

    @Override
    public void closeContainedEntities() {
        for (DataWriter<?> writer : this.writers.values()) {
            try {
                writer.close();
            } catch (AlreadyClosedException a) {
                /* Entity may be closed concurrently by application */
            }
        }
    }

    @Override
    public void suspendPublications() {
        int rc = this.getOld().suspend_publications();
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.suspendPublications() failed.");

    }

    @Override
    public void resumePublications() {
        int rc = this.getOld().resume_publications();
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.resumePublications() failed.");
    }

    @Override
    public void beginCoherentChanges() {
        int rc = this.getOld().begin_coherent_changes();
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.beginCoherentChanges() failed.");
    }

    @Override
    public void endCoherentChanges() {
        int rc = this.getOld().end_coherent_changes();
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.endCoherentChanges() failed.");
    }

    @Override
    public void waitForAcknowledgments(Duration maxWait)
            throws TimeoutException {
        int rc = this.getOld().wait_for_acknowledgments(
                Utilities.convert(this.environment,
                maxWait));
        Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                "Publisher.waitForAcknowledgments() failed.");
    }

    @Override
    public void waitForAcknowledgments(long maxWait, TimeUnit unit)
            throws TimeoutException {
        this.waitForAcknowledgments(this.environment.getSPI().newDuration(
                maxWait, unit));
    }

    @Override
    public DataWriterQos getDefaultDataWriterQos() {
        DDS.DataWriterQosHolder holder = new DDS.DataWriterQosHolder();
        int rc = this.getOld().get_default_datawriter_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Publisher.getDefaultDataWriterQos() failed.");

        return DataWriterQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public void setDefaultDataWriterQos(DataWriterQos qos) {
        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataWriterQos is null.");
        }
        try {
            this.getOld().set_default_datawriter_qos(
                    ((DataWriterQosImpl) qos)
                    .convert());
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Non-OpenSplice DataWriterQos not supported.");
        }

    }

    @Override
    public DataWriterQos copyFromTopicQos(DataWriterQos dwQos, TopicQos tQos) {
        DataWriterQosImpl result;

        if (tQos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied TopicQos is null.");
        }
        if (dwQos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataWriterQos is null.");
        }
        try {
            result = (DataWriterQosImpl) dwQos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Non-OpenSplice DataWriterQos not supported.");
        }
        result.mergeTopicQos(tQos);

        return result;
    }

    @Override
    public StatusCondition<Publisher> getStatusCondition() {
        DDS.StatusCondition oldCondition = this.getOld().get_statuscondition();

        if (oldCondition == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        return new StatusConditionImpl<Publisher>(this.environment,
                oldCondition, this);
    }

    @Override
    public org.omg.dds.domain.DomainParticipant getParent() {
        return this.parent;
    }

    @Override
    protected void destroy() {
        this.closeContainedEntities();
        this.parent.destroyPublisher(this);
    }

    public void destroyDataWriter(
            EntityImpl<DDS.DataWriter, ?, ?, ?, ?> dataWriter) {
        DDS.DataWriter old = dataWriter.getOld();
        int rc = this.getOld().delete_datawriter(old);
        this.writers.remove(old);
        Utilities.checkReturnCode(rc, this.environment,
                "DataWriter.close() failed.");
    }
}
