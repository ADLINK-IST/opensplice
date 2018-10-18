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

public class ReaderLifespanPolicy {
    public boolean used;
    public Time duration;
    
    public static final ReaderLifespanPolicy DEFAULT = new ReaderLifespanPolicy(false, Time.infinite);
    
    /**
     * Constructs a new ReaderLifecyclePolicy.
     *
     * @param _autopurge_nowriter_samples_delay The duration to retain 
     *                                          information.
     */
    public ReaderLifespanPolicy(boolean _used, Time _duration){
        used = _used;
        duration = _duration;
    }
    
    public ReaderLifespanPolicy copy(){
        return new ReaderLifespanPolicy(this.used, this.duration.copy());
    }
}
