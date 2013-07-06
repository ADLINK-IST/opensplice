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
package org.opensplice.common.model;

import java.util.List;

import javax.swing.table.AbstractTableModel;

import org.opensplice.common.model.table.UserDataTableSorter;
import org.opensplice.common.view.table.UserDataTable;

/**
 * Filter that can be applied on data in a UserDataTable. This filter is 
 * meant to filter out data in such a table. It allows the user to only display
 * rows where the value of a specific field has a specific value.
 * 
 * @date Nov 8, 2004 
 */
public class UserDataFilter {
    private String columnName;
    private String value;
    private UserDataTable table;
    
    /**
     * Constructs a new UserDataFilter that can be applied on the supplied 
     * table.
     *  
     *
     * @param _table The table where to apply the filter on.
     * @param _value The value of the field.
     * @param _columnName The field name (which is the column name in a 
     *                    UserDataTable).
     */
    public UserDataFilter(UserDataTable _table, String _value, String _columnName){
        value = _value;
        columnName = _columnName;
        table = _table;
    }
    
    /**
     * Provides access to value.
     * 
     * @return Returns the value.
     */
    public String getValue() {
        return value;
    }
    
    /**
     * Provides access to columnName.
     * 
     * @return Returns the columnName.
     */
    public String getColumnName() {
        return columnName;
    }
    
    /**
     * Provides access to table.
     * 
     * @return Returns the table.
     */
    public UserDataTable getTable() {
        return table;
    }
    
    /**
     * Whether or not the supplied row matches the filter and may be displayed.
     * 
     * @param data The row of data.
     * @return true if it matches, false otherwise.
     */
    public boolean matches(Object[] data){
        String tmp;
        int column = ((AbstractTableModel)table.getModel()).findColumn(columnName);
        table.getColumnModel().getColumnIndex(columnName);
        
        if(data.length > column){
            tmp = ((String)data[column]);
            
            if(tmp == null){
                tmp = "";
            }
            
            if(tmp.equals(value)){
                return true;
            }
        }
        return false;
    }
    
    /**
     * Applies the filter on the table and removes all data from the supplied
     * list that will no longer be displayed.
     * 
     * @param visibleContent The list of currently visible data.
     */
    public void apply(List visibleContent){
        String val;
        int column = table.getColumn(columnName).getModelIndex();
        int rowCount = table.getRowCount();
        UserDataTableSorter sorter = (UserDataTableSorter)table.getModel();
        
        for(int i=0; i<rowCount; i++){
            val = (String)sorter.getValueAt(i, column);
            
            if(val == null){
                val = "";
            }
            if(!(val.equals(value))){
                if(visibleContent != null){
                    synchronized(visibleContent){
                        visibleContent.remove(i);
                    }
                }
                sorter.removeRow(i);
                sorter.resort();
                i--;
                rowCount--;
            }
        }
        table.changeSelection(table.getRowCount()-1, table.getColumnModel().getColumnIndex(columnName), false, false); 
    }
    
    /**
     * Checks whether the supplied object equals this UserDataFilter.
     * 
     * @param obj The object that must equal this object.
     * @return true if the table, columnName and value match, false otherwise.
     */
    public boolean equals(Object obj){
        if(obj instanceof UserDataFilter){
            UserDataFilter filter = (UserDataFilter)obj;
            
            if(filter.getTable().equals(this.table) &&
               filter.getColumnName().equals(this.columnName) &&     
               filter.getValue().equals(this.value))
            {
                return true;
            }
        }
        return false;
    }
    
    public int hashCode(){
        throw new UnsupportedOperationException();
    }
}
