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
package org.opensplice.common.controller;

import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeExpansionListener;

/**  
 * Listens for tree expansions and collapses in the tree where it has been
 * attached to.
 *
 * @date Sep 22, 2004
 */
public class EntityTreeExpansionListener implements TreeExpansionListener {
    
    /**
     * Called when a tree node has collapsed.
     * 
     * @param event The event that occurred.
     */
    @Override
    public void treeCollapsed(TreeExpansionEvent event) {
        
    }

    /**
     * Called when a tree node has expanded.
     * 
     * @param event The event that occurred.
     */
    @Override
    public void treeExpanded(TreeExpansionEvent event) {

    }
}
