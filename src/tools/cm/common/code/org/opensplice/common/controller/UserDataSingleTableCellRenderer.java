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

import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.UIManager;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.table.TableCellRenderer;

import org.opensplice.common.util.Config;

/**
 * Renderer that is capable of rendering cells in a JTable.
 * 
 * @date Nov 8, 2004 
 */
public class UserDataSingleTableCellRenderer extends JLabel implements TableCellRenderer{
    private String[] highlightKeys = null;
    private String[] highlightRequired = null;
    protected static final Border noFocusBorder = new EmptyBorder(1, 1, 1, 1); 
    
    /**
     * Constructs a new UserDataTableCellRenderer that can be attached to
     * a JTable. This class is meant to render cells in a UserDataTable. It
     * colors the background of certain cells green, to notify the user of 
     * keys in the data.
     *
     * @param _highlight  The list of keys.
     *                   
     */
    public UserDataSingleTableCellRenderer(String[] _highlightKeys, String[] _highlightRequired){
        super();
        highlightKeys = _highlightKeys;
        highlightRequired = _highlightRequired;
        setOpaque(true);
        setBorder(noFocusBorder);
    }
    
    /**
     * Provides a component to display in the table according to the supplied 
     * arguments. This function is called by the table when it needs to render a 
     * cell.
     * 
     * @param table The table in which the cell will be drawn.
     * @param value The value of the cell.
     * @param isSelected Whether or not the cell is selected.
     * @param hasFocus Whether or not the cell has the focus.
     * @param row The row of the cell in the table.
     * @param column The column of the cell in the table.
     */
    @Override
    public Component getTableCellRendererComponent(
            JTable table, Object value, boolean isSelected, boolean hasFocus, 
            int row, int column) 
    {
        if (isSelected) {
           super.setForeground(table.getSelectionForeground());
           super.setBackground(table.getSelectionBackground());
        }
        else {
            super.setForeground(table.getForeground());

            // Key highlight takes precedence over required highlight.
            boolean isKey = containsKey(value.toString());
            boolean isReq = containsRequired(value.toString());
            if (isReq){
                super.setBackground(Config.getCustomColor1());
            }
            if (isKey) {
                super.setBackground(Config.getSensitiveColor());
            }
            if (!isKey && !isReq) {
                super.setBackground(table.getBackground());
            }
        }
        setFont(table.getFont());
        
        if(hasFocus) {
            setBorder( UIManager.getBorder("Table.focusCellHighlightBorder") );
            if (table.isCellEditable(row, column)) {
                super.setForeground( UIManager.getColor("Table.focusCellForeground") );
                super.setBackground( UIManager.getColor("Table.focusCellBackground") );
            }
        } else {
            setBorder(noFocusBorder);
        }
        
        if(value == null){
            setText("");
        } else {
            setText(value.toString());
        }
        return this;
    }
    
    
    private boolean containsKey(String key){
        if(highlightKeys == null){
            return false;
        }
        
        for(int i=0; i<highlightKeys.length; i++){
            if(highlightKeys[i].equals(key)){
                return true;
            }
        }
        return false;
    }
    
    private boolean containsRequired(String required){
        if(highlightRequired == null){
            return false;
        }
        
        for(int i=0; i<highlightRequired.length; i++){
            if(highlightRequired[i].equals(required)){
                return true;
            }
        }
        return false;
    }
}
