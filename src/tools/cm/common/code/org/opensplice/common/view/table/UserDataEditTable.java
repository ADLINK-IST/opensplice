/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
     * @param editOutputWindow The statusbar, where information about editing
     *                         and validation will be sent to.
     */
    public UserDataEditTable(UserDataEditTableModel model, StatusPanel editOutputWindow){
        this(model);
        
        ((UserDataEditTableEditor)
                (this.getColumnModel().getColumn(2).getCellEditor())).setStatusListener(editOutputWindow);
    }
    
    public UserDataEditTable(UserDataEditTableModel model, StatusPanel editOutputWindow, String struct){
        this(model, struct);

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
     */
    public UserDataEditTable(UserDataEditTableModel model){
        super(model);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.setCellSelectionEnabled(false);
       
        this.setSurrendersFocusOnKeystroke(true);
        UserDataEditTableEditor editor = new UserDataEditTableEditor(model.getDataType(), this);
        this.getColumnModel().getColumn(2).setCellEditor(editor);
        model.setEditor(editor);
    }
    
    public UserDataEditTable(UserDataEditTableModel model, String struct){
        super(model, struct);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.setCellSelectionEnabled(false);
       
        this.setSurrendersFocusOnKeystroke(true);
        UserDataEditTableEditor editor = new UserDataEditTableEditor(model.getDataType(), this);
        this.getColumnModel().getColumn(2).setCellEditor(editor);
        model.setEditor(editor);
    }
}
