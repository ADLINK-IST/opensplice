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

package DDS;

public class SchedulingPriorityQosPolicyKind {
    private int __value;
    private static int __size = 2;
    private static DDS.SchedulingPriorityQosPolicyKind[] __array = new DDS.SchedulingPriorityQosPolicyKind[__size];

    public static final int _PRIORITY_RELATIVE = 0;
    public static final DDS.SchedulingPriorityQosPolicyKind PRIORITY_RELATIVE = new DDS.SchedulingPriorityQosPolicyKind(
            _PRIORITY_RELATIVE);
    public static final int _PRIORITY_ABSOLUTE = 1;
    public static final DDS.SchedulingPriorityQosPolicyKind PRIORITY_ABSOLUTE = new DDS.SchedulingPriorityQosPolicyKind(
            _PRIORITY_ABSOLUTE);

    public int value() {
        return __value;
    }

    public static DDS.SchedulingPriorityQosPolicyKind from_int(int value) {
        if (value >= 0 && value < __size) {
            return __array[value];
        }
        throw new org.omg.CORBA.BAD_PARAM();
    }

    protected SchedulingPriorityQosPolicyKind(int value) {
        __value = value;
        __array[__value] = this;
    }
} // class SchedulingPriorityQosPolicyKind
