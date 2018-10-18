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

import java.lang.reflect.Field;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.status.Status;
import org.omg.dds.sub.DataReaderListener;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.topic.TopicDescriptionExt;
import org.opensplice.dds.type.TypeSupportProtobuf;

import DDS.SampleInfoSeqHolder;

public class DataReaderProtobuf<PROTOBUF_TYPE, DDS_TYPE> extends
        AbstractDataReader<PROTOBUF_TYPE> {
    private final HashMap<List<Sample<PROTOBUF_TYPE>>, PreAllocatorProtobuf<PROTOBUF_TYPE, DDS_TYPE>> preallocated;
    private final ReflectionDataReader<DDS_TYPE, PROTOBUF_TYPE> reflectionReader;
    private final TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE> typeSupport;

    @SuppressWarnings("unchecked")
    public DataReaderProtobuf(OsplServiceEnvironment environment,
            SubscriberImpl parent,
            TopicDescriptionExt<PROTOBUF_TYPE> topicDescription,
            DataReaderQos qos, DataReaderListener<PROTOBUF_TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        super(environment, parent, topicDescription);
        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataReaderQos is null.");
        }
        if (topicDescription == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied TopicDescription is null.");
        }
        DDS.DataReaderQos oldQos;

        try {
            oldQos = ((DataReaderQosImpl) qos).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot create DataReader with non-OpenSplice qos");
        }

        if (listener != null) {
            this.listener = new DataReaderListenerImpl<PROTOBUF_TYPE>(
                    this.environment, this, listener, true);
        } else {
            this.listener = null;
        }
        DDS.DataReader old = this.parent.getOld().create_datareader(
                topicDescription.getOld(), oldQos, this.listener,
                StatusConverter.convertMask(this.environment, statuses));

        if (old == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        this.setOld(old);
        this.preallocated = new HashMap<List<Sample<PROTOBUF_TYPE>>, PreAllocatorProtobuf<PROTOBUF_TYPE, DDS_TYPE>>();
        this.typeSupport = (TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE>) topicDescription
                .getTypeSupport();
        this.reflectionReader = new ReflectionDataReader<DDS_TYPE, PROTOBUF_TYPE>(
                this.environment, this, this.typeSupport
                        .getTypeSupportStandard().getType());
        this.topicDescription.retain();

        if (this.listener != null) {
            this.listener.setInitialised();
        }
    }

    @Override
    protected void destroy() {
        super.destroy();
        this.topicDescription.close();
    }

    @Override
    public PreAllocator<PROTOBUF_TYPE> getPreAllocator(
            List<Sample<PROTOBUF_TYPE>> samples, Class<?> sampleSeqHolderClz,
            Field sampleSeqHolderValueField) {
        PreAllocatorProtobuf<PROTOBUF_TYPE, DDS_TYPE> pa;

        synchronized (this.preallocated) {
            if (samples != null) {
                pa = this.preallocated.get(samples);
            } else {
                pa = null;
            }
            if (pa == null) {
                pa = new PreAllocatorProtobuf<PROTOBUF_TYPE, DDS_TYPE>(
                        this.environment, this, sampleSeqHolderClz,
                        sampleSeqHolderValueField, samples);
                this.preallocated.put(pa.getSampleList(), pa);
            } else {
                pa.setSampleList(samples);
            }
        }
        return pa;
    }

    @Override
    protected ReflectionDataReader<?, PROTOBUF_TYPE> getReflectionReader() {
        return this.reflectionReader;
    }

    @Override
    public PROTOBUF_TYPE getKeyValue(PROTOBUF_TYPE keyHolder,
            InstanceHandle handle) {
        return this.getKeyValue(handle);
    }

    @Override
    public PROTOBUF_TYPE getKeyValue(InstanceHandle handle) {
        DDS_TYPE ddsData = this.reflectionReader.getKeyValue(handle);

        if (ddsData != null) {
            return this.typeSupport.ddsKeyToProtobuf(ddsData);
        }
        return null;
    }

    @Override
    public InstanceHandle lookupInstance(PROTOBUF_TYPE keyHolder) {
        return this.reflectionReader.lookupInstance(this.typeSupport
                .protobufToDds(keyHolder));
    }

    @Override
    public boolean readNextSample(Sample<PROTOBUF_TYPE> sample) {
        SampleImpl<DDS_TYPE> ddsSample;
        boolean result;

        if (sample == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Provided an invalid null sample.");
        }
        ddsSample = new SampleImpl<DDS_TYPE>(this.environment,
                this.typeSupport.protobufToDds(sample.getData()),
                ((SampleImpl<PROTOBUF_TYPE>) sample).getInfo());
        result = this.reflectionReader.readNextSample(ddsSample);

        if (result == true) {
            if (ddsSample.getInfo().valid_data) {
                ((SampleImpl<PROTOBUF_TYPE>) sample).setContent(
                        this.typeSupport.ddsToProtobuf(ddsSample.getData()),
                        ddsSample.getInfo());
            } else {
                ((SampleImpl<PROTOBUF_TYPE>) sample).setContent(
                        this.typeSupport.ddsKeyToProtobuf(ddsSample
                                .getKeyValue()), ddsSample.getInfo());
            }
        }
        return result;
    }

    @Override
    public boolean takeNextSample(Sample<PROTOBUF_TYPE> sample) {
        SampleImpl<DDS_TYPE> ddsSample;
        boolean result;

        if (sample == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Provided an invalid null sample.");
        }
        ddsSample = new SampleImpl<DDS_TYPE>(this.environment,
                this.typeSupport.protobufToDds(sample.getData()),
                ((SampleImpl<PROTOBUF_TYPE>) sample).getInfo());
        result = this.reflectionReader.takeNextSample(ddsSample);

        if (result == true) {
            if (ddsSample.getInfo().valid_data) {
                ((SampleImpl<PROTOBUF_TYPE>) sample).setContent(
                        this.typeSupport.ddsToProtobuf(ddsSample.getData()),
                        ddsSample.getInfo());
            } else {
                ((SampleImpl<PROTOBUF_TYPE>) sample).setContent(
                        this.typeSupport.ddsKeyToProtobuf(ddsSample
                                .getKeyValue()), ddsSample.getInfo());
            }
        }
        return result;
    }

    @Override
    public Iterator<PROTOBUF_TYPE> createIterator(Object sampleSeqHolder,
            Field sampleSeqHolderValueField, SampleInfoSeqHolder info) {
        return new IteratorProtobuf<PROTOBUF_TYPE, DDS_TYPE>(this.environment,
                this, sampleSeqHolder, sampleSeqHolderValueField, info);
    }
}
