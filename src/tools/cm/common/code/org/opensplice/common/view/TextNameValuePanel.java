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

import javax.swing.JTextField;

/**
 * 
 * 
 * @date Mar 31, 2005 
 */
public class TextNameValuePanel extends NameValuePanel{
    public TextNameValuePanel(
            String fieldName, 
            String defaultValue, 
            boolean emptyInputAllowed)
    {
        this(fieldName, defaultValue, emptyInputAllowed, null, null);
    }
    
    public TextNameValuePanel(
            String fieldName, 
            String defaultValue, 
            boolean emptyInputAllowed,
            Dimension labelDim,
            Dimension fieldDim)
    {
        super(fieldName, defaultValue, emptyInputAllowed, labelDim, fieldDim);
        
        field = new JTextField();
        
        if(defaultValue != null){
            ((JTextField)field).setText(defaultValue);
        }
        field.setMinimumSize(this.fieldDim);
        field.setPreferredSize(this.fieldDim);
        field.setMaximumSize(this.fieldDim);
        
        this.add(field, null);
    }
    
    public Object getValue(){
        return ((JTextField)field).getText();
    }
    
    public void setEnabled(boolean enabled) {
        ((JTextField)field).setEditable(false);
        ((JTextField)field).setEnabled(false);
    }
}
