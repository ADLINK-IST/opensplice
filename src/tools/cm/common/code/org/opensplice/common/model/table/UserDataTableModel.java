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
package org.opensplice.common.model.table;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Vector;

import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.UserDataFilter;

/**
 * Represents a table model that holds a list of UserData instances. Each
 * field in the UserData is represented by a column in the table. Each row 
 * represents an instance of UserData. This table model can only hold UserData
 * instances of one specific type.  
 * 
 * @date Oct 21, 2004 
 */
public class UserDataTableModel extends DefaultTableModel {
    /**
     * The type of the UserData instances that this model can contain. 
     */
    protected MetaType userDataType;
    
    /**
     * List of field names that are visible as colomn in the model.
     */
    protected List visibleFieldNames;
    
    /**
     * List of field names that are not visible as colomn in the model.
     */
    protected List invisibleFieldNames;
    
    /**
     * List of all UserData instances in the model.
     */
    protected List content;
    
    /**
     * List of VISIBLE UserData instances in the model. This List is a subset
     * of the content List.
     */
    protected List visibleContent;
    
    /**
     * Sorter that is able to sort data in this model on column (ascending and
     * descending)
     */
    protected UserDataTableSorter sorter;
    
    /**
     * List of UserDataFilter objects that currently have been applied to the
     * data in the model. The filters determine which data from the content List
     * is also in the visibleContent list. 
     */
    protected List filters;
    
    protected SampleInfoTableModel sampleInfoModel;
    
    /**
     * Constructs a new model that can hold UserData, which type matches the 
     * supplied type. All fields are visible in the model by default.
     *
     * @param _userDataType The type of the UserData that this model can 
     *                      contain.
     */
    public UserDataTableModel(MetaType _userDataType) throws CommonException{
        super();
        if(_userDataType == null){
            throw new CommonException("Data type is not valid.");
        }
        userDataType = _userDataType;
        content = Collections.synchronizedList(new ArrayList());
        visibleContent = Collections.synchronizedList(new ArrayList());
        filters = Collections.synchronizedList(new ArrayList());
        visibleFieldNames = Collections.synchronizedList(new ArrayList());
        invisibleFieldNames = Collections.synchronizedList(new ArrayList());
        String[] fields = userDataType.getExtendedFieldNames();
        
        
        for(int i=0; i<fields.length; i++){
            invisibleFieldNames.add(fields[i]);
        }
        this.makeAllFieldsVisible();
        this.sampleInfoModel = new SampleInfoTableModel();
        int rowCount = this.sampleInfoModel.getRowCount();
        
        for(int i=0; i<rowCount; i++){
            invisibleFieldNames.add("#" + this.sampleInfoModel.getValueAt(i, 0));
        }
        sorter = null;
    }
    
    /**
     * Provides access to the UserData at the specified index.
     * 
     * @param index The index of the UserData.
     * @return The UserData at the supplied ondex, or null if the index was not
     *         available.
     */
    public Sample getDataAt(int index){
        Sample result = null;

        try{
            /*Data might be sorted.*/
            if(sorter != null){
                index = sorter.getModelRow(index);
            }
            synchronized(visibleContent){
                result = (Sample)(visibleContent.get(index));
                
            }
        } catch(IndexOutOfBoundsException e){}
        return result;
    }
    
    public int getVisibleContentCount(){
        return visibleContent.size();
    }
    
    public int getAllContentCount(){
        return content.size();
    }
    
    /**
     * Adds the supplied data to the model if its type matches the type of the
     * model.
     * 
     * @param data The UserData to add.
     * @return true if succeeded, false otherwise. It can fail if the type of
     *         data does not match the type of the table or the data == null.
     */
    public boolean setData(Sample sample){
        boolean match = true;
        String fieldName;
        UserData data = sample.getMessage().getUserData();
        
        if(data == null){
            return false;
        }
        if(data.getUserDataType() != userDataType){
            return false;
        }
        this.sampleInfoModel.setData(sample);
        int columnCount = this.getColumnCount();
        Object[] temp = new Object[columnCount];
        
        for(int i=0; i<columnCount; i++){
            fieldName = this.getColumnName(i);
            
            if(fieldName.startsWith("#")){
                temp[i] = this.sampleInfoModel.getStringValueForRow(fieldName.substring(1));
            } else {
                temp[i] = data.getFieldValue(fieldName);
            }
        }
        
        synchronized(content){
            synchronized(visibleContent){
                synchronized(filters){
                    Iterator iter = filters.iterator();
                    UserDataFilter filter;
                    
                    content.add(sample);
                    
                    while(iter.hasNext() && match){
                        filter = (UserDataFilter)iter.next();
                        
                        if(!(filter.matches(temp))){
                            match = false;
                        }
                    }
                    if(match){
                        this.addRow(temp);
                        visibleContent.add(sample);
                        
                        if(sorter != null){
                            sorter.resort();
                        }
                    } else {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    
    /**
     * Provides access to the names of the fields that are currently visible as
     * columns.
     * 
     * @return The list of visible field names.
     */
    public String[] getVisibleFieldNames(){
        String[] result = null;
        
        synchronized(visibleFieldNames){
            result = (String[])(visibleFieldNames.toArray(
                    new String[visibleFieldNames.size()]));
        }
        return result;
    }
    
    /**
     * Provides access to the names of the fields that are currently not 
     * visible as columns.
     * 
     * @return The list of invisible field names.
     */
    public String[] getInvisibleFieldNames(){
        String[] result = null;
        
        synchronized(invisibleFieldNames){
            result = (String[])(invisibleFieldNames.toArray(
                    new String[invisibleFieldNames.size()]));
        }
        return result;
    }
    
    /**
     * Makes the supplied field name visible as column in the table.
     * 
     * @param fieldName The name of the field that must be made visible.
     *                  A sample info field must be pre-fixed with '#'.
     */
    public void makeFieldVisible(String fieldName){
        UserData data;
        Sample sample;
        
        if(!(this.listContainsValue(visibleFieldNames, fieldName))){
            synchronized(visibleFieldNames){
                synchronized(invisibleFieldNames){
                    synchronized(content){
                        visibleFieldNames.add(fieldName);
                        invisibleFieldNames.remove(fieldName);
                        
                        Vector columnData = new Vector();
                        ListIterator iter = content.listIterator();
                        
                        while(iter.hasNext()){
                            sample = (Sample)iter.next();
                            
                            if(fieldName.startsWith("#")){
                                this.sampleInfoModel.setData(sample);
                                columnData.add(this.sampleInfoModel.getStringValueForRow(fieldName.substring(1)));
                            } else {
                                data = sample.getMessage().getUserData();
                                columnData.add(data.getFieldValue(fieldName));
                            }
                        }
                        this.addColumn(fieldName, columnData);
                    }
                }
            }
        }
    }
    
    /**
     * Makes the supplied field name invisible as column in the table.
     * 
     * @param fieldName The name of the field that must be made invisible.
     */
    public void makeFieldInvisible(String fieldName){
        if(!(this.listContainsValue(invisibleFieldNames, fieldName))){
            synchronized(visibleFieldNames){
                synchronized(invisibleFieldNames){
                    visibleFieldNames.remove(fieldName);
                    invisibleFieldNames.add(fieldName);
                    
                    this.removeColumn(fieldName);
                }
            }
        }
    }
    
    /**
     * Makes all fields be visible as columns.
     */
    public void makeAllFieldsVisible(){
        String[] names = userDataType.getExtendedFieldNames();
        
        for(int i=0; i<names.length; i++){
            this.makeFieldVisible(names[i]);
        }
    }
    
    /**
     * Makes all fields invisible.
     */
    public void makeAllFieldsInvisible(){
        String[] names = userDataType.getExtendedFieldNames();
        
        for(int i=0; i<names.length; i++){
            this.makeFieldInvisible(names[i]);
        }
    }
    
    public void makeAllInfoFieldsVisible(){
        String sampleInfoFieldName;
        int rowCount = this.sampleInfoModel.getRowCount();
        
        for(int i=0; i<rowCount; i++){
            sampleInfoFieldName = "#" + this.sampleInfoModel.getValueAt(i, 0);
            this.makeFieldVisible(sampleInfoFieldName);
        }
    }
    
    public void makeAllInfoFieldsInvisible(){
        String sampleInfoFieldName;
        int rowCount = this.sampleInfoModel.getRowCount();
        
        for(int i=0; i<rowCount; i++){
            sampleInfoFieldName = "#" + this.sampleInfoModel.getValueAt(i, 0);
            this.makeFieldInvisible(sampleInfoFieldName);
        }
    }
    
    /**
     * Makes sure the model is not editable.
     * 
     * @return Always false.
     */
    public boolean isCellEditable(int row, int column){
        return false;
    }
    
    public boolean isFieldVisible(String fieldName){
        boolean result = false;
        
        if(fieldName != null){
            synchronized(visibleFieldNames){
                result = visibleFieldNames.contains(fieldName);
            }
        }
        return result;
    }
    
    
    /**
     * Removes data from beginRow until endRow from the model.
     * 
     * @param beginRow The begin row.
     * @param endRow The end row.
     */
    public synchronized void clear(int beginRow, int endRow){
        if(beginRow <= endRow){
            if((beginRow >= 0) && (endRow < this.getRowCount()) ){
                
                synchronized(content){
                    for(int i=beginRow; i<=endRow; i++){
                        content.remove(beginRow);
                        visibleContent.remove(beginRow);
                        this.removeRow(beginRow);
                    }
                }
            }
        }
    }
    
    /**
     * Sets the sorter that sorts data in this model.
     * 
     * @param _sorter The sorter to set.
     */
    public void setSorter(UserDataTableSorter _sorter){
        sorter = _sorter;
    }
    
    /**
     * Provides access to the type of UserData in the Sample objects that are
     * in this model.
     * 
     * @return The UserData type.
     */
    public MetaType getUserDataType(){
        return userDataType;
    }
    
    /**
     * Adds a filter to the data. Data that does not match the filter is removed
     * from the visibleContent.
     * 
     * @param filter The filter to apply to the data.
     */
    public void addFilter(UserDataFilter filter){
        synchronized(filters){
            filters.add(filter);
        }
        filter.apply(visibleContent);
    }
    
    /**
     * Removes the supplied filter from the data. Data that was filtered out
     * of the visibleContent by this filter is readded to the visibleContent.
     * 
     * @param filter The filter to remove.
     */
    public void removeFilter(UserDataFilter filter){
        Sample[] copy = null;
        
        synchronized(filters){
            filters.remove(filter);
        }
                
        int rowCount = this.getRowCount();
        
        for(int i=0; i<rowCount; i++){
            this.removeRow(0);
        }
        synchronized(content){
            copy = (Sample[])content.toArray(new Sample[content.size()]);
            content.clear();
            
            synchronized(visibleContent){
                visibleContent.clear();
            }
        }
        for(int i=0; i<copy.length; i++){
            this.setData(copy[i]);
        }
    }
    
    /**
     * Checks whether the supplied filter is currently applied to the model.
     * 
     * @param filter The filter to look for.
     * @return true if it is currently applied, false otherwise.
     */
    public boolean containsFilter(UserDataFilter filter){
        UserDataFilter f;
        
        synchronized(filters){
            Iterator iter = filters.iterator();
            
            while(iter.hasNext()){
                f = (UserDataFilter)iter.next();
                
                if(f.equals(filter)){
                    return true;
                }
            }
        }
        return false;
    }
    
    /**
     * Checks whether one or more filters have currently been applied to the 
     * data.
     * 
     * @return true, if one or more filters have been applied, false otherwise.
     */
    public boolean containsFilter(){
        boolean result = false;
        
        synchronized(filters){
           if(filters.size() > 0){
               result = true;
           }
       }
       return result;
    }
    
    /**
     * Removes all filters that have currently been applied. 
     */
    public void removeFilters(){
        synchronized(filters){
            synchronized(content){
                filters.clear();
                int rowCount = this.getRowCount();
                
                for(int i=0; i<rowCount; i++){
                    this.removeRow(0);
                }
                for(int i=0; i<content.size(); i++){
                    this.setData((Sample)content.get(i));
                }
            }
        }
    }
    
    /**
     * Removes all data from the model.
     */
    public synchronized void clear(){
        synchronized(content){
            synchronized(visibleContent){
                content.clear();
                visibleContent.clear();
                int rowCount = this.getRowCount();
                
                for(int i=0; i<rowCount; i++){
                    this.removeRow(0);
                }
            }
        }
    }
    
    private void removeColumn(String columnName){
        int column = columnIdentifiers.indexOf(columnName);
        columnIdentifiers.removeElement(columnName);
        dataVector.setSize(getRowCount()); 

        for (int i=0; i < dataVector.size(); i++) {
            ((Vector)dataVector.elementAt(i)).removeElementAt(column);
        }
        fireTableStructureChanged();
    }
    
    /**
     * Checks if the supplied string is in the supplied list.
     * 
     * @param list The list to look in.
     * @param value The value to look for.
     * @return true if value is in the list, false otherwise.
     */
    private boolean listContainsValue(List list, String value){
        if(list.indexOf(value) == -1){
            return false;
        }
        return true;
    }
}
