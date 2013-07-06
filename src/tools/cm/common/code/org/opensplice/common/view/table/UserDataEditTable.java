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
 * Contains all SPLICE DDS C&M Tooling common table components.
 */
package org.opensplice.common.view.table;

import org.opensplice.common.controller.UserDataEditTableEditor;
import org.opensplice.common.model.table.UserDataEditTableModel;
import org.opensplice.common.view.StatusPanel;

/**
 * Table that displays an UserDataEditTableModel. So it displays one instance
 * of UserData and allows the user to edit the values of the fields in the 
 * UserData by assigning a UserDataTableEditor to the model.
 * 
 * @date Oct 28, 2004 
 */
public class UserDataEditTable extends UserDataSingleTable {
    
    /**
     * Constructs a new UserDataEditTable from the supplied arguments. The
     * table will display the supplied model and attaches a 
     * UserDataEditTableEditor to it, which makes it possible to edit values
     * of fields in the data held by the model.
     *
     * @param model The model that must be displayed in the table.
     * @param keyList The keys in the data of the model. The background of 
     *                these fields will be colored as defined by the 
     *                UserDataSingleTableCellRenderer.
     * @param editOutputWindow The statusbar, where information about editing
     *                         and validation will be sent to.
     */
    public UserDataEditTable(UserDataEditTableModel model, String keyList, StatusPanel editOutputWindow){
        this(model, keyList);
        
        ((UserDataEditTableEditor)
                (this.getColumnModel().getColumn(2).getCellEditor())).setStatusListener(editOutputWindow);
    }
    
    public UserDataEditTable(UserDataEditTableModel model, String keyList, StatusPanel editOutputWindow, String struct){
        this(model, keyList, struct);

        ((UserDataEditTableEditor)
                (this.getColumnModel().getColumn(2).getCellEditor())).setStatusListener(editOutputWindow);
    }
    
    /**
     * Constructs a new UserDataEditTable from the supplied arguments. The
     * table will display the supplied model and attaches a 
     * UserDataEditTableEditor to it, which makes it possible to edit values
     * of fields in the data held by the model.
     *  
     *
     * @param model The model that must be displayed in the table.
     * @param keyList The keys in the data of the model. The background of 
     *                these fields will be colored as defined by the 
     *                UserDataSingleTableCellRenderer.
     */
    public UserDataEditTable(UserDataEditTableModel model, String keyList){
        super(model, keyList);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.setCellSelectionEnabled(false);
       
        this.setSurrendersFocusOnKeystroke(true);
        UserDataEditTableEditor editor = new UserDataEditTableEditor(model.getDataType(), this);
        this.getColumnModel().getColumn(2).setCellEditor(editor);
        model.setEditor(editor);
    }
    
    public UserDataEditTable(UserDataEditTableModel model, String keyList, String struct){
        super(model, keyList, struct);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.setCellSelectionEnabled(false);
       
        this.setSurrendersFocusOnKeystroke(true);
        UserDataEditTableEditor editor = new UserDataEditTableEditor(model.getDataType(), this);
        this.getColumnModel().getColumn(2).setCellEditor(editor);
        model.setEditor(editor);
    }
}
