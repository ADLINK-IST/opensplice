/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
package org.opensplice.cm.status;

/**
 * The liveliness of one or more Writer that were writing instances read 
 * through the DataReader has changed. Some DataWriter have become "active" 
 * or "inactive".
 * 
 * @date Oct 12, 2004 
 */
public class LivelinessChangedInfo{
    private long alive_count;
    private long not_alive_count;
    private long alive_count_change;
    private long not_alive_count_change;
    private String last_publication_handle;
   
    /**
     * Creates a new LivelinessChangedInfo according to the supplied arguments.
     *  
     *
     * @param _active_count The total count of currently active Writers that
     *                      write the Topic the DataReader reads.
     * @param _inactive_count The total count of currently inactive Writers that
     *                        write the Topic the DataReader reads.
     * @param _active_count_change The change in the alive_count since the 
     *                             last time the listener was called or the
     *                             status was read.
     * @param _inactive_count_change The change in the not_alive_count since the
     *                               last time the listener was called or the 
     *                               status was read.
     */
    public LivelinessChangedInfo(
            long alive_count,
            long not_alive_count,
            long alive_count_change,
            long not_alive_count_change,
            String last_publication_handle)
    {
        this.alive_count = alive_count;
        this.not_alive_count = not_alive_count;
        this.alive_count_change = alive_count_change;
        this.not_alive_count_change = not_alive_count_change;
        this.last_publication_handle = last_publication_handle;
    }
    
    /**
     * Provides access to alive_count.
     * 
     * @return Returns the alive_count.
     */
    public long getAliveCount() {
        return alive_count;
    }
    
    /**
     * Provides access to active_count_change.
     * 
     * @return Returns the active_count_change.
     */
    public long getAliveCountChange() {
        return alive_count_change;
    }
    
    /**
     * Provides access to not_alive_count.
     * 
     * @return Returns the not_alive_count.
     */
    public long getNotAliveCount() {
        return not_alive_count;
    }
    
    /**
     * Provides access to inactive_count_change.
     * 
     * @return Returns the inactive_count_change.
     */
    public long getNotAliveCountChange() {
        return not_alive_count_change;
    }
    
    /**
     * Provides access to instance_handle.
     * 
     * @return Returns the instance_handle.
     */
    public String getLastPublicationHandle() {
        return this.last_publication_handle;
    }
}
