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

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
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
     * 
     */
    private static final long      serialVersionUID = -6808068073551184983L;

    /**
     * The type of the UserData instances that this model can contain.
     */
    protected MetaType userDataType;

    /**
     * List of field names that are visible as colomn in the model.
     */
    protected List<String>         visibleFieldNames;

    /**
     * List of field names that are not visible as colomn in the model.
     */
    protected List<String>         invisibleFieldNames;

    /**
     * List of all UserData instances in the model.
     */
    protected List<Sample>         content;

    /**
     * List of VISIBLE UserData instances in the model. This List is a subset
     * of the content List.
     */
    protected List<Sample>         visibleContent;

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
    protected List<UserDataFilter> filters;

    protected SampleInfoTableModel sampleInfoModel;
    protected String               structDetail = null;
    protected String[]             names        = null;

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
        content = Collections.synchronizedList(new ArrayList<Sample>());
        visibleContent = Collections.synchronizedList(new ArrayList<Sample>());
        filters = Collections.synchronizedList(new ArrayList<UserDataFilter>());
        visibleFieldNames = Collections.synchronizedList(new ArrayList<String>());
        invisibleFieldNames = Collections.synchronizedList(new ArrayList<String>());
        LinkedHashMap<String, String> lhm = userDataType.collectAllFieldNames(1, false);
        names = lhm.keySet().toArray(new String[0]);

        invisibleFieldNames.add("index");
        for (int i = 0; i < names.length; i++) {
            invisibleFieldNames.add(names[i]);
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
     * Constructs a new model that can hold UserData, which type matches the
     * supplied type. All fields are visible in the model by default.
     *
     * @param _userDataType The type of the UserData that this model can
     *                      contain.
     */
    public UserDataTableModel(MetaType _userDataType, String struct) throws CommonException{
        super();
        if(_userDataType == null){
            throw new CommonException("Data type is not valid.");
        }
        structDetail = struct;
        userDataType = _userDataType;
        content = Collections.synchronizedList(new ArrayList<Sample>());
        visibleContent = Collections.synchronizedList(new ArrayList<Sample>());
        filters = Collections.synchronizedList(new ArrayList<UserDataFilter>());
        visibleFieldNames = Collections.synchronizedList(new ArrayList<String>());
        invisibleFieldNames = Collections.synchronizedList(new ArrayList<String>());
        LinkedHashMap<String, String> lhm = userDataType.collectAllFieldNames(1, struct, false);
        names = lhm.keySet().toArray(new String[0]);


        invisibleFieldNames.add("index");
        for (int i = 0; i < names.length; i++) {
            invisibleFieldNames.add(names[i]);
        }
        
        this.makeAllFieldsVisible(struct);
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
            synchronized (visibleContent) {
                synchronized (content) {
                    synchronized (filters) {
                        if (sorter != null) {
                            index = sorter.getModelRow(index);
                        }
                        result = (visibleContent.get(index));
                    }
                }
            }
        } catch(IndexOutOfBoundsException e){}
        return result;
    }

    public int getVisibleContentCount(){
        synchronized (content) {
            return visibleContent.size();
        }
    }

    public int getAllContentCount(){
        synchronized (content) {
            return content.size();
        }
    }
    
    public boolean setDataAt(Sample sample, int index){
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
                    Iterator<UserDataFilter> iter = filters.iterator();
                    UserDataFilter filter;

                    content.add(sample);

                    while(iter.hasNext() && match){
                        filter = iter.next();

                        if(!(filter.matches(temp))){
                            match = false;
                        }
                    }
                    if(match){
                        this.insertRow(index,temp);
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
    
    public boolean addNewSample(Object[] o, Sample sample, int index) {
        boolean match = true;
        synchronized(content){
            synchronized(visibleContent){
                synchronized(filters){
                    Iterator<UserDataFilter> iter = filters.iterator();
                    UserDataFilter filter;

                    content.add(sample);

                    while(iter.hasNext() && match){
                        filter = iter.next();

                        if(!(filter.matches(o))){
                            match = false;
                        }
                    }
                    if(match){
                        this.insertRow(index,o);
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
                    Iterator<UserDataFilter> iter = filters.iterator();
                    UserDataFilter filter;

                    content.add(sample);

                    while(iter.hasNext() && match){
                        filter = iter.next();

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

    private HashMap<String, ArrayList<String>> CollectTableData(UserData data, String struct) {
        LinkedHashSet<String> s;
        List<String> tmp;

        MetaType mt = data.getUserDataType();
        s = new LinkedHashSet<String>(data.getUserData().keySet());
        HashMap<String, ArrayList<String>> tableVales = new HashMap<String, ArrayList<String>>();
        tmp = new ArrayList<String>();
        /* strip [] from userdata */
        for (String key : s) {

            String newKey = mt.removeIndicesFromStruct(key, struct);
            tmp.add(newKey);
            if (tableVales.containsKey(newKey)) {
                ArrayList<String> tmpVal = tableVales.get(newKey);
                tmpVal.add(data.getUserData().get(key));
                tableVales.put(newKey, tmpVal);
            } else {
                ArrayList<String> tmpVal = new ArrayList<String>();
                tmpVal.add(data.getUserData().get(key));
                tableVales.put(newKey, tmpVal);
            }
        }
        return tableVales;
    }

    public boolean setData(Sample sample, String colName) {
        UserData data;
        boolean match = true;

        boolean result = true;
        String typeName = null;
        boolean res = false;
        
        synchronized(content){
            res = !content.contains(sample);
        }
        if (res) {
            if(sample != null){
                data = sample.getMessage().getUserData();
                if (data == null) {
                    return false;
                }
                String columnName = null;
                int columnCount = this.getColumnCount();
                int nrOfRows = 0;
                HashMap<String, ArrayList<String>> tableData = CollectTableData(data, colName);
                for (int j = 1; j < columnCount; j++) {
                    if (tableData.containsKey(this.getColumnName(j))) {
                        ArrayList<String> t = tableData.get(this.getColumnName(j));
                        if (nrOfRows < t.size()) {
                            nrOfRows = t.size();
                        }
                    }
                }
                if (nrOfRows == 0 && tableData.size() > 0) {
                    if (tableData.containsKey(colName)) {
                        ArrayList<String> t = tableData.get(colName);
                        if (nrOfRows < t.size()) {
                            nrOfRows = t.size();
                        }
                    }
                }

                for (int k = 0; k < nrOfRows && result; k++) {
                    Object[] tmp = new Object[columnCount];
                    tmp[0] = k;
                    LinkedHashSet<String> s = new LinkedHashSet<String>(data.getUserData().keySet());
                    for (int i = 1; i < columnCount; i++) {
                        columnName = this.getColumnName(i);
                        String actualFieldName = "";
                        if (columnName.startsWith(structDetail)) {
                            actualFieldName = columnName.substring(structDetail.length());
                        }
                        if (!actualFieldName.startsWith(".value") && tableData.containsKey(columnName)) {
                            ArrayList<String> colVals = tableData.get(columnName);
                            if (k >= colVals.size()) { // we have a union with
                                                      // only 1 kind value pair
                                tmp[i] = colVals.get(0);
                            } else {
                                tmp[i] = colVals.get(k);
                            }
                        } else if (!actualFieldName.startsWith(".value") && tableData.containsKey(colName)) {
                            ArrayList<String> colVals = tableData.get(colName);
                            if (k >= colVals.size()) { // we have a union with
                                                      // only 1 kind value pair
                                tmp[i] = colVals.get(0);
                            } else {
                                tmp[i] = colVals.get(k);
                            }
                        } else {
                            String name = columnName;
                            if (data.isCollection(structDetail) && columnName.startsWith(structDetail)) {
                                name = structDetail + "[" + k + "]" + columnName.substring(structDetail.length());
                            }
                            LinkedHashSet<String> tmpS = new LinkedHashSet<String>(s);
                            String value = null;
                            for (String key : tmpS) {
                                if (key.startsWith(name)) {
                                    if (value != null) {
                                        value = value + "," + data.getUserData().get(key);
                                    } else {
                                        value = data.getUserData().get(key);
                                    }
                                    s.remove(key);
                                }
                            }
                            tmp[i] = value;
                        }
                    }
                    if (tmp != null) {
                        synchronized (content) {
                            synchronized (visibleContent) {
                                synchronized (filters) {
                                    Iterator<UserDataFilter> iter = filters.iterator();
                                    UserDataFilter filter;

                                    content.add(sample);

                                    while (iter.hasNext() && match) {
                                        filter = iter.next();

                                        if (!(filter.matches(tmp))) {
                                            match = false;
                                        }
                                    }
                                    if (match) {
                                        this.addRow(tmp);
                                        visibleContent.add(sample);
                                        if (sorter != null) {
                                            sorter.resort();
                                        }
                                    } else {
                                        result = false;
                                    }
                                }
                            }
                        }
                        result = true;
                    }
                }
            }
        }
        return result;
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
            result = (visibleFieldNames.toArray(
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
            result = (invisibleFieldNames.toArray(
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
        synchronized (visibleFieldNames) {
            if (!(this.listContainsValue(visibleFieldNames, fieldName))) {
                synchronized(invisibleFieldNames){
                    synchronized(content){
                        visibleFieldNames.add(fieldName);
                        invisibleFieldNames.remove(fieldName);

                        Vector<String> columnData = new Vector<String>();
                        ListIterator<Sample> iter = content.listIterator();

                        while(iter.hasNext()){
                            sample = iter.next();

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
        synchronized (invisibleFieldNames) {
            if (!(this.listContainsValue(invisibleFieldNames, fieldName))) {
                synchronized (visibleFieldNames) {
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
        for(int i=0; i<names.length; i++){
            this.makeFieldVisible(names[i]);
        }
    }

    public void makeAllFieldsVisible(String colName){
        this.makeFieldVisible("index");
        for(int i=0; i<names.length; i++){
            this.makeFieldVisible(names[i]);
        }
    }


    /**
     * Makes all fields invisible.
     */
    public void makeAllFieldsInvisible(){
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
    @Override
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
        synchronized (visibleContent) {
            synchronized (content) {
                synchronized (filters) {
                    sorter = _sorter;
                }
            }
        }
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
        synchronized (content) {
            filter.apply(visibleContent);
        }
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
            copy = content.toArray(new Sample[content.size()]);
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
            Iterator<UserDataFilter> iter = filters.iterator();

            while(iter.hasNext()){
                f = iter.next();

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
                    this.setData(content.get(i));
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
    
    public void clean() {
        this.getDataVector().removeAllElements();
    }

    private void removeColumn(String columnName){
        int column = columnIdentifiers.indexOf(columnName);
        columnIdentifiers.removeElement(columnName);
        dataVector.setSize(getRowCount());

        for (int i=0; i < dataVector.size(); i++) {
            ((Vector<?>) dataVector.elementAt(i)).removeElementAt(column);
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
    private boolean listContainsValue(List<String> list, String value) {
        if(list.indexOf(value) == -1){
            return false;
        }
        return true;
    }
}
