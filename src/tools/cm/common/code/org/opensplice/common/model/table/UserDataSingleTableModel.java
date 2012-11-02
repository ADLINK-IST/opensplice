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

import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.common.CommonException;

/**
 * Table model that holds one instance of UserData. Data can be assigned to
 * the model as well as retrieved.
 * 
 * @date Oct 21, 2004 
 */
public class UserDataSingleTableModel extends DefaultTableModel {
    /**
     * Creates a new table model, which makes it possible to edit
     * values of fields in UserData instances. The UserData type must be the
     * same as the supplied type.  
     *
     * @param _userDataType The type of the UserData that can be edited.
     * @param initFieldValue Whether or not to initialize to values of the 
     *                       fields in the data to a valid value. If not, values
     *                       are not assigned and all cells that display a
     *                       value will display 'N/A'.
     */
    public UserDataSingleTableModel(MetaType _userDataType, boolean initFieldValues) {
        super();
        this.addColumn("Field type");
        this.addColumn("Field name");
        this.addColumn("Field value");
        userDataType = _userDataType;
        ud = new UserData(userDataType);
        this.initModel(initFieldValues);
    }
    
    
    
    /**
     * Called when user wants to edit a field in the table.
     * 
     * Always returns false
     * 
     * @param row The row to edit.
     * @param column The column to edit.
     * @return Always false.
     */
    public boolean isCellEditable(int row, int column){
        return false;
    }

    /**
     * Assigns the supplied value to the supplied field in the UserData.
     * 
     * @param fieldName The name of the field.
     * @param fieldValue The value to assign to the field.
     */ 
    public void setUserDataField(String fieldName, String fieldValue){
        ud.setData(fieldName, fieldValue);
    }
    
    /**
     * Provides access to the UserData, that is currently in the table model.
     * 
     * @return The current UserData.
     */
    public UserData getData() throws CommonException{
        if(ud == null){
            throw new CommonException("Data == null");
        }
        return ud;
    }
    
    /**
     * Sets the supplied data in the model.
     * 
     * @param sample The sample to administrate in this model.
     * @return true if the data != null and the type of the data equals the
     *              type of this model.
     * @todo TODO: Compare data types. 
     */
    public boolean setData(Sample sample){
        UserData data;
        boolean result = false;
        
        if(sample != null){
            //String[] fieldNames = userDataType.getFieldNames();
            data = sample.getMessage().getUserData();
            int rowCount = this.getRowCount();
            
            for(int i=0; i<rowCount; i++){
                String fieldName = (String)this.getValueAt(i, 1);
                String fieldValue = data.getFieldValue(fieldName);
                this.setValueAt(fieldValue, i, 2);
            }
            ud = data;
            result = true;
        }
        return result;
    }
    
    
    /**
     * Provides access to the data type of the data in this table model.
     * 
     * @return The userDataType.
     */
    public MetaType getDataType(){
        return userDataType;
    }
    
    /**
     * Clears the model.
     */
    public void clear(){
        int rowCount = this.getRowCount();
        
        for(int i=0; i<rowCount; i++){
            this.setValueAt("N/A", i, 2);
        }
    }
    
    /**
     * Initializes the model by walking the type and adding all types
     * to its table and setting initial values of the data.
     * 
     * @param validValues whether to set the values of the field to a valid 
     *                    value or not.
     */
    private void initModel(boolean validValues){
        MetaField udt[] = userDataType.getFields();
        MetaField field;
        
        for(int i=0; i<udt.length; i++){
            field = udt[i];
            
            if(field instanceof MetaPrimitive){
                this.addPrimitiveField((MetaPrimitive)field, field.getName(), validValues, null);
            }
            else if(field instanceof MetaCollection){
                this.addCollectionField(null, (MetaCollection)field, field.getName(), validValues);
            }
            else if(field instanceof MetaEnum){
                this.addEnumField((MetaEnum)field, field.getName(), validValues, null);
            }
            else{
                this.addNestedField(field.getName(), field, validValues, null);
            }
        }
    }
    
    /**
     * Adds field to table.
     * 
     * @param name The name of the field.
     * @param field The field to add.
     * @param validValues whether to set the values of the field to a valid 
     *                    value or not.
     */
    private void addNestedField(String name, MetaField field, boolean validValues, MetaField parentField){
        MetaField fields[] = field.getFields();
        MetaField f;
        String nestName;
        
        for(int i=0; i<fields.length; i++){
            f = fields[i];
            nestName = name + "." + f.getName();
            
            if(f instanceof MetaPrimitive){
                this.addPrimitiveField((MetaPrimitive)f, nestName, validValues, field);
            }
            else if(f instanceof MetaCollection){
                this.addCollectionField(field, (MetaCollection)f, nestName, validValues);
            }
            else if(f instanceof MetaEnum){
                this.addEnumField((MetaEnum)f, nestName, validValues, field);
            }
            else{ //CLASS, UNION or STRUCT
                this.addNestedField(nestName, f, validValues, field);
            } 
        }
    }
    
    /**
     * Adds a collection field to the table.
     * 
     * @param colType The collection field.
     * @param prefix The name of the collection field.
     * @param validValues whether to set the values of the field to a valid 
     *                    value or not.
     */
    private void addCollectionField(MetaField parent, MetaCollection colType, String prefix, boolean validValues){
        MetaField subType = colType.getSubType();
        int maxSize = colType.getMaxSize();
        String typeName = colType.getTypeName();
        
        if((maxSize == 0) || (maxSize == -1)){
            Object[] data = new Object[3];
            
            if(colType.getTypeName().equals("c_string")){//string    
                data[0] = colType.getTypeName();
                data[1] = prefix;
                if(validValues){
                    data[2] = "test";
                } else{
                    data[2] = "N/A";
                }
                this.addRow(data);
                ud.setData((String)data[1], (String)data[2]);
            }
            else if(colType.getTypeName().equals("c_wstring")){//wstring    
                data[0] = colType.getTypeName();
                data[1] = prefix;
                if(validValues){
                    data[2] = "test";
                } else{
                    data[2] = "N/A";
                }
                this.addRow(data);
                ud.setData((String)data[1], (String)data[2]);
            }
            else{//Unbounded sequence
                data[0] = colType.getTypeName();
                data[1] = prefix;
                
                if(validValues){
                    if(colType.getMaxSize() == -1){
                        data[2] = "RECURSIVE TYPE";
                    } else if(subType instanceof MetaPrimitive){
                        data[2] = "NULL";
                    } else if(subType instanceof MetaCollection){
                        String subTypeName = subType.getTypeName();
                        
                        if( (subTypeName.startsWith("C_STRING<")) ||
                            (subTypeName.equals("c_string")) ||
                            (subTypeName.equals("c_wstring")) ||
                            (subTypeName.startsWith("C_WSTRING<")))
                        {
                            data[2] = "NULL";
                        } else {
                            data[2] = "UNSUPPORTED";
                        }
                        
                    } else {
                        data[2] = "UNSUPPORTED";
                    }
                } else{
                    data[2] = "N/A";
                }
                this.addRow(data);
            }
        }
        else{
            
            if( (typeName.startsWith("C_STRING<")) ||
                (typeName.startsWith("C_WSTRING<")))
            {
                Object[] data = new Object[3];
                data[0] = colType.getTypeName();
                data[1] = prefix;
                
                if(validValues){
                    data[2] = "";
                    
                } else{
                    data[2] = "N/A";
                }
                this.addRow(data);
                ud.setData((String)data[1], (String)data[2]);                    
            }
            else{
                String tmp;
                int index;
                
                for(int i=0; i<maxSize; i++){
                    index = prefix.lastIndexOf("[");
                    
                    if(index != -1){
                        if(parent instanceof MetaCollection){
                            tmp = prefix.substring(0, index);
                            index = tmp.lastIndexOf("[");
                            
                            while((index != -1)  && (tmp.endsWith("]"))){
                                tmp = tmp.substring(0, index);
                                index = tmp.lastIndexOf("[");
                            }
                            index = tmp.length();
                            tmp += "[" + i + "]";
                            tmp += prefix.substring(index);
                        } else {
                            tmp = prefix + "[" + i + "]";
                        }
                    }
                    else{
                        tmp = prefix + "[" + i + "]";
                    }
                    
                    if(subType instanceof MetaCollection){
                        this.addCollectionField(colType,
                                                (MetaCollection)subType, 
                                                 tmp, validValues);
                    } else if(subType instanceof MetaStruct){
                        this.addNestedField(tmp, subType, validValues, colType);
                    } else if(subType instanceof MetaUnion){
                        this.addNestedField(tmp, subType, validValues, colType);
                    } else if(subType instanceof MetaEnum){
                        this.addEnumField((MetaEnum)subType, tmp, validValues, colType);
                    } else{
                        this.addPrimitiveField((MetaPrimitive)subType, 
                                                tmp, validValues, colType);
                    }
                }
            }
        }
    }
    
    /**
     * Adds a primitive field to the table.
     * 
     * @param primType The primitive field.
     * @param name The name of the field.
     * @param validValues whether to set the values of the field to a valid 
     *                    value or not.
     */
    private void addPrimitiveField(MetaPrimitive primType, String name, boolean validValues, MetaField parent){
        String label;
        Object[] data = new Object[3];
        String typeName = primType.getTypeName();
         
        data[0] = typeName;
        data[1] = name;
        
        if(parent instanceof MetaUnion){
            if(name.endsWith(".switch")){
                label = (String)((MetaUnion)parent).getCases()[0].getLabels().get(0);
                
                if("c_bool".equals(typeName)){
                    try{
                        int value = Integer.parseInt(label);
                        
                        if(value == 1){
                            label = "TRUE";
                        } else {
                            label = "FALSE";
                        }
                    } catch(NumberFormatException nfe){
                        assert("TRUE".equals(label) || "FALSE".equals(label));
                    }
                  
                }
            } else {
                label = null;
            }
        } else {
            label = null;
        }
        
        if("c_bool".equals(typeName)){
            if(validValues){
                if(label != null){
                    data[2] = label.toUpperCase();
                } else {
                    data[2] = "TRUE";
                }
            } else{
                data[2] = "N/A";
            }
        }
        else if("c_char".equals(typeName)){
            if(validValues){
                if(label != null){
                    data[2] = label;
                } else {
                    data[2] = "a";
                }
            } else{
                data[2] = "N/A";
            }
        } 
        else if("c_voidp".equals(typeName)){
            if(validValues){
                data[2] = "NULL";
            } else{
                data[2] = "N/A";
            }
        }
        else{
            if(validValues){
                if(label != null){
                    data[2] = label;
                } else {
                    data[2] = "0";
                }
            } else{
                data[2] = "N/A";
            }
        }
        this.addRow(data);
        ud.setData((String)data[1], (String)data[2]);
    }
    
    /**
     * Adds an enumeration field to the table.
     * 
     * @param enumType The enumeration field.
     * @param name The name of the enumeration.
     * @param validValues whether to set the values of the field to a valid 
     *                    value or not.
     */
    private void addEnumField(MetaEnum enumType, String name, boolean validValues, MetaField parent){
        Object[] data = new Object[3];
        data[0] = enumType.getTypeName();
        data[1] = name;
        
        if(validValues){
            if(parent instanceof MetaUnion){
                if(name.endsWith(".switch")){
                    data[2] = (String)((MetaUnion)parent).getCases()[0].getLabels().get(0);
                } else {
                    data[2] = enumType.getPosValues()[0];
                }
            } else {
                data[2] = enumType.getPosValues()[0];
            }
        } else{
            data[2] = "N/A";
        }
        this.addRow(data);
        ud.setData((String)data[1], (String)data[2]);
    }    
    
    /**
     * The type of the UserData that is currently being held.
     */
    protected MetaType userDataType;
            
    /**
     * The UserData that is currently being held.
     */
    protected UserData ud;
}
