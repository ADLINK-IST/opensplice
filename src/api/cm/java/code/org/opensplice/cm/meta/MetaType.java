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
package org.opensplice.cm.meta;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.StringTokenizer;

/**
 * Represents a type of a Splice database object (c_type). 
 * 
 * This object can be used to get information about the structure of data and
 * so it enables the possibility to read/write data of this type.
 */
public class MetaType {
    /**
     * Constructs new MetaType.
     */   
    public MetaType(String xmlType){
        isValid = false;
        mayFinalize = true;
        metaField = null;
        typedefs = new LinkedHashMap();
        this.xmlType = xmlType;
        this.normalizeXMLType();
    }
    
    private void normalizeXMLType(){
        boolean inTag = false;
        boolean inCloseTag = false;
        boolean inNodeValue = false;
        StringWriter writer = new StringWriter();
        char[] chars = this.xmlType.toCharArray();
        char cur;
        
        for(int i=0; i<chars.length; i++){
            cur = chars[i];
            
            if(inTag){
                if(cur == '>'){
                    inTag = false;
                    
                    if(inCloseTag){
                        inCloseTag = false;
                    } else {
                        inNodeValue = true;
                    }
                } else if(cur == '/'){
                    inCloseTag = true;
                }
                writer.write(cur);
            } else if(inNodeValue){
                if(cur == '<'){
                    inNodeValue = false;
                    inTag = true;
                    writer.write(cur);
                } else {
                    writer.write(cur);
                }
            } else {
                if(cur == ' '){
                    
                } else if(cur == '\n'){
                    
                } else {
                    writer.write(cur);
                }
            }
        }
        this.xmlType = writer.toString();
    }
    
    /**
     * Adds a typedef to the type.
     * 
     * @param typedefName The name of the typedef.
     * @param field The field that is 'typedeffed'.
     */
    public void addTypedef(String typedefName, MetaField field){
        typedefs.put(field, typedefName);
    }
    
    /**
     * Provides access to all typedefs in the type.
     * 
     * @return A hashmap that contains all typedefs in the type.
     */
    public LinkedHashMap getTypedefs(){
        return typedefs;
    }
    
    /**
     * Provides access to the name of the typedef in this type.
     * 
     * @param field The field to look the typedef for.
     * @return The name of the typedef if the field is indeed typedeffed or
     *         null otherwise.
     */
    public String getFieldTypedefName(MetaField field){
        return (String)(typedefs.get(field));
    }
    
    /**
     * Provides access to the isValid boolean.
     * 
     * @return true if the type is valid, false otherwise.
     */
    public boolean isValid(){
        return isValid;
    }
    
    /**
     * Assigns the supplied field to the type.
     * 
     * @param field The field to assign to this type.
     */
    public void setField(MetaField field){
        metaField = field;
        
    }
    
    /**
     * Provides access to the field in the type with the supplied name.
     * 
     * Nested names are allowed.Example
     * @verbatim
       IDL:
       module my_mod{
           struct my_structure{
               long a;
               struct my_inner_struct{
                   long b;             
               } my_struct;
           };
       };
     * @endverbatim
     * Fields in this type can be accessed as:
     * - my_structure.a
     * - my_structure.my_struct.b
     * 
     * @param fieldName The name of the field the resolve the value of.
     * @return The field associated with the supplied fieldname or null if it
     *         cannot be found.
     */
    public MetaField getField(String fieldName){
        StringTokenizer tokenizer = new StringTokenizer(fieldName, ".");
        MetaField current = metaField;
        MetaField temp;
        String token;
        
        while(tokenizer.hasMoreTokens()){
             token = tokenizer.nextToken();
            
             temp = current.getField(token);
             
             if((temp != null) && (tokenizer.hasMoreTokens())){
                current = temp;
             }
             else if(temp != null){
                 return temp;
             }
             else if((temp == null) && (tokenizer.hasMoreTokens())){
                 return null;
             }
             else{
                 /* Maybe it is a subType, continue search
                  * it should look like: memberName[i][j]
                  */
                 StringTokenizer tokenizer2 = new StringTokenizer(token, "[");
                 int depth = tokenizer2.countTokens() - 1;

                 if(depth > 0){
                     String token2 = tokenizer2.nextToken();
                     temp = current.getField(token2);

                     for(int i=0; i<depth; i++){
                         if(temp != null){
                             if(temp instanceof MetaCollection){
                                 temp = ((MetaCollection)temp).getSubType();
                             }
                             else{
                                 return null;
                             }
                         }
                         else{
                             return null;
                         }
                     }
                 }
                 return temp;
             }
        }
        return null;
    }
    
    /**
     * Provides access to all fieldnames in the type.
     * 
     * @return The array of all fieldnames in the type.
     */    
    public String[] getFieldNames(){
        String[] result;
        MetaField[] fields = metaField.getFields();
        ArrayList alist = new ArrayList();
        
        for(int i=0; i<fields.length; i++){
            alist.addAll(fields[i].getFieldNames());
            
        }
        result = (String[])(alist.toArray(new String[1]));
        
        return result;
    }
    
    /**
     * Provides access to all fields in the root field.
     * 
     * @return The array of fields in the root field.
     */ 
    public MetaField[] getFields(){
        return metaField.getFields();
    }
    
    /**
     * Provides access to the root field.
     * 
     * @return The root field of the data.
     */
    public MetaField getRootField(){
        return metaField;
    }
    
    /**
     * Function to notify that the type will not be correct and/or complete
     * after deserialization. 
     * 
     * After calling this function, a call to finalizeType will
     * NOT result in setting the isValid field to true.
     * 
     * @see finalizeType
     */
    public void mayNotBeFinalized(){
        mayFinalize = false;
    }
    
    /**
     * Finalizes the type by setting the isValid field. If the 
     * mayNotBeFinalized() function has been called prior to calling
     * this function, this will not result in a valid type and false will be
     * returned.
     * 
     * @return true if the type is valid, false otherwise.
     * @see mayNotBeFinalized()
     */
    public boolean finalizeType(){
        if(mayFinalize){
            isValid = true;
        }
        return isValid;
    }
    
    public String toXML(){
        return this.xmlType;
    }
    
    public boolean equals(Object obj){
        boolean result = false;
        
        if(obj instanceof MetaType){
            if(((MetaType)obj).toXML().equals(this.xmlType)){
                result = true;
            }
        }
        return result;
    }
    
    /**
     * Provides access to all fieldnames in the type.
     * 
     * @return The array of all fieldnames in the type.
     */    
    public String[] getExtendedFieldNames(){
        ArrayList result = new ArrayList();
        MetaField udt[] = this.getFields();
        MetaField field;
        
        for(int i=0; i<udt.length; i++){
            field = udt[i];
            
            if(field instanceof MetaPrimitive){
                this.addPrimitiveField((MetaPrimitive)field, field.getName(), result);
            }
            else if(field instanceof MetaCollection){
                this.addCollectionField(null, (MetaCollection)field, field.getName(), result);
            }
            else if(field instanceof MetaEnum){
                this.addEnumField((MetaEnum)field, field.getName(), result);
            }
            else{
                this.addNestedField(field.getName(), field, result);
            }
        }
        return ((String[])result.toArray(new String[result.size()]));
    }
    
    private void addNestedField(String name, MetaField field, ArrayList result){
        MetaField fields[] = field.getFields();
        MetaField f;
        String nestName;
        
        for(int i=0; i<fields.length; i++){
            f = fields[i];
            nestName = name + "." + f.getName();
            
            if(f instanceof MetaPrimitive){
                this.addPrimitiveField((MetaPrimitive)f, nestName, result);
            }
            else if(f instanceof MetaCollection){
                this.addCollectionField(field, (MetaCollection)f, nestName, result);
            }
            else if(f instanceof MetaEnum){
                this.addEnumField((MetaEnum)f, nestName, result);
            }
            else{ //CLASS, UNION or STRUCT
                this.addNestedField(nestName, f, result);
            } 
        }
    }
    
    private void addCollectionField(MetaField parent, MetaCollection colType, String prefix, ArrayList result){
        MetaField subType = colType.getSubType();
        int maxSize = colType.getMaxSize();
        
        if( (maxSize == 0) || 
            (colType.getTypeName().startsWith("C_STRING<")) ||
            (colType.getTypeName().startsWith("C_WSTRING<")))
        {
            result.add(prefix);
        } else{
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
                                             tmp, result);
                    
                } else if(subType instanceof MetaStruct){
                    this.addNestedField(tmp, subType, result);
                } else if(subType instanceof MetaUnion){
                    this.addNestedField(tmp, subType, result);
                } else if(subType instanceof MetaEnum){
                    this.addEnumField((MetaEnum)subType, tmp, result);
                } else{
                    this.addPrimitiveField((MetaPrimitive)subType, 
                                            tmp, result);
                }
            }
        }
    }
    
    private void addPrimitiveField(MetaPrimitive primType, String name, ArrayList result){
        result.add(name);
    }
    
    private void addEnumField(MetaEnum enumType, String name, ArrayList result){
        result.add(name);
    }    
    
    /**
     * The root field in the type.
     */
    private MetaField metaField;
    
    /**
     * Boolean that specifies if the type is valid.
     * This field is used by the deserializer, so it can specify if the type
     * could be completely deserialized. If this is not the case, this field 
     * will be false. 
     */ 
    private boolean mayFinalize;
    
    /**
     * Boolean that specifies if the type is complete and correct
     * This is used to catch unknown types in the deserializer. In the
     * future it is not necessary anymore.
     */
    private boolean isValid;
    
    /**
     * Ordered HashMap that contains all typedefs in the type.
     * (<MetaField field, String typedefName>)
     */
    private LinkedHashMap typedefs;
    
    private String xmlType;
}
