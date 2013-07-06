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
    
    protected void initLabel(){
        return;
    }
    
    public Object getValue() {
        QoSTable table = (QoSTable)field;
        EntityQoSTableModel model = (EntityQoSTableModel)(table.getModel());
        
        try {
            table.applyCurrentQoS();
        } catch (CommonException e) {}
        return model.getQoS();
    }

    public void setEnabled(boolean enabled) {
        ((QoSTable)field).setEnabled(enabled);

    }

}
