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

public class AvgValue extends AbstractValue {
    protected long count;
    protected float value;
    
    public AvgValue(String name, long count, float value) {
        super(name);
        this.count = count;
        this.value = value;
    }

    public long getCount() {
        return count;
    }

    public float getValue() {
        return value;
    }
}
