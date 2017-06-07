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
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.TimeBasedFilter;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class TimeBasedFilterImpl extends QosPolicyImpl implements
        TimeBasedFilter {
    private static final long serialVersionUID = -7710151999422912449L;
    private final Duration minimumSeparation;

    public TimeBasedFilterImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.minimumSeparation = environment.getSPI().zeroDuration();
    }
    
    public TimeBasedFilterImpl(OsplServiceEnvironment environment,
            Duration minimumSeparation) {
        super(environment);
        
        if(minimumSeparation == null){
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid mimimumSeparation duration.");
        }
        this.minimumSeparation = minimumSeparation;
    }

    @Override
    public Duration getMinimumSeparation() {
        return this.minimumSeparation;
    }

    @Override
    public TimeBasedFilter withMinimumSeparation(Duration minimumSeparation) {
        return new TimeBasedFilterImpl(this.environment, minimumSeparation);
    }

    @Override
    public TimeBasedFilter withMinimumSeparation(long minimumSeparation,
            TimeUnit unit) {
        return new TimeBasedFilterImpl(this.environment, this.environment
                .getSPI().newDuration(minimumSeparation, unit));
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return TimeBasedFilter.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof TimeBasedFilterImpl)) {
            return false;
        }
        return this.minimumSeparation
                .equals(((TimeBasedFilterImpl) other).minimumSeparation);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.minimumSeparation.hashCode();
    }
}
