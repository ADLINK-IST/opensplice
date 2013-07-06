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
                this.max_instances,
                this.max_samples,
                this.max_samples_per_instance);
    }
}
