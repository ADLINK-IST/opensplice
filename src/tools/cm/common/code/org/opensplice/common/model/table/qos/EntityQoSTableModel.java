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
 * Contains SPLICE DDS C&M Tooling table model components that are able to hold
 * and resolve QoS information of Entity objects. 
 */
package org.opensplice.common.model.table.qos;

import java.util.HashSet;

import javax.swing.AbstractCellEditor;
import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.qos.QoS;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.QoSTableEditor;

/**
 * Component that holds the QoS information of a specific Entity. This class
 * has been defined abstract because only descendants of this class can 
 * actually exist.
 * 
 * @date Jan 10, 2005 
 */
public abstract class EntityQoSTableModel extends DefaultTableModel {

    private static final long    serialVersionUID    = -251046106776534295L;

    /**
     * The Entity where to retrieve and hold the QoS of. 
     */
    protected Entity entity;
    
    protected QoS currentQos;
    
    protected AbstractCellEditor editor = null;
    
    protected HashSet<Integer>   nonEditRows         = null;
    protected HashSet<Integer>   nonEditRowsNoEntity = null;

    /**
     * Constructs a new table model that is capable of retrieving and
     * administrating the QoS of the supplied Entity.
     * 
     * @param _entity The Entity, which QoS must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public EntityQoSTableModel(Entity _entity) throws CommonException{
        super();
        entity = _entity;
        currentQos = null;
        this.addColumn("Name");
        this.addColumn("Field");
        this.addColumn("Value");
        nonEditRows = new HashSet<Integer>();
        nonEditRowsNoEntity = new HashSet<Integer>();
        this.init();
        if(!this.update()){
            throw new CommonException("Entity not available anymore");
        }
    }
    
    EntityQoSTableModel(QoS currentQos) {
        super();
        this.entity = null;
        this.currentQos = currentQos;
        this.addColumn("Name");
        this.addColumn("Field");
        this.addColumn("Value");
        nonEditRows = new HashSet<Integer>();
        nonEditRowsNoEntity = new HashSet<Integer>();
        this.init();
    }

    EntityQoSTableModel(QoS currentQos, boolean editable) {
        super();
        this.entity = null;
        this.currentQos = currentQos;
        this.addColumn("");
        this.addColumn("Name");
        this.addColumn("Field");
        this.addColumn("Value");
        nonEditRows = new HashSet<Integer>();
        nonEditRowsNoEntity = new HashSet<Integer>();
        this.init();
    }
    
    /**
     * Initializes the table model.
     *
     */
    protected abstract void init();
    
    /**
     * Updates the QoS information in the model by retrieving the QoS
     * of the Entity and setting this in the table model.
     * 
     * @return true if succeeded, false otherwise.
     */
    public abstract boolean update();
    
    protected void cancelEditing(){
        if(editor != null){
            editor.cancelCellEditing();
           
        }
    }
    
    public QoS getQoS(){
        return currentQos;
    }
    
    public void setQoS(QoS qos){
        currentQos = qos;
    }
    
    public void applyQoS() throws CommonException{
        try {
            if(editor != null){
                if(editor instanceof QoSTableEditor){
                    if(((QoSTableEditor)editor).isEditing()){
                        boolean valid = ((QoSTableEditor)editor).testAssignment().isValid();
                        
                        if(valid){
                            editor.stopCellEditing();
                        } else {
                            throw new CommonException("Current input not valid.");
                        }
                    }
                } else {
                    editor.stopCellEditing();
                }
            }
            if(entity != null){
                entity.setQoS(currentQos);
            }
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
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
        boolean result = false;
        int colValue = this.getColumnCount() - 1;

        if (this.getColumnCount() > 3 && col == 0) {
            result = true;
        } else if (this.getColumnCount() > 3 && (Boolean) this.getValueAt(row, 0)) {
            result = false;
        } else {
            if (col == colValue) {
                Integer it = new Integer(row);
                if (entity == null && nonEditRowsNoEntity.contains(it)) {
                    result = false;
                } else if (entity == null) {
                    result = true;
                } else if (!(entity.isEnabled())) {
                    result = true;
                } else {
                    if (nonEditRows.contains(it)) {
                        result = false;
                    } else {
                        result = true;
                    }
                }
            }
        }
        return result;
    }
    
    public void setEditor(AbstractCellEditor editor){
        this.editor = editor;
    }

    public HashSet<Integer> getNonEditRowsNoEntity() {
        return nonEditRowsNoEntity;
    }

    public void setNonEditRowsNoEntity(HashSet<Integer> nonEditRowsNoEntity) {
        this.nonEditRowsNoEntity = nonEditRowsNoEntity;
    }
}
