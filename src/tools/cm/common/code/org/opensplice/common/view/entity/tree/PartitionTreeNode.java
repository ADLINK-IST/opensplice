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
import org.opensplice.cm.Participant;
import org.opensplice.common.CommonException;
import org.opensplice.common.util.Report;

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
    @Override
    public void refresh() throws CommonException{
        Participant participant = (Participant)userObject;
        Entity[] entities = null;

        try {
            entities = participant.resolveAllDomains();
        } catch (CMException e) {
            Report.getInstance().writeErrorLog(e.getMessage());
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
