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
package org.opensplice.common.view.table;

import javax.swing.JTable;

import org.opensplice.common.controller.UserDataSingleTableCellRenderer;
import org.opensplice.common.model.table.UserDataSingleTableModel;

/**
 * Table that displays a UserDataSingleTableModel. So it displays one instance
 * of UserData.
 * 
 * @date Oct 22, 2004 
 */
public class UserDataSingleTable extends JTable{
    protected String userDataKeys = null;
    protected String structName = null;
    
    /**
     * Constructs a new UserDataEditTable from the supplied arguments. The
     * table will display the supplied model.
     *
     * @param model The model that must be displayed in the table.
     * @param keyList The keys in the data of the model. The background of 
     *                these fields will be colored as defined by the 
     *                UserDataSingleTableCellRenderer.
     */
    public UserDataSingleTable(UserDataSingleTableModel model, String keyList){
        super(model);
        
        if(keyList != null){
            this.setKeyList(keyList);
        }
    }
    
    public UserDataSingleTable(UserDataSingleTableModel model, String keyList, String struct){
        super(model);
        structName = struct;
        if(keyList != null){
            this.setKeyList(keyList);
        }
    }
    
    
    /**
     * Sets the key list of the data in this table. Backgrounds of cells that 
     * hold these fields will be colored as defined by the 
     * UserDataSingleTableCellRenderer.
     * 
     * @param keyList The list of keys of the contained data.
     */
    public void setKeyList(String keyList){
        String[] keys;
        
        if(keyList != null){
            userDataKeys = keyList;
            keys = keyList.split(",");
            UserDataSingleTableCellRenderer renderer = new UserDataSingleTableCellRenderer(keys);
            this.getColumnModel().getColumn(1).setCellRenderer(renderer);
        }
    }
    
    /**
     * Provides access to userDataKeys.
     * 
     * @return Returns the userDataKeys.
     */
    public String getUserDataKeys() {
        return userDataKeys;
    }
    
    /**
     * Provides access to the structName.
     * 
     * @return Returns the structName.
     */
    public String getStructName() {
        return structName;
    }
}
