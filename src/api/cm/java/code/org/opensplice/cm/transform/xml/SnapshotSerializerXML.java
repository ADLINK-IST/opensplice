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
package org.opensplice.cm.transform.xml;

import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Snapshot;
import org.opensplice.cm.transform.SnapshotSerializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * The XML implementation of an SnapshotSerializer. It is capable of 
 * transforming a Snapshot object into a serialized XML representation.
 * 
 * @date Oct 20, 2004 
 */
public class SnapshotSerializerXML implements SnapshotSerializer{
    @Override
    public String serializeSnapshot(Snapshot snapshot) throws TransformationException {
        String result = null;
        
        if(snapshot == null){
            throw new TransformationException("Supplied Snapshot is not valid.");
        }
        if(snapshot instanceof ReaderSnapshot){
            result = "<readerSnapshot><id>" + snapshot.getId() + "</id></readerSnapshot>";
        } else {
            result = "<writerSnapshot><id>" + snapshot.getId() + "</id></writerSnapshot>";
        }
        return result;
    }
}
