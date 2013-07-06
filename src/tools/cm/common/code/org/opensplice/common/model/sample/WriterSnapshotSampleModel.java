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
package org.opensplice.common.model.sample;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;
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
    public WriterSnapshotSampleModel(WriterSnapshot _snapshot) throws CommonException {
        super(_snapshot);
        sampleInfoModel = new WriterHistoryInfoTableModel();
    }
    
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
        return result;
    }
    
    public String getDataTypeKeyList() throws CommonException {
        String result = null;
        Topic t = this.getTopic();
        
        if(t != null){
            result = super.getKeyList(t);
        } else {
            throw new CommonException("Data type could not be retrieved.");
        }
        return result;
    }

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
