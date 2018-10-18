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
package org.opensplice.cmdataadapter;

import java.util.concurrent.TimeUnit;

import org.opensplice.cm.Time;

public class ComparableTime extends Time implements Comparable<ComparableTime> {

    public ComparableTime(int sec, int nsec) {
        super(sec, nsec);
    }


    /**
     * Compares one ComparableTime with another.
     *
     * @return Zero if both objects' sec and nsec are equal, 1 if this objects'
     *         sec and nsec are greater, and -1 if the other objects' sec and
     *         nsec are greater.
     */
    @Override
    public int compareTo(ComparableTime o) {
        if (this.sec > o.sec) {
            return 1;
        } else if (sec < o.sec) {
            return -1;
        } else {
            if (nsec > o.nsec) {
                return 1;
            } else if (nsec < o.nsec) {
                return -1;
            } else {
                return 0;
            }
        }
    }

    public final long getMilliSecs() {
        return TimeUnit.SECONDS.toMillis(sec)
                + TimeUnit.NANOSECONDS.toMillis(nsec);
    }
}
