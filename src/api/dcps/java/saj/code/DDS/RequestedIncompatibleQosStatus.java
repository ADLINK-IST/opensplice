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

public final class RequestedIncompatibleQosStatus {
    public int total_count = 0;
    public int total_count_change = 0;
    public int last_policy_id = 0;
    public DDS.QosPolicyCount policies[] = null;

    public RequestedIncompatibleQosStatus() {
    } // ctor

    public RequestedIncompatibleQosStatus(int _total_count,
            int _total_count_change, int _last_policy_id,
            DDS.QosPolicyCount[] _policies) {
        total_count = _total_count;
        total_count_change = _total_count_change;
        last_policy_id = _last_policy_id;
        policies = _policies;
    } // ctor

} // class RequestedIncompatibleQosStatus
