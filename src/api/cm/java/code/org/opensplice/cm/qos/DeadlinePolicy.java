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
