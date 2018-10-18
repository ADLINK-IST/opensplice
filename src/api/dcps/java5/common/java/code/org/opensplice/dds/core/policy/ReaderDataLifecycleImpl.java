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
import org.omg.dds.core.policy.ReaderDataLifecycle;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class ReaderDataLifecycleImpl extends QosPolicyImpl implements
        org.opensplice.dds.core.policy.ReaderDataLifecycle {
    private static final long serialVersionUID = 6465409872651281346L;
    private final Duration autoPurgeNoWriterSamplesDelay;
    private final Duration autoPurgeDisposedSamplesDelay;
    private final boolean autoPurgeDisposeAll;
    private Kind invalidSampleVisibility;

    public ReaderDataLifecycleImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.autoPurgeNoWriterSamplesDelay = environment.getSPI()
                .infiniteDuration();
        this.autoPurgeDisposedSamplesDelay = environment.getSPI()
                .infiniteDuration();
        this.autoPurgeDisposeAll = false;
        this.invalidSampleVisibility = Kind.MINIMUM;
    }

    public ReaderDataLifecycleImpl(OsplServiceEnvironment environment,
            Duration autoPurgeNoWriterSamplesDelay,
            Duration autoPurgeDisposedSamplesDelay,
            boolean autoPurgeDisposeAll,
            Kind invalidSampleVisibility) {
        super(environment);

        if (autoPurgeNoWriterSamplesDelay == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid autoPurgeNoWriterSamplesDelay duration.");
        }
        if (autoPurgeDisposedSamplesDelay == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid autoPurgeDisposedSamplesDelay duration.");
        }
        if (invalidSampleVisibility == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid invalidSampleVisibility.");
        }
        this.autoPurgeNoWriterSamplesDelay = autoPurgeNoWriterSamplesDelay;
        this.autoPurgeDisposedSamplesDelay = autoPurgeDisposedSamplesDelay;
        this.autoPurgeDisposeAll = autoPurgeDisposeAll;
        this.invalidSampleVisibility = invalidSampleVisibility;
    }

    @Override
    public Duration getAutoPurgeNoWriterSamplesDelay() {
        return this.autoPurgeNoWriterSamplesDelay;
    }

    @Override
    public Duration getAutoPurgeDisposedSamplesDelay() {
        return this.autoPurgeDisposedSamplesDelay;
    }

    @Override
    public boolean getAutoPurgeDisposeAll() {
        return this.autoPurgeDisposeAll;
    }

    @Override
    public Kind getInvalidSampleInvisibility() {
        return this.invalidSampleVisibility;
    }

    @Override
    public ReaderDataLifecycle withAutoPurgeNoWriterSamplesDelay(
            Duration autoPurgeNoWriterSamplesDelay) {
        return new ReaderDataLifecycleImpl(this.environment,
                autoPurgeNoWriterSamplesDelay,
                this.autoPurgeDisposedSamplesDelay,
                this.autoPurgeDisposeAll,
                this.invalidSampleVisibility);
    }

    @Override
    public ReaderDataLifecycle withAutoPurgeNoWriterSamplesDelay(
            long autoPurgeNoWriterSamplesDelay, TimeUnit unit) {
        return new ReaderDataLifecycleImpl(this.environment, this.environment
                .getSPI().newDuration(autoPurgeNoWriterSamplesDelay, unit),
                this.autoPurgeDisposedSamplesDelay,
                this.autoPurgeDisposeAll,
                this.invalidSampleVisibility);
    }

    @Override
    public ReaderDataLifecycle withAutoPurgeDisposedSamplesDelay(
            Duration autoPurgeDisposedSamplesDelay) {
        return new ReaderDataLifecycleImpl(this.environment,
                this.autoPurgeNoWriterSamplesDelay,
                autoPurgeDisposedSamplesDelay,
                this.autoPurgeDisposeAll,
                this.invalidSampleVisibility);
    }

    @Override
    public org.opensplice.dds.core.policy.ReaderDataLifecycle withAutoPurgeDisposeAll() {
        return new ReaderDataLifecycleImpl(this.environment,
                this.autoPurgeNoWriterSamplesDelay,
                this.autoPurgeDisposedSamplesDelay,
                true,
                this.invalidSampleVisibility);
    }

    @Override
    public org.opensplice.dds.core.policy.ReaderDataLifecycle withInvalidSampleInvisibility(
            Kind kind) {
        return new ReaderDataLifecycleImpl(this.environment,
                this.autoPurgeNoWriterSamplesDelay,
                this.autoPurgeDisposedSamplesDelay,
                this.autoPurgeDisposeAll,
                kind);
    }

    @Override
    public ReaderDataLifecycle withAutoPurgeDisposedSamplesDelay(
            long autoPurgeDisposedSamplesDelay, TimeUnit unit) {
        return new ReaderDataLifecycleImpl(this.environment,
                this.autoPurgeNoWriterSamplesDelay, this.environment.getSPI()
                        .newDuration(autoPurgeDisposedSamplesDelay, unit),
                        this.autoPurgeDisposeAll,
                        this.invalidSampleVisibility);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return ReaderDataLifecycle.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof ReaderDataLifecycleImpl)) {
            return false;
        }
        ReaderDataLifecycleImpl r = (ReaderDataLifecycleImpl) other;

        if (this.autoPurgeDisposeAll != r.autoPurgeDisposeAll) {
            return false;
        }
        if (this.invalidSampleVisibility != r.invalidSampleVisibility) {
            return false;
        }
        if (!this.autoPurgeDisposedSamplesDelay
                .equals(r.autoPurgeDisposedSamplesDelay)) {
            return false;
        }
        return this.autoPurgeNoWriterSamplesDelay
                .equals(r.autoPurgeNoWriterSamplesDelay);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + (this.autoPurgeDisposeAll ? 1 : 0);
        result = prime * result + this.autoPurgeDisposedSamplesDelay.hashCode();

        return prime * result + this.autoPurgeNoWriterSamplesDelay.hashCode();
    }
}
