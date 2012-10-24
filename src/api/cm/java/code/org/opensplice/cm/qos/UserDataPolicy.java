/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
    
    /**
     * The size of the value.
     */
    public int size;
    
    public static final UserDataPolicy DEFAULT = new UserDataPolicy(null);
    
    /**
     * Constructs a new UserDataPolicy.
     *
     * @param _value The userData.
     */
    public UserDataPolicy(byte[] _value){
        value = _value;
        
        if(value == null){
            size = 0;
        } else {
            size = value.length;
        }
    }
    
    /**
     * Sets the value to the supplied value.
     *
     * @param value The value to set.
     */
    public void setValue(byte[] value) {
        this.value = value;
        
        if(value == null){
            size = 0;
        } else {
            size = value.length;
        }
    }
    
    /**
     * Constructs the String representation of the policy.
     * 
     * @return The String representation of the policy.
     */
    public String toString(){
        String result = "";
        
        if(value != null){
            if(value.length > 0){
                result += "[";
                
                for(int i=0; i<value.length; i++){
                    if(i != 0){
                        result += ", " + value[i];
                    } else {
                        result += value[i];
                    }
                }
                result += "]";
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
