/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
    private final String[] posValues;
    
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
  
    @Override
    public String toString(){
        StringBuffer buf = new StringBuffer();
        buf.append(super.toString());
        buf.append(" {");
        for (int i = 0; i < posValues.length; i++) {
            if(i != 0){
                buf.append(", " + posValues[i]);
            } else {
                buf.append(posValues[i]);
            }
        }
        buf.append("}");
        return buf.toString();
    }
}

