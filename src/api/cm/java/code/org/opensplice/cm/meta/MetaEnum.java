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

/**
 * Represents an enumeration field in a Splice database type.
 * 
 * @date May 24, 2004
 */
public class MetaEnum extends MetaField{
    /**
     * Array of possible values of the enumeration.
     */
    private String[] posValues;
    
    /**
     * Constructs a new enumeration field.
     * 
     * @param _name The name of the enumeration
     * @param _typeName The type name of the enumeration.
     * @param _posValues The array of possible values of the enumeration.
     */
    public MetaEnum(String _name, String _typeName, String[] _posValues) {
        super(_name, _typeName);
        posValues = _posValues;
    }
    
    /**
     * Provides access to the possible values of the enumeration.
     * 
     * @return The array of possible enumeration values.
     */
    public String[] getPosValues(){
        return posValues;
    }
    
    /**
     * Validates whether the supplied value is a possible value for this
     * enumeration.
     * 
     * @param value The value to validate.
     * @return true if it is a possible value, false otherwise.
     */
    public boolean validateValue(String val){
        for(int i=0; i<posValues.length; i++){
            if(posValues[i].equals(val)){
                return true;
            }
        }
        return false;
    }
  
    public String toString(){
        String s = super.toString();
        
        s += " {";
        
        for(int i=0; i<posValues.length; i++){
            if(i != 0){
                s+= ", " + posValues[i];
            } else{
                s+= posValues[i];
            }
        }
        s+= "}";
        return s;
    }
}

