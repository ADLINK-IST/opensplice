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
package org.opensplice.config.meta;

import org.opensplice.common.util.ConfigModeIntializer;

public abstract class MetaValue extends MetaNode {
    Object defaultValue;
    
    public MetaValue(String doc, Object defaultValue, String dimension) {
        super(doc, ConfigModeIntializer.COMMUNITY, dimension);
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
