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
package org.opensplice.common.model.table.status;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Writer;
import org.opensplice.cm.status.WriterStatus;
import org.opensplice.common.CommonException;

/**
 * Concrete descendant of the EntityStatusTableModel object. Its responsibility
 * is to retrieve and administrate the Status of a Writer entity.
 * 
 * @date Oct 19, 2004 
 */
public class WriterStatusTableModel extends EntityStatusTableModel {
    /**
     * Constructs a new table model that holds the Status of the supplied
     * Writer.
     *
     * @param _entity The Writer, which Status must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public WriterStatusTableModel(Writer _entity) throws CommonException {
        super(_entity);
    }

    @Override
    protected void init() {
        Object[] data = new Object[3];
        data[2] = "N/A";
        
        data[0] = "LIVELINESS_LOST";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[0] = "OFFERED_DEADLINE_MISSED";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "last_instance_handle";
        this.addRow(data);
        
        data[0] = "OFFERED_INCOMPATIBLE_QOS";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "last_policy_id";
        this.addRow(data);
        
        data[1] = "policies";
        this.addRow(data);
        
        data[0] = "PUBLICATION_MATCHED";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "current_count";
        this.addRow(data);
        
        data[1] = "current_count_change";
        this.addRow(data);
        
        data[1] = "last_subscription_handle";
        this.addRow(data);
    }

    @Override
    public boolean update() {
        WriterStatus status;
        
        try {
            status = (WriterStatus)(entity.getStatus());
        } catch (CMException e) {
            return false;
        }
        super.updateState(status);
        
        if(status.getLivelinessLost() != null){
            this.setValueAt(Long.toString(status.getLivelinessLost().getTotalCount()), 1, 2);
            this.setValueAt(Long.toString(status.getLivelinessLost().getTotalCountChange()), 2, 2);
        }
        if(status.getDeadlineMissed() != null){
            this.setValueAt(Long.toString(status.getDeadlineMissed().getTotalCount()), 3, 2);
            this.setValueAt(Long.toString(status.getDeadlineMissed().getTotalCountChange()), 4, 2);
            this.setValueAt(status.getDeadlineMissed().getLastInstanceHandle(), 5, 2);
        }
        if(status.getIncompatibleQos() != null){
            this.setValueAt(Long.toString(status.getIncompatibleQos().getTotalCount()), 6, 2);
            this.setValueAt(Long.toString(status.getIncompatibleQos().getTotalCountChange()), 7, 2);
            this.setValueAt(Long.toString(status.getIncompatibleQos().getLastPolicyId()) +
                    " (" + status.getIncompatibleQos().getLastPolicyIdName() + ")", 8, 2);
            
            Long[] qpc = status.getIncompatibleQos().getPolicies();
            
            if(qpc != null){
                StringBuffer buf = new StringBuffer();
                buf.append("[");
                for(int i=0; i< qpc.length; i++){
                    if(i==0){
                        buf.append(qpc[i].longValue());
                    } else {
                        buf.append(", " + qpc[i].longValue());
                    }
                }
                buf.append("]");
                this.setValueAt(buf.toString(), 9, 2);
            } else {
                this.setValueAt("NULL", 9, 2);
            }
        }
        if(status.getPublicationMatch() != null){
            this.setValueAt(Long.toString(status.getPublicationMatch().getTotalCount()), 10, 2);
            this.setValueAt(Long.toString(status.getPublicationMatch().getTotalCountChange()), 11, 2);
            this.setValueAt(Long.toString(status.getPublicationMatch().getCurrentCount()), 12, 2);
            this.setValueAt(Long.toString(status.getPublicationMatch().getCurrentCountChange()), 13, 2);
            this.setValueAt(status.getPublicationMatch().getLastInstanceHandle(), 14, 2);
        }
        return true;
    }
}
