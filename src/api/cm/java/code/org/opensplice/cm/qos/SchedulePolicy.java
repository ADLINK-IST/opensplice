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

public class SchedulePolicy {
    public ScheduleKind kind;
    public SchedulePriorityKind priorityKind;
    public int priority;
    
    public static final SchedulePolicy DEFAULT = new SchedulePolicy(
                                                    ScheduleKind.DEFAULT, 
                                                    SchedulePriorityKind.RELATIVE,
                                                    0);
    
    public SchedulePolicy(ScheduleKind _kind, SchedulePriorityKind _priorityKind, int _priority){
        kind = _kind;
        priorityKind = _priorityKind;
        priority = _priority;
    }
    
    public SchedulePolicy copy(){
        return new SchedulePolicy(this.kind, this.priorityKind, this.priority);
    }
}
