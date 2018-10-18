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
import org.omg.dds.core.policy.Deadline;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class DeadlineImpl extends QosPolicyImpl implements Deadline {
    private static final long serialVersionUID = -3533726043203132941L;
    private final Duration period;

    public DeadlineImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.period = environment.getSPI().infiniteDuration();
    }

    public DeadlineImpl(OsplServiceEnvironment environment, long period,
            TimeUnit unit) {
        super(environment);
        this.period = Duration.newDuration(period, unit, environment);
    }

    public DeadlineImpl(OsplServiceEnvironment environment, Duration period) {
        super(environment);
        this.period = period;

        if (period == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid period.");
        }
    }

    @Override
    public Comparable<Deadline> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(Deadline o) {
        return this.period.compareTo(o.getPeriod());
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof DeadlineImpl)) {
            return false;
        }
        return this.period.equals(((DeadlineImpl) other).period);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.period.hashCode();
    }

    @Override
    public Duration getPeriod() {
        return this.period;
    }

    @Override
    public Deadline withPeriod(Duration period) {
        return new DeadlineImpl(this.environment, period);
    }

    @Override
    public Deadline withPeriod(long period, TimeUnit unit) {
        return new DeadlineImpl(this.environment, period, unit);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Deadline.class;
    }
}
