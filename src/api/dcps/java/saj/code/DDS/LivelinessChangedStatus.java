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

public final class LivelinessChangedStatus {
    public int alive_count = 0;
    public int not_alive_count = 0;
    public int alive_count_change = 0;
    public int not_alive_count_change = 0;
    public long last_publication_handle = 0L;

    public LivelinessChangedStatus() {
    } // ctor

    public LivelinessChangedStatus(int _alive_count, int _not_alive_count,
            int _alive_count_change, int _not_alive_count_change,
            long _last_publication_handle) {
        alive_count = _alive_count;
        not_alive_count = _not_alive_count;
        alive_count_change = _alive_count_change;
        not_alive_count_change = _not_alive_count_change;
        last_publication_handle = _last_publication_handle;
    } // ctor

} // class LivelinessChangedStatus
