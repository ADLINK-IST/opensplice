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
package org.opensplice.cm.status;

/**
 * Concrete descendant of Status that represents the Status of a Partition. 
 * 
 * @date Oct 19, 2004 
 */
public class PartitionStatus extends Status {
    private GroupsChangedInfo groupsChanged;
    
    /**
     *  Constructs a new PartitionStatus from the supplied arguments.
     *
     * @param _state The Status state, which currently has no meaning in this 
     *               Status.
     * @param _groupsChanged Information about the groups of the Partition.
     */
    public PartitionStatus(String _state, GroupsChangedInfo _groupsChanged) {
        super(_state);
        groupsChanged = _groupsChanged;
    }
    
    /**
     * Provides access to groupsChanged.
     * 
     * @return Returns the groupsChanged.
     */
    public GroupsChangedInfo getGroupsChanged() {
        return groupsChanged;
    }
}
