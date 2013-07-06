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

import org.opensplice.common.model.table.SampleInfoTableModel;
import java.awt.Component;
import java.awt.Font;

import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.UIManager;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.table.TableCellRenderer;

import org.opensplice.common.util.Config;

/**
 * Renderer that is capable of rendering cells in a SampleInfoTable.
 * 
 */
public class SampleInfoTableCellRenderer extends JLabel implements TableCellRenderer {
    protected static final Border noFocusBorder = new EmptyBorder(1, 1, 1, 1);
    private SampleInfoTableModel model;

    /**
     * Constructs a new SampleInfoTableCellRenderer that can be attached to a
     * JTable. This class is meant to render cells in a JTable that holds a
     * SampleInfoModel.
     *  
     */
    public SampleInfoTableCellRenderer(SampleInfoTableModel model) {
        super();
        this.model = model;
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
    public Component getTableCellRendererComponent(JTable table, Object value,
            boolean isSelected, boolean hasFocus, int row, int column) {
        if(!table.isEnabled()){
            setToolTipText("Table not enabled.");
            setFont(table.getFont().deriveFont(Font.ITALIC));
            super.setForeground(java.awt.Color.LIGHT_GRAY);
            super.setBackground(table.getBackground());
        } else if (isSelected) {
            super.setForeground(table.getSelectionForeground());
            super.setBackground(table.getSelectionBackground());
        } else {
            super.setForeground(table.getForeground());
            
            if(!model.isCellValueReliableInSnapshot(row, column)){
                if(!("N/A".equals(value))){
                    super.setForeground(Config.getInactiveColor());
                    String attr = model.getValueAt(row, column-1).toString();
                    setToolTipText("The value '" + value.toString() + "' for SampleInfo field '" + attr + "' may not be correct.");
                    setFont(table.getFont().deriveFont(Font.ITALIC));
                } else {
                    super.setBackground(table.getBackground());
                    setFont(table.getFont());
                }
            } else {
                super.setBackground(table.getBackground());
                setFont(table.getFont());
            }
        }
        
        if (hasFocus) {
            setBorder(UIManager.getBorder("Table.focusCellHighlightBorder"));
            if (table.isCellEditable(row, column)) {
                super.setForeground(UIManager
                        .getColor("Table.focusCellForeground"));
                super.setBackground(UIManager
                        .getColor("Table.focusCellBackground"));
            }
        } else {
            setBorder(noFocusBorder);
        }

        if (value == null) {
            setText("");
        } else {
            setText(value.toString());
        }
        return this;
    }
} 
