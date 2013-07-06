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


public abstract class MetaValue extends MetaNode {
    Object defaultValue;
    
    public MetaValue(String doc, Object defaultValue) {
        super(doc);
        this.defaultValue = defaultValue;
        
    }

    public Object getDefaultValue() {
        return this.defaultValue;
    }

    public abstract boolean setDefaultValue(Object defaultValue); 
    
    @Override
    public String toString(){
        return "Value (" + defaultValue.getClass().toString().substring(defaultValue.getClass().toString().lastIndexOf('.') + 1) + ") DefaultValue: " + defaultValue.toString();
    }
    
    @Override
    public boolean equals(Object object){
        boolean result;
        MetaValue mv;
        
        if(object instanceof MetaValue){
            mv = (MetaValue)object;
            if((this.defaultValue == null) || (mv.getDefaultValue() == null)){
                if(this.defaultValue != mv.getDefaultValue()){
                    result = false;
                } else {
                    result = true;
                }
            } else if(this.defaultValue.equals(mv.getDefaultValue())){
                result = true;
            } else {
                result = false;
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
        hash = 31 * hash + var_gen_code;
        return hash;
    }
}
