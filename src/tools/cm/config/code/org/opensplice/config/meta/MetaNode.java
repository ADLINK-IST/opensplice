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

import org.opensplice.common.util.ConfigModeIntializer;

public abstract class MetaNode {
    String doc;
    String version;
    
    public MetaNode(String doc, String version){
        this.doc = doc;
        this.version = version;
    }
    
    public MetaNode(String doc){
        this(doc,ConfigModeIntializer.COMMUNITY);
    }

    public String getDoc() {
        return doc;
    }

    public void setDoc(String doc) {
        this.doc = doc;
    }
    
    public String getVersion() {
        return this.version;
    }
}
