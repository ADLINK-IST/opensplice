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
package org.opensplice.common.view.entity.tree;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.Participant;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of a RootTreeNode. It responsibility is to keep track
 * of the Participant and all the Participant objects that are available in the 
 * same kernel the Participant participates in.
 *  
 * @date Sep 28, 2004
 */
public class ParticipantTreeNode extends RootTreeNode{
    /**
     * Creates a new ParticipantTreeNode by resolving all Participant entities
     * and displaying them as children in the tree.
     * 
     * @param _e The Participant that participates in the same kernel as the
     *           participants that need to be displayed.
     * @param _tree The tree the node is the root node of.
     */
    public ParticipantTreeNode(Participant _e, EntityTree _tree) {
        super(_e, _tree);
    }
    
    /**
     * Refreshes this node and all of its expanded descendants. This is realized
     * by resolving all Participant entities and their owned entities. Entities 
     * that are no longer available, will be removed from the tree and new
     * entities are added to the tree.
     * @throws CommonException Thrown when the node could not be refreshed.
     */
    public void refresh() throws CommonException {
        Participant participant = (Participant)userObject;
        Entity[] entities = null;
        
        try {
            entities = participant.resolveAllParticipants();
        } catch (CMException e) {
            this.remove();
            throw new CommonException("Entity tree could not be refreshed.");
        }
        int childCount = entities.length;
        
        for(int i=0; i<childCount; i++){
            OwnedEntityTreeNode child = (OwnedEntityTreeNode)(this.resolveChildNode(entities[i]));
            
            if(child != null){
                entities[i].free();
            } else {
                child = new OwnedEntityTreeNode(entities[i], tree);
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
