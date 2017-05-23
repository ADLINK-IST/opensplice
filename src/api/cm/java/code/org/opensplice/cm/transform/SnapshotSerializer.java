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
