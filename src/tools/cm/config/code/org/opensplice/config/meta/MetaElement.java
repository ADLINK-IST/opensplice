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

import java.util.ArrayList;

public class MetaElement extends MetaNode {
    private String name;
    private int minOccurrences;
    private int maxOccurrences;
    private ArrayList<MetaNode> children;
    
    public MetaElement(String doc, String name, int minOccurrences, int maxOccurrences, ArrayList<MetaNode> children, String version) {
        super(doc,version);
        this.name = name;
        this.minOccurrences = minOccurrences;
        this.maxOccurrences = maxOccurrences;
        this.children = children;
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
        hash = 31 * hash + var_gen_code;
        return hash;
    }
    
    public String toString(){
        StringBuffer buf = new StringBuffer();
        buf.append("\nElement\n");
        buf.append("-Name: " + this.name + "\n");
        buf.append("-Version: " + this.version + "\n");
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
