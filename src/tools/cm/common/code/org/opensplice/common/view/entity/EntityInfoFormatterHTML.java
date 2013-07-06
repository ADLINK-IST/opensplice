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

import java.awt.Color;
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
 * Implementation of EntityInfoFormatter that formats output in HTML
 * format.
 * 
 * @date Jun 7, 2004
 */
public class EntityInfoFormatterHTML extends EntityInfoFormatter{
    private final String color1 = this.getColorString(Color.RED.darker());
    private final String color2 = this.getColorString(Color.GREEN.darker().darker());
    private final String color3 = this.getColorString(Color.BLUE.darker());
    private final String color4 = this.getColorString(Color.MAGENTA.darker().darker());
    
    /**
     * Constructs new formatter that formats data in a HTML representation.
     */
    public EntityInfoFormatterHTML(){
        super();
        tab = "&nbsp;&nbsp;&nbsp;&nbsp;";
        separator = "";
        newLine = "<br>";
    }
    
    /**
     * Creates HTML representation of the supplied object. Object types that 
     * are supported are:
     * - String
     * - MetaType
     * 
     * If the type is not supported, no error will occur.
     */
    @Override
    public synchronized String getValue(Object object){
        curObject = object;
        if(object == null){
            curValue = "<html><body>No selection</body></html>";
        }
        else if(object instanceof MetaType){
            this.setUserDataType((MetaType)object);
        }
        else if(object instanceof String){
            String entityInfo = ((String)object).replaceAll("\\t", "&nbsp;");
            entityInfo = entityInfo.replaceAll("<", "&lt;");
            entityInfo = entityInfo.replaceAll(">", "&gt;");
            entityInfo = entityInfo.replaceAll("\\n", "<br>");
            curValue = "<html><body>" + entityInfo +  "</body></html>";
        }
        else{
            curValue = "<html><body>Cannot show info of " + object.getClass().toString() + "</body></html>";
        }
        return curValue;
    }
    
    private String getColorString(Color c){
        int r;
        String tmp;
        String result = "#";
        
        r = c.getRed();
        tmp = Integer.toHexString(r);
        
        if(tmp.length() == 1){
            result += "0";
        }
        result += Integer.toHexString(r);
        r = c.getGreen();
        
        tmp = Integer.toHexString(r);
        
        if(tmp.length() == 1){
            result += "0";
        }
        result += tmp;
        r = c.getBlue();
        tmp = Integer.toHexString(r);
        
        if(tmp.length() == 1){
            result += "0";
        }
        result += tmp;
        return result;
        
    }
    
    /**
     * Creates an HTML representation of the supplied type.
     * 
     * @param type The type to create a HTML representation of.
     */
    private void setUserDataType(MetaType type){
        curValue = typeCache.get(type);
        
        if(curValue == null){
            MetaField field = type.getRootField();
            
            writer = new StringWriter();
            writer.write("<html><body>");
            this.drawTypedefs(type);
            this.drawMetaField(field, "", type);
            writer.write("</body></html>");
            writer.flush();
            curValue = writer.toString();
            typeCache.put(type, curValue);
        }
    }
    
    /**
     * Creates a HTML representation of the typedefs in the type.
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
                writer.write("<b color=\"" + color1 + "\">typedef</b> ");
                this.drawTypedefMetaField(field, "", type);
                writer.write(" <font color=\"" + color2 + "\">" + typedefName + "</font>;" + newLine + newLine + separator);
                finished.add(typedefName);    
            }
        }
    }
    
    /**
     * Creates a HTML representation of a typedef field.
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
     * Creates a HTML representation of a field.
     * 
     * @param field The field to create the HTML representation of.
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
     * Creates a HTML representation of a class.
     * 
     * @param field The class to create the HTML representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawClass(MetaClass field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + "<font color=\"" + color4 + "\">" + typedefName + "</font> " + "<font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
        else{
            writer.write(indent + "<b color=\"" + color1 + "\">class</b> <b color=\"" + color2 + "\">" + field.getName() + "</b> <b>{</b>" + newLine);
            MetaField[] fields = field.getFields();
            
            for(int i=0; i<fields.length; i++){
                this.drawMetaField(fields[i], indent + tab, type);  
            }
            writer.write(indent + "<b>};</b>" + newLine + separator);
        }
    }
    
    /**
     * Creates a HTML representation of a typedef class.
     * 
     * @param field The 'typedeffed' class.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefClass(MetaClass field, String indent, MetaType type){
        writer.write(indent + "<b color=\"" + color1 + "\">class</b> <b color=\"" + color2 + "\">" + field.getName() + "</b> <b>{</b>" + newLine);
        MetaField[] fields = field.getFields();
    
        for(int i=0; i<fields.length; i++){
            this.drawMetaField(fields[i], indent + tab, type);  
        }
        writer.write(indent + "<b>}</b>" + separator);
    }
    
    /**
     * Creates a HTML representation of a collection.
     * 
     * @param field The collection to create the HTML representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawCollection(MetaCollection field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + "<font color=\"" + color4 + "\">" + typedefName + "</font> " + "<font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
        else{
            writer.write(indent + field.getTypeName().replaceAll("<", "&lt;").replaceAll(">", "&gt;") + " <font color=\"" + color3 + "\">" + field.getName() + "</font> " + ";" + newLine + separator);
        }
    }
    
    /**
     * Creates a HTML representation of a typedef collection.
     * 
     * @param field The 'typedeffed' collection.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefCollection(MetaCollection field, String indent, MetaType type){
        writer.write(indent + field.getTypeName().replaceAll("<", "&lt;").replaceAll(">", "&gt;"));
    }
    
    /**
     * Creates a HTML representation of a enumeration.
     * 
     * @param field The enumeration to create the HTML representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawEnum(MetaEnum field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + "<font color=\"" + color4 + "\">" + typedefName + "</font> " + "<font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
        else{
            writer.write(indent + "<b color=\"" + color1 + "\">enum</b> <b color=\"" + color2 + "\">" + field.getTypeName() + "</b> <b>{</b>" + newLine);
            String[] posValues = field.getPosValues();
            
            for(int i=0; i<posValues.length; i++){
                if(i != (posValues.length-1)){
                    writer.write(indent + tab + "<i color=\"gray\">" + posValues[i] + "</i>, " + newLine);
                }
                else{
                    writer.write(indent + tab + "<i color=\"gray\">" + posValues[i] + "</i>" + newLine);
                }
            }
            writer.write(indent + "<b>}</b> <font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
    }
    
    /**
     * Creates a HTML representation of a typedef enumeration.
     * 
     * @param field The 'typedeffed' enumeration.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefEnum(MetaEnum field, String indent, MetaType type){
        writer.write(indent + "<b color=\"" + color1 + "\">enum</b> <font color=\"" + color3 + "\">" + field.getTypeName() + "</font> <b>{</b>" + newLine);
        String[] posValues = field.getPosValues();
    
        for(int i=0; i<posValues.length; i++){
            if(i != (posValues.length-1)){
                writer.write(indent + tab + "<i color=\"gray\">" + posValues[i] + "</i>," + newLine);
            }
            else{
                writer.write(indent + tab + "<i color=\"gray\">" +posValues[i] + "</i>");
            }
        }
        writer.write(newLine + indent + "<b>}</b> ");
    }
    
    /**
     * Creates a HTML representation of a primitive.
     * 
     * @param field The primitive to create the HTML representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawPrimitive(MetaPrimitive field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + "<font color=\"" + color4 + "\">" + 
                    typedefName + "</font> " + "<font color=\"" + color3 +
                    "\">" + field.getName() + "</font>;" + newLine + separator);
        }
        else{
            writer.write(indent  + field.getTypeName() + " <font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
    }
    
    /**
     * Creates a HTML representation of a typedef primitive.
     * 
     * @param field The 'typedeffed' primitive.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefPrimitive(MetaPrimitive field, String indent, MetaType type){
        writer.write(indent + "<font color=\"" + color3 + "\">" + field.getTypeName() + "</font>");
    }
    
    private void drawStruct(MetaStruct field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + "<font color=\"" + color4 + "\">" + 
                    typedefName + "</font> " + "<font color=\"" + color3 +
                    "\">" + field.getName() + "</font>;" + newLine + separator);
        }
        else{
            writer.write(indent + "<b color=\"" + color1 + "\">struct</b> <b color=\"" + color2 + "\">" + field.getTypeName() + "</b> <b>{</b>" + newLine);
            MetaField[] fields = field.getFields();
            
            for(int i=0; i<fields.length; i++){
                this.drawMetaField(fields[i], indent + tab, type);
            }
            writer.write(indent + "<b>}</b> " + "<font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
    }
    
    /**
     * Creates a HTML representation of a typedef structure.
     * 
     * @param field The 'typedeffed' structure.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefStruct(MetaStruct field, String indent, MetaType type){
        writer.write(indent + "<b color=\"" + color1 + "\">struct</b> <font color=\"" + color3 + "\">" + field.getTypeName() + "</font> <b>{</b>" + newLine);
        MetaField[] fields = field.getFields();
    
        for(int i=0; i<fields.length; i++){
            this.drawMetaField(fields[i], indent + tab, type);
        }
        writer.write(indent + "<b>}</b>");
    }
    
    /**
     * Creates a HTML representation of a union.
     * 
     * @param field The union to create the HTML representation of.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawUnion(MetaUnion field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field);
        if(typedefName != null){
            writer.write(indent + "<font color=\"" + color4 + "\">" + 
                    typedefName + "</font> " + "<font color=\"" + color3 +
                    "\">" + field.getName() + "</font>;" + newLine + separator);
        }
        else{
            writer.write(indent + "<b color=\"" + color1 + "\">union</b> " + field.getTypeName() + 
                        " <b color=\"" + color2 + "\">switch</b>(" + field.getDiscriminator().getTypeName() + ") {" + newLine);
            MetaUnionCase[] cases = field.getCases();
            ArrayList labels;
            
            for(int i=0; i<cases.length; i++){
                labels = cases[i].getLabels();
                
                for(int j=0; j<labels.size(); j++){
                    if("".equals(labels.get(j))){
                        writer.write(indent + tab + "<b color=\"" + color1 + "\">default :</b>" + newLine);
                    } else { 
                        writer.write(indent + tab + "<b color=\"" + color1 + "\">case</b> " + labels.get(j) + "<b>:</b>" + newLine);
                    }
                }
                this.drawMetaField(cases[i].getField(), indent + tab + tab, type);
            }
                    
            writer.write(indent + "<b>}</b> " + "<font color=\"" + color3 + "\">" + field.getName() + "</font>;" + newLine + separator);
        }
    }
    
    /**
     * Creates a HTML representation of a typedef union.
     * 
     * @param field The 'typedeffed' union.
     * @param indent The indentation to draw.
     * @param type The complete type.
     */
    private void drawTypedefUnion(MetaUnion field, String indent, MetaType type){
        String typedefName = type.getFieldTypedefName(field.getDiscriminator());
        if(typedefName != null){
            writer.write(indent + "<b color=\"" + color1 + "\">union</b> <font color=\"" + color3 + "\">" + field.getTypeName() + "</font>" + 
                        " switch(" + typedefName + ") <b>{</b>" + newLine);
        }
        else{
            writer.write(indent + "<b color=\"" + color1 + "\">union</b> <font color=\"" + color3 + "\">" + field.getTypeName() + "</font>" + 
                        " switch(" + field.getDiscriminator().getTypeName() + ") <b>{</b>" + newLine);
        }
        MetaUnionCase[] cases = field.getCases();
        ArrayList labels;
        
        for(int i=0; i<cases.length; i++){
            labels = cases[i].getLabels();
            
            for(int j=0; j<labels.size(); j++){
                if("".equals(labels.get(j))){
                    writer.write(indent + tab + "<b color=\"" + color1 + "\">default :</b>" + newLine);
                } else {
                    writer.write(indent + tab + "<b color=\"" + color1 + "\">case</b> " + labels.get(j) + ":" + newLine);
                }
            }
            this.drawMetaField(cases[i].getField(), indent + tab + tab, type);
        }            
        writer.write(indent + "<b>}</b>");
    }
}
