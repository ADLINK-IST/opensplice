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
package org.opensplice.common.model.table;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;

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

    protected HashMap<String, ArrayList<String>> CollectSingleTableData(UserData data, String struct) {
        LinkedHashSet<String> s;
        List<String> tmp;

        MetaType mt = data.getUserDataType();
        s = new LinkedHashSet<String>(data.getUserData().keySet());
        HashMap<String, ArrayList<String>> tableVales = new HashMap<String, ArrayList<String>>();
        tmp = new ArrayList<String>();
        /* strip [] from userdata */
        for (String key : s) {

            String newKey = mt.removeIndicesFromStruct(key, struct);
            tmp.add(newKey);
            if (tableVales.containsKey(newKey)) {
                ArrayList<String> tmpVal = tableVales.get(newKey);
                tmpVal.add(data.getUserData().get(key));
                tableVales.put(newKey, tmpVal);
            } else {
                ArrayList<String> tmpVal = new ArrayList<String>();
                tmpVal.add(data.getUserData().get(key));
                tableVales.put(newKey, tmpVal);
            }
        }
        return tableVales;
    }

    public boolean setData(Sample sample, String colName) {
        return setData(sample, colName, -1);
    }

    public boolean setData(Sample sample, String colName, int index) {
        UserData data;
        boolean result = true;
        if (sample != null) {
            data = sample.getMessage().getUserData();
            int rowCount = this.getRowCount();
            if (data == null) {
                return false;
            }
            HashMap<String, ArrayList<String>> tableData = CollectSingleTableData(data, colName);
            for (int i = 0; i < rowCount; i++) {
                String fName = (String) this.getValueAt(i, 1);
                if (index >= 0) {
                    if (fName.startsWith(colName)) {
                        String tmp = fName.substring(colName.length());
                        fName = colName + "[" + index + "]" + tmp;
                    }
                }

                if (tableData.containsKey(fName)) {
                    ArrayList<String> colVals = tableData.get(fName);
                    if (index >= 0) {
                        this.setValueAt(colVals.get(index), i, 2);
                    } else {
                        this.setValueAt(colVals.toString(), i, 2);
                    }
                } else if (tableData.containsKey(colName)) {
                    ArrayList<String> colVals = tableData.get(colName);
                    if (index >= 0) {
                        this.setValueAt(colVals.get(index), i, 2);
                    } else {
                        this.setValueAt(colVals.toString(), i, 2);
                    }
                } else {
                    LinkedHashSet<String> s = new LinkedHashSet<String>(data.getUserData().keySet());
                    String value = null;
                    for (String key : s) {
                        if (key.startsWith(fName)) {
                            if (value != null) {
                                value = value + "," + data.getUserData().get(key);
                            } else {
                                value = data.getUserData().get(key);
                            }
                        }
                    }
                    if (value == null) {
                        LinkedHashSet<String> td = new LinkedHashSet<String>(tableData.keySet());
                        for (String key : td) {
                            if (key.startsWith(fName)) {
                                if (value != null) {
                                    value = value + "," + tableData.get(key).toString();
                                } else {
                                    value = tableData.get(key).toString();
                                }
                            }
                        }
                    }
                    this.setValueAt(value, i, 2);
                }
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
