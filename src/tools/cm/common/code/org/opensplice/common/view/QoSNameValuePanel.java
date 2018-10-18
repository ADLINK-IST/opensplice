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

import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.qos.EntityQoSTableModel;
import org.opensplice.common.view.table.QoSTable;

public class QoSNameValuePanel extends NameValuePanel {
    
    public QoSNameValuePanel(
            String fieldName,
            EntityQoSTableModel qosModel,
            Object defaultValue, 
            Dimension fieldDim)
    {
        super(fieldName, qosModel.getQoS(), false, null, fieldDim);
        
        field = new QoSTable(qosModel, null);
        this.add(field);
    }
    
    @Override
    protected void initLabel(){
        return;
    }
    
    @Override
    public Object getValue() {
        QoSTable table = (QoSTable)field;
        EntityQoSTableModel model = (EntityQoSTableModel)(table.getModel());
        
        try {
            table.applyCurrentQoS();
        } catch (CommonException e) {}
        return model.getQoS();
    }

    @Override
    public void setEnabled(boolean enabled) {
        ((QoSTable)field).setEnabled(enabled);

    }

}
