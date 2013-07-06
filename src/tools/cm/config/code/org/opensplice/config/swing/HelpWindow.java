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

import java.awt.BorderLayout;

import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.WindowConstants;
import javax.swing.JSplitPane;

import org.opensplice.common.view.StatusPanel;
import org.opensplice.config.meta.MetaConfiguration;
import javax.swing.JScrollPane;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;

public class HelpWindow extends JFrame implements TreeSelectionListener {
    private static final long serialVersionUID = 11222322324L;
    private JPanel jContentPane = null;
    private JSplitPane eastWestSplitPane = null;
    private StatusPanel statusPanel = null;
    private MetaConfiguration metaConfiguration = null;
    private MetaElementTree elementTree = null;
    private JScrollPane elementTreeScrollPane = null;
    private JScrollPane docScrollPane = null;
    private MetaNodeDocPane docPane = null;
    
    /**
     * This is the default constructor
     */
    public HelpWindow(MetaConfiguration metaConfiguration) {
        super();
        this.metaConfiguration = metaConfiguration;
        initialize();
    }

    /**
     * This method initializes this
     * 
     * @return void
     */
    private void initialize() {
        this.setSize(640, 480);
        this.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        this.setContentPane(getJContentPane());
        this.setTitle("OpenSplice Configurator | Help");        
        getElementTree().getSelectionModel().addTreeSelectionListener(this);
        getDocPane().setNode(getElementTree().getSelectedMetaElement());
    }

    /**
     * This method initializes jContentPane
     * 
     * @return javax.swing.JPanel
     */
    private JPanel getJContentPane() {
        if (jContentPane == null) {
            jContentPane = new JPanel();
            jContentPane.setLayout(new BorderLayout());
            jContentPane.add(getEastWestSplitPane(), BorderLayout.CENTER);
            jContentPane.add(getStatusPanel(), BorderLayout.SOUTH);
        }
        return jContentPane;
    }

    /**
     * This method initializes eastWestSplitPane	
     * 	
     * @return javax.swing.JSplitPane	
     */
    private JSplitPane getEastWestSplitPane() {
        if (eastWestSplitPane == null) {
            eastWestSplitPane = new JSplitPane();
            eastWestSplitPane.setDividerSize(5);
            eastWestSplitPane.setDividerLocation(250);
            eastWestSplitPane.setLeftComponent(getElementTreeScrollPane());
            eastWestSplitPane.setRightComponent(getDocScrollPane());
        }
        return eastWestSplitPane;
    }

    private StatusPanel getStatusPanel(){
        if(this.statusPanel == null){
            this.statusPanel = new StatusPanel(800, "", false, false);
        }
        return this.statusPanel;
    }
    
    private MetaElementTree getElementTree(){
        if(this.elementTree == null){
            this.elementTree = new MetaElementTree(metaConfiguration.getRootElement(), getStatusPanel());
        }
        return this.elementTree;
    }

    /**
     * This method initializes elementTreeScrollPane	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getElementTreeScrollPane() {
        if (elementTreeScrollPane == null) {
            elementTreeScrollPane = new JScrollPane();
            elementTreeScrollPane.setViewportView(getElementTree());
        }
        return elementTreeScrollPane;
    }

    /**
     * This method initializes docScrollPane	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getDocScrollPane() {
        if (docScrollPane == null) {
            docScrollPane = new JScrollPane();
            docScrollPane.setViewportView(getDocPane());
        }
        return docScrollPane;
    }

    /**
     * This method initializes codPane	
     * 	
     * @return javax.swing.JEditorPane	
     */
    private MetaNodeDocPane getDocPane() {
        if (docPane == null) {
            docPane = new MetaNodeDocPane();
        }
        return docPane;
    }

    public void valueChanged(TreeSelectionEvent e) {
        getDocPane().setNode(getElementTree().getSelectedMetaElement());
    }
}
