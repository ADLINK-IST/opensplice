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

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import org.omg.dds.sub.Sample;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

import DDS.SampleInfo;
import DDS.SampleInfoSeqHolder;

public class PreAllocatorImpl<TYPE> implements PreAllocator<TYPE> {
    private final OsplServiceEnvironment environment;
    private final Class<?> dataSeqHolderClaz;
    private final Class<TYPE> dataClaz;
    private final Field dataSeqHolderValueField;

    private SampleInfoSeqHolder infoSeqHolder;
    private Object dataSeqHolder;
    private List<Sample<TYPE>> sampleList;
    private int lastLength;

    public PreAllocatorImpl(OsplServiceEnvironment environment,
            Class<?> dataSeqHolderClaz, Field dataSeqHolderValueField,
            Class<TYPE> dataClaz, List<Sample<TYPE>> preAllocated) {
        this.environment = environment;
        this.sampleList = preAllocated;
        this.dataSeqHolderClaz = dataSeqHolderClaz;
        this.dataClaz = dataClaz;
        this.dataSeqHolderValueField = dataSeqHolderValueField;
        this.lastLength = -1;
        this.setSampleList(preAllocated);
    }

    @Override
    public void setSampleList(List<Sample<TYPE>> preAllocated) {
        try {
            if (preAllocated == null) {
                this.sampleList = new ArrayList<Sample<TYPE>>();
                this.infoSeqHolder = new SampleInfoSeqHolder();
                this.dataSeqHolder = dataSeqHolderClaz.newInstance();
            } else if (preAllocated == this.sampleList) {
                if (this.lastLength != preAllocated.size()) {
                    if (this.lastLength == -1) {
                        this.infoSeqHolder = new SampleInfoSeqHolder();
                        this.dataSeqHolder = dataSeqHolderClaz.newInstance();
                    }
                    this.infoSeqHolder.value = new SampleInfo[preAllocated
                            .size()];
                    this.dataSeqHolderValueField.set(this.dataSeqHolder,
                            Array.newInstance(dataClaz, preAllocated.size()));
                    this.copyData();
                }
            } else {
                this.sampleList = preAllocated;
                this.infoSeqHolder = new SampleInfoSeqHolder();
                this.dataSeqHolder = dataSeqHolderClaz.newInstance();
                this.copyData();
            }
            this.lastLength = this.sampleList.size();
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

    private void copyData() {
        int i = 0;

        try {
            this.infoSeqHolder.value = new SampleInfo[this.sampleList.size()];
            this.dataSeqHolderValueField.set(this.dataSeqHolder,
                    Array.newInstance(dataClaz, this.sampleList.size()));
            Object dataValue = this.dataSeqHolderValueField
                    .get(this.dataSeqHolder);

            for (Sample<TYPE> sample : this.sampleList) {
                this.infoSeqHolder.value[i] = ((SampleImpl<TYPE>) sample)
                        .getInfo();
                Array.set(dataValue, i++, sample.getData());
            }
        } catch (IllegalAccessException e) {
            throw new DDSExceptionImpl(this.environment, "Internal error ("
                    + e.getMessage() + ").");
        } catch (ClassCastException ce) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Usage of non-OpenSplice Sample implementation is not supported.");
        }
    }

    @Override
    public void updateReferences() {
        this.updateReferencesImproved();
    }

    @SuppressWarnings("unchecked")
    private void updateReferencesImproved() {
        int index;

        assert (this.lastLength == this.sampleList.size());

        if (this.infoSeqHolder.value.length > this.lastLength) {
            try {
                Object dataValue = this.dataSeqHolderValueField
                        .get(this.dataSeqHolder);
                index = this.lastLength;

                while (index < this.infoSeqHolder.value.length) {
                    this.sampleList.add(new SampleImpl<TYPE>(
                            this.environment,
                            (TYPE)Array.get(dataValue, index),
                            this.infoSeqHolder.value[index]));
                    index++;
                }
            } catch (IllegalArgumentException e) {
                throw new DDSExceptionImpl(this.environment, "Internal error ("
                        + e.getMessage() + ").");
            } catch (IllegalAccessException e) {
                throw new DDSExceptionImpl(this.environment, "Internal error ("
                        + e.getMessage() + ").");
            }
        } else if (this.infoSeqHolder.value.length < this.lastLength) {
            index = this.infoSeqHolder.value.length;
            int curLength = this.lastLength;

            while (index < curLength) {
                this.sampleList.remove(index);
                curLength--;
            }
        }
        this.lastLength = this.sampleList.size();
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
    public List<Sample<TYPE>> getSampleList() {
        return this.sampleList;
    }

}
