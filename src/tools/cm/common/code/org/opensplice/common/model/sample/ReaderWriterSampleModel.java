/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.common.model.sample;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Writer;
import org.opensplice.cm.data.Sample;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.UserDataEditTableModel;

/**
 * Descendant of the ReaderSampleModel component. This components
 * administrates the Sample objects read/taken from a Reader entity and allows
 * the user to write/dispose data using a Writer. 
 * 
 * @date Nov 12, 2004 
 */
public class ReaderWriterSampleModel extends ReaderSampleModel {
    protected Writer writer;
    protected UserDataEditTableModel writerModel;
    
    /**
     * Constructs a new ReaderWriterSampleModel from the supplied Reader +
     * Writer.
     *
     * @param _reader The Reader to read/take data from.
     * @param _writer The Writer to write/dispose data.
     * @throws CommonException Thrown when Reader or Writer are invalid, or the 
     *                         data type could not be resolved.
     */
    public ReaderWriterSampleModel(Reader _reader, Writer _writer) throws CommonException {
        super(_reader);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(writer.getDataType());
        } catch (CMException e) {
            throw new CommonException("Writer data type could not be resolved.");
        } catch (DataTypeUnsupportedException e) {
            throw new CommonException("Writer data type could not be resolved.");
        }
    }
    
    /**
     * Writes the current data in the writer model with the Writer of this model.
     * 
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void write() throws CommonException{
        try {
            writer.write(writerModel.getData());
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
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
    public boolean setSelectedSample(int index){
        boolean result = super.setSelectedSample(index);
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
        writerModel.setData(s.getMessage().getUserData());
        
        return true;
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
