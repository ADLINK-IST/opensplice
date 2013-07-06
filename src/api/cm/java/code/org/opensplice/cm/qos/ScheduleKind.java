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
