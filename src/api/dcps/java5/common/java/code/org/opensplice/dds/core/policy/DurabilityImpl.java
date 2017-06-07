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

import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class DurabilityImpl extends QosPolicyImpl implements Durability {
    private static final long serialVersionUID = -3661996204819852834L;
    private final Kind kind;

    public DurabilityImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.kind = Kind.VOLATILE;
    }

    public DurabilityImpl(OsplServiceEnvironment environment, Kind kind) {
        super(environment);
        this.kind = kind;

        if (this.kind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied kind is null.");
        }
    }

    @Override
    public Comparable<Durability> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(Durability o) {
        // VOLATILE<TRANSIENT_LOCAL<TRANSIENT<PERSISTENT
        Kind other = o.getKind();

        if (other.equals(this.kind)) {
            return 0;
        } else if (other.equals(Kind.VOLATILE)) {
            return 1;
        } else if (other.equals(Kind.TRANSIENT_LOCAL)) {
            if (this.kind.equals(Kind.VOLATILE)) {
                return -1;
            }
            return 1;
        } else if (other.equals(Kind.TRANSIENT)) {
            if (this.kind.equals(Kind.VOLATILE)
                    || this.kind.equals(Kind.TRANSIENT_LOCAL)) {
                return -1;
            }
            return 1;
        }
        return -1;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof DurabilityImpl)) {
            return false;
        }
        return (this.kind == ((DurabilityImpl) other).kind);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.kind.hashCode();
    }

    @Override
    public Kind getKind() {
        return this.kind;
    }

    @Override
    public Durability withKind(Kind kind) {
        return new DurabilityImpl(this.environment, kind);
    }

    @Override
    public Durability withVolatile() {
        return new DurabilityImpl(this.environment, Kind.VOLATILE);
    }

    @Override
    public Durability withTransientLocal() {
        return new DurabilityImpl(this.environment, Kind.TRANSIENT_LOCAL);
    }

    @Override
    public Durability withTransient() {
        return new DurabilityImpl(this.environment, Kind.TRANSIENT);
    }

    @Override
    public Durability withPersistent() {
        return new DurabilityImpl(this.environment, Kind.PERSISTENT);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Durability.class;
    }
}
