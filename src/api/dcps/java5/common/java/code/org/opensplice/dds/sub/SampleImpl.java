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

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.Time;
import org.omg.dds.sub.InstanceState;
import org.omg.dds.sub.SampleState;
import org.omg.dds.sub.ViewState;
import org.opensplice.dds.core.InstanceHandleImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.UnsupportedOperationExceptionImpl;
import org.opensplice.dds.core.Utilities;

public class SampleImpl<TYPE> implements org.opensplice.dds.sub.Sample<TYPE> {
    private static final long serialVersionUID = 1010323161410625511L;
    private transient OsplServiceEnvironment environment;
    private TYPE data;
    private DDS.SampleInfo info;

    public SampleImpl(OsplServiceEnvironment environment, TYPE data,
            DDS.SampleInfo info) {
        this.environment = environment;
        this.data = data;
        this.info = info;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public TYPE getData() {
        if (this.info.valid_data == false) {
            return null;
        }
        return this.data;
    }

    @Override
    public TYPE getKeyValue() {
        return this.data;
    }

    public void setData(TYPE data){
        this.data = data;
    }

    public void setInfo(DDS.SampleInfo info){
        this.info = info;
    }

    public DDS.SampleInfo getInfo(){
        return this.info;
    }

    public void setContent(TYPE data, DDS.SampleInfo info){
        this.data = data;
        this.info = info;
    }

    @Override
    public SampleState getSampleState() {
        return DataStateImpl.getSampleStateFromOld(this.environment,
                this.info.sample_state);
    }

    @Override
    public ViewState getViewState() {
        return DataStateImpl.getViewStateFromOld(this.environment,
                this.info.view_state);
    }

    @Override
    public InstanceState getInstanceState() {
        return DataStateImpl.getInstanceStateFromOld(this.environment,
                this.info.instance_state);
    }

    @Override
    public Time getSourceTimestamp() {
        return Utilities.convert(this.environment, this.info.source_timestamp);
    }

    @Override
    public InstanceHandle getInstanceHandle() {
        return new InstanceHandleImpl(this.environment,
                this.info.instance_handle);
    }

    @Override
    public InstanceHandle getPublicationHandle() {
        return new InstanceHandleImpl(this.environment,
                this.info.publication_handle);
    }

    @Override
    public int getDisposedGenerationCount() {
        return this.info.disposed_generation_count;
    }

    @Override
    public int getNoWritersGenerationCount() {
        return this.info.no_writers_generation_count;
    }

    @Override
    public int getSampleRank() {
        return this.info.sample_rank;
    }

    @Override
    public int getGenerationRank() {
        return this.info.generation_rank;
    }

    @Override
    public int getAbsoluteGenerationRank() {
        return this.info.absolute_generation_rank;
    }

    @Override
    public SampleImpl<TYPE> clone() {
        try {
            @SuppressWarnings("unchecked")
            SampleImpl<TYPE> cloned = (SampleImpl<TYPE>) super.clone();
            cloned.environment = this.environment;
            cloned.data = this.data;
            cloned.info.absolute_generation_rank = this.info.absolute_generation_rank;
            cloned.info.disposed_generation_count = this.info.disposed_generation_count;
            cloned.info.generation_rank = this.info.generation_rank;
            cloned.info.instance_handle = this.info.instance_handle;
            cloned.info.no_writers_generation_count = this.info.no_writers_generation_count;
            cloned.info.publication_handle = this.info.publication_handle;
            cloned.info.reception_timestamp.sec = this.info.reception_timestamp.sec;
            cloned.info.reception_timestamp.nanosec = this.info.reception_timestamp.nanosec;
            cloned.info.sample_rank = this.info.sample_rank;
            cloned.info.sample_state = this.info.sample_state;
            cloned.info.source_timestamp.sec = this.info.source_timestamp.sec;
            cloned.info.source_timestamp.nanosec = this.info.source_timestamp.nanosec;
            cloned.info.valid_data = this.info.valid_data;
            cloned.info.view_state = this.info.view_state;

            return cloned;
        } catch (CloneNotSupportedException e) {
            throw new UnsupportedOperationExceptionImpl(this.environment,
                    "Cloning of Sample not supported.");
        }
    }
}
