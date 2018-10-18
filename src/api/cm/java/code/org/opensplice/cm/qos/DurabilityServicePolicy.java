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

public class DurabilityServicePolicy {
    /**
     * Only needed if kind is TRANSIENT or PERSISTENT. Controls when the 
     * service is able to remove all information regarding a data-instances.
     * By default, zero
     */
    public Time service_cleanup_delay;
    public HistoryQosKind history_kind;
    public int history_depth;
    public int max_samples;
    public int max_instances;
    public int max_samples_per_instance;
    
    public static final DurabilityServicePolicy DEFAULT = 
                                            new DurabilityServicePolicy(
                                                    Time.zero, 
                                                    HistoryQosKind.KEEPLAST,
                                                    1, -1, -1, -1);
    
    public DurabilityServicePolicy(
                    Time service_cleanup_delay,
                    HistoryQosKind history_kind,
                    int history_depth,
                    int max_samples,
                    int max_instances,
                    int max_samples_per_instance)
    {
        this.service_cleanup_delay = service_cleanup_delay;
        this.history_kind = history_kind;
        this.history_depth = history_depth;
        this.max_samples = max_samples;
        this.max_instances = max_instances;
        this.max_samples_per_instance = max_samples_per_instance;
    }
    
    public DurabilityServicePolicy copy(){
        return new DurabilityServicePolicy(
                this.service_cleanup_delay.copy(),
                this.history_kind,
                this.history_depth,
                this.max_samples,
                this.max_instances,
                this.max_samples_per_instance);
    }
}
