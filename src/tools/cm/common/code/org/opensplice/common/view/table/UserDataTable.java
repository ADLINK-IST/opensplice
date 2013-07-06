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
package org.opensplice.common.view.table;

import java.awt.Point;
import java.awt.event.InputEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;

import org.opensplice.common.model.UserDataFilter;
import org.opensplice.common.model.table.UserDataTableModel;
import org.opensplice.common.model.table.UserDataTableSorter;
import org.opensplice.common.util.Config;
import org.opensplice.common.view.StatusPanel;

/**
 * Represents a table that contains a list of UserData instances. Each field
 * in the data is represented as a column and each row in the table represents
 * one instance of UserData. 
 * 
 * This table allows:
 * - Sorting the data on column (ascending and descending).
 * - Removal of columns in data. 
 * - Filtering data on a value in the table. Rows that do not match the applied
 *   filter(s) is made invisible.
 * 
 * @date Oct 22, 2004 
 */
public class UserDataTable extends JTable{
	private String userDataKeys = null;
    
    /**
     * Constructs a new UserDataTable that displays a UserDataTableSorter,
     * which contains a UserDataTableModel. The sorter is (as the name says)
     * able to sort data in the model. 
     *
     * @param sorter The sorter that contains the data and is able to sort the
     *               data in its model.
     * @param keyList The list of keys in the data the model contains.
     */
    public UserDataTable(UserDataTableSorter sorter, String keyList){
        super(sorter);
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        this.setKeyList(keyList);
        this.configureEnclosingScrollPane();
        this.setColumnSizes(100);
    }
    
    /**
     * Sets the width of all columns in the table to the supplied size.
     * 
     * @param size The width to set the columnwidth to.
     */
    private void setColumnSizes(int size){
        TableColumnModel tcm = this.getColumnModel();
        
        for(int i=0; i<tcm.getColumnCount(); i++){
            tcm.getColumn(i).setPreferredWidth(size);
        }
    }
    
    /**
     * Makes the supplied key list visible by coloring the background of the 
     * column headers that contain (parts of) the keylist.
     * 
     * @param keyList The list of keys the table is displaying.
     */
    public void setKeyList(String keyList){
        String[] keys;
        String key;
        
        if(keyList != null){
            userDataKeys = keyList;
            keys = keyList.split(",");
                        
            for(int i=0; i<keys.length; i++){
                key = keys[i];
                
                try{
                    TableColumn tc = this.getColumn(key);
                    
                    if(tc != null){
                        DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();
                        renderer.setBackground(Config.getSensitiveColor());
                        tc.setHeaderRenderer(renderer);
                    }
                } catch(Exception e){
                    /*This key has been removed from the table. Simply proceed.*/
                }
            }
        }
    }
    
    /**
     * Repaints the header of the table.
     *
     */
    public void repaintHeader(){
        this.setKeyList(userDataKeys);
    }
    
    /**
     * Applies a new filter on the table. All row values int the column with the
     * supplied name must contain the supplied value. Rows that do not match 
     * the filter, are made invisible. 
     * 
     * @param columnName The name of the column (clumn header value) to apply 
     *                   the filter on. 
     * @param value The value the cells in the supplied column must contain.
     * @return The newly created and applied filter.
     */
    public UserDataFilter addFilter(String columnName, String value){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
                       
        UserDataFilter filter = new UserDataFilter(this, value, columnName);
        model.addFilter(filter);
        
        TableColumn tc = this.getColumn(columnName);
        
        if(tc != null){
            DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();
            renderer.setBackground(Config.getInactiveColor());
            tc.setCellRenderer(renderer);
            this.repaint();
        }
        return filter;
    }
    
    public boolean isFieldVisible(String field){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        
        return model.isFieldVisible(field);
    }
    
    public void makeFieldVisible(String field){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.makeFieldVisible(field);
    }
    
    public void makeFieldInvisible(String field){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.makeFieldInvisible(field);
    }
    
    public void makeAllFieldsVisible(){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.makeAllFieldsVisible();
    }
    
    public void makeAllInfoFieldsVisible(){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.makeAllInfoFieldsVisible();
    }
    
    public void makeAllFieldsInvisible(){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.makeAllFieldsInvisible();
    }
    
    public void makeAllInfoFieldsInvisible(){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.makeAllInfoFieldsInvisible();
    }
    
    /**
     * Removes the supplied filter from the table. Rows that were made invisible
     * by the supplied filter, are made visible again.
     * 
     * @param filter The filter to remove.
     */
    public void removeFilter(UserDataFilter filter){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
        model.removeFilter(filter);
        TableColumn tc = null;
        
        try{
            tc = this.getColumn(filter.getColumnName());
        } catch(IllegalArgumentException exc){
            return;
        }
        
        if(tc != null){
            DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();
            tc.setCellRenderer(renderer);
            this.repaint();
        }
    }
    
    /**
     * Checks whether a filter on the specified column and value is currently 
     * applied to this table.
     * 
     * @param columnName The name of the column to look for.
     * @param value The value of the cells in the specified column.
     * @return The applied filter if the filter was found, or null if it was
     *         not applied.
     */
    public UserDataFilter resolveFilter(String columnName, String value){
        UserDataFilter result = null;
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        UserDataTableModel model = (UserDataTableModel)(sorter.getModel());
                
        UserDataFilter filter = new UserDataFilter(this, value, columnName);
        
        if(model.containsFilter(filter)){
            result = filter;
        }
        return result;
    }
    
    /**
     * Checks whether one or more filters are currently applied on the table.
     * 
     * @return True if one or more filters are applied, false otherwise.
     */
    public boolean containsFilters(){
        UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        return ((UserDataTableModel)(sorter.getModel())).containsFilter();
    }
    
    /**
     * Sorts the specified column ascending or descending.
     * 
     * @param column The column to sort.
     * @param ascending If true, the data is sorted ascending. It is sorted 
     *                  descending otherwise.
     */
    public void sort(int column, boolean ascending){
        AbstractTableModel aModel = (AbstractTableModel)((UserDataTableSorter)getModel()).getModel();
        String tableColName = null;
        
        /* columns might have been moved to another index in the table
         * Look up index in model.
         */
        if(!(aModel.getColumnName(column).equals(columnModel.getColumn(column).getIdentifier()))){
            boolean found = false;
            int i = 0;
            tableColName = (String)columnModel.getColumn(column).getIdentifier();
            
            while(!found){
                if(aModel.getColumnName(i).equals(tableColName)){
                    column = i;
                    found = true;
                }
                i++;
            }
        }
        ((UserDataTableSorter)(dataModel)).sortByColumn(column, ascending);
    }
    
    /**
     * Repeats the last sort action.
     */
    public void resort(){
        ((UserDataTableSorter)dataModel).resort();
    }
    
    /**
     * Enables the sorting of columns in the table by mouse;
     * - RIGHT_CLICK : Sort ascending
     * - SHIFT + RIGHT_CLICK: Sort descending.
     * 
     * @param output The statusbar where to sent sorting information to.
     */
    public void enableColumnSortingByMouse(StatusPanel output) { 
        final UserDataTableSorter sorter = (UserDataTableSorter)dataModel;
        final UserDataTableModel model = (UserDataTableModel)sorter.getModel();
        final StatusPanel sp = output;
        
        setColumnSelectionAllowed(false);
        
        MouseAdapter sortListener = new MouseAdapter() {           
            @Override
            public void mouseClicked(MouseEvent e) {
                int column = columnModel.getColumnIndexAtX(e.getX());

                /* columns might have been moved to another index in the table
                 * Look up index in model.
                 */
                if(!(model.getColumnName(column).equals(columnModel.getColumn(column).getIdentifier()))){
                    String tableColName = (String) columnModel.getColumn(column).getIdentifier();
                    boolean found = false;
                    int i = 0;
                    
                    while(!found){
                        if(model.getColumnName(i).equals(tableColName)){
                            column = i;
                            found = true;
                        }
                        i++;
                    }
                }
                
                if (e.getButton() == MouseEvent.BUTTON3 && e.getClickCount() == 1 && column != -1) {
                    String tableColName = (String) columnModel.getColumn(column).getIdentifier();
                    int shiftPressed = e.getModifiers()&InputEvent.SHIFT_MASK; 
                    boolean asc = (shiftPressed == 0);
                    sorter.sortByColumn(column, asc);
                    
                    if(getRowCount() > 0){
                        changeSelection(0, 0, false, false);
                    }
                    if(sp != null){
                        String ascDesc;
                        
                        if(asc){
                            ascDesc = "ascending";
                        } else {
                            ascDesc = "descending";
                        }
                        sp.setStatus("Sorted column '" + tableColName + "' " + ascDesc + ".", false, false);
                    }
                }
            }
        };
        JTableHeader th = getTableHeader(); 
        th.addMouseListener(sortListener);
        String msg = "Right click to sort ascending, Shift-right click to sort descending.";
        
        if(th.getToolTipText() != null){
            th.setToolTipText(th.getToolTipText() + " " + msg);
        } else{
            th.setToolTipText(msg);
        }
    }
    
    /**
     * Enables the removal of columns in the table by mouse;
     * - CTRL + RIGHT_CLICK: Remove column.
     * 
     * @param output The statusbar where to sent removal information to.
     */
    public void enableColumnRemovalByMouse(StatusPanel output) {
        final StatusPanel sp = output;
        setColumnSelectionAllowed(false);
        
        MouseAdapter listMouseListener = new MouseAdapter() {           
            @Override
            public void mouseClicked(MouseEvent e) {
                int column = columnAtPoint(new Point(e.getX(), e.getY()));
                
                if (e.getButton() == MouseEvent.BUTTON3 && e.getClickCount() == 1 && column != -1) {                        
                    int ctrlPressed = e.getModifiers()&InputEvent.CTRL_MASK; 
                    boolean ctrl = (ctrlPressed != 0);
                    
                    if(ctrl){
                        TableColumn col = columnModel.getColumn(column);
                        removeColumn(col);
                        //columnModel.removeColumn(col);
                        //((AbstractTableModel)getModel()).fireTableStructureChanged();
                        repaintHeader();
                        repaint();
                        if(sp != null){
                            sp.setStatus("Removed column '" + col.getIdentifier() + "'.", false, false);
                        }
                    }
                }
            }
        };
        JTableHeader th = getTableHeader();
        th.addMouseListener(listMouseListener);
        String msg = "Control-right click to remove column.";
        
        if(th.getToolTipText() != null){
            th.setToolTipText(th.getToolTipText() + " " + msg);
        } else{
            th.setToolTipText(msg);
        }
    }
}
