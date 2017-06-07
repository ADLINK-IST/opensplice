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
package org.opensplice.cm.data;

/**
 * Represents a GID in the Splice kernel.
 * 
 * @date Nov 15, 2004 
 */
public class GID {
    private long localId;
    private long systemId;
    
    /**
     * Creates a new GID from the supplied arguments.
     *  
     *
     * @param _localId The local id.
     * @param _systemId The extended id.
     */
    public GID(long _localId, long _systemId){
        localId = _localId;
        systemId = _systemId;
    }
    
    /**
     * Provides access to localId.
     * 
     * @return Returns the localId.
     */
    public long getLocalId() {
        return localId;
    }
    /**
     * Provides access to systemId.
     * 
     * @return Returns the systemId.
     */
    public long getSystemId() {
        return systemId;
    }
    /**
     * Sets the localId to the supplied value.
     *
     * @param localId The localId to set.
     */
    public void setLocalId(long localId) {
        this.localId = localId;
    }
    /**
     * Sets the systemId to the supplied value.
     *
     * @param systemId The systemId to set.
     */
    public void setSystemId(long systemId) {
        this.systemId = systemId;
    }
}
