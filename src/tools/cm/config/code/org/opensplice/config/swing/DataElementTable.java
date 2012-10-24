/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.config.swing;

import java.awt.Point;

import javax.swing.JPopupMenu;
import javax.swing.JTable;

import org.opensplice.common.view.StatusPanel;
import org.opensplice.config.data.DataElement;
import org.opensplice.config.data.DataNode;

public class DataElementTable extends JTable implements DataNodePopupSupport {
    private static final long serialVersionUID = 3904871676109190600L;
    private DataElementTableModel model;
    private DataElementTableModelEditor editor;
    private DataNodePopup popupController;
    private StatusPanel status;
    
    public DataElementTable(DataNodePopup popup, DataElement element, StatusPanel status){
        super();
        this.model = new DataElementTableModel();
        this.editor = new DataElementTableModelEditor(model);
        this.editor.setStatusListener(status);
        this.model.setElement(element);
        this.status = status;
        this.setModel(model);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.getTableHeader().setReorderingAllowed(false);
        this.setSurrendersFocusOnKeystroke(true);
        this.getColumnModel().getColumn(1).setCellEditor(editor);
        
        if(popup == null){
            this.popupController = new DataNodePopup();
        } else {
            this.popupController = popup;
        }
        this.popupController.registerPopupSupport(this);
    }
    
    public void setStatusListener(StatusPanel  status){
        this.editor.setStatusListener(status);
    }
    
    public DataNode getDataNodeAt(int row){
        this.getSelectionModel().setSelectionInterval(row, row);
        return this.model.getNodeAt(row);
    }

    public DataNode getDataNodeAt(int x, int y) {
        int row = this.rowAtPoint(new Point(x, y));
        return this.getDataNodeAt(row);
    }

    public void showPopup(JPopupMenu popup, int x, int y) {
        popup.show(this, x, y);
    }

    public void setStatus(String message, boolean persistent, boolean busy) {
        if(this.status != null){
            this.status.setStatus(message, persistent, busy);
        }
    }

    public DataElementTableModelEditor getEditor() {
        return this.editor;
    }
}
