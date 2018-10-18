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
/**
 * Contains all Status information of Entity objects. Statusses provide
 * information about the Status of an Entity. 
 */
package org.opensplice.cm.status;

/**
 * This class is applicable for two kinds of statusses and therefore has two
 * meanings:
 * 
 * 1. WriterStatus: The deadline that the DataWriter has committed through its 
 *    QosPolicy DEADLINE was not respected for a specific instance.
 *    (OFFERED_DEADLINE_MISSED)
 * 2. ReaderStatus: The deadline that the DataReader was expecting through its 
 *    QosPolicy DEADLINE was not respected for a specific instance. 
 *    (REQUESTED_DEADLINE_MISSED)
 * 
 * @date Oct 12, 2004 
 */
public class DeadlineMissedInfo{
    private long total_count;
    private long total_count_change;
    private String last_instance_handle;
    
    /**
     * Constructs a new DeadLineMissedInfo according to the supplied arguments.
     *
     * @param _total_count Total cumulative count of the missed deadlines 
     *                     detected.
     * @param _total_count_change The incremental number of deadlines detected 
     *                            since the last time the listener was called 
     *                            or the status was read.
     * @param _last_instance_handle Handle to the last instance for which a 
     *                              deadline was detected.
     */
    public DeadlineMissedInfo(
            long _total_count, 
            long _total_count_change, 
            String _last_instance_handle)
    {
        total_count = _total_count;
        total_count_change = _total_count_change;
        last_instance_handle = _last_instance_handle;
    }
    
    /**
     * Provides access to total_count.
     * 
     * @return Returns the total_count.
     */
    public long getTotalCount() {
        return total_count;
    }
    
    /**
     * Provides access to total_count_change.
     * 
     * @return Returns the total_count_change.
     */
    public long getTotalCountChange() {
        return total_count_change;
    }
    /**
     * Provides access to last_instance_handle.
     * 
     * @return Returns the last_instance_handle.
     */
    public String getLastInstanceHandle() {
        return last_instance_handle;
    }
}
