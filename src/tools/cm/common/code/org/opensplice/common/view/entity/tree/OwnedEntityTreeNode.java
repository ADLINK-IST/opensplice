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
package org.opensplice.common.view.entity.tree;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;

/** 
 * Concrete implementation of an EntityTreeNode, which children contain entities
 * that are owned by the Entity that is associated to this node.
 * 
 * @date Sep 24, 2004
 */
public class OwnedEntityTreeNode extends EntityTreeNode {
    
    /**
     * Creates a OwnedEntityTreeNode which holds the supplied entity and is
     * placed in the supplied tree.
     *
     * @param _e The Entity to attach to this node.
     * @param _tree The EntityTree to place the node in.
     */
    public OwnedEntityTreeNode(Entity _e, EntityTree _tree){
        super(_e, _tree, _tree.getChildrenVisible());
    }
    
    /**
     * Creates a OwnedEntityTreeNode which holds the supplied entity and is
     * placed in the supplied tree.
     *
     * @param _e The Entity to attach to this node.
     * @param _tree The EntityTree to place the node in.
     * @param childrenVisible Whether the children of this node should be resolved.
     */
    public OwnedEntityTreeNode(Entity _e, EntityTree _tree, boolean childrenVisible){
        super(_e, _tree, childrenVisible);
    }
    
    /**
     * Refreshes this node by resolving the owned entities of the Entity
     * that is associated with this node. When an Entity is not available 
     * anymore it will result in the child node being removed. When an Entity
     * was found that is not already a child of this node, a new child node is
     * added to this node.
     * 
     * When the Entity associated with this node is not available anymore, the
     * parent node of this node will be refreshed.
     */
    @Override
    public void refresh(){
        Entity entity = (Entity)userObject;
        Entity[] entities = null;
        
        if(this.childrenVisible){
            try {
                entities = entity.getOwnedEntities(EntityFilter.ENTITY);
            } catch (CMException e) {
                if(CMException.CONNECTION_LOST.equals(e.getMessage())){
                    tree.rootNode.remove();
                } else {
                    this.removeForced();
                }
                /*
                 * Don't refresh the parent, because it potentially makes the
                 * application get into a deadlock, when the entity of this node
                 * is being created and deleted in the splice kernel in the same 
                 * frequency as the refresh action.
                 */
                //p.refresh();
                return;
            }
            int childCount = entities.length;
            
            for(int i=0; i<childCount; i++){
                OwnedEntityTreeNode child = (OwnedEntityTreeNode)(this.resolveChildNode(entities[i]));
                
                if(child != null){
                    entities[i].free();
                } else {
                    child = new OwnedEntityTreeNode(entities[i], tree, this.recursiveVisible); //tree.getChildrenVisible());
                    tree.addNode(child, this);
                }
                if(this.isExpanded()){
                    child.refresh();
                }
            }
            this.removeUnavailableChildren(entities);
        } else {
            this.removeChildren();
        }
    }
    
}
