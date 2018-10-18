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
 * Not used for now.
 * 
 * @date Jan 10, 2005 
 */
public class SimulationPolicy {
    public Time oldTime;
    public Time newTime;
    public double relativeSpeed;
    
    public static final SimulationPolicy DEFAULT = new SimulationPolicy(Time.infinite, Time.infinite, 0);
    
    public SimulationPolicy(Time _oldTime, Time _newTime, double _relativeSpeed){
        oldTime = _oldTime;
        newTime = _newTime;
        relativeSpeed = _relativeSpeed;
    }
    
    public SimulationPolicy copy(){
        return new SimulationPolicy(
                this.oldTime.copy(), 
                this.newTime.copy(), 
                this.relativeSpeed);
    }
}
