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
    public void setValueAt(Object aValue, int aRow, int aColumn) {
        model.setValueAt(aValue, aRow, aColumn); 
    }

    /**
     * Provides access to the number of rows of the model.
     * 
     * @return The number of rows of the model or 0 if no model exists.
     */
    public int getRowCount() {
        return (model == null) ? 0 : model.getRowCount(); 
    }

    /**
     * Provides access to the number of columns of the model.
     * 
     * @return The number of columns of the model or 0 if no model exists.
     */
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
    public Class getColumnClass(int aColumn) {
        return model.getColumnClass(aColumn); 
    }
        
    /**
     * Checks whether the supplied cell may be edited.
     * 
     * @param row The row of the cell.
     * @param column The column of the cell.
     * @return true if ut may be edited, false otherwise.
     */
    public boolean isCellEditable(int row, int column) { 
         return model.isCellEditable(row, column); 
    }
 
    /**
     * Called when the data model changed. All interested components will be
     * notified.
     * 
     * @param e The event that occurred.
     */
    public void tableChanged(TableModelEvent e) {
        fireTableChanged(e);
    }
}

