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
