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
 * This policy is a hint to the infrastructure as to how to set the priority of 
 * the underlying transport used to send the data.
 * 
 * @date Jan 10, 2005 
 */
public class TransportPolicy {
    /**
     * This policy is a hint to the infrastructure as to how to set the 
     * priority of the underlying transport used to send the data. The default 
     * value of the value is zero.
     */
    public int value;
    
    public static final TransportPolicy DEFAULT = new TransportPolicy(0);
    
    /**
     * Constructs a new TransportPolicy.
     *  
     *
     * @param _value The priority.
     */
    public TransportPolicy(int _value){
        value = _value;
    }
    
    public TransportPolicy copy(){
        return new TransportPolicy(this.value);
    }
}
