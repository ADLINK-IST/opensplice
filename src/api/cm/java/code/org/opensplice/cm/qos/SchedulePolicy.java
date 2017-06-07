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
