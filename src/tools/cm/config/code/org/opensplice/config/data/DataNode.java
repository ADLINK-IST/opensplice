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
package org.opensplice.config.data;

import java.util.HashSet;

import org.opensplice.config.meta.MetaNode;
import org.w3c.dom.Node;

public abstract class DataNode {
    protected MetaNode metadata;
    protected Node node;
    protected DataNode parent;
    protected DataConfiguration owner;
    private HashSet<DataNode>   dependencies = null;
    
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

    public void addDependency(DataNode dv) {
        if (dependencies == null) {
            dependencies = new HashSet<DataNode>();
        }
        dependencies.add(dv);
    }

    public HashSet<DataNode> getDependencies() {
        return dependencies;
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
