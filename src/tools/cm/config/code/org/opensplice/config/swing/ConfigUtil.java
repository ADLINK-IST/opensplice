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
