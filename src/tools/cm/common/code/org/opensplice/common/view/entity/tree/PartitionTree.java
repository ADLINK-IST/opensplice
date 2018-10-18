/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeSelectionModel;

import org.opensplice.cm.Participant;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.EntityTreeWillExpandListener;
import org.opensplice.common.model.ModelRegister;

/**
 * Concrete implementation of an EntityTree. It represents a tree which holds
 * all Partition entities in the connected kernel and shows its dependent
 * entities as children.
 *
 * @date Oct 5, 2004
 */
public class PartitionTree extends EntityTree {

    /**
     * Constructs a new PartitionTree.
     *
     * @param _participant The participant where to resolve the domains from.
     * @param _register The ModelRegister that needs to be triggered on changes
     *                  in the tree. It may be null when no component needs
     *                  to be triggered.
     * @throws CommonException Thrown when the supplied Participant is not valid.
     */
    public PartitionTree(Participant _participant, ModelRegister _register, boolean childrenVisible) throws CommonException {
        super(_participant, _register, childrenVisible);
        rootNode = new PartitionTreeNode(participant, this);
        this.setModel(new DefaultTreeModel(rootNode));
        this.setEditable(false);
        this.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);
        willExpandListener = new EntityTreeWillExpandListener(this);
        this.addTreeWillExpandListener(willExpandListener);
        rootNode.refresh();
    }

}
