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
package org.opensplice.common.view;

import java.awt.BorderLayout;
import java.awt.Point;
import java.awt.event.KeyEvent;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.ListSelectionModel;
import javax.swing.WindowConstants;

import org.opensplice.cm.Publisher;
import org.opensplice.common.controller.CoherentPublishControllerCommon;
import org.opensplice.common.model.CoherentPublishModel;

/**
 * Represents a frame that is capable of displaying data from a Publisher.
 *
 * It allows the user to:
 * - Begin/end coherent changes on a publisher
 * - Select a writer that the publisher owns to write data from
 *
 * It displays written data:
 * - Instance key values
 * - Write state
 * - Source writer
 *
 * It displays publisher's writers:
 * -Writer name
 *
 * @date Aug 26, 2016
 */
public abstract class CoherentPublishFrameCommon extends JFrame {

    private static final long serialVersionUID = -7321644128043691541L;

    protected CoherentPublishModel model;
    protected CoherentPublishControllerCommon controller;
    private JTable coherentSetTable;
    private JScrollPane coherentSetScrollPane;
    private JPanel cp;
    protected JMenuBar dataMenuBar;
    private JPanel bottomPanel;
    private JPanel buttonPanel;
    private StatusPanel statusPanel;
    private JList writerList;
    private JMenuItem writeMenuItem;
    private JButton beginCoherentButton;
    private JButton endCoherentButton;
    private JMenu fileMenu;
    private JMenuItem closeItem;
    private JMenu editMenu;
    private JMenuItem beginCoherentItem;
    private JMenuItem endCoherentItem;
    private JPanel writerListPanel;
    private JLabel writerListLabel;
    private JScrollPane writerListScrollPane;
    private JMenuItem viewMenuItem;
    private JMenuItem refreshTreeItem;


    public CoherentPublishFrameCommon(String title, CoherentPublishModel _model,
            CoherentPublishControllerCommon _controller) {
        super(title);
        model = _model;
        controller = _controller;
        addWindowListener(controller);
        initialize();
    }

    private void initialize() {
        this.setJMenuBar(getDataMenuBar());
        this.setSize(510, 450);
        this.setContentPane(getCp());
        this.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
        this.pack();
    }

    /**
     * This method initializes cp
     * 
     * @return javax.swing.JPanel
     */
    private JPanel getCp() {
        if(cp == null) {
            cp = new JPanel();
            BorderLayout bl = new java.awt.BorderLayout();
            bl.setVgap(2);
            bl.setHgap(2);
            cp.setLayout(bl);
            cp.add(getBottomPanel(), java.awt.BorderLayout.SOUTH);
            cp.add(getCoherentSetScrollPane(), java.awt.BorderLayout.CENTER);
            cp.add(getWriterListPanel(), java.awt.BorderLayout.EAST);
        }
        return cp;
    }

    private JPanel getWriterListPanel() {
        if (writerListPanel == null) {
            writerListPanel = new JPanel();
            writerListPanel.setLayout(new javax.swing.BoxLayout(writerListPanel, BoxLayout.Y_AXIS));
            writerListPanel.setPreferredSize(new java.awt.Dimension(150, 0));
            writerListPanel.add(getWriterListLabel());
            writerListPanel.add(getWriterListScrollPane());
        }
        return writerListPanel;
    }

    private JLabel getWriterListLabel() {
        if (writerListLabel == null) {
            writerListLabel = new JLabel("Writer List");
            writerListLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        }
        return writerListLabel;
    }

    private JScrollPane getWriterListScrollPane() {
        if (writerListScrollPane == null) {
            writerListScrollPane = new JScrollPane(getWriterList());
        }
        return writerListScrollPane;
    }

    public JList getWriterList() {
        if (writerList == null) {
            writerList = new JList(model.getWriterListModel());
            writerList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            writerList.setToolTipText("Right click to view actions.");
            writerList.addMouseListener(controller);
        }
        return writerList;
    }

    /**
     * Constructs a popup menu which contains items to:
     * -Write data from the selected Writer
     *
     * @param p
     *            The point where the mouse was clicked and the popup will
     *            appear.
     * @return The created popup menu.
     */
    public JPopupMenu getWriterListPopup(Point p){
        JPopupMenu popupMenu = new JPopupMenu("Actions");

        popupMenu.add(getWriteMenuItem());
        return popupMenu;
    }

    public JPopupMenu getTablePopup(Point p){
        JPopupMenu popupMenu = new JPopupMenu("Actions");

        popupMenu.add(getWriteMenuItem());
        popupMenu.add(getViewMenuItem());
        return popupMenu;
    }

    public JMenuItem getWriteMenuItem() {
        if (writeMenuItem == null) {
            writeMenuItem = new JMenuItem();
            writeMenuItem.setText("Write data");
            writeMenuItem.addActionListener(controller);
        }
        writeMenuItem.setActionCommand("write_data");
        return writeMenuItem;
    }

    private JMenuItem getViewMenuItem() {
        if (viewMenuItem == null) {
            viewMenuItem = new JMenuItem();
            viewMenuItem.setText("View data");
            viewMenuItem.addActionListener(controller);
        }
        viewMenuItem.setActionCommand("view_data");
        return viewMenuItem;
    }

    /**
     * This method initializes dataMenuBar
     * 
     * @return javax.swing.JMenuBar
     */
    protected JMenuBar getDataMenuBar() {
        if (dataMenuBar == null) {
            dataMenuBar = new JMenuBar();
            dataMenuBar.add(getFileMenu());
            dataMenuBar.add(getEditMenu());
        }
        return dataMenuBar;
    }

    /**
     * This method initializes coherentSetPanel
     * 
     * @return javax.swing.JTable
     */
    private JScrollPane getCoherentSetScrollPane() {
        if (coherentSetScrollPane == null) {
            coherentSetScrollPane = new JScrollPane(getCoherentSetTable());
        }
        return coherentSetScrollPane;
    }

    /**
     * This method initializes coherentSetTable
     * 
     * @return javax.swing.JTable
     */
    public JTable getCoherentSetTable() {
        if (coherentSetTable == null) {
            coherentSetTable = new JTable(model.getCoherentSetTableModel());
            coherentSetTable.setToolTipText("Right click to view actions.");
            coherentSetTable.addMouseListener(controller);
        }
        return coherentSetTable;
    }

    /**
     * This method initializes bottomPanel
     * 
     * @return javax.swing.JPanel
     */
    private JPanel getBottomPanel() {
        if (bottomPanel == null) {
            bottomPanel = new JPanel();
            bottomPanel.setLayout(new BorderLayout());
            bottomPanel.setPreferredSize(new java.awt.Dimension(380,50));
            bottomPanel.add(getButtonPanel(), java.awt.BorderLayout.CENTER);
            bottomPanel.add(getStatusPanel(), java.awt.BorderLayout.SOUTH);
        }
        return bottomPanel;
    }


    private JPanel getButtonPanel() {
        if (buttonPanel == null) {
            buttonPanel = new JPanel();
            buttonPanel.setPreferredSize(new java.awt.Dimension(380,25));
            buttonPanel.add(getBeginCoherentButton());
            buttonPanel.add(getEndCoherentButton());
            updateCoherentActionsEnabled(false);
        }
        return buttonPanel;
    }

    private JButton getBeginCoherentButton() {
        if (beginCoherentButton == null) {
            beginCoherentButton = new JButton();
            beginCoherentButton.setPreferredSize(new java.awt.Dimension(210,20));
            beginCoherentButton.setText("Begin coherent changes");
            beginCoherentButton.setActionCommand("begin_coherent_changes");
            beginCoherentButton.setToolTipText("Start a new set. Clears the table.");
            beginCoherentButton.addActionListener(controller);
        }
        return beginCoherentButton;
    }

    private JButton getEndCoherentButton() {
        if (endCoherentButton == null) {
            endCoherentButton = new JButton();
            endCoherentButton.setPreferredSize(new java.awt.Dimension(210,20));
            endCoherentButton.setText("End coherent changes");
            endCoherentButton.setActionCommand("end_coherent_changes");
            endCoherentButton.setToolTipText("Makes the set available to coherent subscribers.");
            endCoherentButton.addActionListener(controller);
        }
        return endCoherentButton;
    }

    private StatusPanel  getStatusPanel() {
        if (statusPanel == null) {
            statusPanel = new StatusPanel(380, "Ready", true, true);
            statusPanel.setConnected(true, null);
        }
        return statusPanel;
    }

    /**
     * This method initializes fileMenu
     * 
     * @return javax.swing.JMenu
     */
    protected JMenu getFileMenu() {
        if (fileMenu == null) {
            fileMenu = new JMenu();
            fileMenu.setText("File");
            fileMenu.setMnemonic(java.awt.event.KeyEvent.VK_F);
            fileMenu.addSeparator();
            fileMenu.add(getCloseItem());
        }
        return fileMenu;
    }

    /**
     * This method initializes closeItem
     * 
     * @return javax.swing.JMenuItem
     */
    private JMenuItem getCloseItem() {
        if (closeItem == null) {
            closeItem = new JMenuItem();
            closeItem.setText("Close");
            closeItem.setActionCommand("close");
            closeItem.setMnemonic(java.awt.event.KeyEvent.VK_C);
            closeItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, java.awt.Event.CTRL_MASK, false));
            closeItem.addActionListener(controller);
        }
        return closeItem;
    }

    /**
     * This method initializes editMenu
     * 
     * @return javax.swing.JMenu
     */
    protected JMenu getEditMenu() {
        if (editMenu == null) {
            editMenu = new JMenu();
            editMenu.setText("Edit");
            editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
            editMenu.add(getBeginCoherentItem());
            editMenu.add(getEndCoherentItem());
            editMenu.addSeparator();
            editMenu.add(getRefreshTreeItem());
        }
        return editMenu;
    }

    /**
     * This method initializes beginCoherentItem
     * 
     * @return javax.swing.JMenuItem
     */
    private JMenuItem getBeginCoherentItem() {
        if (beginCoherentItem == null) {
            beginCoherentItem = new JMenuItem();
            beginCoherentItem.setText("Begin coherent changes");
            beginCoherentItem.setActionCommand("begin_coherent_changes");
            beginCoherentItem.setMnemonic(java.awt.event.KeyEvent.VK_B);
            beginCoherentItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_B, java.awt.Event.CTRL_MASK, false));
            beginCoherentItem.addActionListener(controller);
        }
        return beginCoherentItem;
    }

    /**
     * This method initializes endCoherentItem
     * 
     * @return javax.swing.JMenuItem
     */
    private JMenuItem getEndCoherentItem() {
        if (endCoherentItem == null) {
            endCoherentItem = new JMenuItem();
            endCoherentItem.setText("End coherent changes");
            endCoherentItem.setActionCommand("end_coherent_changes");
            endCoherentItem.setMnemonic(java.awt.event.KeyEvent.VK_E);
            endCoherentItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_E, java.awt.Event.CTRL_MASK, false));
            endCoherentItem.addActionListener(controller);
        }
        return endCoherentItem;
    }

    /**
     * This method initializes refreshTreeItem.
     *
     * @return the refreshTreeItem.
     */
    private JMenuItem getRefreshTreeItem() {
        if (refreshTreeItem == null) {
            refreshTreeItem = new JMenuItem();
            refreshTreeItem.setText("Refresh writer list");
            refreshTreeItem.setMnemonic(KeyEvent.VK_R);
            refreshTreeItem.setActionCommand("refresh_writerlist");
            refreshTreeItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F5, 0, false));
            refreshTreeItem.addActionListener(controller);
        }
        return refreshTreeItem;
    }

    /**
     * Sets the status in the status panel of the frame.
     * 
     * @param message
     *            The message to display.
     * @param persistent
     *            Whether or not to leave the message until another message is
     *            displayed or
     * @param busy
     *            Determines whether or not the progress bar will be set to
     *            indeterminate mode (busy).
     */
    public void setStatus(String message, boolean persistent, boolean busy){
        statusPanel.setStatus(message, persistent, busy);
    }

    public Publisher getPublisher() {
        return model.getPublisher();
    }

    /**
     * Sets the availability of the model in the status panel.
     * 
     * @param available
     *            Whether or not the model is available.
     * @param msg
     *            The message to display when hovering above the availability
     *            in the status panel.
     */
    public void setAvailable(boolean available, String msg){
        statusPanel.setConnected(available, msg);
    }

    /**
     * Enables or disables the view component. 
     * 
     * This is done when a dialog is shown and the user must
     * provide input before proceeding.
     */
    public void setViewEnabled(boolean enabled) {
        this.setEnabled(enabled);
        this.setFocusable(enabled);
    }

    public void updateCoherentActionsEnabled(boolean hasBegun) {
        beginCoherentButton.setEnabled(!hasBegun);
        endCoherentButton.setEnabled(hasBegun);
    }

    @Override
    public String toString(){
        return this.getTitle();
    }
}
