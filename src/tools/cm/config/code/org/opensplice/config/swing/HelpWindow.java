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
package org.opensplice.config.swing;

import java.awt.BorderLayout;

import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.WindowConstants;
import javax.swing.JSplitPane;

import org.opensplice.common.util.ConfigModeIntializer;
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

    public static final String    OSPL_HELP_WINDOW_TITLE      = "Vortex OpenSplice Configurator | Help";
    public static final String    LITE_HELP_WINDOW_TITLE      = "Vortex Lite Configurator | Help";

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
        setWindowTitle(ConfigModeIntializer.CONFIGURATOR_MODE);
        getElementTree().getSelectionModel().addTreeSelectionListener(this);
        getDocPane().setNode(getElementTree().getSelectedMetaElement());
    }

    public void setWindowTitle (int configMode) {
        String windowTitle = "Configurator | Help";
        if (configMode == ConfigModeIntializer.LITE_MODE) {
            windowTitle = LITE_HELP_WINDOW_TITLE;
        } else {
            windowTitle = OSPL_HELP_WINDOW_TITLE;
        }
        this.setTitle(windowTitle);
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

    @Override
    public void valueChanged(TreeSelectionEvent e) {
        getDocPane().setNode(getElementTree().getSelectedMetaElement());
    }
}
