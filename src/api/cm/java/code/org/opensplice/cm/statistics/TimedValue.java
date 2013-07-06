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

import org.opensplice.cm.Time;

public class TimedValue extends Value {
    protected Time lastUpdate;

    public TimedValue(String name, long value, Time lastUpdate) {
        super(name, value);
        this.lastUpdate = lastUpdate;
    }

    public Time getLastUpdate() {
        return lastUpdate;
    }

    public void setLastUpdate(Time lastUpdate) {
        this.lastUpdate = lastUpdate;
    }
}
