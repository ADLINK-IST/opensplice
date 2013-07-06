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
package org.opensplice.common.view.entity;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.opensplice.cm.meta.MetaClass;
import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.cm.meta.MetaUnionCase;

/**
 * Concrete descendant of EntityInfoFormatter that formats output in plain text
 * format.
 * 
 * @date Jun 7, 2004
 */
public class EntityInfoFormatterText extends EntityInfoFormatter{
    
    /**
     * Creates a new plain text formatter.
     */
    public EntityInfoFormatterText(){
        super();
        tab = "    ";
        separator = "";
        newLine = "\n";
        curValue = "";
    }
    
    /**
     * Creates plain text representation of the supplied object. Object types 
     * that are supported:
     * - String
     * - MetaType
     * 
     * If the type is not supported, no error will occur.
     */
    @Override
    public synchronized String getValue(Object object){
        curObject = object;
        if(object == null){
            curValue = "No selection";
        }
        else if(object instanceof MetaType){
            this.setUserDataType((MetaType)object);
        }
        else if(object instanceof String){
            curValue = ((String)object);
        }
        else{
            curValue = "Cannot show info of " + object.getClass().toString();
        }
        return curValue;
    }
    
    /**
     * Creates an plain text representation of the supplied type.
     * 
     * @param type The type to create a plain text representation of.
     */
    private void setUserDataType(MetaType type){
        curValue = typeCache.get(type);
        
        if(curValue == null){
            MetaField field = type.getRootField();
            writer = new StringWriter();
            this.drawTypedefs(type);
            this.drawMetaField(field, "", type);
            writer.flush();
            curValue = writer.toString();
            typeCache.put(type, curValue);
        }
        
    }
    
    /**
     * Creates a plain text representation of the typedefs in the type.
     * 
     * @param type The type to extract the typedefs from.
     */
    private void drawTypedefs(MetaType type){
        
        LinkedHashMap<MetaField, String> typedefs = type.getTypedefs();
        HashSet<String> finished = new HashSet<String>();
        
        for (Iterator<Entry<MetaField, String>> it = typedefs.entrySet().iterator(); it.hasNext();) {
            Map.Entry<MetaField, String> entry = it.next();
            MetaField field = entry.getKey();
            String typedefName = entry.getValue();
            if(!(finished.contains(typedefName))){
                writer.write("typedef ");
                this.drawTypedefMetaField(field, "", type);
                writer.write(" " + typedefName + ";" + newLine + newLine + separator);
                finished.add(typedefName);
            }
        }
    }
    
    /**
     * Creates a plain text representation of a typedef field.
     * 
     * @param field The 'typedeffed' field.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefMetaField(MetaField field, String indent, MetaType type){
        if(field instanceof MetaClass){
            this.drawTypedefClass((MetaClass)field, indent, type);
        }
        else if(field instanceof MetaCollection){
            this.drawTypedefCollection((MetaCollection)field, indent, type);
        }
        else if(field instanceof MetaEnum){
            this.drawTypedefEnum((MetaEnum)field, indent, type);
        }
        else if(field instanceof MetaPrimitive){
            this.drawTypedefPrimitive((MetaPrimitive)field, indent, type);
        }
        else if(field instanceof MetaStruct){
            this.drawTypedefStruct((MetaStruct)field, indent, type);
        }
        else if(field instanceof MetaUnion){
            this.drawTypedefUnion((MetaUnion)field, indent, type);
        }
        else{
            //Do nothing
        }
    }
    
    /**
     * Creates a plain text representation of a field.
     * 
     * @param field The field to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawMetaField(MetaField field, String indent, MetaType type){
        if(field instanceof MetaClass){
            this.drawClass((MetaClass)field, indent, type);
        }
        else if(field instanceof MetaCollection){
            this.drawCollection((MetaCollection)field, indent, type);
        }
        else if(field instanceof MetaEnum){
            this.drawEnum((MetaEnum)field, indent, type);
        }
        else if(field instanceof MetaPrimitive){
            this.drawPrimitive((MetaPrimitive)field, indent, type);
        }
        else if(field instanceof MetaStruct){
            this.drawStruct((MetaStruct)field, indent, type);
        }
        else if(field instanceof MetaUnion){
            this.drawUnion((MetaUnion)field, indent, type);
        }
        else{
            //Do nothing
        }
    }
    
    /**
     * Creates a plain text representation of a class.
     * 
     * @param field The class to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawClass(MetaClass field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + typedefName + " " + field.getName() + ";" + newLine + separator);
        }
        else{
            writer.write(indent + "class " + field.getName() + "{" + newLine);
            MetaField[] fields = field.getFields();
            
            for(int i=0; i<fields.length; i++){
                this.drawMetaField(fields[i], indent + tab, type);  
            }
            writer.write(indent + "};" + newLine + separator);
        }
    }
    
    /**
     * Creates a plain text representation of a typedef class.
     * 
     * @param field The 'typedeffed' class.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefClass(MetaClass field, String indent, MetaType type){
        writer.write(indent + "class " + field.getName() + "{" + newLine);
        MetaField[] fields = field.getFields();
    
        for(int i=0; i<fields.length; i++){
            this.drawMetaField(fields[i], indent + tab, type);  
        }
        writer.write(indent + "}" + separator);
    }
    
    /**
     * Creates a plain text representation of a collection.
     * 
     * @param field The collection to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawCollection(MetaCollection field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + typedefName + " " + field.getName() + ";" + newLine + separator);
        }
        else{
            writer.write(indent + field.getTypeName() + " " + field.getName() + ";" + newLine + separator);
        }
    }
    
    /**
     * Creates a plain text representation of a typedef collection.
     * 
     * @param field The 'typedeffed' collection.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefCollection(MetaCollection field, String indent, MetaType type){
        writer.write(indent + field.getTypeName());
    }
    
    /**
     * Creates a plain text representation of a enumeration.
     * 
     * @param field The enumeration to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawEnum(MetaEnum field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + typedefName + " " + field.getName() + ";" + newLine + separator);
        }
        else{
            writer.write(indent +"enum " + field.getTypeName() + " {" + newLine);
            String[] posValues = field.getPosValues();
            
            for(int i=0; i<posValues.length; i++){
                if(i != (posValues.length-1)){
                    writer.write(indent + tab + posValues[i] + ", " + newLine);
                }
                else{
                    writer.write(indent + tab + posValues[i] + newLine);
                }
            }
            writer.write(indent + "} " + field.getName() + ";" + newLine + separator);
        }
    }
    
    /**
     * Creates a plain text representation of a typedef enumeration.
     * 
     * @param field The 'typedeffed' enumeration.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefEnum(MetaEnum field, String indent, MetaType type){
        writer.write(indent +"enum " + field.getTypeName() + " {" + newLine + 
                     indent + tab);
        String[] posValues = field.getPosValues();
    
        for(int i=0; i<posValues.length; i++){
            if(i != (posValues.length-1)){
                writer.write(posValues[i] + ", ");
            }
            else{
                writer.write(posValues[i]);
            }
        }
        writer.write(newLine + indent + "} ");
    }
    
    /**
     * Creates a plain text representation of a primitive.
     * 
     * @param field The primitive to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawPrimitive(MetaPrimitive field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + typedefName + " " + field.getName() + ";" + newLine + separator);
        }
        else{
            writer.write(indent + field.getTypeName() + " " + field.getName() + ";" + newLine + separator);
        }
    }
    
    /**
     * Creates a plain text representation of a typedef primitive.
     * 
     * @param field The 'typedeffed' primitive.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefPrimitive(MetaPrimitive field, String indent, MetaType type){
        writer.write(indent + field.getTypeName());
    }
    
    /**
     * Creates a plain text representation of a structure.
     * 
     * @param field The structure to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawStruct(MetaStruct field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + typedefName + " " + field.getName() + ";" + newLine + separator);
        }
        else{
            writer.write(indent + "struct " + field.getTypeName() + " {" + newLine);
            MetaField[] fields = field.getFields();
            
            for(int i=0; i<fields.length; i++){
                this.drawMetaField(fields[i], indent + tab, type);
            }
            writer.write(indent + "} " + field.getName() + ";" + newLine + separator);
        }
    }
    
    /**
     * Creates a plain text representation of a typedef structure.
     * 
     * @param field The 'typedeffed' structure.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefStruct(MetaStruct field, String indent, MetaType type){
        writer.write(indent + "struct " + field.getTypeName() + " {" + newLine);
        MetaField[] fields = field.getFields();
    
        for(int i=0; i<fields.length; i++){
            this.drawMetaField(fields[i], indent + tab, type);
        }
        writer.write(indent + "}");
    }
    
    /**
     * Creates a plain text representation of a union.
     * 
     * @param field The union to create the plain text representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawUnion(MetaUnion field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + typedefName + " " + field.getName() + ";" + newLine + separator);
        }
        else{
            writer.write(indent + "union " + field.getTypeName() + 
                        " switch(" + field.getDiscriminator().getTypeName() + ") {" + newLine);
            MetaUnionCase[] cases = field.getCases();
            ArrayList labels;
            
            for(int i=0; i<cases.length; i++){
                labels = cases[i].getLabels();
                
                for(int j=0; j<labels.size(); j++){
                    if("".equals(labels.get(j))){
                        writer.write(indent + tab + "default :" + newLine);
                    } else {
                        writer.write(indent + tab + "case " + labels.get(j) + ":" + newLine);
                    }
                }
                this.drawMetaField(cases[i].getField(), indent + tab + tab, type);
            }
                    
            writer.write(indent + "} " + field.getName() + ";" + newLine + separator);
        }
    }
    
    
    /**
     * Creates a plain text representation of a typedef union.
     * 
     * @param field The 'typedeffed' union.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefUnion(MetaUnion field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field.getDiscriminator());
        if(typedefName != null){
            writer.write(indent + "union " + field.getTypeName() + 
                        " switch(" + typedefName + ") {" + newLine);
        }
        else{
            writer.write(indent + "union " + field.getTypeName() + 
                        " switch(" + field.getDiscriminator().getTypeName() + ") {" + newLine);
        }
        MetaUnionCase[] cases = field.getCases();
        ArrayList labels;
        
        for(int i=0; i<cases.length; i++){
            labels = cases[i].getLabels();
            
            for(int j=0; j<labels.size(); j++){
                if("".equals(labels.get(j))){
                    writer.write(indent + tab + "default :" + newLine);
                } else {
                    writer.write(indent + tab + "case " + labels.get(j) + ":" + newLine);
                }
            }
            this.drawMetaField(cases[i].getField(), indent + tab + tab, type);
        }            
        writer.write(indent + "}");
    }
}
