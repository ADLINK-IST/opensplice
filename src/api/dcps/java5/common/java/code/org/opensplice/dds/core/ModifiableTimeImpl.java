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
package org.opensplice.dds.core;

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;
import org.omg.dds.core.ModifiableTime;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.Time;

public class ModifiableTimeImpl extends ModifiableTime {
    private static final long serialVersionUID = -5170318400663698911L;
    protected final transient OsplServiceEnvironment environment;
    private long seconds;
    private long nanoseconds;
    private long totalNanos;
    private final static int INFINITE_SECONDS = DurationImpl.INFINITE_SECONDS;
    private final static int INFINITE_NANOSECONDS = DurationImpl.INFINITE_NANOSECONDS;

    public ModifiableTimeImpl(OsplServiceEnvironment environment,
            long duration, TimeUnit unit) {
        super();

        if (unit == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Illegal TimeUnit provided (null).");
        }
        this.environment = environment;
        if (duration == Long.MAX_VALUE) {
            this.seconds = INFINITE_SECONDS;
            this.nanoseconds = INFINITE_NANOSECONDS;
            this.totalNanos = Long.MAX_VALUE;
        } else {
            this.seconds = TimeUnit.SECONDS.convert(duration, unit);
            this.totalNanos = TimeUnit.NANOSECONDS.convert(duration, unit);
            this.nanoseconds = this.totalNanos
                    - (this.seconds * 1000 * 1000 * 1000);
        }
    }

    public ModifiableTimeImpl(OsplServiceEnvironment environment, long seconds,
            long nanoseconds) {
        super();
        this.environment = environment;
        this.seconds = seconds;
        this.nanoseconds = nanoseconds;

        if (this.seconds == INFINITE_SECONDS
                && this.nanoseconds == INFINITE_NANOSECONDS) {
            this.totalNanos = Long.MAX_VALUE;
        } else {
            this.totalNanos = this.seconds * 1000 * 1000 * 1000
                    + this.nanoseconds;
        }
    }

    public ModifiableTimeImpl normalize() {
        long sec = this.seconds;
        long nsec = this.nanoseconds;

        if (sec == INFINITE_SECONDS) {
            return new ModifiableTimeImpl(this.environment, INFINITE_SECONDS,
                    INFINITE_NANOSECONDS);
        }
        while (nsec >= 1000000000) {
            sec += 1;
            nsec -= 1000000000;

            if (sec == INFINITE_SECONDS) {
                return new ModifiableTimeImpl(this.environment,
                        INFINITE_SECONDS, INFINITE_NANOSECONDS);
            }
        }
        return new ModifiableTimeImpl(this.environment, sec, nsec);
    }

    protected void normalizeMe() {
        ModifiableTimeImpl normalized = this.normalize();
        this.seconds = normalized.seconds;
        this.nanoseconds = normalized.nanoseconds;
        this.totalNanos = normalized.totalNanos;
    }

    @Override
    public int compareTo(Time o) {
        if (o == null) {
            return 1;
        }
        long sec = o.getTime(TimeUnit.SECONDS);
        long nsec = o.getRemainder(TimeUnit.SECONDS, TimeUnit.NANOSECONDS);

        if ((this.nanoseconds >= 1000000000)
                && ((this.nanoseconds != INFINITE_NANOSECONDS) || (this.seconds != INFINITE_SECONDS))) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Time " + this.seconds + " seconds, "
                            + this.nanoseconds + " nanoseconds.");
        }
        if ((nsec >= 1000000000)
                && ((nsec != Long.MAX_VALUE) || (sec != Long.MAX_VALUE))) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Time " + sec + " seconds, " + nsec
                            + " nanoseconds.");
        }
        if (this.seconds > sec)
            return 1;
        if (this.seconds < sec)
            return -1;
        if (this.nanoseconds > nsec)
            return 1;
        if (this.nanoseconds < nsec)
            return -1;
        return 0;
    }

    @Override
    public boolean equals(Object other) {
        if (other instanceof ModifiableTimeImpl) {
            if (((ModifiableTimeImpl) other).compareTo(this) == 0) {
                return true;
            }
        }
        return false;
    }

    @Override
    public int hashCode() {
        return 31 * 17 + (int) (this.totalNanos ^ (this.totalNanos >>> 32));
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public void copyFrom(Time src) {
        if (src == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Time (null) provided.");
        }
        this.seconds = src.getTime(TimeUnit.SECONDS);
        this.nanoseconds = src.getRemainder(TimeUnit.SECONDS,
                TimeUnit.NANOSECONDS);
        this.totalNanos = this.nanoseconds + this.seconds * 1000 * 1000 * 1000;

    }

    @Override
    public Time immutableCopy() {
        return new TimeImpl(this.environment, this.seconds, this.nanoseconds);
    }

    @Override
    public void setTime(long time, TimeUnit unit) {
        if (unit == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal TimeUnit (null) provided.");
        }
        this.seconds = TimeUnit.SECONDS.convert(time, unit);
        this.totalNanos = TimeUnit.NANOSECONDS.convert(time, unit);
        this.nanoseconds = this.totalNanos
                - (this.seconds * 1000 * 1000 * 1000);

    }

    @Override
    public void add(Duration duration) {
        if (duration == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Duration (null) provided.");
        }
        if (!this.isValid()) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot add duration to invalid time.");
        }
        long sec = duration.getDuration(TimeUnit.SECONDS);
        long nsec = duration.getRemainder(TimeUnit.SECONDS,
                TimeUnit.NANOSECONDS);

        if (!((this.nanoseconds < 1000000000) || (this.nanoseconds == INFINITE_NANOSECONDS))) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration");
        }
        if (!((nsec < 1000000000) || (nsec == Long.MAX_VALUE))) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration");
        }

        if (this.nanoseconds == INFINITE_NANOSECONDS) {
            if (this.seconds == INFINITE_SECONDS) {
                this.seconds = INFINITE_SECONDS;
                this.nanoseconds = INFINITE_NANOSECONDS;
                this.totalNanos = Long.MAX_VALUE;
                return;
            }
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration " + this.seconds + " seconds, "
                            + this.nanoseconds + " nanoseconds.");

        }
        if (nsec == Long.MAX_VALUE) {
            if (sec == Long.MAX_VALUE) {
                this.seconds = INFINITE_SECONDS;
                this.nanoseconds = INFINITE_NANOSECONDS;
                this.totalNanos = Long.MAX_VALUE;
                return;
            }
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration " + sec + " seconds, " + nsec
                            + " nanoseconds.");
        }
        if (sec > 0) {
            if (INFINITE_SECONDS - sec <= this.seconds) {
                /*
                 * is identical to 'INFINITE_SECONDS <= this.seconds + sec' In
                 * other words the sum is larger than infinite, so results must
                 * be infinite.
                 */
                this.seconds = INFINITE_SECONDS;
                this.nanoseconds = INFINITE_NANOSECONDS;
                this.totalNanos = Long.MAX_VALUE;
                return;
            }
        }
        this.seconds += sec;
        this.nanoseconds += nsec;
        this.normalizeMe();

    }

    @Override
    public void add(long duration, TimeUnit unit) {
        DurationImpl temp = new DurationImpl(this.environment, duration, unit)
                .normalize();

        this.add(temp);
    }

    public boolean isInfinite() {
        return (this.totalNanos == Long.MAX_VALUE) ? true : false;
    }

    @Override
    public void subtract(Duration duration) {
        if (duration == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Duration (null) provided.");
        }
        if (!this.isValid()) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot subtract duration from invalid time.");
        }
        if (this.isInfinite() && duration.isInfinite()) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot subtract infinite duration from infinite time.");
        }
        long sec = duration.getDuration(TimeUnit.SECONDS);
        long nsec = duration.getRemainder(TimeUnit.SECONDS,
                TimeUnit.NANOSECONDS);

        if (!((this.nanoseconds < 1000000000) || (this.nanoseconds == INFINITE_NANOSECONDS))) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration " + this.seconds + " seconds, "
                            + this.nanoseconds + " nanoseconds.");
        }
        if (!((nsec < 1000000000) || (nsec == Long.MAX_VALUE))) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration " + sec + " seconds, " + nsec
                            + " nanoseconds.");
        }

        if (this.nanoseconds == INFINITE_NANOSECONDS) {
            if (this.seconds == INFINITE_SECONDS) {
                this.seconds = INFINITE_SECONDS;
                this.nanoseconds = INFINITE_NANOSECONDS;
                this.totalNanos = Long.MAX_VALUE;
                return;
            }
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration " + this.seconds + " seconds, "
                            + this.nanoseconds + " nanoseconds.");
        }
        if (nsec == Long.MAX_VALUE) {
            if (sec == Long.MAX_VALUE) {
                this.seconds = INFINITE_SECONDS;
                this.nanoseconds = INFINITE_NANOSECONDS;
                this.totalNanos = Long.MAX_VALUE;
                return;
            }
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal duration " + sec + " seconds, " + nsec
                            + " nanoseconds.");
        }
        if ((sec <= 0) && (INFINITE_SECONDS + sec <= this.seconds)) {
            /*
             * is identical to '-(INFINITE_SECONDS >= this.seconds - sec)' In
             * other words the sum is larger than infinite, so results must be
             * infinite.
             */
            this.seconds = INFINITE_SECONDS;
            this.nanoseconds = INFINITE_NANOSECONDS;
            this.totalNanos = Long.MAX_VALUE;
            return;
        }
        this.seconds -= sec;

        if (this.seconds == INFINITE_SECONDS) {
            this.seconds = INFINITE_SECONDS;
            this.nanoseconds = INFINITE_NANOSECONDS;
            this.totalNanos = Long.MAX_VALUE;
            return;
        }
        this.nanoseconds -= nsec;

        if (this.nanoseconds < 0) {
            this.seconds--;
            this.nanoseconds += 1000000000;
        }
        this.normalizeMe();
    }

    @Override
    public void subtract(long duration, TimeUnit unit) {
        if (unit == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal TimeUnit (null) provided.");
        }
        DurationImpl temp = new DurationImpl(this.environment, duration, unit)
                .normalize();

        this.subtract(temp);
    }

    @Override
    public long getTime(TimeUnit inThisUnit) {
        if (inThisUnit == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal TimeUnit (null) provided.");
        }
        if (this.isInfinite()) {
            return Long.MAX_VALUE;
        }
        return inThisUnit.convert(this.totalNanos, TimeUnit.NANOSECONDS);
    }

    @Override
    public long getRemainder(TimeUnit primaryUnit, TimeUnit remainderUnit) {
        if ((primaryUnit == null) || (remainderUnit == null)) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal TimeUnit (null) provided.");
        }
        if (remainderUnit.compareTo(primaryUnit) >= 0) {
            return 0;
        }
        if (this.isInfinite()) {
            return Long.MAX_VALUE;
        }
        long primaryDuration = primaryUnit.convert(this.totalNanos,
                TimeUnit.NANOSECONDS);
        long primaryNanoDuration = TimeUnit.NANOSECONDS.convert(
                primaryDuration, primaryUnit);
        long leftOverNanos = this.totalNanos - primaryNanoDuration;

        return remainderUnit.convert(leftOverNanos, TimeUnit.NANOSECONDS);
    }

    @Override
    public boolean isValid() {
        if (this.seconds == DDS.TIMESTAMP_INVALID_SEC.value
                && this.nanoseconds == DDS.TIMESTAMP_INVALID_NSEC.value) {
            return false;
        }
        if ((this.nanoseconds >= 1000000000)
                && ((this.nanoseconds != INFINITE_NANOSECONDS) || (this.seconds != INFINITE_SECONDS))) {
            return false;
        }
        return true;
    }

    @Override
    public ModifiableTime modifiableCopy() {
        return new ModifiableTimeImpl(this.environment, this.seconds,
                this.nanoseconds);
    }

    public DDS.Time_t convert() {
        return new DDS.Time_t((int) this.seconds, (int) this.nanoseconds);
    }

    @Override
    public String toString() {
        if (this.isInfinite()) {
            return "infinite";
        }
        return this.totalNanos + " ns";
    }
}
