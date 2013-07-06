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
import java.awt.Font;

import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.UIManager;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.table.TableCellRenderer;

import org.opensplice.common.util.Config;

/**
 * Renderer that is capable of rendering cells in a QoSTable.
 * 
 * @date Nov 8, 2004
 */
public class QoSTableCellRenderer extends JLabel implements TableCellRenderer {
    protected static final Border noFocusBorder = new EmptyBorder(1, 1, 1, 1);

    /**
     * Constructs a new UserDataTableCellRenderer that can be attached to a
     * JTable. This class is meant to render cells in a UserDataTable. It colors
     * the background of certain cells green, to notify the user of keys in the
     * data.
     * 
     * @param _highlight The list of keys.
     *  
     */
    public QoSTableCellRenderer() {
        super();
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

            if ((!(table.isCellEditable(row, table.getColumnCount() - 1))) && (column == table.getColumnCount() - 1)) {
                super.setForeground(Config.getInactiveColor());
                setToolTipText("This policy is not editable.");
                setFont(table.getFont().deriveFont(Font.ITALIC));
            } else {
                super.setBackground(table.getBackground());
                setToolTipText("Click to edit policy.");
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
