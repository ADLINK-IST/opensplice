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
/**
 * Contains SPLICE DDS C&M Tooling table model components that are able to hold
 * and resolve status information of Entity objects. 
 */
package org.opensplice.common.model.table.status;

import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.Entity;
import org.opensplice.cm.status.Status;
import org.opensplice.common.CommonException;

/**
 * Component that holds the status information of a specific Entity. This class
 * has been defined abstract because only descendants of this class can 
 * actually exist.
 * 
 * @date Oct 13, 2004 
 */
public abstract class EntityStatusTableModel extends DefaultTableModel{
    /**
     * The Entity where to retrieve and hold the Status of. 
     */
    protected Entity entity;
    
    /**
     * Constructs a new table model that is capable of retrieving and
     * administrating the Status of the supplied Entity.
     * 
     * @param _entity The Entity, which Status must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public EntityStatusTableModel(Entity _entity) throws CommonException{
        super();
        entity = _entity;
        
        this.addColumn("Name");
        this.addColumn("Field");
        this.addColumn("Value");
        
        Object[] data = new Object[3];
        data[2] = "N/A";
        data[0] = "STATE";
        data[1] = "";
        this.addRow(data);
        
        this.init();
        
        if(!this.update()){
            throw new CommonException("Entity not available (anymore).");
        }
    }
    
    /**
     * Initializes the table model.
     *
     */
    protected abstract void init();
    
    /**
     * Updates the Status information in the model by retrieving the Status
     * of the Entity and setting this in the table model.
     * 
     * @return true if succeeded, false otherwise.
     */
    public abstract boolean update();
    
    /**
     * Updates the state field of the Status.
     * 
     * @param s The Status where the state is located in.
     */
    public void updateState(Status s){
        this.setValueAt(s.getState(), 0, 2);
    }
    
    /**
     * Overrides parent class to make sure the user cannot edit the data.
     * 
     * @param row The row to edit.
     * @param col The column to edit.
     * @return Always false.
     */
    @Override
    public boolean isCellEditable(int row, int col){
        return false;
    }
}
