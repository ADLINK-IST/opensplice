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
 * Specifies the maximum acceptable delay from the time the data is written 
 * until the data is inserted in the receiver's application-cache and the 
 * receiving application is notified of the fact. This policy is a hint to the 
 * service, not something that must be monitored or enforced. The service is not 
 * required to track or alert the user of any violation.
 * 
 * @date Jan 10, 2005 
 */
public class LatencyPolicy {
    /**
     * Specifies the maximum acceptable delay from the time the data is written 
     * until the data is inserted in the receiver's application-cache and the 
     * receiving application is notified of the fact. This policy is a hint to 
     * the service, not something that must be monitored or enforced. The 
     * service is not required to track or alert the user of any violation. The 
     * default value of the duration is zero indicating that the delay should be
     * minimized.
     */
    public Time duration;
    
    public static final LatencyPolicy DEFAULT = new LatencyPolicy(Time.zero);
    
    /**
     * Constructs a new LatencyPolicy.
     *  
     *
     * @param _duration The duration.
     */
    public LatencyPolicy(Time _duration){
        duration = _duration;
    }
    
    public LatencyPolicy copy(){
        return new LatencyPolicy(this.duration.copy());
    }
}
