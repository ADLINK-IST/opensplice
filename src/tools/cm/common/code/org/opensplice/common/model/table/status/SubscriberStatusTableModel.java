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
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.status.SubscriberStatus;
import org.opensplice.common.CommonException;

/**
 * Concrete descendant of the EntityStatusTableModel object. Its responsibility
 * is to retrieve and administrate the Status of a Subscriber entity.
 * 
 * @date Oct 19, 2004 
 */
public class SubscriberStatusTableModel extends EntityStatusTableModel {
    /**
     * Constructs a new table model that holds the Status of the supplied
     * Subscriber.  
     *
     * @param _entity The Subscriber, which Status must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public SubscriberStatusTableModel(Subscriber _entity) throws CommonException {
        super(_entity);
    }

    protected void init() {
        Object[] data = new Object[3];
        this.setValueAt("DATA_ON_READERS", 0, 0);
        data[2] = "N/A";
        
        data[0] = "SAMPLE_LOST";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
    }

    public boolean update() {
        SubscriberStatus status;
        
        try {
            status = (SubscriberStatus)(entity.getStatus());
        } catch (CMException e) {
            return false;
        }
        super.updateState(status);
        
        if(status.getSampleLost() != null){
            this.setValueAt(Long.toString(status.getSampleLost().getTotalCount()), 1, 2);
            this.setValueAt(Long.toString(status.getSampleLost().getTotalCountChange()), 2, 2);
            
        }
        
        return true;
    }
}
