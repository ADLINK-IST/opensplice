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
package org.opensplice.common.model.table;

import javax.swing.table.*; 
import javax.swing.event.TableModelListener; 
import javax.swing.event.TableModelEvent; 

/**
 * Abstract table model implementation, which is used by the UserDataTableSorter
 * class. 
 *
 * @date Nov 5, 2004 
 */
public class TableMap extends AbstractTableModel 
                      implements TableModelListener {
    
    /**
     * The table model.
     */
    protected TableModel model; 

    /**
     * Provides access to the table model.
     * 
     * @return The table model.
     */
    public TableModel getModel() {
        return model;
    }

    /**
     * Sets the model.
     * 
     * @param model The model that will be set as the model of this class.
     */
    public void setModel(TableModel model) {
        this.model = model; 
        model.addTableModelListener(this); 
    }

    /**
     * Provides access to the value of the cell located at the supplied row and
     * column.
     * 
     * @param aRow The row of the cell.
     * @param aColumn The column of the cell.
     * @return The value of the cell.
     */
    @Override
    public Object getValueAt(int aRow, int aColumn) {
        return model.getValueAt(aRow, aColumn); 
    }
        
    /**
     * Sets the value of the cell located at the supplied row and column to
     * the supplied value.
     * 
     * @param aValue The value to set.
     * @param aRow The row of the cell.
     * @param aColumn The column of the cell.
     */
    @Override
    public void setValueAt(Object aValue, int aRow, int aColumn) {
        model.setValueAt(aValue, aRow, aColumn); 
    }

    /**
     * Provides access to the number of rows of the model.
     * 
     * @return The number of rows of the model or 0 if no model exists.
     */
    @Override
    public int getRowCount() {
        return (model == null) ? 0 : model.getRowCount(); 
    }

    /**
     * Provides access to the number of columns of the model.
     * 
     * @return The number of columns of the model or 0 if no model exists.
     */
    @Override
    public int getColumnCount() {
        return (model == null) ? 0 : model.getColumnCount(); 
    }
       
    /**
     * Provides access to the column name of the column located at the supplied
     * index.
     * 
     * @param aColumn The column index.
     * @return The name of the column,
     */
    @Override
    public String getColumnName(int aColumn) {
        return model.getColumnName(aColumn); 
    }

    /**
     * Provides access to the Class of the values in the cells in the supplied
     * column.
     * 
     * @param aColumn The column index.
     * @return The Class of the column cells.
     */
    @Override
    public Class<?> getColumnClass(int aColumn) {
        return model.getColumnClass(aColumn); 
    }
        
    /**
     * Checks whether the supplied cell may be edited.
     * 
     * @param row The row of the cell.
     * @param column The column of the cell.
     * @return true if ut may be edited, false otherwise.
     */
    @Override
    public boolean isCellEditable(int row, int column) { 
         return model.isCellEditable(row, column); 
    }
 
    /**
     * Called when the data model changed. All interested components will be
     * notified.
     * 
     * @param e The event that occurred.
     */
    @Override
    public void tableChanged(TableModelEvent e) {
        fireTableChanged(e);
    }
}

