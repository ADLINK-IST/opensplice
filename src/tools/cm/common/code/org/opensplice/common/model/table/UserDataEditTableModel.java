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

import java.util.LinkedHashSet;

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaCollection;
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

    public UserDataEditTableModel(MetaType _userDataType, UserData initialData) {
        super(_userDataType, true);
        if (initialData != null) {
            setData(initialData);
        }
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
            else if (ud.isCollection(name) &&
                    ((MetaCollection) ud.getUserDataType().getField(name)).getSubType() instanceof MetaCollection)
            {
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

    public UserData getDataNoValidate() {
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

    public boolean setData(UserData data) {
        boolean result = false;

            if(data != null){
                ud.getUserData().putAll(data.getUserData());
                int rowCount = this.getRowCount();
                LinkedHashSet<String> s = new LinkedHashSet<String>(data.getUserData().keySet());
                for (int i = 0; i < rowCount; i++) {
                    String fieldName = (String) this.getValueAt(i, 1);
                    try {
                        if (ud.getUserDataType() != null && fieldName != null && ud.getUserDataType().getField(fieldName).getTypeName().startsWith("C_SEQUENCE")) {
                            this.setValueAt(ud.getFieldValue(fieldName), i, 2);
                            result = true;
                        } else {
                            StringBuilder fieldValue = new StringBuilder();
                            LinkedHashSet<String> tmpS = new LinkedHashSet<String>(s);
                            for (String key : tmpS) {
                                String value = (data.getUserData().get(key));
                                String newKey = key.replaceAll("[\\[0-9]*]", "");
                                /*
                                 * if the key is equal to the fieldName we got the same data
                                 * so ignore for GUI
                                 */
                                if (newKey.startsWith(fieldName)) {
                                    if (fieldValue.length() != 0) {
                                        fieldValue.append(",");
                                    }
                                    fieldValue.append(value);
                                    s.remove(key);
                                }
                            }
                            if (fieldValue.length() != 0) {
                                this.setValueAt(fieldValue.toString(), i, 2);
                            }
                        }
                    } catch (NullPointerException npe) {
                        // ignore see OSPL-10362
                    }
                }
                result = true;
            }
        return result;
    }

    @Override
    public boolean setData(Sample sample, String colName) {
        return setData(sample, colName, 0);
    }

    @Override
    public boolean setData(Sample sample, String colName, int index) {
        boolean result = true;

        if (sample != null) {
            UserData data = sample.getMessage().getUserData();
            int rowCount = this.getRowCount();
            if (data == null) {
                return false;
            }
            ud.getUserData().putAll(data.getUserData());
            for (int i = 0; i < rowCount; i++) {
                String tableRowFName = (String) this.getValueAt(i, 1);
                String structMember = "";
                int dotIndex = tableRowFName.lastIndexOf('.');
                if (dotIndex != -1) {
                    structMember = tableRowFName.substring(dotIndex);
                }

                String fName = colName;
                Object fType = data.getUserDataType().getField(fName);
                if (fType instanceof MetaCollection && ((MetaCollection) fType).getSubType() instanceof MetaCollection) {
                    this.setValueAt(data.getFieldValue(fName), i, 2);
                } else {
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
        ud.getUserData().clear();
    }


    /**
     * The editor that offers the cell editors and validates input.
     */
    protected UserDataEditTableEditor editor;


}
