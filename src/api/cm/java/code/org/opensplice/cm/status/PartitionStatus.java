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
