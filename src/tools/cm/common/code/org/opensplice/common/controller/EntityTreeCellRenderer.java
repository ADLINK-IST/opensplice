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
package org.opensplice.common.controller;

import java.awt.Color;
import java.awt.Component;
import java.awt.Font;

import javax.swing.JLabel;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import org.opensplice.cm.Entity;
import org.opensplice.common.util.Config;
import org.opensplice.common.view.entity.tree.EntityTree;
import org.opensplice.common.view.entity.tree.EntityTreeNode;



/**
 * Renderer that is capable of rendering an EntityTree.
 * 
 * @date Nov 18, 2004 
 */
public class EntityTreeCellRenderer extends DefaultTreeCellRenderer {
    private final Color hideRelationsColor = Config.getInactiveColor();
    private final Color showRelationsColor = Config.getActiveColor();
    private final Color entityDisabledColor = Config.getWarningColor();
    /**
     * Constructs a new EntityTreeCellRenderer.
     *
     */
    public EntityTreeCellRenderer(){
        super();
    }
    
    /**
     * Returns the Component that will be displayed in the GUI as a tree node.
     * The difference between the default renderer is that it displays the
     * extended string representation of an Entity.
     * 
     * 
     * @param tree The tree which must visualize the supplied value.
     * @param value The object to display in the tree.
     * @param sel Whether or not the object is selected.
     * @param expanded Whether or not the node is currently expanded.
     * @param leaf Whether or not the node is a leaf.
     * @param row The row of the node in the tree.
     * @param focus Whether or not the node has the focus.
     */
    @Override
    public Component getTreeCellRendererComponent(
            JTree tree, 
            Object value, 
            boolean sel, 
            boolean expanded, 
            boolean leaf, 
            int row, 
            boolean focus)
    {
        JLabel label;
        Component result = super.getTreeCellRendererComponent(tree, value, sel,
                                    expanded, leaf, row, focus);
        
        
        if ((result instanceof JLabel) && (value instanceof EntityTreeNode))
        {
            String str = null;
            Entity e = (Entity)((EntityTreeNode)value).getUserObject();
            
            
            if(e != null){
                str = e.toStringExtended();
                label = ((JLabel)result); 
                label.setText(str);
                label.setToolTipText(str);

                if(!(e.isEnabled())){
                    label.setForeground(entityDisabledColor);
                }
            }
            if (tree instanceof EntityTree) {
                EntityTree eTree = (EntityTree) tree;
                if (eTree.getChildrenVisible() != ((EntityTreeNode) value).getChildrenVisible()) {
                    if (eTree.getChildrenVisible()) {
                        ((JLabel) result).setForeground(hideRelationsColor);
                    } else {
                        ((JLabel) result).setForeground(showRelationsColor);
                    }
                }
            }
        }
        return result;
    }
}
