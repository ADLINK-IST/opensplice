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
 * Controls the criteria used to determine the logical order among changes made 
 * by Publisher entities to the same instance of data (i.e., matching Topic and 
 * key).
 * 
 * @date Jan 10, 2005 
 */
public class OrderbyPolicy {
    /**
     * Controls the criteria used to determine the logical order among changes 
     * made by Publisher entities to the same instance of data (i.e., matching 
     * Topic and key).
     */
    public OrderbyKind kind;
    
    public static final OrderbyPolicy DEFAULT = new OrderbyPolicy(OrderbyKind.BY_RECEPTION_TIMESTAMP);
    /**
     * Constructs a new OrderbyPolicy.
     *  
     *
     * @param _kind The orderby kind.
     */
    public OrderbyPolicy(OrderbyKind _kind){
        kind = _kind;
    }
    
    public OrderbyPolicy copy(){
        return new OrderbyPolicy(this.kind);
    }
}
