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

/**
 * Tables which contain an instance of UserDataTableSorter can access
 * the actual model because of this class. The table thinks it is accessing
 * its model, but actually it is accessing the UserDataTableSorter that 
 * translates the call into the right call for its UserDataTableModel. 
 *  
 * It looks like this:
@verbatim

        -----------------------
        |      JTable         |
        -----------------------
                ^   |
                |   v
        -----------------------
        | UserDataTableSorter |
        -----------------------
                ^   |
                |   v
        -----------------------
        |  UserDataTableModel |
        -----------------------
@endverbatim
 * 
 * @date Nov 5, 2004 
 */
import java.util.Date;
import java.util.Vector;

import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;

import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.common.view.table.UserDataTable;

public class UserDataTableSorter extends TableMap {
    /**
     * Contains the mapping between actual index of the rows in the model and
     * the sorted index. 
     */
    protected int indexes[];
    
    /**
     * The list of currently sorted columns.
     */
    protected Vector sortingColumns = new Vector();
    
    /**
     * The way of the last sort. (ascending -> true, descending -> false) 
     */
    protected boolean ascending = true;
    
    /**
     * Total number of compares.
     */
    protected int compares;
    
    /**
     * Last column that has been sorted.
     */
    protected int lastSortColumn = -1;

    /**
     * Constructs a new sorter.
     */
    public UserDataTableSorter() {
        indexes = new int[0]; // for consistency
    }

    /** 
     * Constructs a new sorter for the supplied model. 
     *
     * @param model The model to sort.
     */
    public UserDataTableSorter(TableModel model) {
        setModel(model);
    }

    /**
     * Sets the model to sort.
     * 
     * @param model The model to sort.
     */
    @Override
    public void setModel(TableModel model) {
        super.setModel(model); 
        reallocateIndexes();
        
        if(model instanceof UserDataTableModel){
            ((UserDataTableModel)model).setSorter(this);
        }
    }

    @Override
    public void tableChanged(TableModelEvent e) {
        reallocateIndexes();

        super.tableChanged(e);
    }

    /* 
     * The mapping only affects the contents of the data rows.
     * Pass all requests to these rows through the mapping array: "indexes".
     */
    @Override
    public Object getValueAt(int aRow, int aColumn) {
        checkModel();
        return model.getValueAt(indexes[aRow], aColumn);
    }
    
    /**
     * Removes the supplied rows from the model.
     * 
     * @param row The row to remove.
     */
    public void removeRow(int row) {
        checkModel();
        
        if(model instanceof DefaultTableModel){
            ((DefaultTableModel)model).removeRow(indexes[row]);
        }
    }
    
    /**
     * Provides access to the row in the sorted model according to the sorted 
     * row.
     * 
     * @param sorterRow The sorter row.
     * @return The row index in the actual model.
     */
    public int getModelRow(int sorterRow){
        return indexes[sorterRow];
    }

    @Override
    public void setValueAt(Object aValue, int aRow, int aColumn) {
        checkModel();
        model.setValueAt(aValue, indexes[aRow], aColumn);
    }

    /**
     * Sorts the supplied column. The ascending boolean is used to determine the
     * way of sorting.
     * 
     * @param column The column to sort.
     */
    public void sortByColumn(int column) {
        sortByColumn(column, true);
    }

    /**
     * Sorts the supplied column ascending or descending.
     * 
     * @param column The column to sort.
     * @param asc If true, sort ascending, or else sort descending.
     */
    public void sortByColumn(int column, boolean asc) {
        this.ascending = asc;
        sortingColumns.removeAllElements();
        sortingColumns.addElement(new Integer(column));
        sort(this);
        lastSortColumn = column;
        super.tableChanged(new TableModelEvent(this)); 
    }
    
    /**
     * Reexecutes the last sort action.
     */
    public void resort(){
        if(lastSortColumn != -1){
            this.sortByColumn(lastSortColumn, ascending);
        }
    }
    
    /**
     * Removes the supplied column from the supplied table.
     * 
     * @param column The column to remove.
     * @param tableView The table to remove the column from.
     */
    public void removeColumn(int column, JTable tableView){
        TableColumnModel columnModel = tableView.getColumnModel();
        TableColumn col = columnModel.getColumn(column);
        tableView.removeColumn(col);
        tableView.repaint();
        
        if(tableView instanceof UserDataTable){
            ((UserDataTable)tableView).repaintHeader();
        }
    }
    
    private void n2sort() {
        for (int i = 0; i < getRowCount(); i++) {
            for (int j = i+1; j < getRowCount(); j++) {
                if (compare(indexes[i], indexes[j]) == -1) {
                    swap(i, j);
                }
            }
        }
    }

    /*
     * Algorithm requires twice the space of an in-place algorithm and makes
     * NlogN assigments shuttling the values between the two
     * arrays. The number of compares appears to vary between N-1 and
     * NlogN depending on the initial order but the main reason for
     * using it here is that, unlike qsort, it is stable.
     */
    private void shuttlesort(int from[], int to[], int low, int high) {
        if (high - low < 2) {
            return;
        }
        int middle = (low + high) >>> 1;
        shuttlesort(to, from, low, middle);
        shuttlesort(to, from, middle, high);

        int p = low;
        int q = middle;

        /* This is an optional short-cut; at each recursive call,
        check to see if the elements in this subset are already
        ordered.  If so, no further comparisons are needed; the
        sub-array can just be copied.  The array must be copied rather
        than assigned otherwise sister calls in the recursion might
        get out of sinc.  When the number of elements is three they
        are partitioned so that the first set, [low, mid), has one
        element and and the second, [mid, high), has two. We skip the
        optimisation when the number of elements is three or less as
        the first compare in the normal merge will produce the same
        sequence of steps. This optimisation seems to be worthwhile
        for partially ordered lists but some analysis is needed to
        find out how the performance drops to Nlog(N) as the initial
        order diminishes - it may drop very quickly.  */

        if (high - low >= 4 && compare(from[middle-1], from[middle]) <= 0) {
            for (int i = low; i < high; i++) {
                to[i] = from[i];
            }
            return;
        }

        // A normal merge. 

        for (int i = low; i < high; i++) {
            if (q >= high || (p < middle && compare(from[p], from[q]) <= 0)) {
                to[i] = from[p++];
            }
            else {
                to[i] = from[q++];
            }
        }
    }

    private void swap(int i, int j) {
        int tmp = indexes[i];
        indexes[i] = indexes[j];
        indexes[j] = tmp;
    }
    
    private void checkModel() {
        if (indexes.length != model.getRowCount()) {
            //System.err.println("Sorter not informed of a change in model.");
        }
    }
    
    private int compareRowsByColumn(int row1, int row2, int column) {
        Class type = model.getColumnClass(column);
        TableModel data = model;

        // Check for nulls.
        
        Object o1 = data.getValueAt(row1, column);
        Object o2 = data.getValueAt(row2, column); 

        // If both values are null, return 0.
        if (o1 == null && o2 == null) {
            return 0; 
        } else if (o1 == null) { // Define null less than everything. 
            return -1; 
        } else if (o2 == null) { 
            return 1; 
        }

        /*
         * Copy all returned values from the getValue call in case
         * an optimised model is reusing one object to return many
         * values.  The Number subclasses in the JDK are immutable and
         * so will not be used in this way but other subclasses of
         * Number might want to do this to save space and avoid
         * unnecessary heap allocation.
         */
        if(model instanceof UserDataTableModel){
            String columnName = this.getColumnName(column);
            MetaField field = ((UserDataTableModel)model).getUserDataType().getField(columnName);
            
            if(field instanceof MetaPrimitive){
                String typeName = field.getTypeName();
                
                if( "c_long".equals(typeName) || 
                    "c_ulong".equals(typeName) || 
                    "c_octet".equals(typeName) ||
                    "c_short".equals(typeName) ||
                    "c_ushort".equals(typeName) ||
                    "c_float".equals(typeName) ||
                    "c_double".equals(typeName) ||
                    "c_longlong".equals(typeName) ||
                    "c_ulonglong".equals(typeName)
                    )
                {
                    double d1 = Double.parseDouble((String)data.getValueAt(row1, column));
                    double d2 = Double.parseDouble((String)data.getValueAt(row2, column));
                    
                    if (d1 < d2) {
                        return -1;
                    } else if (d1 > d2) {
                        return 1;
                    } else {
                        return 0;
                    }
                }
            }
        }
            
        if (type.getSuperclass() == java.lang.Number.class) {
            double d1 = Double.parseDouble((String)data.getValueAt(row1, column));
            double d2 = Double.parseDouble((String)data.getValueAt(row2, column));
            
            if (d1 < d2) {
                return -1;
            } else if (d1 > d2) {
                return 1;
            } else {
                return 0;
            }
        } else if (type == java.util.Date.class) {
            Date d1 = (Date)data.getValueAt(row1, column);
            long n1 = d1.getTime();
            Date d2 = (Date)data.getValueAt(row2, column);
            long n2 = d2.getTime();

            if (n1 < n2) {
                return -1;
            } else if (n1 > n2) {
                return 1;
            } else {
                return 0;
            }
        } else if (type == String.class) {
            String s1 = (String)data.getValueAt(row1, column);
            String s2    = (String)data.getValueAt(row2, column);
            int result = s1.compareTo(s2);

            if (result < 0) {
                return -1;
            } else if (result > 0) {
                return 1;
            } else {
                return 0;
            }
        } else if (type == Boolean.class) {
            Boolean bool1 = (Boolean)data.getValueAt(row1, column);
            boolean b1 = bool1.booleanValue();
            Boolean bool2 = (Boolean)data.getValueAt(row2, column);
            boolean b2 = bool2.booleanValue();

            if (b1 == b2) {
                return 0;
            } else if (b1) { // Define false < true
                return 1;
            } else {
                return -1;
            }
        } else {
            Object v1 = data.getValueAt(row1, column);
            String s1 = v1.toString();
            Object v2 = data.getValueAt(row2, column);
            String s2 = v2.toString();
            int result = s1.compareTo(s2);

            if (result < 0) {
                return -1;
            } else if (result > 0) {
                return 1;
            } else {
            return 0;
            }
        }
    }

    private int compare(int row1, int row2) {
        compares++;
        for (int level = 0; level < sortingColumns.size(); level++) {
            Integer column = (Integer)sortingColumns.elementAt(level);
            int result = compareRowsByColumn(row1, row2, column.intValue());
            if (result != 0) {
                return ascending ? result : -result;
            }
        }
        return 0;
    }

    private void reallocateIndexes() {
        int rowCount = model.getRowCount();

        // Set up a new array of indexes with the right number of elements
        // for the new data model.
        indexes = new int[rowCount];

        // Initialise with the identity mapping.
        for (int row = 0; row < rowCount; row++) {
            indexes[row] = row;
        }
    }
    
    private void sort(Object sender) {
        checkModel();

        compares = 0;
        shuttlesort(indexes.clone(), indexes, 0, indexes.length);
    }
}

