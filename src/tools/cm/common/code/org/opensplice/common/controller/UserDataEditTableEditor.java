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
package org.opensplice.common.controller;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.EventObject;
import java.util.StringTokenizer;

import javax.swing.AbstractCellEditor;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.table.TableCellEditor;

import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.common.model.table.UserDataEditTableModel;
import org.opensplice.common.util.Config;
import org.opensplice.common.view.StatusPanel;
import org.opensplice.common.view.table.UserDataEditTable;


/**
 * Represents an editor that handles user editing in an UserDataEditTable. It
 * validates input and assigns valid values to the fields in the data.
 * 
 * @date Nov 9, 2004 
 */
public class UserDataEditTableEditor extends AbstractCellEditor implements TableCellEditor, ActionListener, KeyListener {

    private static final long      serialVersionUID = 8681953476555069267L;
    private Object curValue = null;
    private MetaUnion curUnion = null;
    private MetaType type = null;
    private UserDataEditTable view = null;
    private UserDataEditTableModel model = null;
    private int editRow, editColumn;
    private StatusPanel status = null;
    private Component curEditor = null;
    private final Color editColor = Config.getInputColor();
    private final Color errorColor = Config.getIncorrectColor();
    
    /**
     * Constructs a new editor for that is able to edit UserData of the supplied
     * type.
     *  
     * @param _type The type of the UserData that must be edited.
     * @param _view The table that displays the graphical representation of the
     *              data.
     */
    public UserDataEditTableEditor(MetaType _type, UserDataEditTable _view){
        super();
        type = _type;
        view = _view;
        model = (UserDataEditTableModel)view.getModel();
    }

    @Override
    public boolean isCellEditable(EventObject e) {
        JFrame frame = (JFrame) SwingUtilities.getRoot(status);
        frame.firePropertyChange("enableSaveButton", 0, 1);
        return true;
    }

    /**
     * Constructs and returns the editor component for the supplied cell in the
     * table. The editor type depends on the type of the data in the cell:
     * - Primitive (except boolean): JTextField
     * - Boolean: JComboBox
     * - Enumeration: JComboBox
     * 
     * @param table The table in which the user wants to edit.
     * @param value The current value in the cell.
     * @param isSelected Whether or not the cell is currently selected.
     * @param row The row in the table the user wants to edit.
     * @param column The column in the table the user wants to edit.
     * 
     * @return The editor for the supplied cell.
     */
    @Override
    public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
        MetaField potential;
        editRow = row;
        editColumn = column;
        Component result = null;
        curValue = value;
        curUnion = null;
        String name = (String) (model.getValueAt(row, column - 1));

        if (result == null) {
            MetaField editField = type.getField(name);
            
            if(editField instanceof MetaEnum){
                if(name.endsWith(".switch")){
                    potential = type.getField(name.substring(0, name.length()-7));
                    
                    if(potential instanceof MetaUnion){
                        curUnion = (MetaUnion)potential;
                    }
                }
                result = this.getEnumEditor(row, column, value, (MetaEnum)editField);
            } else if (editField instanceof MetaPrimitive) {
                if(name.endsWith(".switch")){
                    potential = type.getField(name.substring(0, name.length()-7));
                    
                    if(potential instanceof MetaUnion){
                        curUnion = (MetaUnion)potential;
                    }
                }
                result = this.getPrimitiveEditor(row, column, value);
            } else if (editField instanceof MetaStruct) {
                result = this.getPrimitiveEditor(row, column, value);
            } else if (editField == null) {
                result = this.getPrimitiveEditor(row, column, value);
            } else if (editField instanceof MetaCollection) {
                result = this.getCollectionEditor(row, column, value, (MetaCollection)editField, name);
            } else {
            }
        }
        curEditor = result;
        curEditor.addKeyListener(this);
        return result;
    }

    /**
     * Provides access to the current value of the cell that is being edited.
     * 
     * @return The current value of the field that is being edited.
     */
    @Override
    public Object getCellEditorValue() {
        return curValue;
    }
    
    /**
     * Stores the supplied object that will receive validation and assigment
     * information from now.
     * 
     * @param _status The component that will receive the information.
     */
    public void setStatusListener(StatusPanel  _status){
        status = _status;
    }
    
    /**
     * Constructs an editor for a primitive field.
     * 
     * In case of a boolean a JComboBox with the possible values "TRUE" and "FALSE"
     * will be returned. In all other cases a JTextField will be returned.
     * 
     * @param row The row that is being edited.
     * @param column The column that is being edited.
     * @param value The current value of the field that is being edited.
     * @return The editor that is associated with the type of the data that is
     * being edited.
     */
    private Component getPrimitiveEditor(int row, int column, Object value){
        Component temp;
        String primType = (String)model.getValueAt(row, 0);
        
        if("c_bool".equals(primType)){
            String[] items = {"FALSE", "TRUE"};
            temp = new JComboBox(items);
            ((JComboBox)temp).setSelectedItem(value);
            ((JComboBox)temp).addActionListener(this);
        }
        else{
            temp = new JTextField(20);
            ((JTextField)temp).setText((String)(value));
            ((JTextField)temp).addActionListener(this);
        }
        temp.setBackground(editColor);
        return temp;
    }
    
    /**
     * Constructs an editor for a enumeration field.
     * 
     * A JComboBox filled with all possible values will be returned.
     * 
     * @param row The row that is being edited.
     * @param column The column that is being edited.
     * @param value The current value of the field.
     * @param editField The enumeration field that is being edited.
     * @return The editor that is associated with the type of the data that is being
     *         edited.
     */
    private Component getEnumEditor(int row, int column, Object value, MetaEnum editField){
        JComboBox temp = new JComboBox(editField.getPosValues());
        temp.setSelectedItem(value);
        temp.setBackground(editColor);
        temp.addActionListener(this);
        return temp;
    }
    
    /**
     * Constructs an editor for a collection field.
     * 
     * This is done by looking up the editor for the subtype of the collection.
     * 
     * @param row The row that is being edited.
     * @param column The column that is being edited.
     * @param value The current value of the field.
     * @param editField The field that is being edited.
     * @return The editor that is associated with the type of the data that is being
     *         edited.
     */
    private Component getCollectionEditor(int row, int column, Object value, MetaCollection editField, String nameRest){
        String name;
        MetaField subType = editField.getSubType();
        Component temp = null;
        
        while(subType instanceof MetaCollection){
            subType = ((MetaCollection)subType).getSubType();
        }
        
        if(subType instanceof MetaEnum){
            temp = this.getEnumEditor(row, column, value, (MetaEnum)subType);
        } else if(subType instanceof MetaStruct){
            temp = this.getStructEditor(row, column, value, (MetaStruct)subType, nameRest);
        } else {//primitive
            if(subType instanceof MetaUnion){
                name = (String)model.getValueAt(row, column-1);
                
                if(name.endsWith(".switch")){
                    curUnion = (MetaUnion)subType;
                }
            }
            temp = this.getPrimitiveEditor(row, column, value);
        }
        temp.setBackground(editColor);
        return temp;
    }
    
    private Component getStructEditor(int row, int column, Object value, MetaStruct editField, String nameRest){
        String name, nextName;
        MetaField structField;
        Component component;
        
        int index = nameRest.indexOf('[');
        
        if(index == -1){
            index = nameRest.indexOf('.');
            
            if(index == -1){
                name = nameRest;
            } else {
                name = nameRest.substring(0, index);
            }
        } else {
            name = nameRest.substring(0, index);
        }
        
        
        structField = editField.getField(name);
        
        if(structField instanceof MetaStruct){
            nextName = nameRest.substring(index+1);
            component = this.getStructEditor(row, column, value, (MetaStruct)structField, nextName);
        } else if(structField instanceof MetaEnum){
            component = this.getEnumEditor(row, column, value, (MetaEnum)structField);
        } else if(structField instanceof MetaCollection){
            component = this.getCollectionEditor(row, column, value, (MetaCollection)structField, name);
        } else {
            component = this.getPrimitiveEditor(row, column, value);
        }
        return component;
    }
    
    /**
     * Called when a user confirms its input.
     * 
     * This function validates the input of the user. If it succeeds the value
     * is assigned to the field, the user will be notified of its error.
     * 
     * @param e The event that occurred.
     */
    @Override
    public void actionPerformed(ActionEvent e){
        if(e.getSource().equals(curEditor)){
           this.assign();
        }
    }
    
    
    /**
     * Called when editing has been cancelled.
     */
    @Override
    public void cancelCellEditing(){
        super.cancelCellEditing();
        
        if(status != null){
            status.setStatus("Editing cancelled", false, false);
        }
    }
    
    /**
     * Called when editing has been stopped.
     */
    @Override
    public boolean stopCellEditing(){
        boolean result = true;
        
        if(curEditor != null){
            result = this.assign().isValid();
        }
        if(result){
            result = super.stopCellEditing();
        }
        return result;
    }
    
    private AssignmentResult assign(){
        AssignmentResult test = this.testAssignment(true);
        

        if(test.isValid()){
            if(curEditor instanceof JComboBox){
                JComboBox source = (JComboBox)curEditor;
                curValue = source.getSelectedItem();
            } else if(curEditor instanceof JTextField){
                JTextField source = (JTextField)curEditor;
                curValue = source.getText();
            }
            if(status != null){
                status.setStatus("Input valid.", false, false);
            }
            model.setUserDataField( (String)model.getValueAt(editRow, editColumn-1), (String)curValue);
            curEditor.removeKeyListener(this);
            curEditor = null;
            fireEditingStopped();
        } else if(status != null){
            status.setStatus("Error: Invalid input " + test.getErrorMessage().toLowerCase(), false, false);
        }
        return test;
    }
    
    /**
     * Checks whether the current value in the editor is valid and will be
     * accepted as final input.
     * 
     * @return The AssigmentResult that tells whether the input is valid and
     *         if not; the reason why. 
     */
    public AssignmentResult testAssignment(boolean updateSource){
        AssignmentResult result = new AssignmentResult(true, null);
        
        if(curEditor != null){
            if(curEditor instanceof JTextField){
                JTextField source = (JTextField)curEditor;
                
                String typeName = (String)(model.getValueAt(editRow, editColumn-2));
                String typeNameTmp = typeName;

                if (typeName.startsWith("C_SEQUENCE<")) {
                    int ending = typeName.indexOf(">");
                    typeNameTmp = typeName.substring(11, ending);
                } else if (typeName.startsWith("C_ARRAY<")) {
                    int ending = typeName.indexOf(">");
                    typeNameTmp = typeName.substring(8, ending);
                }
                typeName = "INVALID";

                String text = source.getText();
                
                try{
                    if("c_voidp".equals(typeName)){
                        source.setText((String)curValue);
                        if(status != null){
                            status.setStatus("Warning: Void pointers cannot be changed", false, false);
                        }
                    } 
                    else if(typeName.startsWith("C_SEQUENCE") || typeName.startsWith("C_ARRAY<")){
                        String fieldName = (String)(model.getValueAt(editRow, editColumn-1));
                        MetaCollection mc = (MetaCollection)(type.getField(fieldName));
                        
                        if(mc == null){
                            mc = (MetaCollection)(type.getField(this.getTypeNameForField(fieldName)));
                        }
                        
                        if(mc != null){
                            if(mc.getMaxSize() == 0){
                                MetaField subType = mc.getSubType();
                                String subTypeName = subType.getTypeName();
                                
                                if(text.equals("[]")){
                                    /*Ok, do nothing*/
                                } else if(text.equals("NULL")){
                                    /*Ok, do nothing*/
                                } else if(  (subType instanceof MetaPrimitive) ||
                                            ((subType instanceof MetaCollection) &&
                                            ((subTypeName.equals("c_string")) ||
                                            (subTypeName.equals("c_wstring")) ||
                                            (subTypeName.startsWith("C_STRING<")) ||
                                            (subTypeName.startsWith("C_WSTRING<")))))
                                {
                                    
                                    if(text.equals("[]")){
                                        /* Ok, do nothing*/
                                    } else if(text.length() <= 2){
                                        throw new NumberFormatException("Input not valid.");
                                    } else if(text.endsWith(",]")){
                                        throw new NumberFormatException("Input not valid.");
                                    } else if(text.startsWith("[") && text.endsWith("]")){
                                        String token;
                                        StringTokenizer strTok = new StringTokenizer(text.substring(1, text.length()-1), ",");
                                        while(strTok.hasMoreTokens()){
                                            token = strTok.nextToken();
                                            this.handlePrimitive(token, subTypeName);
                                        }
                                    } else {
                                        throw new NumberFormatException("Invalid value");
                                    }
                                    
                                } else {
                                    source.setText((String)curValue);
                                    
                                    if(status != null){
                                        status.setStatus("Warning: Unbounded sequences of this type not supported. They " +
                                                         "will be interpreted as an empty sequence.", false, false);
                                    }
                                }
                            } else {
                                if(status != null){
                                    status.setStatus("Warning: Field is a recursive type, which cannot be modified here. The first " +
                                                     "recursion will be interpreted as a NULL pointer.", false, false);
                                }
                                source.setText((String)curValue);
                            }
                        } else {
                            if(status != null){
                                status.setStatus("Error: Could not resolve type of field '" + fieldName + "'.", false, false);
                            }
                        }
                        
                    }
                    else if(typeName.startsWith("C_ARRAY<")){
                        String fieldName = (String)(model.getValueAt(editRow, editColumn-1));
                        MetaCollection mc = (MetaCollection)(type.getField(fieldName));
                        
                        if(mc.getMaxSize() == 0){
                            MetaField subType = mc.getSubType();
                            
                            if(subType instanceof MetaPrimitive){
                                
                            } else if(subType instanceof MetaCollection) {
                                String subTypeName = subType.getTypeName();
                                
                                if( (subTypeName.equals("c_string")) ||
                                    (subTypeName.equals("c_wstring")))
                                    
                                {
                                    
                                } else if(  (subTypeName.startsWith("C_STRING<")) ||
                                            (subTypeName.startsWith("C_WSTRING<")))
                                {
                                    
                                } else {
                                    if(status != null){
                                        status.setStatus("Warning: Unbounded arrays not supported. They " +
                                                         "will be interpreted as an empty array.", false, false);
                                    }
                                    source.setText((String)curValue);
                                }
                            }
                            
                        } else {
                            if(status != null){
                                status.setStatus("Warning: Recursive types not supported. The " +
                                                 "first occurrence will be interpreted as a NULL pointer.", 
                                                 false, false);
                            }
                            source.setText((String)curValue);
                        }
                    } else if (typeName.equals("INVALID")) {
                        String value = this.handlePrimitive(text, typeNameTmp);
                        if(updateSource){
                            source.setText(value);
                        }
                    } else {
                        String value = this.handlePrimitive(text, typeName);
                        
                        if(updateSource){
                            source.setText(value);
                        }
                    }
                    result = new AssignmentResult(true, null);
                }
                catch(NumberFormatException ne){
                    result = new AssignmentResult(false, "Parsing failed " + (ne.getMessage()).toLowerCase());
                }
            }
            else if(curEditor instanceof JComboBox){
                if(curUnion != null){
                    if(!curUnion.labelExists(((JComboBox)curEditor).getSelectedItem().toString())){
                        result = new AssignmentResult(false, "Invalid union switch value");
                        curEditor.setBackground(errorColor);
                    } else {
                        result = new AssignmentResult(true, null);
                    }
                } else {
                    result = new AssignmentResult(true, null);
                }
            }
        }
        return result;
    }
    
    private String getTypeNameForField(String fieldName) {
        String result = null;
        String token;
        int index;
        StringTokenizer tokenizer = new StringTokenizer(fieldName, ".");
        
        while(tokenizer.hasMoreTokens()){
            token = tokenizer.nextToken();
            index = token.indexOf('[');
            
            if(index == -1){
                if(result == null){
                    result = token;
                } else {
                    result += "." + token;
                }
            } else {
                if(result == null){
                    result = token.substring(0, index);
                } else {
                    result += "." + token.substring(0, index);
                }
            }
        }
        return result;
    }

    private String handlePrimitive(String text, String typeName) throws NumberFormatException{
        String result;
        
        if("c_char".equals(typeName)){//TODO: Java char is superset of IDL char
            if(text.length() != 1){
                throw new NumberFormatException("Length of char must be 1.");
            }
            result = text;
        }
        else if("c_octet".equals(typeName)){
            byte test = Byte.parseByte(text);
            result = Byte.toString(test);
        }
        else if("c_short".equals(typeName)){
            short test = Short.parseShort(text);
            result = Short.toString(test);
        }
        else if("c_ushort".equals(typeName)){
            if(text.startsWith("-")){
                throw new NumberFormatException("Unsigned short cannot be negative.");
            }
            int test = Integer.parseInt(text);
            result = Integer.toString(test);
        }
        else if("c_long".equals(typeName)){
            int test = Integer.parseInt(text);
            result = Integer.toString(test);
        }
        else if("c_ulong".equals(typeName)){
            if(text.startsWith("-")){
                throw new NumberFormatException("Unsigned long cannot be negative.");
            }
            long test = Long.parseLong(text);
            
            if(test > 4294967295L){
                throw new NumberFormatException("Unsigned long max == 4294967295.");
            }
            result = Long.toString(test);
        }
        else if("c_longlong".equals(typeName)){
            long test = Long.parseLong(text);
            result = Long.toString(test);
        }
        else if("c_ulonglong".equals(typeName)){//TODO: unsigned long long not available in Java
            char temp;
            if(text.length() > 20){
                throw new NumberFormatException("Invalid unsigned long long.");
            }
            else{
                for(int i=0; i<text.length(); i++){
                    temp = text.charAt(i);
                    if(temp < '0' || temp > '9'){
                        throw new NumberFormatException("Invalid unsigned long long.");
                    }
                }
                long test = Long.parseLong(text);
                result = Long.toString(test);
            }
        }
        else if("c_float".equals(typeName)){
            float test = Float.parseFloat(text);
            result = Float.toString(test);
        } 
        else if("c_double".equals(typeName)){
            double test = Double.parseDouble(text);
            result = Double.toString(test);
        } 
        else if(    (typeName.startsWith("C_STRING<")) ||
                    (typeName.startsWith("C_WSTRING<")))
        {
            String temp = typeName.substring(typeName.indexOf('<') + 1, typeName.indexOf('>'));
            int size  = Integer.parseInt(temp);
            
            if(text.length() > size){
                throw new NumberFormatException("length " + text.length() + " > " + size);
            }
            if(text.indexOf("]]>") != -1){
                throw new NumberFormatException("String cannot contain ']]>'");
            }
            result = text;
        } 
        else if("c_string".equals(typeName)){
            if(text.indexOf("]]>") != -1){
                throw new NumberFormatException("String cannot contain ']]>'");
            }
            result = text;
        } else {
            result = text;
        }
        
        if(curUnion != null){
            if(!curUnion.labelExists(text)){
                throw new NumberFormatException("Invalid union switch value");
            }
        }
        
        return result;
    }

    /**
     * Called when the user releases a key on the keyboard. When currently 
     * editing a field, the input is validated. When a status listener is
     * attached status information is provided to that listener.
     */
    @Override
    public void keyReleased(KeyEvent e) {
        if(curEditor != null){
            if(e.getSource() instanceof JTextField){
                AssignmentResult test = this.testAssignment(false);
                
                if(test.isValid()){
                    curEditor.setBackground(editColor);
                } else {
                    curEditor.setBackground(errorColor);
                    
                    if(status != null){
                        status.setStatus(test.getErrorMessage(), false, false);
                    }
                }
            } else if(e.getSource() instanceof JComboBox){
                if(curUnion != null){
                    if(!curUnion.labelExists((String)((JComboBox)e.getSource()).getSelectedItem())){
                        curEditor.setBackground(errorColor);
                        
                        if(status != null){
                            status.setStatus("Error: Invalid union switch value", false, false);
                        }
                    } else {
                        stopCellEditing();
                    }
                } else {
                    stopCellEditing();
                }
            }
        }       
    }

    /**
     * Does nothing.
     */
    @Override
    public void keyTyped(KeyEvent e) {}
    
    /**
     * Does nothing.
     */
    @Override
    public void keyPressed(KeyEvent e) {}
}
