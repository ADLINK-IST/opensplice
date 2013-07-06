/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.common.model.sample;

import java.util.HashMap;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Writer;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
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
    
    public ReaderWriterDetailSampleModel(Reader _reader, Writer _writer, String structName, Sample sample) throws CommonException {
        super(_reader, structName, sample);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(writer.getDataType(), structName);
        } catch (CMException e) {
            throw new CommonException("Writer data type could not be resolved.");
        } catch (DataTypeUnsupportedException e) {
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
    
    public ReaderWriterDetailSampleModel(Reader _reader, Writer _writer, String structName, UserData data) throws CommonException {
        super(_reader, structName, data);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(writer.getDataType(), structName);
        } catch (CMException e) {
            throw new CommonException("Writer data type could not be resolved.");
        } catch (DataTypeUnsupportedException e) {
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
     * Provides access to the Writer of this model.
     * 
     * @return The Writer of the model.
     */
    public Writer getWriter(){
        return writer;
    }
}
