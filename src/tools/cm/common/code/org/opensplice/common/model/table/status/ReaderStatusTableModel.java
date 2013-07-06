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
import org.opensplice.cm.Reader;
import org.opensplice.cm.status.ReaderStatus;
import org.opensplice.cm.status.SampleRejectedKind;
import org.opensplice.common.CommonException;

/**
 * Concrete descendant of the EntityStatusTableModel object. Its responsibility
 * is to retrieve and administrate the Status of a Reader entity.
 * 
 * @date Oct 19, 2004 
 */
public class ReaderStatusTableModel extends EntityStatusTableModel {
    /**
     * Constructs a new table model that holds the Status of the supplied
     * Reader.
     *
     * @param _entity The Reader, which Status must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public ReaderStatusTableModel(Reader _entity) throws CommonException {
        super(_entity);
    }

    @Override
    protected void init() {
        Object[] data = new Object[3];
        this.setValueAt("DATA_AVAILABLE", 0, 0);
        data[2] = "N/A";
        
        data[0] = "LIVELINESS_CHANGED";
        data[1] = "alive_count";
        this.addRow(data);
        
        data[1] = "alive_count_change";
        this.addRow(data);
        
        data[1] = "not_alive_count";
        this.addRow(data);
        
        data[1] = "not_alive_count_change";
        this.addRow(data);
        
        data[1] = "last_publication_handle";
        this.addRow(data);
        
        data[0] = "SAMPLE_REJECTED";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "last_reason";
        this.addRow(data);
        
        data[1] = "last_instance_handle";
        this.addRow(data);
        
        data[0] = "REQUESTED_DEADLINE_MISSED";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "last_instance_handle";
        this.addRow(data);
        
        data[0] = "REQUESTED_INCOMPATIBLE_QOS";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "last_policy_id";
        this.addRow(data);
        
        data[1] = "policies";
        this.addRow(data);
        
        data[0] = "SAMPLE_LOST";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[0] = "SUBSCRIPTION_MATCHED";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
        
        data[1] = "current_count";
        this.addRow(data);
        
        data[1] = "current_count_change";
        this.addRow(data);
        
        data[1] = "last_publication_handle";
        this.addRow(data);
    }

    @Override
    public boolean update() {
        ReaderStatus status;
        int row = 1;
        
        try {
            status = (ReaderStatus)(entity.getStatus());
        } catch (CMException e) {
            return false;
        }
        super.updateState(status);
        
        if(status.getLivelinessChanged() != null){
            this.setValueAt(Long.toString(status.getLivelinessChanged().getAliveCount()), row++, 2);
            this.setValueAt(Long.toString(status.getLivelinessChanged().getAliveCountChange()), row++, 2);
            this.setValueAt(Long.toString(status.getLivelinessChanged().getNotAliveCount()), row++, 2);
            this.setValueAt(Long.toString(status.getLivelinessChanged().getNotAliveCountChange()), row++, 2);
            this.setValueAt(status.getLivelinessChanged().getLastPublicationHandle(), row++, 2);
        }
        if(status.getSampleRejected() != null){
            this.setValueAt(Long.toString(status.getSampleRejected().getTotalCount()), row++, 2);
            this.setValueAt(Long.toString(status.getSampleRejected().getTotalCountChange()), row++, 2);
            this.setValueAt(SampleRejectedKind.get_string(status.getSampleRejected().getLastReason()), row++, 2);
            this.setValueAt(status.getSampleRejected().getLastInstanceHandle(), row++, 2);
        }
        if(status.getDeadlineMissed() != null){
            this.setValueAt(Long.toString(status.getDeadlineMissed().getTotalCount()), row++, 2);
            this.setValueAt(Long.toString(status.getDeadlineMissed().getTotalCountChange()), row++, 2);
            this.setValueAt(status.getDeadlineMissed().getLastInstanceHandle(), row++, 2);
        }
        if(status.getIncompatibleQos() != null){
            this.setValueAt(Long.toString(status.getIncompatibleQos().getTotalCount()), row++, 2);
            this.setValueAt(Long.toString(status.getIncompatibleQos().getTotalCountChange()), row++, 2);
            this.setValueAt(Long.toString(status.getIncompatibleQos().getLastPolicyId()) +
                    " (" + status.getIncompatibleQos().getLastPolicyIdName() + ")", row++, 2);
        
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
                this.setValueAt(buf.toString(), row++, 2);
            } else {
                this.setValueAt("NULL", row++, 2);
            }
        }
        if(status.getSampleLost() != null){
            this.setValueAt(Long.toString(status.getSampleLost().getTotalCount()), row++, 2);
            this.setValueAt(Long.toString(status.getSampleLost().getTotalCountChange()), row++, 2);
            
        }
        if(status.getSubscriptionMatch() != null){
            this.setValueAt(Long.toString(status.getSubscriptionMatch().getTotalCount()), row++, 2);
            this.setValueAt(Long.toString(status.getSubscriptionMatch().getTotalCountChange()), row++, 2);
            this.setValueAt(Long.toString(status.getSubscriptionMatch().getCurrentCount()), row++, 2);
            this.setValueAt(Long.toString(status.getSubscriptionMatch().getCurrentCountChange()), row++, 2);
            this.setValueAt(status.getSubscriptionMatch().getLastInstanceHandle(), row++, 2);
        }
        
        return true;
    }
}
