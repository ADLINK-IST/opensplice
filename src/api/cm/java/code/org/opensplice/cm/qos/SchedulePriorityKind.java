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
