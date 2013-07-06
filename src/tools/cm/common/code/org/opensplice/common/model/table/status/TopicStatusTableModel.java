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
import org.opensplice.cm.Topic;
import org.opensplice.cm.status.TopicStatus;
import org.opensplice.common.CommonException;

/**
 * Concrete descendant of the EntityStatusTableModel object. Its responsibility
 * is to retrieve and administrate the Status of a Topic entity.
 * 
 * @date Oct 19, 2004 
 */
public class TopicStatusTableModel extends EntityStatusTableModel {
    /**
     * Constructs a new table model that holds the Status of the supplied
     * Topic.
     *
     * @param _entity The Topic, which Status must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public TopicStatusTableModel(Topic _entity) throws CommonException {
        super(_entity);
    }

    protected void init() {
        Object[] data = new Object[3];
        data[2] = "N/A";
        
        data[0] = "INCONSISTENT_TOPIC";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
    }

    public boolean update() {
        TopicStatus status;
        
        try {
            status = (TopicStatus)(entity.getStatus());
        } catch (CMException e) {
            return false;
        }
        super.updateState(status);
        
        if(status.getInconsistentTopic() != null){
            this.setValueAt(Long.toString(status.getInconsistentTopic().getTotalCount()), 1, 2);
            this.setValueAt(Long.toString(status.getInconsistentTopic().getTotalCountChange()), 2, 2);
        }
        
        return true;
    }
}
