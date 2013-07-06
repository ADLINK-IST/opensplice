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
package org.opensplice.config.meta;

import java.util.ArrayList;

public class MetaValueEnum extends MetaValue {
    private ArrayList<String> posValues;
    
    public MetaValueEnum(String doc, String defaultValue, ArrayList<String> posValues) {
        super(doc, defaultValue);
        assert(defaultValue != null);
        
        this.posValues = new ArrayList<String>();
        
        if(posValues != null){
            this.posValues.addAll(posValues);
        }
        
        if(!this.posValues.contains(defaultValue)){
            this.posValues.add(defaultValue);
        }
    }

    public String[] getPosValues() {
        return this.posValues.toArray(new String[this.posValues.size()]);
    }

    public boolean setPosValues(ArrayList<String> posValues) {
        boolean result;
        
        if((posValues != null) && (posValues.size() > 0)){
            this.posValues.clear();
            this.posValues.addAll(posValues);
            
            if(!posValues.contains(defaultValue)){
                this.defaultValue = this.posValues.get(0);
            }
            result = true;
        } else {
            result = false;
        }
        return result;
    }
    
    public boolean addPosValue(String value){
        return this.posValues.add(value);
    }

    public boolean removePosValue(String value){
        boolean result;
        
        if(!defaultValue.equals(value)){
            result = this.posValues.remove(value);
        } else {
            result = false;
        }
        return result;
    }
    
    public boolean setDefaultValue(Object defaultValue) {
        boolean result = false;
        
        if(defaultValue instanceof String){
            this.defaultValue = defaultValue;
            
            if(!this.posValues.contains(defaultValue)){
                result = this.addPosValue((String)defaultValue);
            }
        } else {
            result = false;
        }
        return result;
    }
    
    @Override
    public int hashCode() {
        int var_gen_code;
        int hash = 13;
        var_gen_code = (null == defaultValue ? 0 : defaultValue.hashCode());
        var_gen_code += (null == posValues ? 0 : posValues.hashCode());
        hash = 31 * hash + var_gen_code;
        return hash;
    }
}
