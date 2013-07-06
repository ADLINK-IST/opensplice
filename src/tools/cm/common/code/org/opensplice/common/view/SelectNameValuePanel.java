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
package org.opensplice.common.view;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Arrays;

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
            field = new JComboBox();
        } else {
            Arrays.sort(values);
            field = new JComboBox(values);

            if (defaultValue != null) {
                ((JComboBox) field).setSelectedItem(defaultValue);
            }
            field.setToolTipText((((JComboBox) field).getSelectedItem()).toString());
            ((JComboBox) field).setRenderer(new SelectNameValuePanelRenderer());

            ((JComboBox) field).addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent evt) {
                    if (((JComboBox) field).getSelectedItem() != null) {
                        field.setToolTipText((((JComboBox) field).getSelectedItem()).toString());
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
        ((JComboBox) field).removeItemAt(index);
        ((JComboBox) field).insertItemAt(values, index);
    }

    public void insertSelectNameValuePanel(Object values) {
        ((JComboBox) field).addItem(values);
    }

    public void removeSelectNameValuePanel(Object values) {
        ((JComboBox) field).removeItem(values);
    }

    @Override
    public Object getValue(){
        return ((JComboBox)field).getSelectedItem();
    }
    
    @Override
    public void setEnabled(boolean enabled) {
        ((JComboBox)field).setEditable(false);
        ((JComboBox)field).setEnabled(false);
     
    }
    
    private static class SelectNameValuePanelRenderer extends DefaultListCellRenderer {
        @Override
        public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
            JLabel out = (JLabel)super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
            if (value != null) {
                out.setToolTipText(value.toString());
                out.createToolTip().setVisible(true);
            }
            return out;
        }
    }
}
