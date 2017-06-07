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
package org.opensplice.common.model.sample;

import java.util.HashMap;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.UserDataEditTableModel;

/**
 * Descendant of the ReaderSampleModel component. This components
 * administrates the Sample objects read/taken from a Reader entity and allows
 * the user to write/dispose data using a Writer. 
 * 
 * @date Nov 12, 2004 
 */
public class ReaderWriterDetailSampleModel extends ReaderDetailSampleModel {
    protected Writer writer;
    protected UserDataEditTableModel writerModel;
    protected boolean dataUpdated = false;
    
    /**
     * Constructs a new ReaderWriterSampleModel from the supplied Reader +
     * Writer.
     *
     * @param _reader The Reader to read/take data from.
     * @param _writer The Writer to write/dispose data.
     * @throws CommonException Thrown when Reader or Writer are invalid, or the 
     *                         data type could not be resolved.
     */
    
    public ReaderWriterDetailSampleModel(Reader _reader, Writer _writer, String structName,
            Sample sample, TypeEvolution _typeEvolution) throws CommonException {
        super(_reader, structName, sample, _typeEvolution);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(typeInfo.getMetaType(typeEvolution), structName);
            writerModel.setData(sample);
        } catch (CmDataException e) {
            throw new CommonException("Writer data type could not be resolved.");
        }
    }
    
    /**
     * Constructs a new ReaderWriterSampleModel from the supplied Reader +
     * Writer.
     *
     * @param _reader The Reader to read/take data from.
     * @param _writer The Writer to write/dispose data.
     * @throws CommonException Thrown when Reader or Writer are invalid, or the 
     *                         data type could not be resolved.
     */
    
    public ReaderWriterDetailSampleModel(Reader _reader, Writer _writer, String structName,
            UserData data, TypeEvolution _typeEvolution) throws CommonException {
        super(_reader, structName, data, _typeEvolution);
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(typeInfo.getMetaType(typeEvolution), structName);
            writerModel.setData(data);
        } catch (CmDataException e) {
            throw new CommonException("Writer data type could not be resolved.");
        }
    }
    
    public ReaderWriterDetailSampleModel(Writer _writer, String structName,
            UserData data, TypeEvolution _typeEvolution) throws CommonException {
        super(_writer, structName, data, _typeEvolution);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(typeInfo.getMetaType(typeEvolution), structName);
            writerModel.setData(data);
        } catch (CmDataException e) {
            throw new CommonException("Writer data type could not be resolved.");
        }
    }
    
    public void setDataUpdated(boolean b) {
        dataUpdated = b;
    }
    
    public boolean getDataUpdated() {
        return dataUpdated;
    }
    
    @Override
    public void clean(){
        writerModel.getDataVector().removeAllElements();
    }
        
    /**
     * Disposes the supplied data with the Writer of the model.
     * 
     * @param data The data to dispose.
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void dispose() throws CommonException{
        try {
            writer.dispose(writerModel.getData());
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
    }
    
    public boolean isCollectionType(String name) throws CommonException{
        boolean result = false;

        try {
            UserData data = writerModel.getData();
            if (data != null) {
                result = data.isCollection(name);
            }
        } catch (CommonException e) {
            throw new CommonException(e.getMessage());
        }
        
        return result;
    }
    
    /**
     * WriteDisposes the supplied data with the Writer of the model.
     * 
     * @param data The data to writeDispose.
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void writeDispose() throws CommonException{
        try {
            writer.writeDispose(writerModel.getData());
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
    }
    
    /**
     * Registers the supplied data with the Writer of the model.
     * 
     * @param data The data to register.
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void register() throws CommonException{
        try {
            writer.register(writerModel.getData());
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
    }
    
    /**
     * Unregisters the supplied data with the Writer of the model.
     * 
     * @param data The data to unregister.
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void unregister() throws CommonException{
        try {
            writer.unregister(writerModel.getData());
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
    }
    
    /**
     * Provides access to the writerModel.
     *  
     * @return The writerModel (UserDataEditTableModel).
     */
    public UserDataEditTableModel getWriterModel(){
        return writerModel;
    }

    /**
     * Selects the Sample at the supplied index. The sampleInfoModel is filled
     * with the Sample/Message info of the Sample at the supplied index and
     * the singleUserDataModel is filled with the UserData within the Sample
     * at the supplied index.
     * 
     * @param index The index of the Sample in the list of samples.
     * @return When the supplied index is not available, false is returned,
     *         true otherwise. 
     */
    @Override
    public boolean setSelectedSample(int index, String struct){
        boolean result = super.setSelectedSample(index, struct);
        Sample s = null;
        if(!result){
            return false;
        }
        try{
            s = userDataModel.getDataAt(index);
        } catch(IndexOutOfBoundsException e){}
        
        if(s == null){
            return false;
        } else if(s.getMessage() == null){
            return false;
        } else if(s.getMessage().getUserData() == null){
            return false;
        }
        UserData data = s.getMessage().getUserData();
        if (data.isCollection(struct)) {
            writerModel.setData(s, struct, index);
        } else {
            writerModel.setData(s, struct);
        }
        return true;
    }
    
    /**
     * Adds a Sample to the model.
     * 
     * @param s
     *            The Sample to add.
     * @return true if it was added and is visible (not filtered out by a
     *         UserDataFilter), false otherwise.
     */

    public void updateWriterData(HashMap<String,String> data) {
        getToBeWrittenUserData().getUserData().putAll(data);
    }

    /**
     * Clears the existing sample in this model, and replaces it with
     * the supplied Sample.
     *
     * @param s The Sample to add.
     * @return true if it was added and is visible (not filtered out by a
     *         UserDataFilter), false otherwise.
     */
    public boolean addSample(Sample s, String struct, int index) {
        boolean added = super.addSample(s,struct);
        if(added && writerModel != null){
            writerModel.clear();
            writerModel.setData(s,struct, index);
        }
        return added;
    }

    /**
     * Provides access to the Writer of this model.
     * 
     * @return The Writer of the model.
     */
    public Writer getWriter(){
        return writer;
    }

    /**
     * Retrieve the Topic Entity for the Writer or Reader. This reference must be freed by the caller.
     * @return the Topic Entity associated with the Writer or Reader.
     * @throws CommonException
     */
    protected Topic getTopic(Entity entity) throws CommonException{
        if (entity instanceof Reader) {
            return super.getTopic(entity);
        }
        Topic result = null;
        Writer w = (Writer) entity;

        try {
            result = (Topic) w.getOwnedEntities(EntityFilter.TOPIC)[0];
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
        return result;
    }
}
