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

/**
 * Specifies the behavior of the Writer with regards to the lifecycle of the 
 * datainstances it manages.
 * 
 * @date Jan 10, 2005 
 */
public class WriterLifecyclePolicy {
    /**
     * Controls whether a Writer will automatically dispose instances each time
     * they are unregistered. The setting autodispose_unregistered_instances = 
     * TRUE indicates that unregistered instances will also be considered 
     * disposed. By default, TRUE.
     */
    public boolean autodispose_unregistered_instances;
    
    public Time autopurge_suspended_samples_delay;
    
    public Time autounregister_instance_delay;
    
    public static final WriterLifecyclePolicy DEFAULT = new WriterLifecyclePolicy(true, Time.infinite, Time.infinite);
    
    /**
     * Constructs a new WriterLifecyclePolicy.
     *
     * @param _autodispose_unregistered_instances Whether to dispose instances.
     */
    public WriterLifecyclePolicy(boolean _autodispose_unregistered_instances, Time _autopurge_suspended_samples_delay, Time _autounregister_instance_delay){
        autodispose_unregistered_instances = _autodispose_unregistered_instances;
        autopurge_suspended_samples_delay = _autopurge_suspended_samples_delay; 
        autounregister_instance_delay = _autounregister_instance_delay;
    }
    
    public WriterLifecyclePolicy copy(){
        return new WriterLifecyclePolicy(
                this.autodispose_unregistered_instances, 
                this.autopurge_suspended_samples_delay.copy(), 
                this.autounregister_instance_delay.copy());
    }
}
