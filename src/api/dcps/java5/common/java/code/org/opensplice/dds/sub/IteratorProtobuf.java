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

import java.lang.reflect.Field;
import java.util.ArrayList;

import org.omg.dds.sub.Sample;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.type.TypeSupportProtobuf;

import DDS.SampleInfoSeqHolder;

public class IteratorProtobuf<PROTOBUF_TYPE, DDS_TYPE> extends
        AbstractIterator<PROTOBUF_TYPE> implements
        Sample.Iterator<PROTOBUF_TYPE> {
    private ArrayList<PROTOBUF_TYPE> data;

    public IteratorProtobuf(OsplServiceEnvironment environment,
            DataReaderProtobuf<PROTOBUF_TYPE, DDS_TYPE> reader,
            Object sampleSeqHolder, Field dataSeqHolderValue,
            SampleInfoSeqHolder infoSeqHolder) {
        super(environment, reader, sampleSeqHolder, dataSeqHolderValue,
                infoSeqHolder);
    }

    @SuppressWarnings("unchecked")
    @Override
    protected SampleImpl<PROTOBUF_TYPE>[] setupSampleList() {
        try {
            TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE> typeSupport;
            DDS_TYPE[] ddsData;

            typeSupport = (TypeSupportProtobuf<PROTOBUF_TYPE, DDS_TYPE>) this.reader
                    .getTopicDescription().getTypeSupport();
            ddsData = (DDS_TYPE[]) dataSeqHolderValue.get(sampleSeqHolder);

            this.data = new ArrayList<PROTOBUF_TYPE>(ddsData.length);

            for (int i = 0; i < ddsData.length; i++) {
                if (this.infoSeqHolder.value[i].valid_data) {
                    this.data.add(typeSupport.ddsToProtobuf(ddsData[i]));
                } else {
                    this.data.add(typeSupport.ddsKeyToProtobuf(ddsData[i]));
                }
            }
        } catch (SecurityException e) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Not allowed to access field "
                            + dataSeqHolderValue.getName() + " in "
                            + dataSeqHolderValue.getClass().getName() + "("
                            + e.getMessage() + ").");
        } catch (IllegalArgumentException e) {
            throw new DDSExceptionImpl(environment, "Cannot find "
                    + dataSeqHolderValue.getName() + " in "
                    + dataSeqHolderValue.getClass().getName() + "("
                    + e.getMessage() + ").");
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(environment, "No access to field "
                    + dataSeqHolderValue.getName() + " in "
                    + dataSeqHolderValue.getClass().getName() + "("
                    + e.getMessage() + ").");
        }
        return new SampleImpl[this.data.size()];
    }

    @Override
    protected PROTOBUF_TYPE getData(int index) {
        return this.data.get(index);
    }
}
