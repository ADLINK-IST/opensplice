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
package org.opensplice.common.model.table;

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
     * Creates a new table model, which makes it possible to edit
     * values of fields in UserData instances. The UserData type must be the
     * same as the supplied type.  
     *
     * @param _userDataType The type of the UserData that can be edited.
     */
    public UserDataEditTableModel(MetaType _userDataType) {
        super(_userDataType, true);
    }
    
    /**
     * Called when user wants to edit a field in the table.
     * 
     * Returns true if the user wants to edit one of the value fields and the
     *         editor is set, false otherwise.
     * 
     * @param row The row to edit.
     * @param column The column to edit.
     * @return true if editable, false otherwise.
     */
    public boolean isCellEditable(int row, int column){
        boolean result;
        if(editor == null){
            result = false;
        }
        else if(column < 2){
            result = false;
        }
        else{
            result = true;
        }
        return result;
    }
    
    /**
     * Provides access to the UserData, that is currently in the table.
     * 
     * @return The current UserData.
     */
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
     * Sets the supplied data in the model.
     * 
     * @param data The data to administrate in this model.
     * @return true if the data != null and the type of the data equals the
     *              type of this model.
     * @todo TODO: Compare data types. 
     */
    public boolean setData(UserData data){
        boolean result = false;
        
        if(data != null){
            int rowCount = this.getRowCount();
            
            for(int i=0; i<rowCount; i++){
                String fieldName = (String)this.getValueAt(i, 1);
                String fieldValue = data.getFieldValue(fieldName);
                
                if((fieldValue != null) && (!(fieldValue.equals("NULL")))){
                    this.setValueAt(fieldValue, i, 2);
                    ud.setData(fieldName, fieldValue);
                }
            }
            result = true;
        }
        return result;
    }
    
    /**
     * Sets the supplied editor as the editor for data in this model.
     * 
     * @param _editor The new editor.
     */
    public void setEditor(UserDataEditTableEditor _editor){
        editor = _editor;
    }
    
    public void clear(){}
        
    /**
     * The editor that offers the cell editors and validates input. 
     */
    protected UserDataEditTableEditor editor;
}
