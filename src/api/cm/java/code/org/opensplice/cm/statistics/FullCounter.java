/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
