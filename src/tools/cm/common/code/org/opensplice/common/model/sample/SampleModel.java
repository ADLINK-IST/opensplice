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
package org.opensplice.common.model.sample;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Reader;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.cmdataadapter.protobuf.ProtobufDataAdapterFactory;
import org.opensplice.cmdataadapter.protobuf.ProtobufFieldProperties;
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
     * @throws CommonException Thrown when no data could be retrieved from the
     *
     */
    public abstract Sample read() throws CommonException, SampleModelSizeException;

    /**
     * Takes a Sample from its entity.
     *
     * @throws SampleModelSizeException Thrown when no more data can be added
     *                                  to this model.
     * @throws CommonException Thrown when no data could be retrieved from the
     *                         reader.
     */
    public abstract Sample take() throws CommonException, SampleModelSizeException;

    public abstract Sample readNext() throws CommonException, SampleModelSizeException;

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

    /* check if the given name is an collection */
    public boolean isCollection(String name, int index) {
        boolean result = false;
        UserData data = this.getUserDataModel().getDataAt(index).getMessage().getUserData();
        if (data != null) {
            result = data.isCollection(name);
        }
        return result;
    }

    /* check if the given name is an unboundedSequence */
    public boolean isUnboundedSequence(String name, int index) {
        boolean result = false;
        UserData data = this.getUserDataModel().getDataAt(index).getMessage().getUserData();
        if (data != null) {
            result = data.isUnboundedSequence(name);
        }
        return result;
    }

    public void initiateToBeWrittenUserData(UserData data) {
        toBeWrittenUserData = data;
    }

    public UserData getToBeWrittenUserData() {
        return toBeWrittenUserData;
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

    /**
     * Gets the list of keys for the data type that this SampleModel is holding.
     * @return A String containing comma separated key values.
     * @throws CommonException Thrown when there is a problem retrieving the keys from the type metadata.
     */
    public String getKeyList() throws CommonException{
        String[] keyList;
        StringBuilder result = new StringBuilder();

        try {
            keyList = typeInfo.getKeys();
        } catch (CmDataException e) {
            throw new CommonException (e.getMessage());
        }

        for(int i=0; i<keyList.length; i++){
            if (result.length() != 0) {
                result.append(",");
            }
            result.append(keyList[i].replaceAll("^userData\\.", ""));
        }
        return result.toString();
    }

    /**
     * Get the ProtobufFieldProperties for every field name in the data type.
     *
     * @return A Map of field names to their corresponding ProtobufFieldProperties.
     *         If the Protobuf feature is disabled in this build of Tuner, then returns an empty Map.
     * @throws CommonException If there is a problem while retrieving the ProtobufFieldProperties
     *         from the protobuf metadata.
     */
    public Map<String, ProtobufFieldProperties> getProtobufFieldProperties() throws CommonException {
        if (ProtobufDataAdapterFactory.getInstance().isEnabled() &&
                typeInfo.getDataRepresentationId() == TypeInfo.GPB_DATA_ID) {
                try {
                    Map<String, ProtobufFieldProperties> result = new HashMap<String, ProtobufFieldProperties>();
                    for (String fieldName : typeEvolution.getMetaType().getFieldNames()) {
                        result.put(fieldName, ProtobufDataAdapterFactory.getInstance()
                                .getFieldProperties(fieldName, typeEvolution));
                    }
                    return result;
                } catch (CmDataException e) {
                    throw new CommonException (e.getMessage());
                }
        }
        return new HashMap<String, ProtobufFieldProperties>();
    }

    /**
     * Get the TypeEvolution that this SampleModel is using to adapt UserData with.
     * @return the TypeEvolution
     */
    public TypeEvolution getTypeEvolution() {
        return typeEvolution;
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

    /**
     * Adds a Sample to the model.
     *
     * @param s The Sample to add.
     * @return true if it was added and is visible (not filtered out by a
     *         UserDataFilter), false otherwise.
     */
    protected boolean addSample(Sample s, String struct){
        boolean added = userDataModel.setData(s,struct);

        if(added){
            singleUserDataModel.setData(s,struct);
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

    protected UserData toBeWrittenUserData = null;

    protected TypeInfo typeInfo = null;

    protected TypeEvolution typeEvolution = null;
}
