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
package org.opensplice.config.meta;

public class MetaValueString extends MetaValue {
    private int maxLength;
    
    public MetaValueString(String doc, String defaultValue, int maxLength) {
        super(doc, defaultValue);
        this.maxLength = maxLength;
    }

    public int getMaxLength() {
        return this.maxLength;
    }

    public void setMaxLength(int maxLength) {
        this.maxLength = maxLength;
    }

    public boolean setDefaultValue(Object defaultValue) {
        boolean result = false;
        
        if(defaultValue instanceof String){
            this.defaultValue = defaultValue;
            result = true;
        }
        return result;
    }
}
