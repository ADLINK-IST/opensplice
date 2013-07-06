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
