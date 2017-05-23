/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
