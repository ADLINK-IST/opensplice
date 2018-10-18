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

import org.omg.dds.core.policy.Ownership;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class OwnershipImpl extends QosPolicyImpl implements Ownership {
    private static final long serialVersionUID = 5349819428623413367L;
    private final Kind kind;

    public OwnershipImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.kind = Kind.SHARED;
    }

    public OwnershipImpl(OsplServiceEnvironment environment, Kind kind) {
        super(environment);

        if (kind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied kind is invalid.");
        }
        this.kind = kind;
    }

    @Override
    public Comparable<Ownership> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(Ownership o) {
        if (this.kind.equals(o.getKind())) {
            return 0;
        } else if (this.kind.equals(Kind.SHARED)) {
            return -1;
        }
        return 1;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof OwnershipImpl)) {
            return false;
        }
        return this.kind.equals(((OwnershipImpl) other).kind);
    }

    @Override
    public Kind getKind() {
        return this.kind;
    }

    @Override
    public Ownership withKind(Kind kind) {
        return new OwnershipImpl(this.environment, kind);
    }

    @Override
    public Ownership withShared() {
        return new OwnershipImpl(this.environment, Kind.SHARED);
    }

    @Override
    public Ownership withExclusive() {
        return new OwnershipImpl(this.environment, Kind.EXCLUSIVE);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Ownership.class;
    }

    @Override
    public int hashCode() {
        return 17 * 31 + this.kind.hashCode();
    }
}
