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
 * User data not known by the middleware, but distributed by means of built-in 
 * topics
 * 
 * @date Jan 10, 2005 
 */
public class UserDataPolicy {
    /**
     * User data not known by the middleware, but distributed by means of 
     * built-in topics. The default value is an empty (zerosized) sequence.
     */
    public byte[] value;
    
    public static final UserDataPolicy DEFAULT = new UserDataPolicy(null);
    
    /**
     * Constructs a new UserDataPolicy.
     *
     * @param _value The userData.
     */
    public UserDataPolicy(byte[] _value){
        value = _value;
    }
    
    /**
     * Sets the value to the supplied value.
     *
     * @param value The value to set.
     */
    public void setValue(byte[] value) {
        this.value = value;
    }
    
    /**
     * Constructs the String representation of the policy.
     * 
     * @return The String representation of the policy.
     */
    @Override
    public String toString(){
        String result = "";
        
        if(value != null){
            if(value.length > 0){
                StringBuffer buf = new StringBuffer();
                buf.append("[");
                for (int i = 0; i < value.length; ++i) {
                    if(i != 0){
                        buf.append(", " + value[i]);
                    } else {
                        buf.append(value[i]);
                    }
                }
                buf.append("]");
                result = buf.toString();
            }
        } else {
            result = "null";
        }
        return result;
    }
    
    public UserDataPolicy copy(){
        return new UserDataPolicy(this.value);
    }
}
