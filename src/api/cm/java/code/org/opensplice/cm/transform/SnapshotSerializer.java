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

import org.opensplice.cm.Snapshot;

/**
 * Interface that must be implemented by each class that offers facilities
 * for serializing a Snapshot and that wants be supported by the 
 * DataTransformerFactory. 
 */
public interface SnapshotSerializer {
    /**
     * Serializes a Snapshot into its serialized representation. 
     * 
     * @param snapshot The Snapshot to serialize.
     * @return The serialized representation of the supplied Snapshot.
     */
    public String serializeSnapshot(Snapshot snapshot) throws TransformationException;
}
