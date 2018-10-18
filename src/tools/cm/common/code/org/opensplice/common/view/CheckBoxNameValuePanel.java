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

    @Override
    public Object getValue() {
        return new Boolean(((JCheckBox)field).isSelected());        
    }

    @Override
    public void setEnabled(boolean enabled) {
        ((JCheckBox)field).setEnabled(enabled);
    }

}
