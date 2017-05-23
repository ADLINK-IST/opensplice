/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

    @Override
    protected void init() {
        Object[] data = new Object[3];
        data[2] = "N/A";
        
        data[0] = "INCONSISTENT_TOPIC";
        data[1] = "total_count";
        this.addRow(data);
        
        data[1] = "total_count_change";
        this.addRow(data);
    }

    @Override
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
