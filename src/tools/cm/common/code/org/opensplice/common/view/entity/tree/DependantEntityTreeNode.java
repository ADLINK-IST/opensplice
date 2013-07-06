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
/**
 * Contains all SPLICE DDS C&M Tooling view tree components that hold Entity
 * information.
 */
package org.opensplice.common.view.entity.tree;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Partition;
import org.opensplice.cm.Topic;

/**
 * Represents a tree node in an EntityTree. It holds a specific Entity. The
 * Entity depends on the entities that are contained by the child nodes of
 * this tree node. This class is a concrete descendant of EntityTreeNode.
 * 
 * @date Oct 4, 2004
 */
public class DependantEntityTreeNode extends EntityTreeNode {

    /**
     * Creates a new DependantEntityTreeNode, which holds the supplied Entity
     * and is created in the supplied tree.
     * 
     * @param _e The Entity that the node needs to hold.
     * @param _tree The tree the node is located in.
     */
    public DependantEntityTreeNode(Entity _e, EntityTree _tree) {
        super(_e, _tree, _tree.getChildrenVisible());
    }
    
    /**
     * Creates a new DependantEntityTreeNode, which holds the supplied Entity
     * and is created in the supplied tree.
     * 
     * @param _e The Entity that the node needs to hold.
     * @param _tree The tree the node is located in.
     * @param childrenVisible Whether the children of this node should be resolved.
     */
    public DependantEntityTreeNode(Entity _e, EntityTree _tree, boolean childrenVisible) {
        super(_e, _tree, childrenVisible);
    }
    
    /**
     * Refreshes this node by resolving the dependant entities of the Entity
     * that is associated with this node. When an Entity is not available 
     * anymore it will result in the child node being removed. When an Entity
     * was found that is not already a child of this node, a new child node is
     * added to this node.
     * 
     * When the Entity associated with this node is not available anymore, the
     * parent node of this node will be refreshed.
     */
    public void refresh() {
        Entity entity = (Entity)userObject;
        Entity[] entities = null;
        boolean proceed;
        
        if(this.childrenVisible){
            Object rootNode = tree.getTreeModel().getRoot();
            
            /*
             * Determine whether to resolve dependant entities here. There is
             * a cyclomatic reference in the dependant entities. This is as
             * expected, but tooling should not display it.
             */
            if(this.getParent() instanceof RootTreeNode){
                proceed = true;
            } else if((entity instanceof Topic) && (rootNode instanceof TopicTreeNode)){
                proceed = false;
            } else if((entity instanceof Partition) && (rootNode instanceof PartitionTreeNode)){
                proceed = false;
            } else {
                proceed = true;
            }
            
            if(proceed){
                try {
                    entities = entity.getDependantEntities(EntityFilter.ENTITY);
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
                    DependantEntityTreeNode child = (DependantEntityTreeNode)(this.resolveChildNode(entities[i]));
                    
                    if(child != null){
                        entities[i].free();
                    } else {
                        child = new DependantEntityTreeNode(entities[i], tree, this.recursiveVisible);//tree.getChildrenVisible());
                        tree.addNode(child, this);
                    }
                    if(this.isExpanded()){
                        child.refresh();
                    }
                }
                this.removeUnavailableChildren(entities);
            }
        } else {
            this.removeChildren();
        }
    }
}
