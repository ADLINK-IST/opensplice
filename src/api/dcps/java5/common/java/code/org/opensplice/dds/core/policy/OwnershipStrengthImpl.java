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

import org.omg.dds.core.policy.OwnershipStrength;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class OwnershipStrengthImpl extends QosPolicyImpl implements
        OwnershipStrength {
    private static final long serialVersionUID = -1785002609550828172L;
    private final int value;

    public OwnershipStrengthImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.value = 0;
    }
    
    public OwnershipStrengthImpl(OsplServiceEnvironment environment, int value) {
        super(environment);
        this.value = value;
    }

    @Override
    public int getValue() {
        return this.value;
    }

    @Override
    public OwnershipStrength withValue(int value) {
        return new OwnershipStrengthImpl(this.environment, value);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return OwnershipStrength.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof OwnershipStrengthImpl)) {
            return false;
        }
        return (this.value == ((OwnershipStrengthImpl) other).value);
    }

    @Override
    public int hashCode() {
        return 17 * 31 + this.value;
    }
}
