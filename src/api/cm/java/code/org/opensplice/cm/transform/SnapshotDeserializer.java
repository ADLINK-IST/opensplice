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
