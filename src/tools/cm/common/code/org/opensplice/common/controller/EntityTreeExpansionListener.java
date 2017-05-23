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
