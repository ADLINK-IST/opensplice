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
