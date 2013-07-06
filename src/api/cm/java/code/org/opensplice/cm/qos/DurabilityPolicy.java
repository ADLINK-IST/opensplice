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


/**
 * This policy expresses if the data should 'outlive' their writing time.
 * 
 * @date Jan 10, 2005 
 */
public class DurabilityPolicy {
    /**
     * The kind that determines the durability kind.
     */
    public DurabilityKind kind;
    
    public static final DurabilityPolicy DEFAULT = new DurabilityPolicy(DurabilityKind.VOLATILE);
    
    /**
     * Constructs a new DurabilityPolicy.
     *  
     *
     * @param _kind The durability kind.
     */
    public DurabilityPolicy(DurabilityKind _kind){
        kind = _kind;
    }
    
    public DurabilityPolicy copy(){
        return new DurabilityPolicy(this.kind);
    }
}
