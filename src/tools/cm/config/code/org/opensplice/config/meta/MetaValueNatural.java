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

public abstract class MetaValueNatural extends MetaValue {
    Object maxValue;
    Object minValue;
    
    public MetaValueNatural(String doc, Object defaultValue, Object maxValue, Object minValue) {
        super(doc, defaultValue);
        this.minValue = minValue;
        this.maxValue = maxValue;
    }

    public Object getMaxValue() {
        return this.maxValue;
    }

    public Object getMinValue() {
        return this.minValue;
    }
    
    public boolean equals(Object object){
        MetaValueNatural mn;
        boolean result = super.equals(object);
        
        if(result){
            if(object instanceof MetaValueNatural){
                mn = (MetaValueNatural)object;
                
                if((mn.getMaxValue() == null) && (this.maxValue != null)){
                    result = false;
                } else if((mn.getMaxValue() != null) && (this.maxValue == null)){
                    result = false;
                } else if( ((mn.getMaxValue() == null) && (this.maxValue == null)) ||
                           ((mn.getMaxValue().equals(this.maxValue))))
                {
                    if((mn.getMinValue() == null) && (this.minValue != null)){
                        result = false;
                    } else if((mn.getMinValue() != null) && (this.minValue == null)){
                        result = false;
                    } else if( ((mn.getMinValue() == null) && (this.minValue == null)) ||
                               ((mn.getMinValue().equals(this.minValue))))
                    {
                        result = true;
                    } else {
                        result = false;
                    }
                } else {
                    result = false;
                }
            } else {
                result = false;
            }
        }
        return result;
    }
    
    @Override
    public int hashCode() {
        int var_gen_code;
        int hash = 13;
        var_gen_code = (null == maxValue ? 0 : maxValue.hashCode());
        var_gen_code += (null == minValue ? 0 : minValue.hashCode());
        hash = 31 * hash + var_gen_code;
        return hash;
    }
    
    public abstract boolean setMaxValue(Object maxValue);
    
    public abstract boolean setMinValue(Object minValue);
    
    public String toString(){
        String result = super.toString();
        
        result += ", MaxValue: " + maxValue.toString() + ", MinValue: " + minValue.toString();
        
        return result;
    }
}
