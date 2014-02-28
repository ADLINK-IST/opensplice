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
import java.util.LinkedHashSet;

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.AssignmentResult;
import org.opensplice.common.controller.UserDataEditTableEditor;

/**
 * Table model which contains one instance of UserData and offers facilities to
 * edit the values of the fields in the data.
 * 
 * @date Oct 28, 2004
 */
public class UserDataEditTableModel extends UserDataSingleTableModel {
    /**
     *
     */
    private static final long serialVersionUID = -4385577459386129238L;

    /**
     * Creates a new table model, which makes it possible to edit values of
     * fields in UserData instances. The UserData type must be the same as the
     * supplied type.
     * 
     * @param _userDataType
     *            The type of the UserData that can be edited.
     */
    public UserDataEditTableModel(MetaType _userDataType) {
        super(_userDataType, true);
    }

    public UserDataEditTableModel(MetaType _userDataType, String _struct) {
        super(_userDataType, true, _struct);
    }

    /**
     * Called when user wants to edit a field in the table.
     * 
     * Returns true if the user wants to edit one of the value fields and the
     * editor is set, false otherwise.
     * 
     * @param row
     *            The row to edit.
     * @param column
     *            The column to edit.
     * @return true if editable, false otherwise.
     */
    @Override
    public boolean isCellEditable(int row, int column){
        boolean result;
        if (column == 2) {
            String name = (String)this.getValueAt(row, column-1);
            int rows = this.getRowCount();
            if (ud.isStringCollection(name)) {
                result = true;
            } else if (ud.isUnboundedSequence(name) && rows > 1) {
                result = false;
            }
            else if (ud.isCollection(name) && rows >1 && structName != null) {
                result = false;
            }
            else if(editor == null){
                result = false;
            }
            else{
                result = true;
            }
        } else {
            result = false;
        }
        return result;
    }

    /**
     * Provides access to the UserData, that is currently in the table.
     * 
     * @return The current UserData.
     */
    @Override
    public UserData getData() throws CommonException {
        if(editor != null){
            AssignmentResult result = editor.testAssignment(false);
            editor.stopCellEditing();

            if(!result.isValid()){
                throw new CommonException(result.getErrorMessage());
            }
        }
        return ud;
    }

    /**
     * Provides stops the table edit so edited data can be written.
     * 
     */
    @Override
    public void stopEdit() throws CommonException {
        if (editor != null) {
            AssignmentResult result = editor.testAssignment(false);
            editor.stopCellEditing();

            if (!result.isValid()) {
                throw new CommonException(result.getErrorMessage());
            }
        }
    }

    /**
     * Sets the supplied data in the model.
     * 
     * @param data
     *            The data to administrate in this model.
     * @return true if the data != null and the type of the data equals the type
     *         of this model.
     * @todo TODO: Compare data types.
     */

    public boolean setData(UserData data, String colName, int index) {
        boolean result = true;
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
        return result;
    }

    public boolean setData(UserData data){
        boolean result = false;
        if(data != null){
            ud.getUserData().putAll(data.getUserData());

            int rowCount = this.getRowCount();
            LinkedHashSet<String> s = new LinkedHashSet<String>(data.getUserData().keySet());
            for (int i = 0; i < rowCount; i++) {
                String fieldName = (String) this.getValueAt(i, 1);
                String fieldValue = null;

                LinkedHashSet<String> tmpS = new LinkedHashSet<String>(s);
                for (String key : tmpS) {
                    String value = (data.getUserData().get(key));
                    String newKey = key.replaceAll("[\\[0-9]*]", "");
                    /*
                     * if the key is equal to the fieldName we got the same data
                     * so ignore for GUI
                     */
                    if (newKey.startsWith(fieldName)) {
                        if (fieldValue == null) {
                            fieldValue = value;
                        } else {
                            fieldValue = fieldValue + "," + value;
                         }
                        this.setValueAt(fieldValue, i, 2);
                        s.remove(key);
                     }
                }
            }
            result = true;
        }
        return result;
    }

    @Override
    public boolean setData(Sample sample, String colName) {
        UserData data;
        boolean result = true;

        if (sample != null) {
            data = sample.getMessage().getUserData();
            int rowCount = this.getRowCount();
            if (data == null) {
                return false;
            }
            ud.getUserData().putAll(data.getUserData());
            HashMap<String, ArrayList<String>> tableData = CollectSingleTableData(data, colName);
            for (int i = 0; i < rowCount; i++) {
                String fName = (String) this.getValueAt(i, 1);
                if (tableData.containsKey(fName)) {
                    ArrayList<String> colVals = tableData.get(fName);
                    this.setValueAt(colVals.toString(), i, 2);
                } else if (tableData.containsKey(colName)) {
                    ArrayList<String> colVals = tableData.get(colName);
                    this.setValueAt(colVals.toString(), i, 2);
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
        }
        return result;
    }

    /**
     * Sets the supplied editor as the editor for data in this model.
     * 
     * @param _editor
     *            The new editor.
     */
    public void setEditor(UserDataEditTableEditor _editor){
        editor = _editor;
    }

    @Override
    public void clear(){
        for (int i = 0; i < this.getRowCount(); i++) {
            this.setValueAt("", i, 2);
        }
    }


    /**
     * The editor that offers the cell editors and validates input.
     */
    protected UserDataEditTableEditor editor;


}
