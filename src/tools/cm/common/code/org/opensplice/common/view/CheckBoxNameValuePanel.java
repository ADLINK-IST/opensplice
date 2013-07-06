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

import java.awt.Dimension;

import javax.swing.JCheckBox;

/**
 * 
 * 
 * @date Apr 5, 2005 
 */
public class CheckBoxNameValuePanel extends NameValuePanel {
    public CheckBoxNameValuePanel(String fieldName, boolean selected){
        this(fieldName, selected, null, null);
    }
    
    /**
     *  
     *
     * @param fieldName
     * @param defaultValue
     * @param labelDim
     * @param fieldDim
     */
    public CheckBoxNameValuePanel(String fieldName, boolean selected, 
                                Dimension labelDim, Dimension fieldDim) 
    {
        super(fieldName, null, false, labelDim, fieldDim);
        
        field = new JCheckBox();
        ((JCheckBox)field).setSelected(selected);
        this.add(field);
    }

    public Object getValue() {
        return new Boolean(((JCheckBox)field).isSelected());        
    }

    public void setEnabled(boolean enabled) {
        ((JCheckBox)field).setEnabled(enabled);
    }

}
