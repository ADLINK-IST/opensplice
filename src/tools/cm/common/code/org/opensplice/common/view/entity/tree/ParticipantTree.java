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
 * all participants in the connected kernel and shows its owned entities as
 * children.
 *  
 * @date Sep 1, 2004
 */
public class ParticipantTree extends EntityTree {
    /**
     * Constructs a new ParticipantTree.
     * 
     * @param _participant The participant where to resolve the participants 
     *                     from.
     * @param _register The ModelRegister that needs to be triggered on changes
     *                  in the tree. It may be null when no component needs
     *                  to be triggered.
     * @throws CommonException Thrown when the supplied Participant is not valid.
     */
    public ParticipantTree(Participant participant, ModelRegister register, boolean childrenVisible) throws CommonException {
        super(participant, register, childrenVisible);
        rootNode = new ParticipantTreeNode(participant, this);
        this.setModel(new DefaultTreeModel(rootNode));
        this.setEditable(false);
        this.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);
        willExpandListener = new EntityTreeWillExpandListener(this);
        this.addTreeWillExpandListener(willExpandListener);
        rootNode.refresh();
    }
}
