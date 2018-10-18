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
package org.opensplice.cm.qos;

import org.opensplice.cm.Time;

/**
 * Indicates the level of reliability offered/requested by the service.
 * 
 * @date Jan 10, 2005 
 */
public class ReliabilityPolicy {
    /**
     * Indicates the level of reliability offered/requested by the Service.
     */
    public ReliabilityKind kind;
    
    /**
     * This setting applies only to the case where kind=RELIABLE and the 
     * HISTORY is KEEP_ALL. The value of the max_blocking_time indicates the 
     * maximum time the operation Writer.write is allowed to block if the 
     * Writer does not have space to store the value written.
     */
    public Time max_blocking_time;

    /**
     * This setting applies only to the case where kind=RELIABLE.
     * If the value of the synchronous is true for a DataWriter then it will
     * wait until all data is acknoledged by all DataReaders for which the 
     * synchronous value is also true.
     */
    public Boolean synchronous;

    public static final ReliabilityPolicy DEFAULT =
        new ReliabilityPolicy(ReliabilityKind.BESTEFFORT,
                              new Time(0, 100000000),
                              false);
    
    public static final ReliabilityPolicy WRITER_DEFAULT = new ReliabilityPolicy(
            ReliabilityKind.RELIABLE, new Time(0, 100000000), false);

    /**
     * Constructs a new ReliabilityPolicy.
     *
     * @param _kind The reliability kind.
     * @param _max_blocking_time The maximum time to block a write.
     */
    public ReliabilityPolicy(ReliabilityKind _kind,
                             Time _max_blocking_time,
                             Boolean _synchronous){
        kind = _kind;
        max_blocking_time = _max_blocking_time;
        synchronous = _synchronous;
    }
    
    public ReliabilityPolicy copy(){
        return new ReliabilityPolicy(this.kind,
                                     this.max_blocking_time.copy(),
                                     this.synchronous);
    }
}
