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

public class SchedulingClassQosPolicyKind {
    private int __value;
    private static int __size = 3;
    private static DDS.SchedulingClassQosPolicyKind[] __array = new DDS.SchedulingClassQosPolicyKind[__size];

    public static final int _SCHEDULE_DEFAULT = 0;
    public static final DDS.SchedulingClassQosPolicyKind SCHEDULE_DEFAULT = new DDS.SchedulingClassQosPolicyKind(
            _SCHEDULE_DEFAULT);
    public static final int _SCHEDULE_TIMESHARING = 1;
    public static final DDS.SchedulingClassQosPolicyKind SCHEDULE_TIMESHARING = new DDS.SchedulingClassQosPolicyKind(
            _SCHEDULE_TIMESHARING);
    public static final int _SCHEDULE_REALTIME = 2;
    public static final DDS.SchedulingClassQosPolicyKind SCHEDULE_REALTIME = new DDS.SchedulingClassQosPolicyKind(
            _SCHEDULE_REALTIME);

    public int value() {
        return __value;
    }

    public static DDS.SchedulingClassQosPolicyKind from_int(int value) {
        if (value >= 0 && value < __size) {
            return __array[value];
        }
        throw new org.omg.CORBA.BAD_PARAM();
    }

    protected SchedulingClassQosPolicyKind(int value) {
        __value = value;
        __array[__value] = this;
    }
} // class SchedulingClassQosPolicyKind
