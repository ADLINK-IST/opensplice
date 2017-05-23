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
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class ReliabilityImpl extends QosPolicyImpl implements Reliability {
    private static final long serialVersionUID = 6430015862797188917L;
    private final Kind kind;
    private final Duration maxBlockingTime;
    private final boolean synchronous;

    public ReliabilityImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.kind = Kind.BEST_EFFORT;
        this.maxBlockingTime = environment.getSPI().newDuration(100,
                TimeUnit.MILLISECONDS);
        this.synchronous = false;
    }

    public ReliabilityImpl(OsplServiceEnvironment environment, Kind kind,
            Duration maxBlockingTime, boolean synchronous) {
        super(environment);

        if (kind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid kind.");
        }
        if (maxBlockingTime == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid maxBlockingTime.");
        }
        this.kind = kind;
        this.maxBlockingTime = maxBlockingTime;
        this.synchronous = synchronous;
    }

    @Override
    public Comparable<org.omg.dds.core.policy.Reliability> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(org.omg.dds.core.policy.Reliability o) {
        if (this.kind.equals(o.getKind())) {
            int max = this.maxBlockingTime.compareTo(o.getMaxBlockingTime());

            if(max == 0){
                if(o instanceof ReliabilityImpl){
                    if (this.synchronous == ((ReliabilityImpl) o)
                            .isSynchronous()) {
                        return 0;
                    } else if (this.synchronous) {
                        return 1;
                    }
                    return -1;
                }
            }
            return max;
        } else if (this.kind.equals(Kind.RELIABLE)) {
            return 1;
        }
        return -1;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof ReliabilityImpl)) {
            return false;
        }
        ReliabilityImpl r = (ReliabilityImpl) other;

        if (this.kind != r.kind) {
            return false;
        }
        if (this.synchronous != r.synchronous) {
            return false;
        }
        return this.maxBlockingTime.equals(r.maxBlockingTime);
    }

    @Override
    public Kind getKind() {
        return this.kind;
    }

    @Override
    public Duration getMaxBlockingTime() {
        return this.maxBlockingTime;
    }

    @Override
    public Reliability withKind(Kind kind) {
        return new ReliabilityImpl(this.environment, kind,
                this.maxBlockingTime, this.synchronous);
    }

    @Override
    public Reliability withMaxBlockingTime(Duration maxBlockingTime) {
        return new ReliabilityImpl(this.environment, this.kind,
                maxBlockingTime, this.synchronous);
    }

    @Override
    public Reliability withMaxBlockingTime(long maxBlockingTime, TimeUnit unit) {
        return new ReliabilityImpl(this.environment, this.kind,
                this.environment.getSPI().newDuration(maxBlockingTime, unit),
                this.synchronous);
    }

    @Override
    public Reliability withBestEffort() {
        return new ReliabilityImpl(this.environment, Kind.BEST_EFFORT,
                this.maxBlockingTime, this.synchronous);
    }

    @Override
    public Reliability withReliable() {
        return new ReliabilityImpl(this.environment, Kind.RELIABLE,
                this.maxBlockingTime, this.synchronous);
    }

    @Override
    public boolean isSynchronous() {
        return this.synchronous;
    }

    @Override
    public Reliability withSynchronous(boolean synchronous) {
        return new ReliabilityImpl(this.environment, this.kind,
                this.maxBlockingTime, synchronous);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return org.omg.dds.core.policy.Reliability.class;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + this.kind.hashCode();
        result = prime * result + (this.synchronous ? 1 : 0);

        return prime * result + this.maxBlockingTime.hashCode();
    }

}
