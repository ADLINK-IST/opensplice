/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
package org.opensplice.config.meta;

public abstract class MetaValueNatural extends MetaValue {
    Object maxValue;
    Object minValue;
    
    public MetaValueNatural(String doc, Object defaultValue, Object maxValue,
            Object minValue, String dimension) {
        super(doc, defaultValue, dimension);
        this.minValue = minValue;
        this.maxValue = maxValue;
    }

    public Object getMaxValue() {
        return this.maxValue;
    }

    public Object getMinValue() {
        return this.minValue;
    }
    
    @Override
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
    
    @Override
    public String toString(){
        String result = super.toString();
        
        result += ", MaxValue: " + maxValue.toString() + ", MinValue: " + minValue.toString();
        
        return result;
    }
}
