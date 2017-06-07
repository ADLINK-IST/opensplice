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
package org.opensplice.dds.core.policy;

import java.util.concurrent.TimeUnit;

import org.omg.dds.core.Duration;
import org.omg.dds.core.policy.Lifespan;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class LifespanImpl extends QosPolicyImpl implements Lifespan {
    private static final long serialVersionUID = 7903452866315787071L;
    private final Duration duration;

    public LifespanImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.duration = environment.getSPI().infiniteDuration();
    }

    public LifespanImpl(OsplServiceEnvironment environment, Duration duration) {
        super(environment);

        if (duration == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid duration.");
        }
        this.duration = duration;
    }

    @Override
    public Duration getDuration() {
        return this.duration;
    }

    @Override
    public Lifespan withDuration(Duration duration) {
        return new LifespanImpl(this.environment, duration);
    }

    @Override
    public Lifespan withDuration(long duration, TimeUnit unit) {
        return new LifespanImpl(this.environment, this.environment.getSPI()
                .newDuration(duration, unit));
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Lifespan.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof LifespanImpl)) {
            return false;
        }
        return this.duration.equals(((LifespanImpl) other).duration);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.duration.hashCode();
    }

}
