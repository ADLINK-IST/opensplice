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

import java.util.ArrayList;

public class MetaElement extends MetaNode {
    private String name;
    private int minOccurrences;
    private int maxOccurrences;
    private boolean hidden;
    private ArrayList<MetaNode> children;

    public MetaElement(String doc, String name, int minOccurrences,
            int maxOccurrences, ArrayList<MetaNode> children, String version,
            String dimension, boolean hidden) {
        super(doc, version, dimension);
        this.name = name;
        this.minOccurrences = minOccurrences;
        this.maxOccurrences = maxOccurrences;
        this.children = children;
        this.hidden = hidden;
    }

    public boolean isHidden() {
        return this.hidden;
    }

    public int getMaxOccurrences() {
        return this.maxOccurrences;
    }

    public int getMinOccurrences() {
        return this.minOccurrences;
    }

    public String getName() {
        return this.name;
    }

    public boolean addChild(MetaNode child){
        return this.children.add(child);
    }

    public boolean removeChild(MetaNode child){
        return this.children.remove(child);
    }

    public MetaNode[] getChildren(){
        return this.children.toArray(new MetaNode[this.children.size()]);
    }

    @Override
    public boolean equals(Object object){
        boolean result;
        MetaElement me;

        if(object instanceof MetaElement){
            me = (MetaElement)object;

            if(this.name.equals(me.getName())){
                MetaNode[] meChildren = me.getChildren();
                MetaNode[] children = this.getChildren();

                if(this.maxOccurrences != me.getMaxOccurrences()){
                    result = false;
                } else if(this.minOccurrences != me.getMinOccurrences()){
                    result = false;
                } else if(meChildren.length != children.length){
                    result = false;
                } else {
                    result = true;

                    for(int i=0; i<children.length && result; i++){
                        if(!(meChildren[i].equals(children[i]))){
                            result = false;
                        }
                    }
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
        var_gen_code = minOccurrences;
        var_gen_code += maxOccurrences;
        var_gen_code += (null == children ? 0 : children.hashCode());
        var_gen_code += (null == name ? 0 : name.hashCode());
        var_gen_code += hidden ? 1 : 0;
        hash = 31 * hash + var_gen_code;
        return hash;
    }

    @Override
    public String toString(){
        StringBuffer buf = new StringBuffer();
        buf.append("\nElement\n");
        buf.append("-Name: " + this.name + "\n");
        buf.append("-Version: " + this.version + "\n");
        buf.append("-Hidden: " + this.hidden + "\n");
        buf.append("-MinOcccurrences: " + this.minOccurrences + "\n");
        buf.append("-MaxOcccurrences: " + this.maxOccurrences + "\n");
        if(this.children.size() > 0){
            buf.append("-Children: ");
            for(MetaNode child: children){
                buf.append(child.toString().replaceAll("\n", "\n\t"));
            }
        }
        return buf.toString();
    }

    public boolean hasElementChildren(){
        for(MetaNode mn: this.getChildren()){
            if(mn instanceof MetaElement){
                return true;
            }
        }
        return false;
    }

    public boolean hasValueChildren(){
        for(MetaNode mn: this.getChildren()){
            if(mn instanceof MetaValue){
                return true;
            }
        }
        return false;
    }
}
