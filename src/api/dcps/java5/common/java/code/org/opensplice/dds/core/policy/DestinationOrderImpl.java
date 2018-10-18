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

import org.omg.dds.core.policy.DestinationOrder;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class DestinationOrderImpl extends QosPolicyImpl implements
        DestinationOrder {
    private static final long serialVersionUID = 2722089115467848342L;
    private final Kind kind;

    public DestinationOrderImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.kind = Kind.BY_RECEPTION_TIMESTAMP;
    }

    public DestinationOrderImpl(OsplServiceEnvironment environment, Kind kind) {
        super(environment);

        if (kind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid Kind.");
        }
        this.kind = kind;
    }

    @Override
    public Comparable<DestinationOrder> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(DestinationOrder o) {
        // BY_RECEPTION_TIMESTAMP < BY_SOURCE_TIMESTAMP
        if (this.kind.equals(o.getKind())) {
            return 0;
        } else if (this.kind.equals(Kind.BY_RECEPTION_TIMESTAMP)) {
            return -1;
        }
        return 1;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof DestinationOrderImpl)) {
            return false;
        }
        return (this.kind == ((DestinationOrderImpl) other).kind);
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
    public DestinationOrder withKind(Kind kind) {
        return new DestinationOrderImpl(this.environment, kind);
    }

    @Override
    public DestinationOrder withReceptionTimestamp() {
        return new DestinationOrderImpl(this.environment,
                Kind.BY_RECEPTION_TIMESTAMP);
    }

    @Override
    public DestinationOrder withSourceTimestamp() {
        return new DestinationOrderImpl(this.environment,
                Kind.BY_SOURCE_TIMESTAMP);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return DestinationOrder.class;
    }

}
