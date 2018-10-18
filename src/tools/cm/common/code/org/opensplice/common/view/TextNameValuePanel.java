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
    
    @Override
    public Object getValue(){
        return ((JTextField)field).getText();
    }
    
    @Override
    public void setEnabled(boolean enabled) {
        ((JTextField)field).setEditable(false);
        ((JTextField)field).setEnabled(false);
    }
}
