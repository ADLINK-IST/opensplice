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
package org.opensplice.common.model.sample;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.WriterHistoryInfoTableModel;

/**
 * Concrete descendant of the SampleModel component. This components
 * administrates the Sample objects read/taken from a WriterSnapshot entity.
 * 
 * @date Nov 17, 2004 
 */
public class WriterSnapshotSampleModel extends SnapshotSampleModel{

    /**
     *  Constructs the model for the supplied WriterSnapshot.
     *
     * @param _snapshot The snapshot where data is read/taken from.
     * @throws CommonException Thrown when the data type could not be retrieved.
     */
    public WriterSnapshotSampleModel(WriterSnapshot _snapshot, TypeEvolution _typeEvolution) throws CommonException {
        super(_snapshot,_typeEvolution);
        sampleInfoModel = new WriterHistoryInfoTableModel();
    }
    
    @Override
    public Topic getTopic() throws CommonException{
        Topic result = null;
        Writer w = ((WriterSnapshot)snapshot).getWriter();
        
        try {
            Entity[] tops = w.getOwnedEntities(EntityFilter.TOPIC);
            
            if(tops.length > 0){
                result = (Topic)tops[0];
            }
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
        if (result == null) {
            throw new CommonException("Unable to resolve Topic entity from selected Writer.");
        }
        return result;
    }

    @Override
    protected String getPartitions() throws CommonException {
        String partitions = null;
        Entity[] entities;
        Entity entity;
        
        try {
            Writer w = ((WriterSnapshot)snapshot).getWriter();
            entities = w.getDependantEntities(EntityFilter.PUBLISHER);
            
            if(entities.length > 0){
                entity = entities[0];
                
                for(int i=1; i<entities.length; i++){
                    entities[i].free();
                }
                entities = entity.getOwnedEntities(EntityFilter.PARTITION);
                
                for(int i=0; i<entities.length; i++){
                    if(partitions == null){
                        partitions = entities[i].getName();
                    } else {
                        partitions += "," + entities[i].getName();
                    }
                    entities[i].free();
                }
                entity.free();
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
        return partitions;
    }
}
