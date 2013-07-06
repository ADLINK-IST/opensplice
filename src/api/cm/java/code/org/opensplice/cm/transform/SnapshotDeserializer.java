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
package org.opensplice.cm.transform;

import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;

/**
 * Interface that must be implemented by each class that offers facilities
 * for deserializing a Snapshot and that wants be supported by the 
 * DataTransformerFactory.
 * 
 * @date Oct 20, 2004 
 */
public interface SnapshotDeserializer {
    /**
     * Deserializes a serialized object into a ReaderSnapshot. 
     * 
     * @param serialized The serialized ReaderSnapshot
     * @param reader The Reader where the Snapshot has been taken from.
     * @return The Java ReaderSnapshot representation of the serialized object.
     */
    public ReaderSnapshot deserializeReaderSnapshot(Object serialized, Reader reader) throws TransformationException;
    
    /**
     * Deserializes a serialized object into a WriterSnapshot. 
     * 
     * @param serialized The serialized WriterSnapshot
     * @param reader The Writer where the Snapshot has been taken from.
     * @return The Java WriterSnapshot representation of the serialized object.
     */
    public WriterSnapshot deserializeWriterSnapshot(Object serialized, Writer writer) throws TransformationException;
}
