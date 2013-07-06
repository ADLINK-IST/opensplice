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
 * Determines the mechanism and parameters used by the application to determine 
 * whether an Entity is "active" (alive). The "liveliness" status of an Entity 
 * is used to maintain instance ownership in combination with the setting of the
 * OwnershipPolicy. The DataReader requests that liveliness of the writers is 
 * maintained by the requested means and loss of liveliness is detected with 
 * delay not to exceed the lease_duration. The Writer commits to signalling its
 * liveliness using the stated means at intervals not to exceed the
 * lease_duration.
 * 
 * @date Jan 10, 2005 
 */
public class LivelinessPolicy {
    /**
     * The liveliness kind.
     */
    public LivelinessKind kind;
    
    /**
     * The default value of the lease_duration is infinite.
     */
    public Time lease_duration;
    
    public static final LivelinessPolicy DEFAULT = new LivelinessPolicy(LivelinessKind.AUTOMATIC, Time.infinite);
    
    /**
     * Constructs a new LivelinessPolicy.
     *  
     *
     * @param _kind The liveliness kind.
     * @param _lease_duration The lease duration.
     */
    public LivelinessPolicy(LivelinessKind _kind, Time _lease_duration){
        kind = _kind;
        lease_duration = _lease_duration;
    }
    
    public LivelinessPolicy copy(){
        return new LivelinessPolicy(this.kind, this.lease_duration.copy());
    }
}
