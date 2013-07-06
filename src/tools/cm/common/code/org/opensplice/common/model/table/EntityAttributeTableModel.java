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
 * Contains all common SPLICE DDS C&M Tooling model components that are 
 * administrated in table model components.
 */
package org.opensplice.common.model.table;

import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.*;
import org.opensplice.common.util.Config;
import org.opensplice.cm.data.State;
/**
 * Descendant of DefaultTableModel that is capable of resolving and holding
 * attribute information and values of Entity objects. Its responsibility is
 * to resolve, hold and update the value of the attributes of the attached
 * Entity.
 * 
 * @date Oct 1, 2004
 */
public class EntityAttributeTableModel extends DefaultTableModel{
    /**
     * The entity that is attached to this model.
     */
    private Entity entity;
    private int serviceStateRow;
    private int waitsetEventMaskRow;
    private boolean showInternals;
    
    /**
     * Creates a new model that holds the attributes and their values of the 
     * supplied entity.
     * 
     * @param _entity The Entity where to resolve the attribute values of.
     */
    public EntityAttributeTableModel(Entity _entity){
        super();
        entity = _entity;
        this.addColumn("Field name");
        this.addColumn("Field value");
        this.initialize();
    }
    
    /** 
     * Updates the values of the attributes in the model. The update fails
     * when the entity is not available anymore.
     * 
     * @return Whether or not the update succeeded. true if succeeded, false
     *         otherwise.
     */
    public boolean update(){
        if(entity.isFreed()){
            return false;
        }
        if(entity instanceof Service){
            ServiceState state = null;
            
            try {
                state = ((Service)entity).getState();
            } catch (CMException e) {
                entity.free();
                return false;
            }
            this.setValueAt(ServiceStateKind.getString(
                                state.getServiceStateKind()), serviceStateRow, 1);
        } else if(entity instanceof Waitset){
            int mask;
            Event event;
            
            if(showInternals){
                try {
                    mask = ((Waitset)entity).getEventMask();
                    event = new Event(mask);
                } catch (CMException e) {
                    entity.free();
                    return false;
                }
                this.setValueAt(event.toString(), waitsetEventMaskRow, 1);
            }
        }
        return true;
    }
    
    /**
     * Initializes the model by resolving the attributes and their values.
     */
    protected void initialize(){
        Config config = Config.getInstance();
        serviceStateRow = 0;
        
        this.addKindRow();
        this.addNameRow();
        serviceStateRow += 3;
        
        showInternals = new Boolean(config.getProperty("display_internals")).booleanValue();
        
        if(showInternals){
            this.addIndexRow();
            this.addSerialRow();
            this.addPointerRow();
            
            serviceStateRow += 3;
        }
        waitsetEventMaskRow = serviceStateRow;
        this.addEnabledRow();
        
        Object[] content = new Object[2];

        if(entity instanceof Topic){
            content[0] = "key list";
            content[1] = ((Topic)entity).getKeyList();
            this.addRow(content);
            
            content[0] = "type name";
            content[1] = ((Topic)entity).getTypeName();
            this.addRow(content);
        } else if(entity instanceof Query){
            content[0] = "expression";
            String expression = ((Query)entity).getExpression();
            
            if(expression == null){
                content[1] = "";
            } else {
                content[1] = expression;
            }
            this.addRow(content);
            
            content[0] = "parameters";
            String params = ((Query)entity).getExpressionParams();
            
            if(params == null){
                content[1] = "";
            } else {
                content[1] = params;
            }
            this.addRow(content);
            
            State state = ((Query)entity).getInstanceState();
            content[0] = "instance_state";
            content[1] = getInstanceState(state);
            this.addRow(content);
            
            state = ((Query)entity).getSampleState();
            content[0] = "sample_state";
            content[1] = getSampleState(state);
            this.addRow(content);
            
            state = ((Query)entity).getViewState();
            content[0] = "view_state";
            content[1] = getViewState(state);
            this.addRow(content);
        }  else if(entity instanceof Service){
            ServiceState state = null;
            
            try {
                state = ((Service)entity).getState();
                
                content[0] = "state name";
                content[1] = state.getName();
                this.addRow(content);
                serviceStateRow++;
                
                content[0] = "state kind";
                content[1] = ServiceStateKind.getString(state.getServiceStateKind());
                this.addRow(content);
            } catch (CMException e) {
                content[0] = "state name";
                content[1] = "N/A";
                this.addRow(content);
                
                content[0] = "state kind";
                content[1] = "N/A";
                this.addRow(content);
            }
        } else if(entity instanceof Waitset){
            if(showInternals){
                try {
                    int mask = ((Waitset)entity).getEventMask();
                    content[0] = "event mask";
                    content[1] = new Event(mask).toString();
                    this.addRow(content);
                } catch (CMException e) {
                    content[0] = "event mask";
                    content[1] = "N/A";
                    this.addRow(content);
                }
            }
        }
    }
       
    private void addKindRow(){
        String kind = entity.getClass().getName();
        String pckg = entity.getClass().getPackage().getName();
        
        if(kind.endsWith("Impl")){
            kind = kind.substring(pckg.length()+1, kind.length()-4);
        } else {
            kind = kind.substring(pckg.length()+1);
        }
        
        Object[] content = new Object[2];
        content[0] = "kind";
        content[1] = kind;
        
        this.addRow(content);
    }
    
    private void addNameRow(){
        Object[] content = new Object[2];
        content[0] = "name";
        content[1] = entity.getName();
        
        this.addRow(content);
    }
    
    private void addIndexRow(){
        Object[] content = new Object[2];
        content[0] = "handle index";
        content[1] = Long.toString(entity.getIndex());
        
        this.addRow(content);
    }
    
    private void addSerialRow(){
        Object[] content = new Object[2];
        content[0] = "handle serial";
        content[1] = Long.toString(entity.getSerial());
        
        this.addRow(content);
    }
    
    private void addPointerRow(){
        Object[] content = new Object[2];
        content[0] = "address";
        content[1] = "0x" + entity.getPointer();
        
        this.addRow(content);
    }
    
    private void addEnabledRow(){
        Object[] content = new Object[2];
        content[0] = "enabled";
        content[1] = Boolean.toString(entity.isEnabled());
        
        this.addRow(content);
    }
    
    /**
     * Makes sure the table model cannot be edited by the user.
     * 
     * @param row The row that the user wants to edit.
     * @param  column The column that the user wants to edit.
     * @return Always returns false.
     */
    public boolean isCellEditable(int row, int column){
        return false;
    }
    
    protected String getSampleState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            
            if(value == 0){
                result = "ANY";
            } else {   
                if((value & (State.READ)) == State.READ){
                    result = "READ";
                } else {
                    result = "NOT_READ";
                }
            }
        } else {
            result = "N/A";
        }
        return result;
    }
    
    protected String getViewState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            
            if(value == 0){
                result = "ANY";
            } else {
                if((value & (State.NEW)) == State.NEW){
                    result = "NEW";
                } else {
                    result = "NOT_NEW";
                }
            }
        } else {
            result = "N/A";
        }
        return result;
    }
    
    protected String getInstanceState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            if(value == 0){
                result = "ANY";
            } else {
                if( ((value & (State.DISPOSED)) == State.DISPOSED) &&
                    ((value & (State.NOWRITERS)) == State.NOWRITERS)) 
                {
                    result = "NOT_ALIVE";
                } else if( ((value & (State.DISPOSED)) == State.DISPOSED) &&
                           ((value & (State.WRITE)) == State.WRITE)) 
                {
                    result = "ALIVE | NOT_ALIVE_DISPOSED";
                } else if( ((value & (State.NOWRITERS)) == State.NOWRITERS) &&
                           ((value & (State.WRITE)) == State.WRITE)) 
                {
                    result = "ALIVE | NOT_ALIVE_NO_WRITERS";
                } else if((value & (State.DISPOSED)) == State.DISPOSED){
                    result = "NOT_ALIVE_DISPOSED";
                } else if((value & (State.NOWRITERS)) == State.NOWRITERS){
                    result = "NOT_ALIVE_NO_WRITERS";
                } else {
                    result = "ALIVE";
                }
            }
        } else {
            result = "N/A";
        }
        return result;
    }
}
