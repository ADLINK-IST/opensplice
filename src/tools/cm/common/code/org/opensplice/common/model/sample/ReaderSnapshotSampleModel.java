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


import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Topic;
import org.opensplice.common.CommonException;


/**
 * Concrete descendant of the SampleModel component. This components
 * administrates the Sample objects read/taken from a ReaderSnapshot entity.
 * 
 * @date Oct 21, 2004 
 */
public class ReaderSnapshotSampleModel extends SnapshotSampleModel {
    
    /**
     * Constructs the model for the supplied ReaderSnapshot.
     *
     * @param _snapshot The snapshot where data is read/taken from.
     * @throws CommonException Thrown when the data type could not be retrieved.
     */
    public ReaderSnapshotSampleModel(ReaderSnapshot _snapshot) throws CommonException{
        super(_snapshot);
    }
    
    protected String getPartitions() throws CommonException{
        return getPartitions(((ReaderSnapshot)snapshot).getReader());
    }    
    
    protected Topic getTopic() throws CommonException{
        Reader r = ((ReaderSnapshot)snapshot).getReader();
        return super.getTopic(r);
    }

    public String getDataTypeKeyList() throws CommonException {
        String result = null;
        Topic t = this.getTopic();

        if(t != null){
            result = t.getKeyList();
        }
        return result;
    }

}
