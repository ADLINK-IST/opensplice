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
package org.opensplice.common.model.table;

import java.util.LinkedHashMap;
import java.util.LinkedHashSet;

import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.common.CommonException;

/**
 * Table model that holds one instance of UserData. Data can be assigned to
 * the model as well as retrieved.
 *
 * @date Oct 21, 2004
 */
public class UserDataSingleTableModel extends DefaultTableModel {
    /**
     * 
     */
    private static final long serialVersionUID = -2818640876069836948L;

    /**
     * Creates a new table model, which makes it possible to edit values of
     * fields in UserData instances. The UserData type must be the same as the
     * supplied type.
     * 
     * @param _userDataType
     *            The type of the UserData that can be edited.
     * @param initFieldValue
     *            Whether or not to initialize to values of the fields in the
     *            data to a valid value. If not, values are not assigned and all
     *            cells that display a value will display 'N/A'.
     */
    public UserDataSingleTableModel(MetaType _userDataType, boolean initFieldValues) {
        super();
        this.addColumn("Field type");
        this.addColumn("Field name");
        this.addColumn("Field value");
        userDataType = _userDataType;
        ud = new UserData(userDataType);
        this.initModel(initFieldValues);
        
    }

    public UserDataSingleTableModel(MetaType _userDataType, boolean initFieldValues, String struct) {
        super();
        this.addColumn("Field type");
        this.addColumn("Field name");
        this.addColumn("Field value");
        userDataType = _userDataType;
        ud = new UserData(userDataType);
        structName = struct;
        this.initModel(initFieldValues,struct);
    }



    /**
     * Called when user wants to edit a field in the table.
     *
     * Always returns false
     *
     * @param row The row to edit.
     * @param column The column to edit.
     * @return Always false.
     */
    @Override
    public boolean isCellEditable(int row, int column){
        return false;
    }

    /**
     * Assigns the supplied value to the supplied field in the UserData.
     *
     * @param fieldName The name of the field.
     * @param fieldValue The value to assign to the field.
     */
    public void setUserDataField(String fieldName, String fieldValue){
        ud.setData(fieldName, fieldValue);
    }

    /**
     * Provides access to the UserData, that is currently in the table model.
     *
     * @return The current UserData.
     */
    public UserData getData() throws CommonException{
        if(ud == null){
            throw new CommonException("Data == null");
        }
        return ud;
    }

    /**
     * Sets the supplied data in the model.
     *
     * @param sample The sample to administrate in this model.
     * @return true if the data != null and the type of the data equals the
     *              type of this model.
     * @todo TODO: Compare data types.
     */
    public boolean setData(Sample sample){
        UserData data;
        boolean result = false;

        if(sample != null){
            data = sample.getMessage().getUserData();
            int rowCount = this.getRowCount();

            for(int i=0; i<rowCount; i++){
                String fieldName = (String)this.getValueAt(i, 1);
                String fieldValue = data.getFieldValue(fieldName);
                this.setValueAt(fieldValue, i, 2);
            }
            ud = data;
            result = true;
        }
        return result;
    }

    public boolean setData(Sample sample, String colName) {
        return setData(sample, colName, 0);
    }

    /**
     * Set the data in the Single table according to the supplied source UserData, the
     * desired collection name, and the index of the collection we are viewing.
     * @param sample
     * @param colName
     * @param index
     * @return
     */
    public boolean setData(Sample sample, String colName, int index) {
        UserData data;
        boolean result = true;
        if (sample != null) {
            data = sample.getMessage().getUserData();
            int rowCount = this.getRowCount();
            if (data == null) {
                return false;
            }
            for (int i = 0; i < rowCount; i++) {
                String tableRowFName = (String) this.getValueAt(i, 1);
                String structMember = "";
                int dotIndex = tableRowFName.lastIndexOf('.');
                if (dotIndex != -1) {
                    structMember = tableRowFName.substring(dotIndex);
                }

                String fName = colName;
                int indexindex = colName.indexOf('[' ,colName.lastIndexOf('.'));
                String colNameStripped = colName;
                String colNameColIndex = "";
                if (indexindex != -1) {
                    colNameStripped = colName.substring(0, indexindex);
                    colNameColIndex = colName.substring(indexindex);
                }
                fName = colNameStripped + "[" + index + "]" + colNameColIndex + structMember;
                this.setValueAt(data.getFieldValue(fName), i, 2);
            }
            ud = data;
        }

        return result;
    }

    /**
     * Provides access to the data type of the data in this table model.
     *
     * @return The userDataType.
     */
    public MetaType getDataType(){
        return userDataType;
    }

    /**
     * Clears the model.
     */
    public void clear(){
        int rowCount = this.getRowCount();

        for(int i=0; i<rowCount; i++){
            this.setValueAt("N/A", i, 2);
        }
    }
    
    /**
     * Clears the model.
     */
    public void clean(){
        this.getDataVector().removeAllElements();
    }
    
    public void updateDetailTable(String struct) {
        this.clean();
        this.initModel(false,struct);
    }

    public void updateTable() {
        this.clean();
        this.initModel(true);
    }

    /**
     * Initializes the model by walking the type and adding all types
     * to its table and setting initial values of the data.
     *
     * @param validValues whether to set the values of the field to a valid
     *                    value or not.
     */
    private void initModel(boolean validValues){
        LinkedHashMap<String, String> lhm = userDataType.collectAllFieldNames(1, true);
        LinkedHashSet<String> s = new LinkedHashSet<String>(lhm.keySet());
        for (String key : s) {
            Object[] data = new Object[3];
            String typeName = "";
            MetaField mf = userDataType.getField(key);
            if (mf != null) {
                typeName = mf.getTypeName();
            }
            data[0] = typeName;
            data[1] = key;
            data[2] = lhm.get(key);
            this.addRow(data);
        }
    }

    /**
     * Initializes the model by walking the type and adding all types
     * to its table and setting initial values of the data.
     *
     * @param validValues whether to set the values of the field to a valid
     *                    value or not.
     */
    private void initModel(boolean validValues, String struct){
        LinkedHashMap<String, String> lhm = userDataType.collectAllFieldNames(1, struct, true);
        LinkedHashSet<String> s = new LinkedHashSet<String>(lhm.keySet());
        for (String key : s) {
            Object[] data = new Object[3];
            String typeName = "";
            MetaField mf = userDataType.getField(key);
            if (mf != null) {
                typeName = mf.getTypeName();

            }
            data[0] = typeName;
            data[1] = key;
            data[2] = lhm.get(key);
            this.addRow(data);
        }
    }

    public String getStructName() {
        return structName;
    }

    public boolean isLastEditied() {
        return lastEditied;
    }

    public void setLastEditied(boolean b) {
        lastEditied = b;
    }

    /**
     * The type of the UserData that is currently being held.
     */
    protected MetaType userDataType;

    /**
     * The UserData that is currently being held.
     */
    protected UserData ud;
    
    protected String structName = null;

    protected boolean  lastEditied = false;

    public void stopEdit() throws CommonException {
    } /* nothing to do here */
    
}
