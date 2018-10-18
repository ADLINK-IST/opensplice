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
package org.opensplice.config.swing;

import java.awt.Color;
import java.awt.Component;
import java.util.logging.Logger;

import javax.swing.JTable;
import javax.swing.table.DefaultTableCellRenderer;

import org.opensplice.common.util.ConfigModeIntializer;
import org.opensplice.config.data.DataNode;

public class DataElementTableCellRenderer extends DefaultTableCellRenderer {
    private static final long serialVersionUID = 8091366819992773074L;
    private DataElementTableModel tableModel = null;

    public DataElementTableCellRenderer(DataElementTableModel tableModel){
        this.tableModel = tableModel;
        Logger.getLogger("org.opensplice.config.swing");
    }

    @Override
    public Component getTableCellRendererComponent(JTable table,
                                                   Object value,
                                                   boolean isSelected,
                                                   boolean hasFocus,
                                                   int row,
                                                   int column){
        Component comp = super.getTableCellRendererComponent (table,
           value, isSelected, hasFocus, row, column);
        DataNode node = tableModel.getNodeAt(row);
        node = node.getParent();
        table.setToolTipText(null);
        
        if (ConfigModeIntializer.CONFIGURATOR_MODE != ConfigModeIntializer.COMMERCIAL_MODE) {
            if (node.getMetadata().getVersion().equals(ConfigModeIntializer.COMMERCIAL)) {
                if (ConfigModeIntializer.CONFIGURATOR_MODE != ConfigModeIntializer.COMMUNITY_MODE_FILE_OPEN) {
                    comp.setBackground (Color.LIGHT_GRAY);
                    comp.setForeground (Color.GRAY);
                    table.setToolTipText("This element is not part of the community edition");
                } else {
                    comp.setBackground (Color.RED);
                    comp.setForeground (Color.BLACK);
                    table.setToolTipText("This element is found in the configuration file but not part of the community edition");
                }
            } else {
                comp.setBackground (Color.WHITE);
                comp.setForeground (Color.BLACK);
                table.setToolTipText(null);
            }
        } else {
            comp.setBackground (Color.WHITE);
            comp.setForeground (Color.BLACK);
            table.setToolTipText(null);
        }
        return comp;
    }
}
