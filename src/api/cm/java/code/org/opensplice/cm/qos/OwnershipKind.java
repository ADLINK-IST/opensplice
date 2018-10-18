/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
 * Specifies whether it is allowed for multiple Writers to write the same 
 * instance of the data and if so, how these modifications should be arbitrated.
 * 
 * @date Jan 10, 2005 
 */
public class OwnershipKind {
    public static final int _SHARED         = 0;
    public static final int _EXCLUSIVE      = 1;
    
    /**
     * Indicates shared ownership for each instance. Multiple writers are 
     * allowed to update the same instance and all the updates are made 
     * available to the readers. In other words there is no concept of an 
     * "owner" for the instances. This is the default behavior.
     */
    public static final OwnershipKind SHARED      = new OwnershipKind(_SHARED);
    
    /**
     * Indicates each instance can only be owned by one Writer, but the owner of 
     * an instance can change dynamically. The selection of the owner is 
     * controlled by the setting of the StrengthPolicy. The owner is always set 
     * to be the higheststrength Writer object among the ones currently "active" 
     * (as determined by the LivelinessPolicy).
     */
    public static final OwnershipKind EXCLUSIVE   = new OwnershipKind(_EXCLUSIVE);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(SHARED)){
            result = _SHARED;
        } else if(this.equals(EXCLUSIVE)){
            result = _EXCLUSIVE;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static OwnershipKind from_int(int value){
        OwnershipKind result = null;
        
        if(value == _SHARED){
            result = SHARED;
        } else if(value == _EXCLUSIVE){
            result = EXCLUSIVE;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static OwnershipKind from_string(String value){
        OwnershipKind result = null;
        
        if("V_OWNERSHIP_SHARED".equals(value)){
            result = SHARED;
        } else if("V_OWNERSHIP_EXCLUSIVE".equals(value)){
            result = EXCLUSIVE;
        } else if("SHARED".equals(value)){
            result = SHARED;
        } else if("EXCLUSIVE".equals(value)){
            result = EXCLUSIVE;
        }
        return result;
    }
    
    /**
     * Resolves the string representation of the kind.
     * 
     * @return The string representation of the kind.
     */
    @Override
    public String toString(){
        String result = "UNKNOWN";
        
        if(this.equals(SHARED)){
            result = "SHARED";
        } else if(this.equals(EXCLUSIVE)){
            result = "EXCLUSIVE";
        } 
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(SHARED)){
            result = "V_OWNERSHIP_SHARED";
        } else if(this.equals(EXCLUSIVE)){
            result = "V_OWNERSHIP_EXCLUSIVE";
        } 
        return result;
    }
    
    protected OwnershipKind(int value){}
}
