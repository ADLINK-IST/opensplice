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
/**
 * Supplies all common model components for Splice tooling.
 */
package org.opensplice.common.model.table.status;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Partition;
import org.opensplice.cm.status.PartitionStatus;
import org.opensplice.common.CommonException;

/**
 * Concrete descendant of the EntityStatusTableModel object. Its responsibility
 * is to retrieve and administrate the Status of a Domain entity.
 * 
 * @date Oct 19, 2004 
 */
public class PartitionStatusTableModel extends EntityStatusTableModel {

    /**
     * Constructs a new table model that holds the Status of the supplied
     * Domain.
     *
     * @param _entity The Domain, which Status must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public PartitionStatusTableModel(Partition _entity) throws CommonException {
        super(_entity);
    }

    protected void init() {
        Object[] data = new Object[3];
        data[2] = "N/A";
        
        data[0] = "GROUPS_CHANGED";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
    }

    public boolean update() {
        PartitionStatus status;
        
        try {
            status = (PartitionStatus)(entity.getStatus());
        } catch (CMException e) {
            return false;
        }
        super.updateState(status);
        
        if(status.getGroupsChanged() != null){
            this.setValueAt(Long.toString(status.getGroupsChanged().getTotalCount()), 1, 2);
            this.setValueAt(Long.toString(status.getGroupsChanged().getTotalCountChange()), 2, 2);
        }
        return true;
    }
}
