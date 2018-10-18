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
import org.omg.dds.core.policy.Liveliness;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class LivelinessImpl extends QosPolicyImpl implements Liveliness {
    private static final long serialVersionUID = 8654350221156374238L;
    private final Kind kind;
    private final Duration leaseDuration;

    public LivelinessImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.kind = Kind.AUTOMATIC;
        this.leaseDuration = environment.getSPI().infiniteDuration();
    }

    public LivelinessImpl(OsplServiceEnvironment environment, Kind kind,
            Duration leaseDuration) {
        super(environment);

        if (kind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied kind is null.");
        }
        if (leaseDuration == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied lease duration is null.");
        }
        this.kind = kind;
        this.leaseDuration = leaseDuration;
    }

    @Override
    public Comparable<Liveliness> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(Liveliness o) {
        /* AUTOMATIC < MANUAL_BY_PARTICIPANT < MANUAL_BY_TOPIC and offered
         * leaseDuration <= requested leaseDuration */
        Kind other = o.getKind();

        if (this.kind.equals(other)) {
            return this.leaseDuration.compareTo(o.getLeaseDuration());
        } else if (this.kind.equals(Kind.AUTOMATIC)) {
            return -1;
        } else if (this.kind.equals(Kind.MANUAL_BY_PARTICIPANT)) {
            if (other.equals(Kind.AUTOMATIC)) {
                return 1;
            }
            return -1;
        }
        return 1;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof LivelinessImpl)) {
            return false;
        }
        LivelinessImpl l = (LivelinessImpl) other;

        if (!l.kind.equals(this.kind)) {
            return false;
        }
        return this.leaseDuration.equals(l.leaseDuration);
    }

    @Override
    public Kind getKind() {
        return this.kind;
    }

    @Override
    public Duration getLeaseDuration() {
        return this.leaseDuration;
    }

    @Override
    public Liveliness withKind(Kind kind) {
        return new LivelinessImpl(this.environment, kind, this.leaseDuration);
    }

    @Override
    public Liveliness withLeaseDuration(Duration leaseDuration) {
        return new LivelinessImpl(this.environment, this.kind, leaseDuration);
    }

    @Override
    public Liveliness withLeaseDuration(long leaseDuration, TimeUnit unit) {
        return new LivelinessImpl(this.environment, this.kind, this.environment
                .getSPI().newDuration(leaseDuration, unit));
    }

    @Override
    public Liveliness withAutomatic() {
        return new LivelinessImpl(this.environment, Kind.AUTOMATIC,
                leaseDuration);
    }

    @Override
    public Liveliness withManualByParticipant() {
        return new LivelinessImpl(this.environment, Kind.MANUAL_BY_PARTICIPANT,
                leaseDuration);
    }

    @Override
    public Liveliness withManualByTopic() {
        return new LivelinessImpl(this.environment, Kind.MANUAL_BY_TOPIC,
                leaseDuration);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Liveliness.class;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + this.kind.hashCode();
        return prime * result + this.leaseDuration.hashCode();
    }

}
