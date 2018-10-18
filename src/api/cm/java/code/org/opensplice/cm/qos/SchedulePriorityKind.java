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

public class SchedulePriorityKind {
    public static final int _RELATIVE         = 0;
    public static final int _ABSOLUTE       = 1;
    
    public static final SchedulePriorityKind RELATIVE   = new SchedulePriorityKind(_RELATIVE);
    public static final SchedulePriorityKind ABSOLUTE   = new SchedulePriorityKind(_ABSOLUTE);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(RELATIVE)){
            result = _RELATIVE;
        } else if(this.equals(ABSOLUTE)){
            result = _ABSOLUTE;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static SchedulePriorityKind from_int(int value){
        SchedulePriorityKind result = null;
        
        if(value == _RELATIVE){
            result = RELATIVE;
        } else if(value == _ABSOLUTE){
            result = ABSOLUTE;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static SchedulePriorityKind from_string(String value){
        SchedulePriorityKind result = null;
        
        if("V_SCHED_PRIO_RELATIVE".equals(value)){
            result = RELATIVE;
        } else if("V_SCHED_PRIO_ABSOLUTE".equals(value)){
            result = ABSOLUTE;
        } else if("RELATIVE".equals(value)){
            result = RELATIVE;
        } else if("ABSOLUTE".equals(value)){
            result = ABSOLUTE;
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
        
        if(this.equals(RELATIVE)){
            result = "RELATIVE";
        } else if(this.equals(ABSOLUTE)){
            result = "ABSOLUTE";
        } 
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(RELATIVE)){
            result = "V_SCHED_PRIO_RELATIVE";
        } else if(this.equals(ABSOLUTE)){
            result = "V_SCHED_PRIO_ABSOLUTE";
        } 
        return result;
    }
    
    protected SchedulePriorityKind(int value){}
}
