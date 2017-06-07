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
import org.omg.dds.core.policy.LatencyBudget;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class LatencyBudgetImpl extends QosPolicyImpl implements LatencyBudget {
    private static final long serialVersionUID = 1583305102265712684L;
    private final Duration duration;

    public LatencyBudgetImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.duration = environment.getSPI().zeroDuration();
    }

    public LatencyBudgetImpl(OsplServiceEnvironment environment,
            Duration duration) {
        super(environment);

        if (duration == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid duration.");
        }
        this.duration = duration;
    }

    @Override
    public Comparable<LatencyBudget> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(LatencyBudget o) {
        return this.duration.compareTo(o.getDuration());
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof LatencyBudgetImpl)) {
            return false;
        }
        return this.duration.equals(((LatencyBudgetImpl) other).duration);
    }

    @Override
    public Duration getDuration() {
        return this.duration;
    }

    @Override
    public LatencyBudget withDuration(Duration duration) {
        return new LatencyBudgetImpl(this.environment, duration);
    }

    @Override
    public LatencyBudget withDuration(long duration, TimeUnit unit) {
        return new LatencyBudgetImpl(this.environment, this.environment
                .getSPI().newDuration(duration, unit));
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return LatencyBudget.class;
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.duration.hashCode();
    }

}
