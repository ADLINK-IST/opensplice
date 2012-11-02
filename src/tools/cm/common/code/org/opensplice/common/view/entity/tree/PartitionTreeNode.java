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
package org.opensplice.common.view.entity.tree;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.Participant;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of a RootTreeNode. It responsibility is to keep track
 * of the Participant and all the Domain objects that are available in the same
 * kernel the Participant participates in.
 *  
 * @date Oct 5, 2004
 */
public class PartitionTreeNode extends RootTreeNode {

    /**
     * Creates a new PartitionTreeNode by resolving all Domain entities and 
     * displaying them as children in the tree.
     * 
     * @param _e The Participant that participates in the same kernel as the
     *           domains that need to be displayed.
     * @param _tree The tree the node is the root node of.
     */
    public PartitionTreeNode(Participant _e, EntityTree _tree) {
        super(_e, _tree);
    }

    /**
     * Refreshes this node and all of its expanded descendants. This is realized
     * by resolving all Domain entities and their dependant entities. Entities
     * that are no longer available, will be removed from the tree and new
     * entities are added to the tree.
     * 
     * @throws CommonException Thrown when the node could not be refreshed.
     */
    public void refresh() throws CommonException{
        Participant participant = (Participant)userObject;
        Entity[] entities = null;
        
        try {
            entities = participant.resolveAllDomains();
        } catch (CMException e) {
            System.out.println(e.getMessage());
            this.remove();
            throw new CommonException("Entity tree could not be refreshed.");
        }
        int childCount = entities.length;
        
        for(int i=0; i<childCount; i++){
            DependantEntityTreeNode child = (DependantEntityTreeNode)(this.resolveChildNode(entities[i]));
            
            if(child != null){
                entities[i].free();
            } else {
                child = new DependantEntityTreeNode(entities[i], tree);
                tree.addNode(child, this);
            }
            if(this.isExpanded()){
                child.refresh();
            } else{
                this.expand();
            }
        }
        this.removeUnavailableChildren(entities);
    }
}
