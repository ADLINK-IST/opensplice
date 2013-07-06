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
package org.opensplice.config.swing;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import javax.swing.AbstractCellEditor;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.table.TableCellEditor;

import org.opensplice.common.controller.AssignmentResult;
import org.opensplice.common.util.Config;
import org.opensplice.common.view.StatusPanel;
import org.opensplice.config.data.DataException;
import org.opensplice.config.data.DataValue;
import org.opensplice.config.meta.MetaValue;
import org.opensplice.config.meta.MetaValueBoolean;
import org.opensplice.config.meta.MetaValueEnum;
import org.opensplice.config.meta.MetaValueNatural;
import org.opensplice.config.meta.MetaValueString;

public class DataElementTableModelEditor extends AbstractCellEditor implements TableCellEditor, ActionListener, KeyListener {
    private static final long serialVersionUID = 805706250918260825L;
    private Object curValue = null;
    private StatusPanel status;
    private final Color editColor = Config.getInputColor();
    private final Color errorColor = Config.getIncorrectColor();
    private int editRow, editColumn;
    private Component curEditor = null;
    private DataElementTableModel tableModel = null;
    private DataValue editNode;
    private MetaValue editType;
    
    public DataElementTableModelEditor(DataElementTableModel tableModel){
        this.tableModel = tableModel;
    }
    
    public void setStatusListener(StatusPanel  status){
        this.status = status;
    }
    
    @Override
    public void cancelCellEditing(){
        super.cancelCellEditing();
        curEditor = null;
        
        if(status != null){
            status.setStatus("Editing cancelled", false, false);
        }
    }
    
    @Override
    public boolean stopCellEditing(){
        boolean result = true;
        
        if(curEditor != null){
            result = this.assign().isValid();
            
            if(result){
                result = super.stopCellEditing();
                curEditor = null;
            } else {
                this.cancelCellEditing();
            }
        }
        return result;
    }
    
    @Override
    public Object getCellEditorValue() {
        return curValue;
    }
    
    public boolean isEditing(){
        assert (editRow != -1) && (editColumn != -1): "Value of editRow || editColumn == null";
        return (curEditor != null);
    }
    
    @Override
    public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
        Component result = null;
        
        editRow    = row;
        editColumn = column;
        curValue   = value;
        editNode   = (DataValue)tableModel.getNodeAt(editRow);
        if (editNode != null) {
            editType   = (MetaValue)editNode.getMetadata();
        }
            
        if(editType instanceof MetaValueBoolean){
            Object[] values = {"true", "false"};
            result = new JComboBox(values);
            ((JComboBox)result).setSelectedItem(curValue);
            ((JComboBox)result).addActionListener(this);
        } else if(editType instanceof MetaValueEnum){
            result = new JComboBox();
            
            for(String posValue: ((MetaValueEnum)editType).getPosValues()){
                ((JComboBox)result).addItem(posValue);
            }
            ((JComboBox)result).setSelectedItem(curValue);
            ((JComboBox)result).addActionListener(this);
        } else {
            result = new JTextField(curValue.toString());
        }
        curEditor = result;
        curEditor.setBackground(editColor);
        curEditor.addKeyListener(this);
    
        return result;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if(e.getSource().equals(curEditor)){
            this.assign();
         }
    }

    @Override
    public void keyReleased(KeyEvent e) {
        if(curEditor != null){
            if(e.getSource() instanceof JTextField){
                AssignmentResult test = this.testAssignment();
                
                if(test.isValid()){
                    curEditor.setBackground(editColor);
                    
                    if(status != null){
                        status.setStatus("Current input valid.", false, false);
                    }
                } else {
                    curEditor.setBackground(errorColor);
                    
                    if(status != null){
                        status.setStatus("Error: " + test.getErrorMessage(), false, false);
                    }
                }
            } else if(e.getSource() instanceof JComboBox){
                /*Do nothing.*/
            }
        }     
    }
    
    @Override
    public void keyTyped(KeyEvent e) {}

    @Override
    public void keyPressed(KeyEvent e) {}
    
    public AssignmentResult testAssignment(){
        AssignmentResult result = new AssignmentResult(true, null);
        String value;
        
        if(curEditor != null){
            try {
                if((editType instanceof MetaValueNatural) || (editType instanceof MetaValueString)) {
                    JTextField source = (JTextField)curEditor;
                    value = source.getText();
                    editNode.testSetValue(value);
                } else {
                    // No validation required.
                }
            } catch(DataException ne){
                result = new AssignmentResult(false, "Invalid input: " + (ne.getMessage()).toLowerCase());
            }
        }
        return result;
    }
    
    private AssignmentResult assign(){
        Object value = null;
        AssignmentResult test = this.testAssignment();
        
        if(test.isValid()){
            if(status != null){
                status.setStatus("Input valid.", false, false);
            }
            
            if(curEditor instanceof JTextField){
                value = ((JTextField)curEditor).getText();
            } else if(curEditor instanceof JComboBox){
                value = ((JComboBox)curEditor).getSelectedItem();
            }
            try {
                if(!editNode.getValue().equals(value)){
                    assert editNode.getOwner() != null: "Owner == null (" + editNode.getMetadata() + ")";
                    editNode.getOwner().setValue(editNode, value);

                }
            } catch (DataException e) {
                assert false: "this.testAssignment does not work properly";
            }
            curEditor.removeKeyListener(this);
            curEditor = null;
            editNode = null;
            editType = null;
            fireEditingStopped();
        } else if(status != null){
            status.setStatus("Error: Invalid input " + test.getErrorMessage().toLowerCase(), false, false);
        }
        return test;
    }
}
