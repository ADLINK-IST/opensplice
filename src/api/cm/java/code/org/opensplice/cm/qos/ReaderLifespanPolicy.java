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

public class ReaderLifespanPolicy {
    public boolean used;
    public Time duration;
    
    public static final ReaderLifespanPolicy DEFAULT = new ReaderLifespanPolicy(false, Time.infinite);
    
    /**
     * Constructs a new ReaderLifecyclePolicy.
     *
     * @param _autopurge_nowriter_samples_delay The duration to retain 
     *                                          information.
     */
    public ReaderLifespanPolicy(boolean _used, Time _duration){
        used = _used;
        duration = _duration;
    }
    
    public ReaderLifespanPolicy copy(){
        return new ReaderLifespanPolicy(this.used, this.duration.copy());
    }
}
