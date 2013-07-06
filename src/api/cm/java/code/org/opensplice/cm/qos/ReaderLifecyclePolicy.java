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
 * Specifies the behavior of the DataReader with regards to the lifecycle of 
 * the datainstances it manages.
 * 
 * @date Jan 10, 2005 
 */
public class ReaderLifecyclePolicy {
    /**
     * Indicates the duration the DataReader must retain information regarding
     * instances that have the view_state NOT_ALIVE_NO_WRITERS. By default, 
     * unlimited.
     */
    public Time autopurge_nowriter_samples_delay;
    
    public Time autopurge_disposed_samples_delay;
    
    public boolean enable_invalid_samples;
    
    public static final ReaderLifecyclePolicy DEFAULT = new ReaderLifecyclePolicy(Time.infinite, Time.infinite, true);
    
    /**
     * Constructs a new ReaderLifecyclePolicy.
     *
     * @param _autopurge_nowriter_samples_delay The duration to retain 
     *                                          information.
     */
    public ReaderLifecyclePolicy(Time _autopurge_nowriter_samples_delay,
                                 Time _autopurge_disposed_samples_delay,
                                 boolean _enable_invalid_samples)
    {
        autopurge_nowriter_samples_delay = _autopurge_nowriter_samples_delay;
        autopurge_disposed_samples_delay = _autopurge_disposed_samples_delay;
        enable_invalid_samples           = _enable_invalid_samples;
    }
    
    public ReaderLifecyclePolicy copy(){
        return new ReaderLifecyclePolicy(
                this.autopurge_nowriter_samples_delay.copy(),
                this.autopurge_disposed_samples_delay.copy(),
                this.enable_invalid_samples);
    }
}
