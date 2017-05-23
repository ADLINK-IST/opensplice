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

import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.ResourceLimits;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class ResourceLimitsImpl extends QosPolicyImpl implements ResourceLimits {
    private static final long serialVersionUID = 918915709322634268L;
    private final int maxSamples;
    private final int maxInstances;
    private final int maxSamplesPerInstance;

    public ResourceLimitsImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.maxSamples = -1;
        this.maxInstances = -1;
        this.maxSamplesPerInstance = -1;
    }
    
    public ResourceLimitsImpl(OsplServiceEnvironment environment,
            int maxSamples, int maxInstances, int maxSamplesPerInstance) {
        super(environment);
        this.maxSamples = maxSamples;
        this.maxInstances = maxInstances;
        this.maxSamplesPerInstance = maxSamplesPerInstance;
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
    public ResourceLimits withMaxSamples(int maxSamples) {
        return new ResourceLimitsImpl(this.environment, maxSamples,
                this.maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public ResourceLimits withMaxInstances(int maxInstances) {
        return new ResourceLimitsImpl(this.environment, this.maxSamples,
                maxInstances, this.maxSamplesPerInstance);
    }

    @Override
    public ResourceLimits withMaxSamplesPerInstance(int maxSamplesPerInstance) {
        return new ResourceLimitsImpl(this.environment, this.maxSamples,
                this.maxInstances, maxSamplesPerInstance);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return ResourceLimits.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof ResourceLimitsImpl)) {
            return false;
        }
        ResourceLimitsImpl r = (ResourceLimitsImpl) other;

        if (this.maxInstances != r.maxInstances) {
            return false;
        }
        if (this.maxSamples != r.maxSamples) {
            return false;
        }
        return (this.maxSamplesPerInstance == r.maxSamplesPerInstance);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + this.maxInstances;
        result = prime * result + this.maxSamples;

        return prime * result + this.maxSamplesPerInstance;
    }
}
