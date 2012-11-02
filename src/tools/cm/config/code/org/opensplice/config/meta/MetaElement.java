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

import java.util.ArrayList;

public class MetaElement extends MetaNode {
    private String name;
    private int minOccurrences;
    private int maxOccurrences;
    private ArrayList<MetaNode> children;
    
    public MetaElement(String doc, String name, int minOccurrences, int maxOccurrences, ArrayList<MetaNode> children) {
        super(doc);
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
    
    public String toString(){
        String result = "";
        result += "\nElement\n";
        result += "-Name: " + this.name + "\n";
        result += "-MinOcccurrences: " + this.minOccurrences + "\n";
        result += "-MaxOcccurrences: " + this.maxOccurrences + "\n";
        
        if(this.children.size() > 0){
            result += "-Children: ";
            
            for(MetaNode child: children){
                result += child.toString().replaceAll("\n", "\n\t");
            }
        }
        return result;
        
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
