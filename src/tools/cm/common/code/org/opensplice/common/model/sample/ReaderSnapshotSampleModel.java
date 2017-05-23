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
package org.opensplice.common.model.sample;


import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Topic;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
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
    public ReaderSnapshotSampleModel(ReaderSnapshot _snapshot, TypeEvolution _typeEvolution) throws CommonException{
        super(_snapshot, _typeEvolution);
    }
    
    @Override
    protected String getPartitions() throws CommonException{
        return getPartitions(((ReaderSnapshot)snapshot).getReader());
    }    
    
    @Override
    protected Topic getTopic() throws CommonException{
        Reader r = ((ReaderSnapshot)snapshot).getReader();
        return super.getTopic(r);
    }

}
