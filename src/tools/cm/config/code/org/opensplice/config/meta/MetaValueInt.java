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

public class MetaValueInt extends MetaValueNatural {

    public MetaValueInt(String doc, Integer defaultValue, Integer maxValue, Integer minValue) {
        super(doc, defaultValue, maxValue, minValue);
    }

    public boolean setMaxValue(Object maxValue) {
        boolean result;
        
        if(maxValue instanceof Integer){
            this.maxValue = maxValue;
            result = true;
        } else {
            result = false;
        }
        return result;
    }

    public boolean setMinValue(Object minValue) {
        boolean result;
        
        if(minValue instanceof Integer){
            this.minValue = minValue;
            result = true;
        } else {
            result = false;
        }
        return result;
    }

    public boolean setDefaultValue(Object defaultValue) {
        boolean result;
        
        if(defaultValue instanceof Integer){
            this.defaultValue = defaultValue;
            result = true;
        } else {
            result = false;
        }
        return result;
    }
}
