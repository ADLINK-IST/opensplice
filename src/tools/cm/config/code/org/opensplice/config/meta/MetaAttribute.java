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

public class MetaAttribute extends MetaNode {
    private String name;
    private boolean required;
    private MetaValue value;
    
    public MetaAttribute(String doc, String name, boolean required, MetaValue value, String version) {
        super(doc,version);
        this.name = name;
        this.required = required;
        this.value = value;
    }
    
    public boolean isRequired(){
        return this.required;
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
        hash = 31 * hash + var_gen_code;
        return hash;
    }
    
    public String toString(){
        String result = "";
        result += "\nAttribute\n";
        result += "-Name: " + this.name + "\n";
        result += "-Required: " + this.required + "\n";
        result += "-Version: " + this.version + "\n";
        result += "-Value: " + value.toString();
        
        return result;
    }
}
