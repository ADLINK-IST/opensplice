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
package org.opensplice.config.swing;

import org.opensplice.config.data.DataAttribute;
import org.opensplice.config.data.DataElement;
import org.opensplice.config.data.DataNode;
import org.opensplice.config.meta.MetaAttribute;
import org.opensplice.config.meta.MetaElement;
import org.opensplice.config.meta.MetaNode;

public class ConfigUtil {
    public static String getExtendedDataElementString(DataElement element){
        String firstAttrChild;
        DataNode[] children;
        String name;
        String result = null;
        
        if(element != null){
            children = element.getChildren();
            firstAttrChild = null;
            
            for(int j=0; (j<children.length) && (firstAttrChild == null); j++){
                if(children[j] instanceof DataAttribute){
                    name = ((MetaAttribute)children[j].getMetadata()).getName();
                    
                    if("name".equalsIgnoreCase(name)){
                        firstAttrChild = "[" + ((MetaAttribute)children[j].getMetadata()).getName() + "=";
                        firstAttrChild += ((DataAttribute)children[j]).getValue() + "]";
                    }
                }
            }
            
            for(int j=0; (j<children.length) && (firstAttrChild == null); j++){
                if(children[j] instanceof DataAttribute){
                    firstAttrChild = "[" + ((MetaAttribute)children[j].getMetadata()).getName() + "=";
                    firstAttrChild += ((DataAttribute)children[j]).getValue() + "]";
                }
            }
            if(firstAttrChild == null){
                firstAttrChild = "";
            }
            result = ((MetaElement)element.getMetadata()).getName() + firstAttrChild; 
        }
        return result;
    }
    
    public static String getDataElementString(DataElement element){
        String result = null;
        
        if(element != null){
            result = ((MetaElement)element.getMetadata()).getName();
        }
        return result;
    }
    
    public static String getExtendedMetaElementString(MetaElement element){
        String firstAttrChild;
        MetaNode[] children;
        String result = null;
        
        if(element != null){
            children = element.getChildren();
            firstAttrChild = null;
            
            for(int j=0; (j<children.length) && (firstAttrChild == null); j++){
                if(children[j] instanceof MetaAttribute){
                    firstAttrChild = "[" + ((MetaAttribute)children[j]).getName() + "]";
                }
            }
            if(firstAttrChild == null){
                firstAttrChild = "";
            }
            result = element.getName() + firstAttrChild; 
        }
        return result;
    }
    
    public static String getMetaElementString(MetaElement element){
        String result = null;
        
        if(element != null){
            result = element.getName();
        }
        return result;
    }
}
