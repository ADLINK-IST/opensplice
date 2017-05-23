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
