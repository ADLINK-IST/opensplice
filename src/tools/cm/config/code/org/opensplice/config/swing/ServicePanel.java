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
package org.opensplice.config.swing;

import java.awt.Color;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.TransferHandler;
import javax.swing.border.TitledBorder;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;

import org.opensplice.common.view.StatusPanel;
import org.opensplice.config.data.DataAttribute;
import org.opensplice.config.data.DataElement;
import org.opensplice.config.data.DataNode;
import org.opensplice.config.data.DataValue;
import org.opensplice.config.meta.MetaAttribute;
import org.opensplice.config.meta.MetaElement;
import org.opensplice.config.meta.MetaNode;

public class ServicePanel extends JPanel implements TreeSelectionListener, ListSelectionListener {

    private static final long serialVersionUID = 1L;
    private JSplitPane northSouthSplitPane = null;
    private MetaNodeDocPane docPane = null;
    private JScrollPane docScrollPane = null;
    private JSplitPane eastWestSplitPane = null;
    private JScrollPane elementTreeScrollPane = null;
    private DataElementTree configurationElementTree = null;
    private JScrollPane tableScrollPane = null;
    private DataElementTable configurationTable = null;
    private DataElement serviceElement = null;
    private DataNodePopup popupController = null;
    private StatusPanel status = null;

    /**
     * This is the default constructor
     */
    public ServicePanel(DataElement serviceElement, StatusPanel status) {
        super();
        this.serviceElement = serviceElement;
        this.popupController = new DataNodePopup();
        this.status = status;
        initialize();
    }

    public DataElement getService(){
        return this.serviceElement;
    }

    public DataElementTable getConfigurationTable() {
        if (configurationTable == null) {
            configurationTable = new DataElementTable(this.popupController, serviceElement, this.status);
            configurationTable.getSelectionModel().addListSelectionListener(this);
        }
        return configurationTable;
    }

    @Override
    public void valueChanged(TreeSelectionEvent e) {
        TreePath selectionPath = e.getNewLeadSelectionPath();

        if(selectionPath != null){
            this.getConfigurationTable().getSelectionModel().clearSelection();
            DataElement selection = (DataElement)(
                                        ((DefaultMutableTreeNode)(
                                                selectionPath.getLastPathComponent())).getUserObject());
            this.configurationTable.getEditor().stopCellEditing();
            ((DataElementTableModel)this.configurationTable.getModel()).setElement(selection);
            this.updateDocument(selection);
        }
    }

    @Override
    public void valueChanged(ListSelectionEvent e) {
        if(!e.getValueIsAdjusting()){
            int selectedRow = this.getConfigurationTable().getSelectedRow();

            if(selectedRow != -1){
                this.getConfigurationElementTree().getSelectionModel().clearSelection();
                DataNode selection = this.getConfigurationTable().getDataNodeAt(selectedRow);
                this.updateDocument(selection);
            }
        }
    }

    @Override
    public void setTransferHandler(TransferHandler t){
        super.setTransferHandler(t);
        int count = this.getComponentCount();

        for(int i=0; i<count; i++){
            if(this.getComponent(i) instanceof JComponent){
                ((JComponent)this.getComponent(i)).setTransferHandler(t);
            }
        }
        this.getConfigurationElementTree().setTransferHandler(t);
        this.getConfigurationTable().setTransferHandler(t);
        this.getDocPane().setTransferHandler(t);
    }

    /**
     * This method initializes this
     *
     * @return void
     */
    private void initialize() {
        GridBagConstraints gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.fill = GridBagConstraints.BOTH;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.gridx = 0;
        this.setSize(800, 540);
        this.setLayout(new GridBagLayout());
        this.add(getNorthSouthSplitPane(), gridBagConstraints);
    }

    private JSplitPane getNorthSouthSplitPane() {
        if (northSouthSplitPane == null) {
            northSouthSplitPane = new JSplitPane();
            northSouthSplitPane.setOrientation(JSplitPane.VERTICAL_SPLIT);
            northSouthSplitPane.setDividerLocation(300);
            northSouthSplitPane.setBackground(Color.white);
            northSouthSplitPane.setLeftComponent(getEastWestSplitPane());
            northSouthSplitPane.setRightComponent(getDocScrollPane());
            northSouthSplitPane.setDividerSize(5);
        }
        return northSouthSplitPane;
    }
    /**
     * This method initializes mainSplitPane
     *
     * @return javax.swing.JSplitPane
     */
    private JSplitPane getEastWestSplitPane() {
        if (eastWestSplitPane == null) {
            eastWestSplitPane = new JSplitPane();
            eastWestSplitPane.setDividerLocation(330);
            eastWestSplitPane.setBackground(Color.white);
            eastWestSplitPane.setLeftComponent(getElementTreeScrollPane());
            eastWestSplitPane.setRightComponent(getTableScrollPane());
            eastWestSplitPane.setDividerSize(5);
        }
        return eastWestSplitPane;
    }

    /**
     * This method initializes elementTreeScrollPane
     *
     * @return javax.swing.JScrollPane
     */
    private JScrollPane getElementTreeScrollPane() {
        if (elementTreeScrollPane == null) {
            elementTreeScrollPane = new JScrollPane();
            elementTreeScrollPane.setBorder(
                    BorderFactory.createTitledBorder(null, "Elements",
                            TitledBorder.LEFT, TitledBorder.BOTTOM,
                            new Font("Dialog", Font.BOLD, 12),
                            new Color(51, 51, 51)));
            elementTreeScrollPane.setViewportView(getConfigurationElementTree());
            elementTreeScrollPane.setBackground(Color.WHITE);
        }
        return elementTreeScrollPane;
    }

    /**
     * This method initializes configurationElementTree
     *
     * @return javax.swing.JTree
     */
    private DataElementTree getConfigurationElementTree() {
        if (configurationElementTree == null) {
            configurationElementTree = new DataElementTree(serviceElement, null, this.status);
            configurationElementTree.addTreeSelectionListener(this);
            this.updateDocument(serviceElement);
        }
        return configurationElementTree;
    }

    /**
     * This method initializes tableScrollPane
     *
     * @return javax.swing.JScrollPane
     */
    private JScrollPane getTableScrollPane() {
        if (tableScrollPane == null) {
            tableScrollPane = new JScrollPane();
            tableScrollPane.setViewportView(getConfigurationTable());
            tableScrollPane.setBorder(BorderFactory.createTitledBorder(null, "Attributes", TitledBorder.RIGHT, TitledBorder.BOTTOM, new Font("Dialog", Font.BOLD, 12), new Color(51, 51, 51)));
        }
        return tableScrollPane;
    }

    private JScrollPane getDocScrollPane() {
        if (docScrollPane == null) {
            docScrollPane = new JScrollPane();
            docScrollPane.setViewportView(getDocPane());
            docScrollPane.setBorder(BorderFactory.createTitledBorder(null, "Documentation", TitledBorder.LEFT, TitledBorder.BOTTOM, new Font("Dialog", Font.BOLD, 12), new Color(51, 51, 51)));
            docScrollPane.setBackground(Color.WHITE);
        }
        return docScrollPane;
    }

    private MetaNodeDocPane getDocPane(){
        if (docPane == null) {
            docPane = new MetaNodeDocPane(false, false);
        }
        return docPane;
    }

    private void updateDocument(DataNode node){
        MetaNode meta = null;
        DataNode parent;

        if(node instanceof DataValue){
            parent = node.getParent();

            if(parent != null){
                meta = parent.getMetadata();
            }
        } else {
            meta = node.getMetadata();
        }


        this.getDocScrollPane().setBorder(BorderFactory.createTitledBorder(null,
                "Documentation for '" + this.getXPath(node) + "'",
                TitledBorder.LEFT, TitledBorder.BOTTOM,
                new Font("Dialog", Font.BOLD, 12), new Color(51, 51, 51)));
        this.getDocPane().setNode(meta);
    }

    private String getXPath(DataNode node){
        String result;
        DataNode curNode = node;

        result = "";

        while(curNode != null){
            if(curNode instanceof DataElement){
                result = "/" + ((MetaElement)curNode.getMetadata()).getName() + result;
            } else if(curNode instanceof DataAttribute){
                result = "[@" + ((MetaAttribute)curNode.getMetadata()).getName() + "]" + result;
            }
            curNode = curNode.getParent();
        }
        result = "/" + result;
        return result;
    }
}
