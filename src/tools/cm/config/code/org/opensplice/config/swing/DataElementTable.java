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

import java.awt.Point;

import javax.swing.JPopupMenu;
import javax.swing.JTable;

import org.opensplice.common.view.StatusPanel;
import org.opensplice.config.data.DataElement;
import org.opensplice.config.data.DataNode;

public class DataElementTable extends JTable implements DataNodePopupSupport {
    private static final long serialVersionUID = 3904871676109190600L;
    private final DataElementTableCellRenderer renderer;
    private final DataElementTableModel model;
    private final DataElementTableModelEditor editor;
    private DataNodePopup popupController;
    private final StatusPanel status;
    
    public DataElementTable(DataNodePopup popup, DataElement element, StatusPanel status){
        super();
        this.model = new DataElementTableModel();
        this.editor = new DataElementTableModelEditor(model);
        this.renderer = new DataElementTableCellRenderer(model);
        this.editor.setStatusListener(status);
        this.model.setElement(element);
        this.status = status;
        this.setModel(model);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.getTableHeader().setReorderingAllowed(false);
        this.setSurrendersFocusOnKeystroke(true);
        this.getColumnModel().getColumn(1).setCellEditor(editor);
        this.getColumnModel().getColumn(1).setCellRenderer(renderer);
        
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

    @Override
    public DataNode getDataNodeAt(int x, int y) {
        int row = this.rowAtPoint(new Point(x, y));
        return this.getDataNodeAt(row);
    }

    @Override
    public void showPopup(JPopupMenu popup, int x, int y) {
        popup.show(this, x, y);
    }

    @Override
    public void setStatus(String message, boolean persistent, boolean busy) {
        if(this.status != null){
            this.status.setStatus(message, persistent, busy);
        }
    }

    public DataElementTableModelEditor getEditor() {
        return this.editor;
    }
}
