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
package org.opensplice.common.controller;

import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.tree.ExpandVetoException;

import org.opensplice.common.CommonException;
import org.opensplice.common.view.entity.tree.EntityTree;
import org.opensplice.common.view.entity.tree.EntityTreeNode;

/**
 * Provides a listener that listens for collapse and expansion events in 
 * a EntityTree object. Its responsibility is to refresh the expanded tree
 * nodes and its expanded descendants.
 * 
 * @date Sep 1, 2004
 */
public class EntityTreeWillExpandListener implements TreeWillExpandListener {
    /**
     * Creates a new listener that listens for expansions and collapses in the
     * associated entity tree.
     * 
     * @param _tree The tree, which events are being handled by this listener.
     */
    public EntityTreeWillExpandListener(EntityTree _tree){
        tree = _tree;
        logger = Logger.getLogger("org.opensplice.common.model");
    }
    
    /**
     * Sets the expanded state of the tree node that collapsed to false.
     * This function is called before a tree node collapses in the attached 
     * EntityTree.
     * 
     * @param event The event that has occurred.
     * @throws ExpandVetoException Thrown when the node may not be collapsed.
     */
    @Override
    public void treeWillCollapse(TreeExpansionEvent event)
            throws ExpandVetoException {
        EntityTreeNode node = ((EntityTreeNode)(event.getPath().getLastPathComponent()));
        logger.logp(Level.FINEST, "EntityTreeWillExpandListener", "treeWillCollapse", 
                "Collapse event. " + node.getUserObject());
        node.setExpanded(false);
        //tree.refresh(node);
    }
    
    
    /**
     * Sets the expanded state of the tree node that expanded to true and 
     * refreshes the node and its expanded children. This function is called 
     * before a tree node expands in the attached EntityTree.
     * 
     * @param event The event that has occurred.
     * @throws ExpandVetoException Thrown when the node may not be expanded.
     */
    @Override
    public void treeWillExpand(TreeExpansionEvent event) throws ExpandVetoException {
        
        EntityTreeNode node = ((EntityTreeNode)(event.getPath().getLastPathComponent()));
        int childCount = node.getChildCount();
        logger.logp(Level.FINEST, "EntityTreeWillExpandListener", "treeWillExpand", 
                "Expansion event. " + node.getUserObject() + " childCount: " + childCount);
        node.setExpanded(true);
        try {
            tree.refresh(node);
        } catch (CommonException ce) {
            logger.logp(Level.FINEST, "EntityTreeWillExpandListener", "treeWillExpand", "Update tree exception. "
                    + node.getUserObject() + " childCount: " + childCount);
        }
    }
    
    /**
     * The tree, which collapse/expansion events are being handled by this
     * listener
     */
    private final EntityTree tree;
    
    /**
     * Logging facilities
     */
    private final Logger     logger;
}
