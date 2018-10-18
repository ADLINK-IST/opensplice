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
package org.opensplice.common.view.table;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import javax.swing.JTable;

import org.opensplice.cmdataadapter.protobuf.ProtobufFieldProperties;
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
     */
    public UserDataSingleTable(UserDataSingleTableModel model){
        super(model);
    }
    
    public UserDataSingleTable(UserDataSingleTableModel model, String struct){
        super(model);
        structName = struct;
    }
    
    /**
     * Sets the key list of the data in this table. Backgrounds of cells that 
     * hold these fields will be colored as defined by the 
     * UserDataSingleTableCellRenderer.
     * 
     * @param keyList The list of keys of the contained data.
     * @param protoProps The Map of the data type's field names to their corresponding ProtobufFieldProperties.
     */
    public void setTableHighlightFields(String keyList, Map<String, ProtobufFieldProperties> protoProps){
        String[] keys;
        List<String> requiredFields = new ArrayList<String>();
        for (Entry<String, ProtobufFieldProperties> propsEntry : protoProps.entrySet()) {
            if (propsEntry.getValue().isRequired()) {
                requiredFields.add(propsEntry.getKey());
            }
        }
        if(keyList != null){
            userDataKeys = keyList;
            keys = keyList.split(",");
            UserDataSingleTableCellRenderer renderer =
                    new UserDataSingleTableCellRenderer(keys, requiredFields.toArray(new String[0]));
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
