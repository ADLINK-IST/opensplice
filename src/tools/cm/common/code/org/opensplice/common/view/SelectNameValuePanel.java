/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
        super(fieldName, defaultValue, false, labelDim, fieldDim);
        Arrays.sort(values);
        field = new JComboBox(values);
        
        if(defaultValue != null){
            ((JComboBox)field).setSelectedItem(defaultValue);
        }
        field.setToolTipText((((JComboBox)field).getSelectedItem()).toString());
        ((JComboBox)field).setRenderer(new SelectNameValuePanelRenderer());
        
        ((JComboBox)field).addActionListener(
        new ActionListener(){
            public void actionPerformed(ActionEvent evt){
                field.setToolTipText((((JComboBox)field).getSelectedItem()).toString());
            }
        });
        
        field.setMinimumSize(this.fieldDim);
        field.setPreferredSize(this.fieldDim);
        field.setMaximumSize(this.fieldDim);
        
        this.add(field);
    }
    
    public Object getValue(){
        return ((JComboBox)field).getSelectedItem();
    }
    
    public void setEnabled(boolean enabled) {
        ((JComboBox)field).setEditable(false);
        ((JComboBox)field).setEnabled(false);
     
    }
    
    private class SelectNameValuePanelRenderer extends DefaultListCellRenderer {
        public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
            JLabel out = (JLabel)super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
            out.setToolTipText(value.toString());
            out.createToolTip().setVisible(true);
            return out;
        }
    }
}
