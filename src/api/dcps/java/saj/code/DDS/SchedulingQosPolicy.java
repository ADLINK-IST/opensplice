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

package DDS;

public final class SchedulingQosPolicy {
    public DDS.SchedulingClassQosPolicy scheduling_class = null;
    public DDS.SchedulingPriorityQosPolicy scheduling_priority_kind = null;
    public int scheduling_priority = 0;

    public SchedulingQosPolicy() {
    } // ctor

    public SchedulingQosPolicy(DDS.SchedulingClassQosPolicy _scheduling_class,
            DDS.SchedulingPriorityQosPolicy _scheduling_priority_kind,
            int _scheduling_priority) {
        scheduling_class = _scheduling_class;
        scheduling_priority_kind = _scheduling_priority_kind;
        scheduling_priority = _scheduling_priority;
    } // ctor

} // class SchedulingQosPolicy
