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

import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.TransportPriority;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class TransportPriorityImpl extends QosPolicyImpl implements
TransportPriority {
    private static final long serialVersionUID = -2681488718301095677L;
    private final int value;

    public TransportPriorityImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.value = 0;
    }

    public TransportPriorityImpl(OsplServiceEnvironment environment, int value) {
        super(environment);
        this.value = value;
    }

    @Override
    public int getValue() {
        return this.value;
    }

    @Override
    public TransportPriority withValue(int value) {
        return new TransportPriorityImpl(this.environment, value);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return TransportPriority.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof TransportPriorityImpl)) {
            return false;
        }
        return (this.value == (((TransportPriorityImpl) other).value));
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.value;
    }

}
