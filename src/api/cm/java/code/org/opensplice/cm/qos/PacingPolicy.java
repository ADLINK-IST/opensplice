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
 * Filter that allows a DataReader to specify that it is interested only in 
 * (potentially) a subset of the values of the data. The filter states that the 
 * DataReader does not want to receive more than one value each 
 * minSeparation, regardless of how fast the changes occur. By default 
 * minSeparation=0 indicating DataReader is potentially interested in all 
 * values.
 * 
 * @date Jan 10, 2005 
 */
public class PacingPolicy {
    public Time minSeperation;
    
    public static final PacingPolicy DEFAULT = new PacingPolicy(Time.zero);
    
    public PacingPolicy(Time _minSeperation){
        minSeperation = _minSeperation;
    }
    
    public PacingPolicy copy(){
        return new PacingPolicy(this.minSeperation.copy());
    }
}
