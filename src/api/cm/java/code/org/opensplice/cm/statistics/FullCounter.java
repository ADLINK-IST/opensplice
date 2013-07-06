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

package org.opensplice.cm.statistics;

/**
 * Represents a MaxCounter that also keeps track of its all time minimum and
 * the average.
 * 
 * @date May 12, 2005 
 */
public class FullCounter extends Value {
    /**
     * The minimum value.
     */
    protected TimedValue min;
    
    /**
     * The maximum value. 
     */
    protected TimedValue max;
    
    /**
     * The average value.
     */
    protected AvgValue avg;
    
    
    public FullCounter(String name, long value, TimedValue min, TimedValue max, AvgValue avg) {
        super(name, value);
        this.min = min;
        this.max = max;
        this.avg = avg;        
    }


    public AvgValue getAvg() {
        return avg;
    }


    public void setAvg(AvgValue avg) {
        this.avg = avg;
    }


    public TimedValue getMax() {
        return max;
    }


    public void setMax(TimedValue max) {
        this.max = max;
    }


    public TimedValue getMin() {
        return min;
    }


    public void setMin(TimedValue min) {
        this.min = min;
    }
}
