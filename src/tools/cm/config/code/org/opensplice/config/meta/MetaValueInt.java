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

public class MetaValueInt extends MetaValueNatural {

    public MetaValueInt(String doc, Integer defaultValue, Integer maxValue,
            Integer minValue, String dimension) {
        super(doc, defaultValue, maxValue, minValue, dimension);
    }

    @Override
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

    @Override
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

    @Override
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
