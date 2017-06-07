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

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import org.omg.dds.sub.Sample;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.type.TypeSupportProtobuf;

import DDS.SampleInfoSeqHolder;

public class PreAllocatorProtobuf<PROTOBUF_TYPE, DDS_TYPE> implements
        PreAllocator<PROTOBUF_TYPE> {
    private final OsplServiceEnvironment environment;
    private final Class<?> dataSeqHolderClaz;
    private final Field dataSeqHolderValueField;
    private final TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE> typeSupport;

    private SampleInfoSeqHolder infoSeqHolder;
    private Object dataSeqHolder;
    private List<Sample<PROTOBUF_TYPE>> sampleList;

    @SuppressWarnings("unchecked")
    public PreAllocatorProtobuf(OsplServiceEnvironment environment,
            DataReaderProtobuf<PROTOBUF_TYPE, DDS_TYPE> reader,
            Class<?> dataSeqHolderClaz, Field dataSeqHolderValueField,
            List<Sample<PROTOBUF_TYPE>> preAllocated) {
        this.environment = environment;
        this.sampleList = preAllocated;
        this.dataSeqHolderClaz = dataSeqHolderClaz;
        this.dataSeqHolderValueField = dataSeqHolderValueField;
        this.typeSupport = (TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE>) reader
                .getTopicDescription().getTypeSupport();
        this.setSampleList(preAllocated);
    }

    @Override
    public void setSampleList(List<Sample<PROTOBUF_TYPE>> preAllocated) {

        try {
            if (preAllocated == null) {
                this.sampleList = new ArrayList<Sample<PROTOBUF_TYPE>>();
            } else {
                this.sampleList = preAllocated;
                this.sampleList.clear();
            }
            this.infoSeqHolder = new SampleInfoSeqHolder();
            this.dataSeqHolder = dataSeqHolderClaz.newInstance();
        } catch (InstantiationException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException ce) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Usage of non-OpenSplice Sample implementation is not supported.");
        }
    }

    @SuppressWarnings("unchecked")
    @Override
    public void updateReferences() {
        try {
            Object dataValue = this.dataSeqHolderValueField
                    .get(this.dataSeqHolder);

            for (int i = 0; i < this.infoSeqHolder.value.length; i++) {
                if (this.infoSeqHolder.value[i].valid_data) {
                    this.sampleList.add(new SampleImpl<PROTOBUF_TYPE>(
                            this.environment, this.typeSupport
                                    .ddsToProtobuf((DDS_TYPE) Array.get(
                                            dataValue, i)),
                            this.infoSeqHolder.value[i]));
                } else {
                    this.sampleList
                            .add(new SampleImpl<PROTOBUF_TYPE>(
                                    this.environment, null,
                                    this.infoSeqHolder.value[i]));
                }
            }

        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        }
    }

    @Override
    public Object getDataSeqHolder() {
        return this.dataSeqHolder;
    }

    @Override
    public SampleInfoSeqHolder getInfoSeqHolder() {
        return this.infoSeqHolder;
    }

    @Override
    public List<Sample<PROTOBUF_TYPE>> getSampleList() {
        return this.sampleList;
    }

}
