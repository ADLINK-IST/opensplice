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

import java.io.File;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Topic;
import org.opensplice.cm.data.Sample;
import org.opensplice.common.CommonException;
import org.opensplice.common.SampleModelSizeException;
import org.opensplice.common.model.ModelRegister;
import org.opensplice.common.model.table.SampleInfoTableModel;
import org.opensplice.common.model.table.UserDataSingleTableModel;
import org.opensplice.common.model.table.UserDataTableModel;

/**
 * Represents a container for Samples. The samples are administrated in:
 * - UserDataTableModel :      All UserData of the Sample object that are added
 *                             will be administrated in this component.
 * - UserDataSingleTableModel: This component holds one instance of UserData.
 *                             Which one is determined by the user.
 * - SampleInfoTableModel:     This component holds one instance of SampleInfo
 *                             Which one is determined by the user.
 * - ArrayList:                All added Sample objects are added to this list
 *                             so all added Sample objects can be found later
 *                             on.
 * 
 * Its purpose is to:
 * - Supply a simplification for the administration of a whole bunch of Sample 
 *   objects.
 * - Supply a generic interface for reading/taking data from several types of
 *   entities.
 * - Supply a mechanism to be able to browse through the administration of
 *   samples.
 * 
 * @date Oct 21, 2004 
 */
public abstract class SampleModel extends ModelRegister{
    public SampleModel(){
        sampleInfoModel = new SampleInfoTableModel();
    }
    
    /**
     * Reads a Sample from its reader.
     * 
     * @throws SampleModelSizeException Thrown when no more data can be added
     *                                  to this model.
     * @throws CommonException Thrown when no data could be retreived from the
     *    
     */
    public abstract Sample read() throws CommonException, SampleModelSizeException;
    
    /**
     * Takes a Sample from its entity.
     * 
     * @throws SampleModelSizeException Thrown when no more data can be added
     *                                  to this model.
     * @throws CommonException Thrown when no data could be retreived from the
     *                         reader.
     */
    public abstract Sample take() throws CommonException, SampleModelSizeException;
    
    public abstract Sample readNext() throws CommonException, SampleModelSizeException;
    
    /**
     * Provides access to the key list of the data that is read/taken.
     * 
     * @return The comma separated list of keys.
     */
    public abstract String getDataTypeKeyList() throws CommonException;
    
    /**
     * Exports the complete content of the model to the specified file in XML
     * format.
     * 
     * @param file The file to export the data to.
     * @return The number of samples that have been exported.
     * @throws CommonException Thrown when the file cannot be created or is not 
     *                         accessible
     */
    public abstract int export(File file) throws CommonException;
    
    /**
     * Checks whether the maximum amount of data in the model has been reached.
     * 
     * @throws SampleModelSizeException Thrown when the maximum has been 
     *                                  reached.
     */
    public void checkSize() throws SampleModelSizeException{
        int max = 2000;
        
        if(userDataModel.getRowCount() >= max){
            throw new SampleModelSizeException("Maximum #samples in model reached(" + max + ").");
        }
    }
    
    /**
     * Clears the data in the model.    
     */
    public void clear(){
        userDataModel.clear();
        singleUserDataModel.clear();
        sampleInfoModel.clear();
        
    }
    
    /**
     * Clears a part of the data in the model.
     * 
     * @param beginRow The begin row of the data to remove.
     * @param endRow The end row of the data to remove.
     */
    public void clear(int beginRow, int endRow){
        if(beginRow <= endRow){
            if((beginRow >=0) && (endRow < userDataModel.getRowCount()) ){
                userDataModel.clear(beginRow, endRow);
            }
            singleUserDataModel.clear();
            sampleInfoModel.clear();            
        }
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
        Sample s = null;
        try{
            s = userDataModel.getDataAt(index);
        } catch(IndexOutOfBoundsException e){}
        
        if(s == null){
            return false;
        }
        sampleInfoModel.setData(s);
        
        if(s.getMessage() != null){
            singleUserDataModel.setData(s);
        }
        return true;
    }
    
    /**
     * Provides access to the Sample at the supplied index in the list
     * of samples.
     * 
     * @param index The index of the Sample in the administration.
     * @return The Sample at the supplied index.
     */
    public Sample getSampleAt(int index){
        Sample s = null;
        try{
            s = userDataModel.getDataAt(index);
        } catch(IndexOutOfBoundsException e){}
        return s;
    }
    
    /**
     * Provides access to singleUserDataModel.
     * 
     * @return Returns the singleUserDataModel.
     */
    public UserDataSingleTableModel getSingleUserDataModel() {
        return singleUserDataModel;
    }
    /**
     * Provides access to userDataModel.
     * 
     * @return Returns the userDataModel.
     */
    public UserDataTableModel getUserDataModel() {
        return userDataModel;
    }
    
    /**
     * Provides access to sampleInfoTableModel.
     * 
     * @return Returns the sampleInfoTableModel.
     */
    public SampleInfoTableModel getSampleInfoModel() {
        return sampleInfoModel;
    }
    
    protected String getKeyList(Topic t) throws CommonException{
        String keys = null;
        String result = null;
        String[] keyList;

        keys = t.getKeyList();

        if(keys != null){
            keyList = keys.split(",");

            for(int i=0; i<keyList.length; i++){
                if(result != null){
                    result += "," + keyList[i].substring(9);
                } else {
                    result = keyList[i].substring(9);
                }
            }
        }
        return result;
    }

    /**
     * Adds a Sample to the model.
     * 
     * @param s The Sample to add.
     * @return true if it was added and is visible (not filtered out by a
     *         UserDataFilter), false otherwise.
     */
    protected boolean addSample(Sample s){
        boolean added = userDataModel.setData(s);
        
        if(added){
            singleUserDataModel.setData(s);
            sampleInfoModel.setData(s);
            lastReadSample = s;
        }
        return added;
    }
    
    protected String getPartitions(Reader reader){
        String partitions = null;
        Entity entity;
        
        try {
            Entity[] entities = reader.getDependantEntities(EntityFilter.SUBSCRIBER);
            
            if(entities.length > 0){
                /*Free all but the first one.*/
                for(int i=1; i<entities.length; i++){
                    entities[i].free();
                }
                entity = entities[0];
                entities = entity.getOwnedEntities(EntityFilter.PARTITION);
                entity.free();
                
                for(int i=0; i<entities.length;i++){
                    if(partitions == null){
                        partitions = entities[i].getName();
                    } else {
                        partitions += "," + entities[i].getName();
                    }
                    entities[i].free();
                }
            }
        } catch (CMException e) {}
        return partitions;
    }
    
    /**
     * All UserData of the Sample object that are added will be administrated 
     * in this component.
     */
    protected UserDataTableModel userDataModel;
    
    /**
     * This component holds one instance of UserData. Which one is determined 
     * by the user.
     */
    protected UserDataSingleTableModel singleUserDataModel;
    
    /**
     * This component holds one instance of SampleInfo Which one is determined 
     * by the user.
     */
    protected SampleInfoTableModel sampleInfoModel;
    
    protected Sample lastReadSample = null;
}
