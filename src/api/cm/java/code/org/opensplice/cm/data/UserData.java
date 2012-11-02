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
package org.opensplice.cm.data;

import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Iterator;

import org.opensplice.cm.meta.*;

/**
 * Represents Splice userData
 * 
 * @date May 13, 2004
 */
public class UserData {
    
  
    /**
     * Constructs userdata according to the type.
     * 
     * @param _type The type of the userdata.
     */
    public UserData(MetaType _type){
        data = new LinkedHashMap();
        type = _type;
    }
    
    /**
     * Assigns the supplied field, with the specified value.
     * The name must be the nested name.
     * 
     * @param fieldName The nested name of the field
     * @param fieldValue The value that must be assigned.
     */  
    public void setData(String fieldName, String fieldValue){
        data.put(fieldName, fieldValue);
    }
    
    /**
     * Collects the value of a collection field in this userdata.
     @verbatim
         Example:
         IDL: long my_arr[3];
         Value: [0, 1, 2]
     
         IDL: long my_arr2[2][3]
         Value: [[1,2],[3.4],[5,6]]
     @endverbatim
     *
     * @param colType The type of the collection.
     * @param name The nested fieldname of the collection. 
     * @return The value of the collection in this userdata (as String).
     */
    private String getCollectionFieldValue(MetaCollection colType, String name){
        String fieldName;
        boolean first = true;
        boolean typeIsString = false;
        boolean subTypeIsString = false;
        
        String typeName = colType.getTypeName();
        int size = colType.getMaxSize();
        MetaField subType = colType.getSubType();
        String subTypeName =  subType.getTypeName();
        String value = "[";
        
        if( (typeName.equals("c_string")) ||
            (typeName.equals("c_wstring")) ||
            (typeName.startsWith("C_STRING<")) ||
            (typeName.startsWith("C_WSTRING<")))
        {
            typeIsString = true;
            value = "";
        }
        else if((subTypeName.equals("c_string")) ||
                (subTypeName.equals("c_wstring")) ||
                (subTypeName.startsWith("C_STRING<")) ||
                (subTypeName.startsWith("C_WSTRING<")))
        {
            subTypeIsString = true;
        }
        
        
        
        if(subType instanceof MetaCollection){
            String actualName;
            
            if((size == 0) && (subTypeIsString)){ //unbounded sequence with unbounded strings
                Iterator iter = data.keySet().iterator();
                
                while(iter.hasNext()){
                    fieldName = (String)iter.next();
                    
                    if( (fieldName.startsWith(name)) && 
                        (fieldName.charAt(name.length()) == '['))
                    {
                        if(first){
                            value += (String)data.get(fieldName);
                            first = false;
                        } 
                        else{
                            value += "," + (String)data.get(fieldName);    
                        }
                    }
                }
            }
            else{
                int index;
                
                for(int i=0; i<size; i++){
                    index = name.indexOf('[');
                    
                    if(index != -1){
                        actualName = name.substring(0, index) + "[" + i + "]" +
                                     name.substring(index);
                    } else { 
                        actualName = name + "[" + i + "]";
                    }
                
                    if(i == (size - 1)){ //last, no comma before, no comma after
                        value += this.getCollectionFieldValue(
                                (MetaCollection)subType, actualName);
                    }
                    else{//otherwise; comma after
                        value += this.getCollectionFieldValue(
                                (MetaCollection)subType, actualName)  + ",";
                    }
                }
            }
        }
        else{
            /* I need the String representation of a part of a collection.
             * For a collection like 'long my_array[2][3]' I expect the output:
             * "[[0,1,2],[3,4,5]]" the algorithm below finds a part of this string 
             * according to a specific input. In the data the fields are available like
             * my_array[0][0], my_array[0][1], my_array[0][2], my_array[1][0],
             * my_array[1][1] and my_array[1][2]. 
             * 
             * When I supply 'my_array[1]', I expect the algorithm to find 
             * my_array[0][1], my_array[1][1] and my_array[2][1]. In case of the example
             * I expect "[3,4,5]" as output.
             * 
             * This is done by walking by each field in the data and stripping the
             * fieldName until the first '['. The supplied name is also stripped until
             * the first '['. Now fieldName and name must be equal. After that the 
             * end of the supplied name (that contains the indices) is being compared
             * to the end of the fieldName, if these also match. The field value must be
             * added to the result. 
             * 
             */
            Iterator iter = data.keySet().iterator();
            int index;
            String fieldNameStripped, nameStripped, fieldNameHooks, nameHooks;
            
            index = name.indexOf("[");
            
            if(index != -1){
                nameStripped = name.substring(0, index);
                nameHooks = name.substring(index);
            }
            else{
                nameStripped = name;
                nameHooks = null;
            }
            
            while(iter.hasNext()){
                fieldName = (String)iter.next();
                index = fieldName.indexOf("[");
                
                if(index != -1){
                    fieldNameStripped = fieldName.substring(0, index);
                    fieldNameHooks = fieldName.substring(index);
                }
                else{
                    fieldNameStripped = fieldName;
                    fieldNameHooks = null;
                }
                
                if( fieldNameStripped.equals(nameStripped)){
                    if( (fieldNameHooks == null)|| 
                        (nameHooks == null) || 
                        (fieldNameHooks.endsWith(nameHooks))){
                            
                        if(first){
                            value += (String)data.get(fieldName);
                            first = false;
                        } 
                        else{
                            value += "," + (String)data.get(fieldName);    
                        }
                    }    
                }
            }
        }
        
        if(!typeIsString){
            if(value.equals("[")){
                value = "NULL";
            } else {
                value += "]";
            }
        }
        else if("".equals(value)){
            value = "NULL";
        }
        
        return value;
    }
    
    /**
     * Provides access to the value of the field with the supplied name.
     * 
     * @param fieldName The nested fieldName ('.' separated)
     * @return The value of the field.
     */   
    public String getFieldValue(String fieldName){
        String value = (String)(data.get(fieldName));
        
        /**Maybe it is a collection type.*/
        if(value == null){
            MetaField f;
            f = type.getField(fieldName);
             
            /**Yes it is a collection!!*/
            if( (f != null) && (!("c_string".equals(f.getTypeName()))) ){
                /**This check should not be necessary */ 
                if(f instanceof MetaCollection){
                    value = this.getCollectionFieldValue((MetaCollection)f, fieldName);
                }
            }
        }         
        return value;
    }
    
    /**
     * Provides access to all nested fieldnames in the data.
     * 
     * @return An ordered set of fields in the data.
     */  
    public LinkedHashSet getFieldNames(){
        return new LinkedHashSet(data.keySet());
    }
    
    /**
     * Provides access to all values of the available fields in the data.
     * 
     * @return An ordered set of field values.
     */
    public LinkedHashSet getFieldValues(){
        return new LinkedHashSet(data.values());
    }
    
    public String toString(){
        String result, key, value;
        LinkedHashSet s;
        Iterator i;
        
        s = new LinkedHashSet(data.keySet());
        i =  s.iterator();
        result = "";
        
        while(i.hasNext()){
            key   = (String)(i.next());
            value = (String)(data.get(key));
            result += "" + key + ": " + value + "\n";
        }
        return result;
    }
    
    /**
     * Provides access to the type of the data.
     * 
     * @return The type of the data.
     */
    public MetaType getUserDataType(){
        return type;
    }
    
    /**
     * Map with userdata fields and their values. 
     * <String fieldName, String fieldValue>
     */
    private LinkedHashMap data;
    
    /**
     * The type of the userdata.
     */ 
    private MetaType type;
}
