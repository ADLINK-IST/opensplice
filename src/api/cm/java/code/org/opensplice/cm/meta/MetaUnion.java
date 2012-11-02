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

import java.util.ArrayList;

/**
 * Represents a union field in a Splice database type.
 * 
 * @date Jun 10, 2004
 */
public class MetaUnion extends MetaField{
    /**
     * Constructs a new union field.
     * 
     * @param name The name of the union.
     * @param typeName The type name of the union.
     * @param _discriminator The discriminator of the union.
     * @param _cases The cases within the union.
     */
    public MetaUnion(String name, String typeName, MetaField _discriminator, ArrayList _cases){
        super(name, typeName);
        discriminator = _discriminator;
        cases = _cases;
    }
    
    /**
     * Provides access to the discriminator of the union.
     * 
     * @return The discriminator field of the union.
     */
    public MetaField getDiscriminator(){
        return discriminator;
    }
    
    /**
     * Provides access to all cases within the union.
     * 
     * @return The array of cases within the union.
     */
    public MetaUnionCase[] getCases(){
        MetaUnionCase[] result = (MetaUnionCase[])(cases.toArray(new MetaUnionCase[1]));
        
        return result;
    }
    
    /**
     * Provides access to the union case associated with the supplied
     * discriminator value.
     * 
     * @param discriminatorValue The value of the discriminator.
     * @return The union case that is associated with the supplied discriminator
     *         value.
     */

    public MetaUnionCase getCase(String discriminatorValue){
        MetaUnionCase labelCase;
        String realValue = null;
        
        if(discriminatorValue == null){
            realValue = "0";
        }
        else if(discriminator instanceof MetaEnum){
        /*
            String[] posValues = ((MetaEnum)discriminator).getPosValues();
            String result;
            
            for(int i=0; i<posValues.length; i++){
                result = posValues[i];
                
                if(result.equals(discriminatorValue)){
                    realValue = Integer.toString(i);
                }
            }
        */
            realValue = discriminatorValue;
        }
        else if(discriminator instanceof MetaPrimitive){
            realValue = discriminatorValue;
        }
        else{
            realValue = "0";
        }
        
        for(int i=0; i<cases.size(); i++){
            labelCase = (MetaUnionCase)(cases.get(i));
            
            if(labelCase.containsLabel(realValue)){
                return labelCase;
            }
        }
        return (MetaUnionCase)(cases.get(cases.size()-1));   
    }
    
    public MetaField[] getFields(){
        MetaField[] result = new MetaField[cases.size() + 1];
        
        result[0] = discriminator;
         
        for(int i=0; i<cases.size(); i++){
            result[i+1] = ((MetaUnionCase)(cases.get(i))).getField();
        }
        return result;
    }
    
    public String toString(){
        String s = super.toString();
        s += "\ndiscriminator: " + discriminator.toString();
        
        for(int i=0; i<cases.size(); i++){
            s += "\ncase: " + cases.get(i).toString(); 
        }
        return s;
    }
    
    public MetaField getField(String fieldName){
        MetaUnionCase result;
        
        for(int i=0; i<cases.size(); i++){
            result = (MetaUnionCase)(cases.get(i));
        
            if(fieldName.equals(result.getName())){
                return result.getField();
            }
        }
        if("switch".equals(fieldName)){
            return discriminator;
        }
        return null;
    }
    
    public ArrayList getFieldNames(){
        MetaField field;
        ArrayList result = new ArrayList();
        ArrayList names;
        
        result.add(name + ".switch");
        
        for(int i=0; i<cases.size(); i++){
            field = (MetaField)(cases.get(i));
        
            names = field.getFieldNames();
        
            for(int j=0; j<names.size(); j++){
                result.add(name + "." + ((String)names.get(j)));
            }
        }
        return result;
    }
    
    public boolean labelExists(String label){
        boolean result;
        MetaUnionCase labelCase;
        
        result = false;
        
        if(label != null){
            for(int i=0; (i<cases.size()) && (!result); i++){
                labelCase = (MetaUnionCase)(cases.get(i));
                
                if(labelCase.containsLabel(label)){
                    result = true;
                } else if(labelCase.containsLabel("")){ /*default label*/
                    result = true;
                }
            }
        }
        return result;
    }
    
    /**
     * The discriminator of the union.
     */
    private MetaField discriminator;
    
    /**
     * The cases of the union.(<MetaUnionCase>)
     * 
     */
    private ArrayList cases;
}
