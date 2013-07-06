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

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;

import org.opensplice.cm.Participant;
import org.opensplice.common.CommonException;

/** 
 * Abstract implementation of a EntityTreeNode that represents a root node in
 * an EntityTree. It holds the Participant that is used to resolve entities in
 * the system and is always expanded, however it is not visible for the user.
 * 
 * @date Sep 28, 2004
 */
public abstract class RootTreeNode extends EntityTreeNode{

    /**
     * Creates a new RootTreeNode that holds the supplied Participant and 
     * is the root node of the supplied tree.
     * 
     * @param _e The Participant that is held by the node. 
     * @param _tree The EntityTree the node is located in.
     */
    public RootTreeNode(Participant _e, EntityTree _tree) {
        super(_e, _tree, true);
    }
    
    /**
     * Provides access to the Participant that is being held by this node.
     *  
     * @return The Participant of this node.
     */
    public Participant getParticipant(){
        return (Participant)userObject;
    }
    
    /**
     * Overrides the remove function in the EntityTreeNode class and removes all
     * children of this node and frees the participant. After the Participant
     * is freed a notification is sent to the ModelRegister component of the 
     * EntityTree (entity_tree_freed).
     */
    public void remove(){
        int childCount = this.getChildCount();
        
        for(int i=0; i<childCount; i++){
            ((EntityTreeNode)this.getChildAt(0)).remove();
        }
        tree.clear();
        tree.getParticipant().free();
        tree.fireTreeChanged("freed");
    }
    
    /**
     * Sets the visibility of the children of the children of this node. 
     * @throws CommonException
     */
    public void setChildrenVisible(boolean visible, boolean recursive) throws CommonException{
        for(int i=0; i<this.getChildCount(); i++){
            ((EntityTreeNode)this.getChildAt(i)).setChildrenVisible(visible, recursive);
        }
    }
    
    /**
     * Expands the node.
     */
    protected void expand(){
        if(!(this.isExpanded())){
            if(this.getChildCount() > 0){
                tree.scrollPathToVisible(
                        new TreePath(((DefaultMutableTreeNode)(this.getChildAt(0))).getPath()));
            }
        }
    }
}
