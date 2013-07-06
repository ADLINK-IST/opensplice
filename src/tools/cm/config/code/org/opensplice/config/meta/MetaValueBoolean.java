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

public class MetaValueBoolean extends MetaValue {

    public MetaValueBoolean(String doc, Boolean defaultValue) {
        super(doc, defaultValue);
    }
    
    public boolean setDefaultValue(Object defaultValue) {
        boolean result = false;
        
        if(defaultValue instanceof Boolean){
            this.defaultValue = defaultValue;
            result = true;
        }
        return result;
    }
}
