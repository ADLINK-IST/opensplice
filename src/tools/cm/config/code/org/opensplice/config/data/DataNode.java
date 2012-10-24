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
package org.opensplice.config.data;

import org.w3c.dom.Node;

import org.opensplice.config.meta.MetaNode;

public abstract class DataNode {
    protected MetaNode metadata;
    protected Node node;
    protected DataNode parent;
    protected DataConfiguration owner;
    
    public DataNode(MetaNode metadata, Node node) throws DataException {
        if(metadata == null){
            throw new DataException("Invalid metadata.");
        } else if(node == null){
            throw new DataException("Invalid data.");
        }
        this.metadata = metadata;
        this.node     = node;
        this.parent   = null;
        this.owner    = null;
    }

    public MetaNode getMetadata() {
        return this.metadata;
    }

    public Node getNode() {
        return this.node;
    }

    public DataConfiguration getOwner() {
        return this.owner;
    }

    public void setOwner(DataConfiguration owner) {
        this.owner = owner;
    }
    
    protected void setParent(DataNode node){
        this.parent = node;
    }
    
    public DataNode getParent(){
        return this.parent;
    }
    
}
