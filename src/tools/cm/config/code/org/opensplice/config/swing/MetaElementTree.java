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
package org.opensplice.config.swing;

import java.awt.Component;
import java.awt.Font;

import javax.swing.JLabel;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import org.opensplice.common.view.StatusPanel;
import org.opensplice.config.meta.MetaElement;
import org.opensplice.config.meta.MetaNode;

@SuppressWarnings("serial")
public class MetaElementTree extends JTree implements TreeCellRenderer, TreeSelectionListener{
    private final DefaultMutableTreeNode rootNode;
    private final MetaElement rootElement;
    private final DefaultTreeModel model;
    private final DefaultTreeCellRenderer initialRenderer;
    private final StatusPanel status;
    
    public MetaElementTree(MetaElement rootElement, StatusPanel status){
        super();
        this.initialRenderer = new DefaultTreeCellRenderer();
        this.rootElement     = rootElement;
        this.status          = status;
        this.rootNode        = new DefaultMutableTreeNode(this.rootElement);
        this.model           = ((DefaultTreeModel)this.treeModel);
        this.model.setRoot(this.rootNode);
        this.setCellRenderer(this);
        this.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        this.setEditable(false);
        this.setRootVisible(true);
        this.setShowsRootHandles(true);
        this.initElement(this.rootNode, this.rootElement);
        
        if(this.rootNode.getChildCount() > 0){
            this.scrollPathToVisible(
                    new TreePath(
                            ((DefaultMutableTreeNode)this.rootNode.getFirstChild()).getPath()));
        }
        this.getSelectionModel().addTreeSelectionListener(this);
        this.setSelectionPath(new TreePath(this.rootNode.getPath()));
    }
    
    private void initElement(DefaultMutableTreeNode parent, MetaElement element){
        DefaultMutableTreeNode node;
        
        for(MetaNode child: element.getChildren()){
            if(child instanceof MetaElement){
                node = new DefaultMutableTreeNode(child);
                this.model.insertNodeInto(node, parent, parent.getChildCount());
                this.initElement(node, (MetaElement)child);
            }
        }
    }
    
    @Override
    public Component getTreeCellRendererComponent(JTree tree, Object value, boolean selected, boolean expanded, boolean leaf, int row, boolean hasFocus) {
        Component result = null;
        JLabel temp;
        Object nodeValue;
        
        if(value instanceof DefaultMutableTreeNode){
            nodeValue = ((DefaultMutableTreeNode)value).getUserObject();
            
            if(nodeValue instanceof MetaElement){
                temp = new JLabel(ConfigUtil.getMetaElementString((MetaElement)nodeValue));
                
                if(selected){
                    temp.setForeground(UIManager.getColor("Tree.selectionForeground"));
                    temp.setBackground(UIManager.getColor("Tree.selectionBackground"));
                    temp.setOpaque(true);                   
                } else {
                    temp.setForeground(UIManager.getColor("Tree.textForeground"));
                    temp.setBackground(UIManager.getColor("Tree.textBackground"));
                }
                //temp.setFont(temp.getFont().deriveFont(Font.PLAIN));
                temp.setFont(temp.getFont().deriveFont(Font.BOLD));
                result = temp;
            }
        }
        
        if(result == null){
            result = initialRenderer.getTreeCellRendererComponent(tree, value, selected, expanded, leaf, row, hasFocus);
        }
        return result;
    }
    
    public MetaElement getSelectedMetaElement(){
        MetaElement result = null;
        TreePath path      = this.getSelectionPath();
        
        if(path != null){
            result = (MetaElement)
                ((DefaultMutableTreeNode)path.getLastPathComponent()).getUserObject();
        }
        return result;
    }
    
    public MetaElement getMetaNodeAt(int x, int y) {
        MetaElement retVal = null;
        TreePath path = this.getClosestPathForLocation(x, y);
        if (path != null) {
            this.setSelectionPath(path);
            retVal = (MetaElement)((DefaultMutableTreeNode)path.getLastPathComponent()).getUserObject();
        }
        return retVal;
    }

    @Override
    public void valueChanged(TreeSelectionEvent e) {
        MetaElement me = this.getSelectedMetaElement();
        
        if((me != null) && (status != null)){
            status.setStatus("Element '" + me.getName() + "' selected.", true);
        }
        
    }
}
