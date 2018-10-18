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

import java.awt.Component;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTable;

import org.opensplice.cm.Writer;
import org.opensplice.cm.data.UserData;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.ConnectionNotifier.DisconnectListener;
import org.opensplice.common.model.CoherentPublishModel;
import org.opensplice.common.view.CoherentPublishFrameCommon;

/**
 * Controller that handles user actions on the CoherentPublishFrame
 *
 * @date Aug 26, 2016
 */
public abstract class CoherentPublishControllerCommon
        implements ActionListener, MouseListener, WindowListener, DisconnectListener {

    protected CoherentPublishModel model;
    protected CoherentPublishFrameCommon view;
    protected Writer savedWriter = null;

    public CoherentPublishControllerCommon(CoherentPublishModel _model) {
        model = _model;
        ConnectionNotifier.addDisconnectListener(this);
    }

    public void setView(CoherentPublishFrameCommon _view) {
        view = _view;
    }

    /**
     * Called by the view component when an event occurs (button click, menu
     * selection, etc.)
     *
     * @param e The event that occurred.
     */
    @Override
    public void actionPerformed(ActionEvent e){
        String command = e.getActionCommand();

        try{
            if(command.equals("cancel")){
                handleCancelCommand();
                view.setViewEnabled(true);
                view.toFront();
            }
            else if (command.equals("write_data")) {
                Writer writer = null;
                // Determine which widget spawned the action to get the selected writer
                if (e.getSource().equals(view.getWriterList())) {
                    // Source is from writer list double click
                    writer = getSelectedListWriter();
                } else if (e.getSource().equals(view.getWriteMenuItem())) {
                    // Source is from popup menu item Write data
                    if (((Component)e.getSource()).getParent() instanceof JPopupMenu) {
                        JPopupMenu m = ((JPopupMenu)((Component)e.getSource()).getParent());
                        if (m.getInvoker().equals(view.getWriterList())) {
                            writer = getSelectedListWriter();
                        } else if (m.getInvoker().equals(view.getCoherentSetTable())) {
                            writer = getSelectedTableWriter();
                        }
                    }
                }
                if (writer != null) {
                    handleWriteDataCommand(writer);
                } else {
                    view.setStatus("Selected writer is null.", false, false);
                }
            }
            else if(command.equals("write_data_as")){
                final Writer writer = savedWriter;
                savedWriter = null;
                if (writer != null) {
                    handleWriteDataAsCommand(writer);
                } else {
                    view.setStatus("Error: Entity not available (anymore).", false, false);
                }
            } else if(command.startsWith("view_data")) {
                final Writer writer = getSelectedTableWriter();
                if (writer != null) {
                    handleViewDataCommand(writer);
                    view.setViewEnabled(true);
                } else {
                    view.setStatus("Selected writer is null.", false, false);
                }
            } else if(command.startsWith("begin_coherent_changes")) {
                try {
                    model.beginCoherentChanges();
                    view.updateCoherentActionsEnabled(true);
                    view.setStatus("Beginning new set of coherent changes.", false, false);
                } catch (CommonException ce) {
                    view.setStatus("Error: Begin coherent changes failed. " + ce.getMessage(), false, false);
                }
            } else if(command.startsWith("end_coherent_changes")) {
                try {
                    model.endCoherentChanges();
                    view.updateCoherentActionsEnabled(false);
                    view.setStatus("Coherent set published.", false, false);
                } catch (CommonException ce) {
                    view.setStatus("Error: End coherent changes failed. " + ce.getMessage(), false, false);
                }
            } else if(command.startsWith("refresh_writerlist")) {
                try {
                    model.refreshWriterList();
                    view.setStatus("Writer list refreshed.", false, false);
                } catch (CommonException ce) {
                    view.setStatus("Error: " + ce.getMessage(), false, false);
                }
            } else if(command.equals("close")){
                view.dispose();
            }
        } catch(Exception ex){
            StackTraceElement[] elements = ex.getStackTrace();
            String error0 = "Unspecified error";
            String error = "";

            for(int i=0; i<elements.length; i++){
                error += elements[i].getFileName() + ", " +
                         elements[i].getMethodName() + ", " + elements[i].getLineNumber() + "\n";
                if(i==0){
                    error0 = elements[i].getFileName() + ", " +
                             elements[i].getMethodName() + ", " + elements[i].getLineNumber();
                }
            }
//            logger.logp(Level.SEVERE, "SampleController", "actionPerformed",
//                                "Unhandled exception occurred:\n" + error);
            view.setStatus("Error: " + error0, false, false);
            ex.printStackTrace();
        }
    }

    protected abstract void handleWriteDataCommand(Writer writer);

    protected abstract void handleWriteDataAsCommand(Writer writer);

    protected abstract void handleViewDataCommand(Writer writer);

    protected abstract void handleCancelCommand();

    private Writer getSelectedListWriter() {
        return (Writer) view.getWriterList().getSelectedValue();
    }

    private Writer getSelectedTableWriter() {
        Writer writer = null;
        int rowIndex = view.getCoherentSetTable().getSelectedRow();
        if (rowIndex != -1) {
            writer = (Writer) model.getCoherentSetTableModel().getEntity(rowIndex);
        }
        return writer;
    }

    protected UserData getSelectedTableData() {
        UserData data = null;
        int rowIndex = view.getCoherentSetTable().getSelectedRow();
        if (rowIndex != -1) {
            data = model.getCoherentSetTableModel().getData(rowIndex);
        }
        return data;
    }

    private void maybeShowPopup(MouseEvent e) {
        Object source = e.getSource();

        if (e.isPopupTrigger()) {
            if(source.equals(view.getWriterList())){
                JList writerList = view.getWriterList();
                Point lastPopup = new Point(e.getX(), e.getY());
                int selectionIndex = writerList.locationToIndex(lastPopup);
                if (selectionIndex > -1) {
                    Writer selected = (Writer) writerList.getModel().getElementAt(selectionIndex);
                    writerList.setSelectedValue(selected, false);
                    view.getWriterListPopup(lastPopup).show(writerList, e.getX(), e.getY());
                }
            } else if(source.equals(view.getCoherentSetTable())){
                JTable coherentSetTable = (JTable)source;
                Point lastPopup = new Point(e.getX(), e.getY());
                coherentSetTable.changeSelection(coherentSetTable.rowAtPoint(lastPopup), coherentSetTable.columnAtPoint(lastPopup), false, false);
                view.getTablePopup(lastPopup).show(coherentSetTable, e.getX(), e.getY());
            }
        }
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        Object o = e.getSource();
        String action = null;
        if(o instanceof JList){
            action = "write_data";
        } else if (o instanceof JTable) {
            action = "view_data";
        }
        if(e.getButton() == MouseEvent.BUTTON1
                && e.getClickCount() == 2
                && action != null)
        {
           this.actionPerformed(new ActionEvent(o, 1, action));
        }
    }

    @Override
    public void mouseEntered(MouseEvent e) {}

    @Override
    public void mouseExited(MouseEvent e) {}

    @Override
    public void mousePressed(MouseEvent e) {
        maybeShowPopup(e);
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        maybeShowPopup(e);
    }

    @Override
    public void windowActivated(WindowEvent e) {}

    @Override
    public void windowClosing(WindowEvent e) {
        if (model.isCoherentSetInProgress()) {
            int n = JOptionPane.showConfirmDialog(
                    view,
                    "You are still creating a coherent set. Are you sure you want to close this window?"
                            + " Coherent set will be published as is.",
                    "Confirm set editor exit",
                    JOptionPane.YES_NO_OPTION);

            if(n == JOptionPane.YES_OPTION){
                view.dispose();
            }
        } else {
            view.dispose();
        }
    }

    @Override
    public void windowDeactivated(WindowEvent e) {}

    @Override
    public void windowDeiconified(WindowEvent e) {}

    @Override
    public void windowIconified(WindowEvent e) {}

    @Override
    public void windowOpened(WindowEvent e) {}
}
