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

import javax.swing.JMenuItem;

import org.opensplice.config.data.DataNode;
import org.opensplice.config.meta.MetaNode;

public class DataNodeMenuItem extends JMenuItem {
    private static final long serialVersionUID = -5909316541978936911L;
    private DataNode data;
    private MetaNode childMeta;
    private DataNodePopupSupport source;
    
    public DataNodeMenuItem(String name, DataNode data, MetaNode childMeta){
        super(name);
        this.data = data;
        this.childMeta = childMeta;
        this.source = null;
    }
    
    public DataNodeMenuItem(String name, DataNode data, MetaNode childMeta, DataNodePopupSupport source){
        super(name);
        this.data = data;
        this.childMeta = childMeta;
        this.source = source;
    }
    
    public DataNode getData(){
        return this.data;
    }
    
    public MetaNode getChildMeta(){
        return this.childMeta;
    }
    
    public DataNodePopupSupport getSource(){
        return this.source;
    }
}
