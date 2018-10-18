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

public class SampleRejectedStatusKind {
    private int __value;
    private static int __size = 4;
    private static DDS.SampleRejectedStatusKind[] __array = new DDS.SampleRejectedStatusKind[__size];

    public static final int _NOT_REJECTED = 0;
    public static final DDS.SampleRejectedStatusKind NOT_REJECTED = new DDS.SampleRejectedStatusKind(
            _NOT_REJECTED);
    public static final int _REJECTED_BY_INSTANCES_LIMIT = 1;
    public static final DDS.SampleRejectedStatusKind REJECTED_BY_INSTANCES_LIMIT = new DDS.SampleRejectedStatusKind(
            _REJECTED_BY_INSTANCES_LIMIT);
    public static final int _REJECTED_BY_SAMPLES_LIMIT = 2;
    public static final DDS.SampleRejectedStatusKind REJECTED_BY_SAMPLES_LIMIT = new DDS.SampleRejectedStatusKind(
            _REJECTED_BY_SAMPLES_LIMIT);
    public static final int _REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = 3;
    public static final DDS.SampleRejectedStatusKind REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = new DDS.SampleRejectedStatusKind(
            _REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT);

    public int value() {
        return __value;
    }

    public static DDS.SampleRejectedStatusKind from_int(int value) {
        if (value >= 0 && value < __size) {
            return __array[value];
        }
        throw new org.omg.CORBA.BAD_PARAM();
    }

    protected SampleRejectedStatusKind(int value) {
        __value = value;
        __array[__value] = this;
    }
} // class SampleRejectedStatusKind
