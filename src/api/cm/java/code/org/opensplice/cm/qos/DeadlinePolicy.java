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
 * Provides all Quality of Services of the SPLICE-DDS system.
 */
package org.opensplice.cm.qos;

import org.opensplice.cm.Time;

/**
 * Meaning:
 * DataReader expects a new sample updating the value of each instance at 
 * least once every deadline period. DataWriter indicates that the application 
 * commits to write a new value (using the DataWriter) for each instance managed
 * by the DataWriter at least once every deadline period. The default value of 
 * the deadline period is infinite. 
 * 
 * @date Jan 10, 2005 
 */
public class DeadlinePolicy {
    public Time period;
    public static final DeadlinePolicy DEFAULT = new DeadlinePolicy(Time.infinite);
    
    /**
     * Constructs a new DeadlinePolicy
     * 
     * @param _period The deadline period.
     */
    public DeadlinePolicy(Time _period){
        period = _period;
    }
    
    public DeadlinePolicy copy(){
        return new DeadlinePolicy(this.period.copy());
    }
}
