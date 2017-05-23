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
import org.omg.dds.core.Time;

public class TimeImpl extends ModifiableTimeImpl {
    private static final long serialVersionUID = 7478771004119429231L;

    public TimeImpl(OsplServiceEnvironment environment, long duration,
            TimeUnit unit) {
        super(environment, duration, unit);
    }

    public TimeImpl(OsplServiceEnvironment environment, long seconds,
            long nanoseconds) {
        super(environment, seconds, nanoseconds);
    }

    @Override
    public TimeImpl normalize() {
        ModifiableTimeImpl modTime = super.normalize();

        return (TimeImpl) modTime.immutableCopy();
    }

    @Override
    public void copyFrom(Time src) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Time is not modifiable.");
    }

    @Override
    public Time immutableCopy() {
        return this;
    }

    @Override
    public void setTime(long time, TimeUnit unit) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Time is not modifiable.");
    }

    @Override
    public void add(Duration duration) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Time is not modifiable.");
    }

    @Override
    public void add(long duration, TimeUnit unit) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Time is not modifiable.");
    }

    @Override
    public void subtract(Duration duration) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Time is not modifiable.");
    }

    @Override
    public void subtract(long duration, TimeUnit unit) {
        throw new UnsupportedOperationExceptionImpl(this.environment,
                "Time is not modifiable.");
    }
}
