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

public class ScheduleKind {
    public static final int _DEFAULT                = 0;
    public static final int _TIMESHARING            = 1;
    public static final int _REALTIME               = 2;
    
    public static final ScheduleKind DEFAULT        = new ScheduleKind(_DEFAULT);
    public static final ScheduleKind TIMESHARING    = new ScheduleKind(_TIMESHARING);
    public static final ScheduleKind REALTIME       = new ScheduleKind(_REALTIME);
    
    protected ScheduleKind(int value){}
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(DEFAULT)){
            result = _DEFAULT;
        } else if(this.equals(TIMESHARING)){
            result = _TIMESHARING;
        } else {
            result = _REALTIME;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static ScheduleKind from_int(int value){
        ScheduleKind result = null;
        
        if(value == _DEFAULT){
            result = DEFAULT;
        } else if(value == _TIMESHARING){
            result = TIMESHARING;
        } else if(value == _REALTIME){
            result = REALTIME;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static ScheduleKind from_string(String value){
        ScheduleKind result = null;
        
        if("V_SCHED_DEFAULT".equals(value)){
            result = DEFAULT;
        } else if("V_SCHED_TIMESHARING".equals(value)){
            result = TIMESHARING;
        } else if("V_SCHED_REALTIME".equals(value)){
            result = REALTIME;
        } else if("DEFAULT".equals(value)){
            result = DEFAULT;
        } else if("TIMESHARING".equals(value)){
            result = TIMESHARING;
        } else if("REALTIME".equals(value)){
            result = REALTIME;
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
        
        if(this.equals(DEFAULT)){
            result = "DEFAULT";
        } else if(this.equals(TIMESHARING)){
            result = "TIMESHARING";
        } else {
            result = "REALTIME";
        }
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(DEFAULT)){
            result = "V_SCHED_DEFAULT";
        } else if(this.equals(TIMESHARING)){
            result = "V_SCHED_TIMESHARING";
        } else {
            result = "V_SCHED_REALTIME";
        }
        return result;
    }
}
