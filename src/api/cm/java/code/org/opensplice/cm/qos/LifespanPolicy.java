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
 * Specifies the maximum duration of validity of the data written by the Writer.
 * 
 * @date Jan 10, 2005 
 */
public class LifespanPolicy {
    /**
     * Specifies the maximum duration of validity of the data written by the 
     * Writer. The default value of the lifespan duration is infinite.
     */
    public Time duration;
    
    public static final LifespanPolicy DEFAULT = new LifespanPolicy(Time.infinite);
    
    /**
     * Constructs a new LifespanPolicy.
     *
     * @param _duration The maximum duration.
     */
    public LifespanPolicy(Time _duration){
        duration = _duration;
    }
    
    public LifespanPolicy copy(){
        return new LifespanPolicy(this.duration.copy());
    }
}
