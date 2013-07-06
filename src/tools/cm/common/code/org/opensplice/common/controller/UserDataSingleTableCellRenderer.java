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
    private String[] highlight = null;
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
    public UserDataSingleTableCellRenderer(String[] _highlight){
        super();
        highlight = _highlight;
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
            
            if(containsKey(value.toString())){
                super.setBackground(Config.getSensitiveColor());
            } else {
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
        if(highlight == null){
            return false;
        }
        
        for(int i=0; i<highlight.length; i++){
            if(highlight[i].equals(key)){
                return true;
            }
        }
        return false;
    }
}
