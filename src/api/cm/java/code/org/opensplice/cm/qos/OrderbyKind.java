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
 * key). The default kind is BY_RECEPTION_TIMESTAMP.
 * 
 * @date Jan 10, 2005 
 */
public class OrderbyKind {
    public static final int _BY_RECEPTION_TIMESTAMP     = 0;
    public static final int _BY_SOURCE_TIMESTAMP        = 1;
    
    /**
     * Indicates that data is ordered based on the reception time at each 
     * Subscriber. Since each subscriber may receive the data at different times 
     * there is no guaranteed that the changes will be seen in the same order. 
     * Consequently, it is possible for each subscriber to end up with a 
     * different final value for the data.
     */
    public static final OrderbyKind BY_RECEPTION_TIMESTAMP    = new OrderbyKind(_BY_RECEPTION_TIMESTAMP);
    
    /**
     * Indicates that data is ordered based on a time-stamp placed at the source 
     * (by the service or by the application). In any case this guarantees a 
     * consistent final value for the data in all subscribers.
     */
    public static final OrderbyKind BY_SOURCE_TIMESTAMP       = new OrderbyKind(_BY_SOURCE_TIMESTAMP);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(BY_RECEPTION_TIMESTAMP)){
            result = _BY_RECEPTION_TIMESTAMP;
        } else if(this.equals(BY_SOURCE_TIMESTAMP)){
            result = _BY_SOURCE_TIMESTAMP;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static OrderbyKind from_int(int value){
        OrderbyKind result = null;
        
        if(value == _BY_RECEPTION_TIMESTAMP){
            result = BY_RECEPTION_TIMESTAMP;
        } else if(value == _BY_SOURCE_TIMESTAMP){
            result = BY_SOURCE_TIMESTAMP;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static OrderbyKind from_string(String value){
        OrderbyKind result = null;
        
        if("V_ORDERBY_RECEPTIONTIME".equals(value)){
            result = BY_RECEPTION_TIMESTAMP;
        } else if("V_ORDERBY_SOURCETIME".equals(value)){
            result = BY_SOURCE_TIMESTAMP;
        } else if("BY_RECEPTION_TIMESTAMP".equals(value)){
            result = BY_RECEPTION_TIMESTAMP;
        } else if("BY_SOURCE_TIMESTAMP".equals(value)){
            result = BY_SOURCE_TIMESTAMP;
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
        
        if(this.equals(BY_RECEPTION_TIMESTAMP)){
            result = "BY_RECEPTION_TIMESTAMP";
        } else if(this.equals(BY_SOURCE_TIMESTAMP)){
            result = "BY_SOURCE_TIMESTAMP";
        } 
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(BY_RECEPTION_TIMESTAMP)){
            result = "V_ORDERBY_RECEPTIONTIME";
        } else if(this.equals(BY_SOURCE_TIMESTAMP)){
            result = "V_ORDERBY_SOURCETIME";
        } 
        return result;
    }
    
    protected OrderbyKind(int value){}
}
