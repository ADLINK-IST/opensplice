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
/**
 * Contains all common SPLICE DDS C&M Tooling model components that concern
 * Sample information.
 */
package org.opensplice.common.model.sample;

import org.opensplice.cm.Reader;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.UserDataSingleTableModel;
import org.opensplice.common.model.table.UserDataTableModel;

/**
 * Represents a model that administrates the history of a specific Sample.
 * 
 * @date Oct 25, 2004 
 */
public class HistorySampleModel extends ReaderSampleModel {
    /**
     * The sample of which the history is administrated.
     */
    private Sample sample = null;
    
    /**
     * Creates a new history administration. The table model is filled with the
     * history of the supplied Sample.
     *
     * @param _sample The Sample, which history must be added.
     * @throws CommonException
     */
    public HistorySampleModel(Reader _reader, Sample _sample, TypeEvolution _typeEvolution) throws CommonException{
        super(_reader, _typeEvolution);
        sample = _sample;
        if(sample != null){
            MetaType mt = null;
            try {
                mt = typeInfo.getMetaType();
            } catch (CmDataException e) {
                throw new CommonException(e.getMessage());
            }
            userDataModel = new UserDataTableModel(mt);
            singleUserDataModel = new UserDataSingleTableModel(mt, false);
            this.addSample(sample);
        }
    }
    
    /**
     * Does nothing.
     */
    @Override
    public Sample read() {
        return null;
    }

    /**
     * Does nothing.
     */
    @Override
    public Sample take() {
        return null;
    }

    /**
     * Instead of added the Sample to the list and the userDataTable as its
     * parent class, it adds the history of the Sample to the list and places
     * the history in the userDataTable
     * 
     * @param s The Sample, which history is administrated.
     */
    @Override
    protected boolean addSample(Sample s){
        if(s != null){
            userDataModel.clear();
            singleUserDataModel.clear();
            sampleInfoModel.clear();
            
            Sample previous = s;
            
            while(previous != null){
                previous = previous.getPrevious();
                
                if(previous != null){
                    singleUserDataModel.setData(previous);
                    sampleInfoModel.setData(previous);
                    userDataModel.setData(previous);
                }
            }
            return true;
        }
        return false;
    }
    
    /**
     * Provides access to sample.
     * 
     * @return The sample.
     */
    public Sample getSample(){
        return sample;
    }
}
