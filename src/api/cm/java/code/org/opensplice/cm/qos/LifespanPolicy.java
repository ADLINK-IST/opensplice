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
