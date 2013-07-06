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
/**
 * Contains all common SPLICE DDS C&M Tooling model components that concern
 * Sample information.
 */
package org.opensplice.common.model.sample;

import org.opensplice.cm.Reader;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
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
    public HistorySampleModel(Reader _reader, Sample _sample) throws CommonException{
        super(_reader);
        sample = _sample;
        if(sample != null){
            Message msg = sample.getMessage();
            
            if(msg != null){
                UserData ud = msg.getUserData();
                
                if(ud != null){
                    try {
                        userDataModel = new UserDataTableModel(ud.getUserDataType());
                    } catch (CommonException e) {
                        /*This exception will not occur here.*/
                    }
                    singleUserDataModel = new UserDataSingleTableModel(
                                                    ud.getUserDataType(), false);
                    this.addSample(sample);
                }
            }
        }
    }
    
    /**
     * Does nothing.
     */
    public Sample read() {
        return null;
    }

    /**
     * Does nothing.
     */
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
