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
