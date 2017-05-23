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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.Status;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderListener;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberListener;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.TopicDescription;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.DomainEntityImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.IllegalOperationExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.StatusConditionImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.domain.DomainParticipantImpl;
import org.opensplice.dds.topic.TopicDescriptionExt;
import org.opensplice.dds.type.AbstractTypeSupport;

public class SubscriberImpl
        extends
        DomainEntityImpl<DDS.Subscriber, DomainParticipantImpl, DDS.DomainParticipant, SubscriberQos, SubscriberListener, SubscriberListenerImpl>
        implements Subscriber {
    private final ConcurrentHashMap<DDS.DataReader, AbstractDataReader<?>> readers;
    private final boolean isBuiltin;

    public SubscriberImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl parent, SubscriberQos qos,
            SubscriberListener listener,
            Collection<Class<? extends Status>> statuses) {
        super(environment, parent, parent.getOld());
        DDS.SubscriberQos oldQos;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied SubscriberQos is null.");
        }

        try {
            oldQos = ((SubscriberQosImpl) qos).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot create Subscribe with non-OpenSplice qos");
        }

        if (listener != null) {
            this.listener = new SubscriberListenerImpl(this.environment, this,
                    listener, true);
        } else {
            this.listener = null;
        }
        DDS.Subscriber old = this.parent.getOld().create_subscriber(oldQos,
                this.listener,
                StatusConverter.convertMask(this.environment, statuses));

        if (old == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        this.setOld(old);
        this.readers = new ConcurrentHashMap<DDS.DataReader, AbstractDataReader<?>>();
        this.isBuiltin = false;

        if (this.listener != null) {
            this.listener.setInitialised();
        }
    }

    public SubscriberImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl parent, DDS.Subscriber oldSubscriber) {
        super(environment, parent, parent.getOld());

        if (oldSubscriber == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied Subscriber is invalid (null).");
        }
        this.listener = null;
        this.setOld(oldSubscriber);
        this.readers = new ConcurrentHashMap<DDS.DataReader, AbstractDataReader<?>>();
        this.isBuiltin = true;
    }

    public boolean isBuiltin() {
        return this.isBuiltin;
    }

    private void setListener(SubscriberListener listener, int mask) {
        SubscriberListenerImpl wrapperListener;
        int rc;

        if (listener != null) {
            wrapperListener = new SubscriberListenerImpl(this.environment,
                    this, listener);
        } else {
            wrapperListener = null;
        }
        rc = this.getOld().set_listener(wrapperListener, mask);
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.setListener() failed.");

        this.listener = wrapperListener;
    }

    @Override
    public void setListener(SubscriberListener listener) {
        this.setListener(listener, StatusConverter.getAnyMask());
    }

    @Override
    public void setListener(SubscriberListener listener,
            Collection<Class<? extends Status>> statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public void setListener(SubscriberListener listener,
            Class<? extends Status>... statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public SubscriberQos getQos() {
        DDS.SubscriberQosHolder holder = new DDS.SubscriberQosHolder();
        int rc = this.getOld().get_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.getQos() failed.");

        return SubscriberQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public void setQos(SubscriberQos qos) {
        SubscriberQosImpl q;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied SubscriberQos is null.");
        }
        try {
            q = (SubscriberQosImpl) qos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Setting non-OpenSplice Qos not supported.");
        }
        int rc = this.getOld().set_qos(q.convert());
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.setQos() failed.");

    }

    @Override
    public <TYPE> DataReader<TYPE> createDataReader(TopicDescription<TYPE> topic) {
        return this.createDataReader(topic, this.getDefaultDataReaderQos(),
                null, new HashSet<Class<? extends Status>>());
    }

    @Override
    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic, DataReaderQos qos,
            DataReaderListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        AbstractDataReader<TYPE> reader;
        AbstractTypeSupport<TYPE> typeSupport;

        if (topic == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied Topic is null.");
        }
        synchronized (this.readers) {
            try {
                typeSupport = (AbstractTypeSupport<TYPE>) topic
                        .getTypeSupport();
                reader = typeSupport.createDataReader(this,
                        (TopicDescriptionExt<TYPE>) topic, qos, listener,
                        statuses);
                this.readers.put(reader.getOld(), reader);
            } catch (ClassCastException e) {
                throw new IllegalArgumentExceptionImpl(this.environment,
                        "Cannot create DataReader with non-OpenSplice Topic");
            }
        }
        return reader;
    }

    @Override
    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic, DataReaderQos qos,
            DataReaderListener<TYPE> listener,
            Class<? extends Status>... statuses) {
        return this.createDataReader(topic, qos, listener,
                Arrays.asList(statuses));
    }

    @Override
    public <TYPE> DataReader<TYPE> createDataReader(
            TopicDescription<TYPE> topic, DataReaderQos qos) {
        return this.createDataReader(topic, qos, null,
                new HashSet<Class<? extends Status>>());
    }

    @Override
    public <TYPE> DataReader<TYPE> lookupDataReader(String topicName) {
        if (topicName == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied topicName is null.");
        }
        synchronized (this.readers) {
            for (DataReader<?> reader : this.readers.values()) {
                if (topicName.equals(reader.getTopicDescription().getName())) {
                    try {
                        return reader.cast();
                    } catch (ClassCastException e) {
                        throw new IllegalOperationExceptionImpl(
                                this.environment,
                                "Cannot cast DataReader to desired type.");
                    }
                }
            }
            DDS.DataReader builtinReader = this.getOld()
                    .lookup_datareader(topicName);

            if (builtinReader != null) {
                return this.initBuiltinReader(builtinReader);
            }
        }
        return null;
    }

    @Override
    public <TYPE> DataReader<TYPE> lookupDataReader(
            TopicDescription<TYPE> topicDescription) {
        if (topicDescription == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied topicName is null.");
        }
        synchronized (this.readers) {
            for (DataReader<?> reader : this.readers.values()) {
                if (topicDescription.equals(reader.getTopicDescription())) {
                    try {
                        return reader.cast();
                    } catch (ClassCastException e) {
                        throw new IllegalOperationExceptionImpl(
                                this.environment,
                                "Cannot cast DataReader to desired type.");
                    }
                }
            }
            DDS.DataReader builtinReader = this.getOld()
                    .lookup_datareader(topicDescription.getName());

            if (builtinReader != null) {
                return this.initBuiltinReader(builtinReader, topicDescription);
            }
        }
        return null;
    }

    private <TYPE> DataReaderImpl<TYPE> initBuiltinReader(
            DDS.DataReader oldBuiltin) {
        DataReaderImpl<TYPE> result = null;

        if (oldBuiltin != null) {
            DDS.TopicDescription classicTopicDescription = oldBuiltin
                    .get_topicdescription();

            if (classicTopicDescription != null) {
                TopicDescription<TYPE> td = this.getParent()
                        .lookupTopicDescription(
                                classicTopicDescription.get_name());

                if (td != null) {
                    result = this.initBuiltinReader(oldBuiltin, td);
                }
            } else {
                throw new DDSExceptionImpl(this.environment,
                        "Classic DataReader has no TopicDescription.");
            }
        }
        return result;
    }

    private <TYPE> DataReaderImpl<TYPE> initBuiltinReader(
            DDS.DataReader oldBuiltin, TopicDescription<TYPE> td) {
        DataReaderImpl<TYPE> result = null;

        if (oldBuiltin != null) {
            result = new DataReaderImpl<TYPE>(this.environment, this,
                    (TopicDescriptionExt<TYPE>) td, oldBuiltin);
            this.readers.put(result.getOld(), result);
        }
        return result;
    }

    public <TYPE> DataReader<TYPE> lookupDataReader(DDS.DataReader old) {
        DataReader<TYPE> result;

        synchronized (this.readers) {
            AbstractDataReader<?> found = this.readers.get(old);

            if (found != null) {
                result = found.cast();
            } else if (this.isBuiltin) {
                result = this.initBuiltinReader(old);
            } else {
                result = null;
            }
        }
        return result;
    }

    @Override
    public void closeContainedEntities() {
        for (AbstractDataReader<?> reader : this.readers.values()) {
            try {
                reader.close();
            } catch (AlreadyClosedException a) {
                /* Entity may be closed concurrently by application */
            }
        }
    }

    public Collection<DataReader<?>> getDataReaders(
            Collection<DataReader<?>> readers) {
        DDS.DataReaderSeqHolder oldReaders = new DDS.DataReaderSeqHolder();

        synchronized (this.readers) {
            int rc = this.getOld().get_datareaders(oldReaders,
                    DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value,
                    DDS.ANY_INSTANCE_STATE.value);
            Utilities.checkReturnCode(rc, this.environment,
                    "Subscriber.getDataReaders() failed.");

            for (DDS.DataReader oldReader : oldReaders.value) {
                readers.add(this.readers.get(oldReader));
            }
        }
        return readers;
    }

    @Override
    public Collection<DataReader<?>> getDataReaders() {
        List<DataReader<?>> readers = new ArrayList<DataReader<?>>();
        DDS.DataReaderSeqHolder oldReaders = new DDS.DataReaderSeqHolder();

        synchronized (this.readers) {
            int rc = this.getOld().get_datareaders(oldReaders,
                    DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value,
                    DDS.ANY_INSTANCE_STATE.value);
            Utilities.checkReturnCode(rc, this.environment,
                    "Subscriber.getDataReaders() failed.");

            for (DDS.DataReader oldReader : oldReaders.value) {
                readers.add(this.readers.get(oldReader));
            }
        }
        return readers;
    }
    @Override
    public Collection<DataReader<?>> getDataReaders(DataState dataState) {
        if (dataState == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataState is null.");
        }
        List<DataReader<?>> readers = new ArrayList<DataReader<?>>();
        DDS.DataReaderSeqHolder oldReaders = new DDS.DataReaderSeqHolder();

        try {
            DataStateImpl state = (DataStateImpl) dataState;

            synchronized (this.readers) {
                int rc = this.getOld().get_datareaders(oldReaders,
                        state.getOldSampleState(), state.getOldViewState(),
                        state.getOldInstanceState());
                Utilities.checkReturnCode(rc, this.environment,
                        "Subscriber.getDataReaders() failed.");

                for (DDS.DataReader oldReader : oldReaders.value) {
                    readers.add(this.readers.get(oldReader));
                }
            }
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Non-OpenSplice DataState implementation not supported.");
        }
        return readers;
    }

    @Override
    public void notifyDataReaders() {
        int rc = this.getOld().notify_datareaders();
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.notifyDataReaders() failed.");

    }

    @Override
    public void beginAccess() {
        int rc = this.getOld().begin_access();
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.beginAccess() failed.");
    }

    @Override
    public void endAccess() {
        int rc = this.getOld().end_access();
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.endAccess() failed.");
    }

    @Override
    public DataReaderQos getDefaultDataReaderQos() {
        DDS.DataReaderQosHolder holder = new DDS.DataReaderQosHolder();
        int rc = this.getOld().get_default_datareader_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Subscriber.getDefaultDataReaderQos() failed.");
        return DataReaderQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public void setDefaultDataReaderQos(DataReaderQos qos) {
        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataReaderQoS is null.");
        }
        try {
            this.getOld().set_default_datareader_qos(
                    ((DataReaderQosImpl) qos)
                    .convert());
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Non-OpenSplice DataReaderQos not supported.");
        }
    }

    @Override
    public DataReaderQos copyFromTopicQos(DataReaderQos drQos, TopicQos tQos) {
        DataReaderQosImpl result;

        if (tQos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied TopicQos is null.");
        }
        if (drQos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataReaderQos is null.");
        }
        try {
            result = (DataReaderQosImpl) drQos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Non-OpenSplice DataReaderQos not supported.");
        }
        result.mergeTopicQos(tQos);

        return result;
    }

    @Override
    public StatusCondition<Subscriber> getStatusCondition() {
        DDS.StatusCondition oldCondition = this.getOld().get_statuscondition();

        if (oldCondition == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        return new StatusConditionImpl<Subscriber>(this.environment,
                oldCondition, this);
    }

    @Override
    public org.omg.dds.domain.DomainParticipant getParent() {
        return this.parent;
    }

    @Override
    public DataState createDataState() {
        return new DataStateImpl(this.environment);
    }

    @Override
    protected void destroy() {
        this.closeContainedEntities();
        this.parent.destroySubscriber(this);
    }

    public void destroyDataReader(AbstractDataReader<?> dataReader) {
        DDS.DataReader old = dataReader.getOld();
        old.delete_contained_entities();
        int rc = this.getOld().delete_datareader(old);
        this.readers.remove(old);
        Utilities.checkReturnCode(rc, this.environment,
                "DataReader.close() failed.");
    }
}
