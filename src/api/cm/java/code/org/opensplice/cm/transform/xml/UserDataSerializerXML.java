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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaClass;
import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.cm.meta.MetaUnionCase;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cm.transform.UserDataSerializer;

/**
 * The XML implementation of an UserDataSerializer. It is capable of 
 * transforming a UserData object into a serialized XML representation.
 *  
 * @date Jun 2, 2004
 */
public class UserDataSerializerXML implements UserDataSerializer{
    
    /**
     * Creates a new serializer, that is capable of transforming UserData
     * to its XML representation.
     */ 
    public UserDataSerializerXML(){
        type = null;
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    public synchronized String serializeUserData(UserData data) throws TransformationException{
        if(data == null){
            throw new TransformationException("Supplied UserData is not valid.");
        }
        writer = new StringWriter();
        type = data.getUserDataType();
        MetaField[] fields = type.getFields();
        MetaField field;
        
        writer.write("<object>");
            
        for(int i=0; i<fields.length; i++){
            field = fields[i];
            this.serializeType(field.getName(), field, data, true);
        }
    
        writer.write("</object>");
        writer.flush();
        /*logger.logp(java.util.logging.Level.FINEST, "UserDataSerializerXML", "serializeUserData", 
                                   "Serialized data:\n" + 
                                   writer.toString());*/
        return writer.toString();
    }
    
    private void serializeType(String nestedFieldName, MetaField field, UserData data, boolean printField){
        if(printField){
            writer.write("<" + field.getName() + ">");
        }
        
        if(field instanceof MetaPrimitive){         //Primitive
            this.serializePrimitive(nestedFieldName, data);
        } else if(field instanceof MetaEnum){       //Enumeration
            this.serializeEnum(nestedFieldName, data);
        } else if(field instanceof MetaUnion){      //Union
            this.serializeUnion(nestedFieldName, (MetaUnion)field, data);
        } else if(field instanceof MetaStruct){     //Structure
            this.serializeStruct(nestedFieldName, (MetaStruct)field, data);
        } else if(field instanceof MetaClass){      //Class
            this.serializeClass(nestedFieldName, (MetaClass)field, data);
        } else if(field instanceof MetaCollection){ //Collection
            this.serializeCollection(nestedFieldName, (MetaCollection)field, data);
        }
        if(printField){
            writer.write("</" + field.getName() + ">");
        }
    }
    
    private void serializeCollection(String nestedFieldName, MetaCollection field, UserData data){
        int size, tsize;
        String typeName = field.getTypeName();
                
        if( (typeName.equals("c_string")) ||
            (typeName.equals("c_wstring")) ||
            (typeName.startsWith("C_STRING<")) ||
            (typeName.startsWith("C_WSTRING<")))
        {
            this.serializeString(nestedFieldName, data);
        } else { //Not a String collection.
            size = field.getMaxSize();
            
            if(size == -1){
                tsize = 0;
            } else {
                tsize = size;
            }
            
            if(size == 0){
                this.serializeUnboundedSequence(nestedFieldName, (MetaCollection)field, data);
            } else if(size == -1){ /*recursive type*/
                this.serializeRecursiveType(nestedFieldName, (MetaCollection)field, data);
            } else{
                if(! ((typeName.startsWith("C_ARRAY")) && (tsize != 0))){
                    writer.write("<size>" + size + "</size>");
                }
                for(int i=0; i<size; i++){
                    writer.write("<element>");
                    this.serializeType(getCollectionFieldName(nestedFieldName, i), field.getSubType(), data, false);
                    writer.write("</element>");
                }
            }
        }
    }
    
    private String getCollectionFieldName(String name, int colIndex){
        int index;
        String result = name;
        
        if(name.endsWith("]")){
            index = name.lastIndexOf('[');
            result = name.substring(0, index);
            index = result.lastIndexOf('[');
            
            while((index != -1)  && (result.endsWith("]"))){
                result = result.substring(0, index);
                index = result.lastIndexOf('[');
            }
            index = result.length();
            result += "[" + colIndex + "]";
            result += name.substring(index);
        } else {
            result += "[" + colIndex + "]";
        }
        
        return result;
    }
    
    private void serializeString(String nestedFieldName, UserData data){
        writer.write("<![CDATA[" + data.getFieldValue(nestedFieldName) + "]]>");
    }
    
    private void serializeClass(String nestedFieldName, MetaClass field, UserData data){
        MetaField[] fields = field.getFields();
        
        for(int i=0; i<fields.length; i++){
            this.serializeType(nestedFieldName + "." + fields[i].getName(), fields[i], data, true);
        }
    }
    
    private void serializeStruct(String nestedFieldName, MetaStruct field, UserData data){
        MetaField[] fields = field.getFields();
        
        for(int i=0; i<fields.length; i++){
            this.serializeType(nestedFieldName + "." + fields[i].getName(), fields[i], data, true);
        }
    }
    
    private void serializePrimitive(String nestedFieldName, UserData data){
        String value = data.getFieldValue(nestedFieldName);
        
        if(value == null){
            logger.logp(Level.SEVERE,  "UserDataSerializerXML", 
                    "serializePrimitive", 
                    "Could not find value for field: " + nestedFieldName);
        }
        writer.write(value);
    }
    
    private void serializeEnum(String nestedFieldName, UserData data){
        writer.write(data.getFieldValue(nestedFieldName));
    }
    
    private void serializeUnion(String nestedFieldName, MetaUnion field, UserData data){
        String value = data.getFieldValue(nestedFieldName + ".switch");
        
        MetaUnionCase c = field.getCase(value);
        
        writer.write("<switch>" + value + "</switch>");
        this.walkType(nestedFieldName + "." + c.getField().getName(), c.getField(), data);
    }
    
    
    /**
     * Serializes the supplied field that has the supplied nested name.
     * 
     * @param nestedFieldName The nested field name (that means: including scope) of
     *                        the field.  
     * @param field The field to serialize.
     * @param data The data to serialize.
     */
    private void walkType(String nestedFieldName, MetaField field, UserData data){
                
        writer.write("<" + field.getName() + ">");
        
        
        if(field instanceof MetaPrimitive){         //Primitive
            this.serializeMemberContents(nestedFieldName, data);
        }
        else if(field instanceof MetaEnum){         //Enumeration
            this.serializeMemberContents(nestedFieldName, data);
        }
        else if(field instanceof MetaCollection){   //Collection
            if(field.getTypeName().startsWith("C_SEQUENCE")){
                int size = ((MetaCollection)field).getMaxSize();
                
                if(size == 0){
                    this.serializeUnboundedSequence(nestedFieldName, (MetaCollection)field, data);
                }
                else{
                    writer.write("<size>" + size + "</size>");
                    this.serializeMemberContents(nestedFieldName, data);
                }
            }
            else if(field.getTypeName().startsWith("C_ARRAY")){
                int size = ((MetaCollection)field).getMaxSize();
                
                
                if(size == 0){
                    writer.write("<size>" + size + "</size>");
                    this.serializeUnboundedSequence(nestedFieldName, (MetaCollection)field, data);
                }
                else{
                    this.serializeMemberContents(nestedFieldName, data);
                }
            }
            else{
                this.serializeMemberContents(nestedFieldName, data);
            }
        }
        else if(field instanceof MetaUnion){        //Union
            this.serializeUnionContents(nestedFieldName, (MetaUnion)field, data);
        }
        else{                                           //Class or Structure
            MetaField field2;
            MetaField[] fields = field.getFields();
            
            for(int i=0; i<fields.length; i++){
                field2 = fields[i];
                this.walkType(nestedFieldName + "." + field2.getName(), field2, data);            
            }
        }    
        writer.write("</" + field.getName() + ">");
    }
    
    /**
     * Serializes the contents of the supplied union.
     * 
     * @param nestedFieldName The name of the union, including its scope.
     * @param field The union field to serialize the contents of.
     * @param data The data to serialize.
     */
    private void serializeUnionContents(String nestedFieldName, MetaUnion field, UserData data){
        String value = data.getFieldValue(nestedFieldName + ".switch");
                
        MetaUnionCase c = field.getCase(value);
        
        writer.write("<switch>" + value + "</switch>");
        this.walkType(nestedFieldName + "." + c.getField().getName(), c.getField(), data);
        
    }
    
    /**
     * Serializes the contents of the supplied field. This is done be looking 
     * up the field in the type of the data and retrieving the value of the 
     * field in the userdata.
     * 
     * @param nestedFieldName The name of the field including its scope.
     * @param data The data to serialize.
     */
    private void serializeMemberContents(String nestedFieldName, UserData data){
        boolean isString = false;
        String nestedFieldTypeName = this.removeIndices(nestedFieldName);
        MetaField typeField = type.getField(nestedFieldTypeName);
        MetaField originalType = typeField;
        String value = null;
        
        value = data.getFieldValue(nestedFieldName);
        logger.logp(java.util.logging.Level.FINEST, "UserDataSerializerXML", "serializeMemberContents", "Fieldname: " + nestedFieldName + " Value: " + value);
        
        if(typeField instanceof MetaCollection){
            MetaCollection colType = (MetaCollection)typeField;
            
            while((typeField instanceof MetaCollection ) && (!isString)){
                String typeName = typeField.getTypeName();
                
                if( (typeName.equals("c_string")) ||
                    (typeName.equals("c_wstring")) ||
                    (typeName.startsWith("C_STRING<")) ||
                    (typeName.startsWith("C_WSTRING<")))
                {
                    value = this.getSerializedStringCollection(nestedFieldName, colType, data);
                    isString = true;
                }
                else{
                    typeField = ((MetaCollection)typeField).getSubType();
                    
                }
            }
            
            if(!isString){ //Type is not a string and does not contain strings
                if((originalType instanceof MetaCollection) && (typeField instanceof MetaStruct)){
                    MetaField field2;
                    MetaField[] fields = typeField.getFields();
                    int size  = ((MetaCollection)originalType).getMaxSize();
                    
                    for(int i=0; i<size; i++){
                        writer.write("<element>");
                    
                        for(int j=0; j<fields.length; j++){
                            field2 = fields[j];
                            this.walkType(nestedFieldName + "[" + i + "]." + field2.getName(), field2, data);
                        }
                        writer.write("</element>");
                    }
                    value = "";
                } else if((originalType instanceof MetaCollection) && 
                   (originalType.getTypeName().startsWith("C_SEQUENCE<")))
                {
                    value = this.getSerializedStringCollection(nestedFieldName, colType, data);
                } else {
                    value = value.replaceAll("\\[", "<element>");
                    value = value.replaceAll("\\]", "</element>");
                    value = value.replaceAll(",", "</element><element>");
                }
            }
            else{
                //Do nothing.    
            }
        }
        writer.write(value);
    }
        
    /**
     * Serializes the contents of the supplied collection.
     * 
     * @param nestedFieldName The name of the collection field in the data.
     * @param colType The type of the collection.
     * @param data The data to serialize.
     * @return The serialized representation of the collection data.
     */
    private String getSerializedStringCollection(String nestedFieldName, MetaCollection colType, UserData data){
        String result = null;
        String size = null;
        String typeName = colType.getTypeName();
        int index = nestedFieldName.indexOf('[');
        String temp = null;
        
        if( (typeName.equals("c_string")) ||
            (typeName.equals("c_wstring")) ||
            (typeName.startsWith("C_STRING<")) ||
            (typeName.startsWith("C_WSTRING<")))
        {
            result = "<![CDATA[" + data.getFieldValue(nestedFieldName) + "]]>";
        }
        else if(colType.getSubType() instanceof MetaCollection){
            result = "";
            
            if(colType.getSubType().getTypeName().startsWith("C_SEQUENCE<")){
                size = "<size>" + ((MetaCollection)colType.getSubType()).getMaxSize() + "</size>"; 
            }
            
            
            if(index != -1){
                temp = nestedFieldName.substring(0, index);
                
                for(int i=0; i<colType.getMaxSize(); i++){
                    result += "<element>";
                    
                    if(size != null ){
                        result += size;
                    }
                    result += getSerializedStringCollection(
                            temp + "[" + i + "]" + nestedFieldName.substring(index), 
                                                (MetaCollection)colType.getSubType(),
                                                data);
                    result += "</element>";
                }
            } else {
                for(int i=0; i<colType.getMaxSize(); i++){
                    result += "<element>";
                    
                    if(size != null ){
                        result += size;
                    }
                    result += getSerializedStringCollection(nestedFieldName + "[" + i + "]", 
                                                (MetaCollection)colType.getSubType(),
                                                data);
                    result += "</element>";
                }
            }
        }
        else{
            result = "";
            
            if(index != -1){
                temp = nestedFieldName.substring(0, index);
                
                for(int i=0; i<colType.getMaxSize(); i++){
                    result += "<element>";
                    result += data.getFieldValue(temp + "[" + i + "]" + nestedFieldName.substring(index));
                    result += "</element>";
                }
            } else {
                for(int i=0; i<colType.getMaxSize(); i++){
                    result += "<element>";
                    result += data.getFieldValue(nestedFieldName + "[" + i + "]");
                    result += "</element>";
                }
            }
        }
        return result;
    }
    
    /**
     * Serializes an unbounded sequence.
     * 
     * @param nestedFieldName The name of the field (including scope)
     * @param seqType The type of the sequence.
     * @param data The data to serialize.
     */
    private void serializeUnboundedSequence(String nestedFieldName, MetaCollection seqType, UserData data){
        String value;
        String typeName;
        StringTokenizer tokenizer;
        MetaField subType = seqType.getSubType();
        String token;
        String seqTypeName;
        
        seqTypeName = seqType.getTypeName();
        
        
        if(subType instanceof MetaPrimitive){
            value = data.getFieldValue(nestedFieldName);
            
            if(value != null){
                if(value.equals("NULL")){
                    writer.write("&lt;NULL&gt;");
                } else if(value.equals("[]")){
                    writer.write("<size>0</size>");
                } else {
                    tokenizer = new StringTokenizer(value.substring(1,value.length()-1), ",");
                    int size = tokenizer.countTokens();
                    
                    writer.write("<size>" + size + "</size>");
                    
                    for(int i=0; i<size; i++){
                        token = tokenizer.nextToken();
                        writer.write("<element>" + token + "</element>");
                    }
                }
            } else {
                writer.write("&lt;NULL&gt;");
            }
        } else if(subType instanceof MetaCollection){
            typeName = subType.getTypeName();
            
            if( (typeName.equals("c_string")) ||
                (typeName.equals("c_wstring")) ||
                (typeName.startsWith("C_STRING<")) ||
                (typeName.startsWith("C_WSTRING<")))
            {
                value = data.getFieldValue(nestedFieldName);
                
                if(value != null){
                    if(value.equals("NULL")){
                        writer.write("&lt;NULL&gt;");
                    } else if(value.equals("[]")){
                        if(! (seqTypeName.startsWith("C_ARRAY"))){
                            writer.write("<size>0</size>");
                        }
                    } else {
                        tokenizer = new StringTokenizer(value.substring(1,value.length()-1), ",");
                        int size = tokenizer.countTokens();
                        
                        if(! (seqTypeName.startsWith("C_ARRAY"))){
                            writer.write("<size>" + size + "</size>");
                        }
                        for(int i=0; i<size; i++){
                            token = tokenizer.nextToken();
                            writer.write("<element>" + token + "</element>");
                        }
                    }
                } else {
                    writer.write("&lt;NULL&gt;");
                }
            }
        } else {
            writer.write("&lt;NULL&gt;");
        }
        return;
    }
    
    private void serializeRecursiveType(String nestedFieldName, MetaCollection seqType, UserData data){
        //String value = data.getFieldValue(nestedFieldName);
        String typeName = seqType.getTypeName();
        
        if(! ((typeName.startsWith("C_ARRAY")))){
            writer.write("&lt;NULL&gt;");
        }
        return;
    }
    
    private String removeIndices(String name){
        String result;
        StringWriter w;
        char c;
        int in;
        
        if(name.indexOf('[') == -1){
            result = name;
        } else {
            w = new StringWriter();
            int length = name.length();
            in = 0;
            
            for(int i=0; i<length; i++){
                c = name.charAt(i);
                
                if(c == '['){
                    in++;
                } else if(c == ']'){
                    in--;
                } else if(in == 0){
                    w.append(c);
                }
            }
            result = w.toString();
        }
        
        return result;
    }
    
    /**
     * Used during serialization to write XML to.
     */
    private StringWriter writer;
    
    /**
     * The type of the UserData to serialize.
     */
    private MetaType type;
    
    /**
     * The logging facilities.
     */
    private Logger logger;
}
