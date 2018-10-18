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
