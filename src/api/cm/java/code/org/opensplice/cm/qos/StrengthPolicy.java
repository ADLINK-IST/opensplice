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
