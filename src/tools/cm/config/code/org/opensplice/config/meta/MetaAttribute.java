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

public class MetaAttribute extends MetaNode {
    private String name;
    private boolean required;
    private MetaValue value;
    private boolean hidden;

    public MetaAttribute(String doc, String name, boolean required,
            MetaValue value, String version, String dimension, boolean hidden) {
        super(doc, version, dimension);
        this.name = name;
        this.required = required;
        this.value = value;
        this.hidden = hidden;
    }

    public boolean isRequired(){
        return this.required;
    }

    public boolean isHidden(){
        return this.hidden;
    }

    public String getName() {
        return this.name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public MetaValue getValue() {
        return this.value;
    }

    public void setValue(MetaValue value) {
        this.value = value;
    }

    @Override
    public boolean equals(Object object){
        MetaAttribute ma;
        boolean result;

        if(object instanceof MetaAttribute){
            ma = (MetaAttribute)object;

            if(this.name.equals(ma.getName())){
                if(this.required == ma.isRequired()){
                    if(this.value.equals(ma.getValue())){
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
        } else {
            result = false;
        }
        return result;
    }

    @Override
    public int hashCode() {
        int var_gen_code;
        int hash = 13;
        var_gen_code = required ? 1 : 0;
        var_gen_code += (null == value ? 0 : value.hashCode());
        var_gen_code += (null == name ? 0 : name.hashCode());
        var_gen_code += hidden ? 1 : 0;
        hash = 31 * hash + var_gen_code;
        return hash;
    }

    @Override
    public String toString(){
        String result = "";
        result += "\nAttribute\n";
        result += "-Name: " + this.name + "\n";
        result += "-Required: " + this.required + "\n";
        result += "-Version: " + this.version + "\n";
        result += "-Hidden: " + this.hidden + "\n";
        result += "-Value: " + value.toString();
        return result;
    }
}
