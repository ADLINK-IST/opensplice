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
package org.opensplice.dds.core.policy;

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.DurationImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class ReaderLifespanImpl extends QosPolicyImpl implements ReaderLifespan {
    private static final long serialVersionUID = 6766092830787145265L;
    private final Duration duration;

    public ReaderLifespanImpl(OsplServiceEnvironment environment,
            Duration duration) {
        super(environment);

        if (duration == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid duration.");
        }
        this.duration = duration;
    }

    public ReaderLifespanImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.duration = environment.getSPI().infiniteDuration();
    }

    @Override
    public Duration getDuration() {
        return this.duration;
    }

    @Override
    public ReaderLifespan withDuration(Duration duration) {
        return new ReaderLifespanImpl(this.environment, duration);
    }

    @Override
    public ReaderLifespan withDuration(long duration, TimeUnit unit) {
        return new ReaderLifespanImpl(this.environment, new DurationImpl(
                this.environment, duration, unit));
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return ReaderLifespan.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof ReaderLifespanImpl)) {
            return false;
        }
        return this.duration.equals(((ReaderLifespanImpl) other).duration);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.duration.hashCode();
    }

}
