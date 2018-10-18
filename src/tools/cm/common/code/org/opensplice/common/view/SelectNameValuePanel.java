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
package org.opensplice.common.view;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Arrays;

import javax.swing.DefaultComboBoxModel;
import javax.swing.DefaultListCellRenderer;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;

/**
 * 
 * 
 * @date Mar 31, 2005 
 */
public class SelectNameValuePanel extends NameValuePanel{
    private JComboBox combo;
    /**
     * Creates an input panel with a label and a combobox.
     * @param values The possible values that will be put in the combobox.
     * @param _fieldName The name of the field that is also used as label.
     * @param _emptyInputAllowed Boolean that specifies if empty input is allowed when 
     *                           submitted.
     */
    public SelectNameValuePanel(
            String fieldName, 
            Object[] values, 
            Object defaultValue)
    {
        this(fieldName, values, defaultValue, null, null);
    }
    
    public SelectNameValuePanel(
            String fieldName, 
            Object[] values, 
            Object defaultValue, 
            Dimension labelDim,
            Dimension fieldDim)
    {
        super(fieldName, defaultValue, true, labelDim, fieldDim);
        if (values == null) {
            combo = new JComboBox();
            field = combo;
        } else {
            Arrays.sort(values);
            combo = new JComboBox(values);
            field = combo;

            if (defaultValue != null) {
                combo.setSelectedItem(defaultValue);
            }
            if (combo.getSelectedItem() != null) {
                field.setToolTipText((combo.getSelectedItem()).toString());
            }
            combo.setRenderer(new SelectNameValuePanelRenderer());

            combo.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent evt) {
                    if (combo.getSelectedItem() != null) {
                        field.setToolTipText((combo.getSelectedItem())
                                .toString());
                    }
                }
            });
        }
        field.setMinimumSize(this.fieldDim);
        field.setPreferredSize(this.fieldDim);
        field.setMaximumSize(this.fieldDim);
        this.add(field);
    }
    
    public void updateSelectNameValuePanel(Object values, int index) {
        combo.removeItemAt(index);
        combo.insertItemAt(values, index);
    }

    public void insertSelectNameValuePanel(Object values) {
        combo.addItem(values);
    }

    public void removeSelectNameValuePanel(Object values) {
        combo.removeItem(values);
    }

    @Override
    public Object getValue(){
        return combo.getSelectedItem();
    }
    
    @Override
    public void setEnabled(boolean enabled) {
        combo.setEditable(false);
        combo.setEnabled(false);
     
    }
    
    public void setModel(Object[] values) {
        combo.setModel(new DefaultComboBoxModel(values));
    }
    
    private static class SelectNameValuePanelRenderer extends DefaultListCellRenderer {
        @Override
        public Component getListCellRendererComponent(JList list,
                Object value, int index, boolean isSelected,
                boolean cellHasFocus) {
            JLabel out = (JLabel)super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
            if (value != null) {
                out.setToolTipText(value.toString());
                out.createToolTip().setVisible(true);
            }
            return out;
        }
    }
}
