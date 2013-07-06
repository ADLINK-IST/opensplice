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
 * Specifies whether it is allowed for multiple Writers to write the same 
 * instance of the data and if so, how these modifications should be arbitrated.
 * 
 * @date Jan 10, 2005 
 */
public class OwnershipPolicy {
    /**
     * Determines the ownership.
     */
    public OwnershipKind kind;
    
    public static final OwnershipPolicy DEFAULT = new OwnershipPolicy(OwnershipKind.SHARED);
    
    /**
     * Constructs a new OwnershipPolicy.
     *
     * @param _kind The ownership kind.
     */
    public OwnershipPolicy(OwnershipKind _kind){
        kind = _kind;
    }
    
    public OwnershipPolicy copy(){
        return new OwnershipPolicy(this.kind);
    }
}
