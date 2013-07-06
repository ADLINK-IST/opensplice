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
 * Specifies the value of the "strength" used to arbitrate among multiple Writer
 * objects that attempt to modify the same instance of a dataobject (identified 
 * by Topic + key). This policy only applies if the OwnershipPolicy is of kind
 * EXCLUSIVE. 
 * 
 * @date Jan 10, 2005 
 */
public class StrengthPolicy {
    /**
     * Specifies the value of the "strength" used to arbitrate among multiple 
     * Writer objects that attempt to modify the same instance of a dataobject 
     * (identified by Topic + key). The default value of the is zero.
     */
    public int value;
    
    public static final StrengthPolicy DEFAULT = new StrengthPolicy(0);
    
    /**
     * Constructs a new StrengthPolicy.
     *  
     *
     * @param _value The strength.
     */
    public StrengthPolicy(int _value){
        value = _value;
    }
    
    public StrengthPolicy copy(){
        return new StrengthPolicy(this.value);
    }
}
