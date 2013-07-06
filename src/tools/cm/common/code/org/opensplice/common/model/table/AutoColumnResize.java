package org.opensplice.common.model.table;

import java.awt.Component;

import javax.swing.JTable;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;

public class AutoColumnResize {

    public static void adjustColumnSizeForColumn(JTable table, int column) {

        TableColumnModel columnModel = table.getColumnModel();
        int maxwidth = 0;
        for (int row = 0; row < table.getRowCount(); row++) {
            TableCellRenderer render = table.getCellRenderer(row, column);
            Object value = table.getValueAt(row, column);
            Component component = render.getTableCellRendererComponent(table, value, false, false, row, column);
            if (component != null && component.getPreferredSize() != null) {
                maxwidth = Math.max(component.getPreferredSize().width, maxwidth);
            }
        }
        TableColumn tableColumn = columnModel.getColumn(column);
        tableColumn.setPreferredWidth(maxwidth + 5); /* add +5 for some extra space improves readability */
    }
}