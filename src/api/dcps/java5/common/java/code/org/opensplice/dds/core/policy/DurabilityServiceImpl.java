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
import org.omg.dds.core.policy.DurabilityService;
import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.History.Kind;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.omg.dds.core.policy.QosPolicy;

public class DurabilityServiceImpl extends QosPolicyImpl implements
        DurabilityService {
    private static final long serialVersionUID = -3906397729567497050L;
    private final Duration serviceCleanupDelay;
    private final History history;
    private final int maxSamples;
    private final int maxInstances;
    private final int maxSamplesPerInstance;

    public DurabilityServiceImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.serviceCleanupDelay = environment.getSPI().zeroDuration();
        this.history = new HistoryImpl(environment, Kind.KEEP_LAST, 1);
        this.maxSamples = -1;
        this.maxInstances = -1;
        this.maxSamplesPerInstance = -1;
    }

    public DurabilityServiceImpl(OsplServiceEnvironment environment,
            Duration serviceCleanupDelay, Kind historyKind, int historyDepth,
            int maxSamples, int maxInstances, int maxSamplesPerInstance) {
        super(environment);
        this.serviceCleanupDelay = serviceCleanupDelay;
        this.history = new HistoryImpl(environment, historyKind, historyDepth);
        this.maxSamples = maxSamples;
        this.maxInstances = maxInstances;
        this.maxSamplesPerInstance = maxSamplesPerInstance;

        if (serviceCleanupDelay == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid service cleanup delay.");
        }
    }

    @Override
    public Duration getServiceCleanupDelay() {
        return this.serviceCleanupDelay;
    }

    @Override
    public Kind getHistoryKind() {
        return this.history.getKind();
    }

    @Override
    public int getHistoryDepth() {
        return this.history.getDepth();
    }

    @Override
    public int getMaxSamples() {
        return this.maxSamples;
    }

    @Override
    public int getMaxInstances() {
        return this.maxInstances;
    }

    @Override
    public int getMaxSamplesPerInstance() {
        return this.maxSamplesPerInstance;
    }

    @Override
    public DurabilityService withServiceCleanupDelay(
            Duration serviceCleanupDelay) {
        return new DurabilityServiceImpl(this.environment, serviceCleanupDelay,
                this.history.getKind(), this.history.getDepth(), this.maxSamples,
                this.maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public DurabilityService withServiceCleanupDelay(long serviceCleanupDelay,
            TimeUnit unit) {
        return new DurabilityServiceImpl(this.environment, this.environment
                .getSPI().newDuration(serviceCleanupDelay, unit),
                this.history.getKind(), this.history.getDepth(), this.maxSamples,
                this.maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public DurabilityService withHistoryKind(Kind historyKind) {
        return new DurabilityServiceImpl(this.environment,
                this.serviceCleanupDelay, historyKind, this.history.getDepth(),
                this.maxSamples, this.maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public DurabilityService withHistoryDepth(int historyDepth) {
        return new DurabilityServiceImpl(this.environment,
                this.serviceCleanupDelay, this.history.getKind(), historyDepth,
                this.maxSamples, this.maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public DurabilityService withMaxSamples(int maxSamples) {
        return new DurabilityServiceImpl(this.environment,
                this.serviceCleanupDelay, this.history.getKind(), this.history.getDepth(),
                maxSamples, this.maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public DurabilityService withMaxInstances(int maxInstances) {
        return new DurabilityServiceImpl(this.environment,
                this.serviceCleanupDelay, this.history.getKind(), this.history.getDepth(),
                this.maxSamples, maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public DurabilityService withMaxSamplesPerInstance(int maxSamplesPerInstance) {
        return new DurabilityServiceImpl(this.environment,
                this.serviceCleanupDelay, this.history.getKind(), this.history.getDepth(),
                this.maxSamples, this.maxInstances, maxSamplesPerInstance);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return DurabilityService.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof DurabilityServiceImpl)) {
            return false;
        }
        DurabilityServiceImpl d = (DurabilityServiceImpl) other;

        if (!this.history.equals(d.history)) {
            return false;
        }
        if (this.maxInstances != d.maxInstances) {
            return false;
        }
        if (this.maxSamples != d.maxSamples) {
            return false;
        }
        if (this.maxSamplesPerInstance != d.maxSamplesPerInstance) {
            return false;
        }
        return this.serviceCleanupDelay.equals(d.serviceCleanupDelay);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + this.history.hashCode();
        result = prime * result + this.maxInstances;
        result = prime * result + this.maxSamples;
        result = prime * result + this.maxSamplesPerInstance;
        result = prime * result + this.serviceCleanupDelay.hashCode();

        return result;
    }
}
