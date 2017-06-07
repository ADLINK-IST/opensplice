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

public final class SubscriptionMatchedStatus {
    public int total_count = 0;
    public int total_count_change = 0;
    public int current_count = 0;
    public int current_count_change = 0;
    public long last_publication_handle = 0L;

    public SubscriptionMatchedStatus() {
    } // ctor

    public SubscriptionMatchedStatus(int _total_count, int _total_count_change,
            int _current_count, int _current_count_change,
            long _last_publication_handle) {
        total_count = _total_count;
        total_count_change = _total_count_change;
        current_count = _current_count;
        current_count_change = _current_count_change;
        last_publication_handle = _last_publication_handle;
    } // ctor

} // class SubscriptionMatchStatus
