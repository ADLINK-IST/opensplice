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
