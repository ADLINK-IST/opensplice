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

import org.opensplice.cm.CMException;
import org.opensplice.cm.Reader;
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
    public ReaderWriterSampleModel(Reader _reader, Writer _writer, TypeEvolution _typeEvolution) throws CommonException {
        super(_reader, _typeEvolution);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(typeInfo.getMetaType(typeEvolution));
        } catch (CmDataException e) {
            throw new CommonException("Writer data type could not be resolved.");
        }
    }

    public ReaderWriterSampleModel(Reader _reader, Writer _writer, String struct, TypeEvolution _typeEvolution) throws CommonException {
        super(_reader, struct, _typeEvolution);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        writer = _writer;
        try {
            writerModel = new UserDataEditTableModel(typeInfo.getMetaType(typeEvolution), struct);
        } catch (CmDataException e) {
            throw new CommonException("Writer data type could not be resolved.");
        }
    }

    public boolean isCollectionType(String name) throws CommonException{
        boolean result = false;

        try {
            UserData data = writerModel.getData();
            if (data != null && !data.isStringCollection(name)) {
                result = data.isCollection(name);
            }
        } catch (CommonException e) {
            throw new CommonException(e.getMessage());
        }

        return result;
    }

    /**
     * Writes the current data in the writer model with the Writer of this model.
     *
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void write() throws CommonException{
        try {
            UserData data = writerModel.getData();
            for (int i=0;i<writerModel.getRowCount();i++) {
                String value = (data.getUserData().get(writerModel.getValueAt(i, 1)));
                if (value != null) {
                    data.setData((String)writerModel.getValueAt(i, 1), (String)writerModel.getValueAt(i, 2));
                }
            }
            data = typeInfo.adaptDataForWrite(typeEvolution, data);
            writer.write(data);
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (CmDataException e) {
            throw new CommonException(e.getMessage());
        }
    }

    public boolean isWriterDataValid() throws CommonException {
        boolean result = true;
        UserData data = writerModel.getData();// needed to update the data from the editModel to the writerModel
        for (int i=0;i<writerModel.getRowCount();i++) {
            String name = (String)writerModel.getValueAt(i, 1);
            String value = (data.getUserData().get(name));
            if (value == null || value.equals("NULL")) {
                if (data.isCollection(name)) {
                    result = false;
                }
            }
        }
        return result;
    }

    public boolean isContainsUnboundedSequence() throws CommonException {
        boolean result = true;
        UserData data = writerModel.getData();// needed to update the data from the editModel to the writerModel
        for (int i=0;i<writerModel.getRowCount();i++) {
            String name = (String)writerModel.getValueAt(i, 1);
            String value = (data.getUserData().get(name));
            if (value == null || value.equals("NULL")) {
                if (data.isCollection(name)) {
                    result = false;
                }
            }
        }
        return result;
    }

    /**
     * Writes the data from the UserDataModel with the Writer of this model.
     *
     * @throws CommonException Thrown when the Writer is not available.
     */
    public UserData writeCurrentFrame(UserData currentData) throws CommonException {
        try {
            UserData data = writerModel.getData();
            for (int i=0;i<writerModel.getRowCount();i++) {
                String name = (String)writerModel.getValueAt(i, 1);
                String value = (String)writerModel.getValueAt(i, 2);
                if (name != null && writerModel.isCellEditable(i,2)) {
                    if (value != null) {
                        if (data.isCollection(name) && !data.isStringCollection(name)) {
                            if (!value.startsWith("[")) {
                                value = "[" + value;
                            }
                            if (!value.endsWith("]")) {
                                value =  value +"]";
                            }
                        }
                        currentData.setData(name, value);
                    }
                }
            }
        } catch (CommonException e) {
            throw new CommonException(e.getMessage());
        }
        return currentData;
    }

    public UserData collectUserData(UserData data) throws CommonException {
        if (data == null) {
            data = writerModel.getData();
        } else {
            writerModel.stopEdit();
            /*
             * needed to update the data from the editModel to the writerModel
             */
        }
        for (int i = 0; i < writerModel.getRowCount(); i++) {
            String typeName = (String) writerModel.getValueAt(i, 1);
            if (data.isStringCollection(typeName)) {
                data.setData((String) writerModel.getValueAt(i, 1) + "[0]", (String) writerModel.getValueAt(i, 2));
            }
        }
        return data;
    }

    /**
     * Writes the data from the UserDataModel with the Writer of this model.
     *
     * @throws CommonException Thrown when the Writer is not available.
     */
    public void write(UserData data) throws CommonException{
        try {
            UserData collectedData = collectUserData(data);
            collectedData = typeInfo.adaptDataForWrite(typeEvolution, collectedData);
            writer.write(collectedData);
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (CmDataException e) {
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
        dispose(writerModel.getData());
    }

    public void dispose(UserData data) throws CommonException {
        try {
            UserData collectedData = collectUserData(data);
            collectedData = typeInfo.adaptDataForDispose(typeEvolution, collectedData);
            writer.dispose(collectedData);
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (CmDataException e) {
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
        writeDispose(writerModel.getData());
    }

    public void writeDispose(UserData data) throws CommonException {
        try {
            UserData collectedData = collectUserData(data);
            collectedData = typeInfo.adaptDataForWrite(typeEvolution, collectedData);
            writer.writeDispose(collectedData);
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (CmDataException e) {
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
        register(writerModel.getData());
    }

    public void register(UserData data) throws CommonException {
        try {
            UserData collectedData = collectUserData(data);
            collectedData = typeInfo.adaptDataForDispose(typeEvolution, collectedData);
            writer.register(collectedData);
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (CmDataException e) {
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
        unregister(writerModel.getData());
    }

    public void unregister(UserData data) throws CommonException {
        try {
            UserData collectedData = collectUserData(data);
            collectedData = typeInfo.adaptDataForDispose(typeEvolution, collectedData);
            writer.unregister(collectedData);
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (CmDataException e) {
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
        writerModel.clear();
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
