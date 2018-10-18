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
package org.opensplice.dds.core.status;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.QosPolicyCount;
import org.omg.dds.core.status.RequestedIncompatibleQosStatus;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.QosPolicyCountImpl;

public class RequestedIncompatibleQosStatusImpl extends
        RequestedIncompatibleQosStatus {
    private static final long serialVersionUID = 8046180416846302298L;
    private final transient OsplServiceEnvironment environment;
    private final int totalCount;
    private final int totalCountChange;
    private final Class<? extends QosPolicy> lastPolicyClass;
    private final List<QosPolicyCount> policies;

    public RequestedIncompatibleQosStatusImpl(
            OsplServiceEnvironment environment, int totalCount,
            int totalCountChange, Class<? extends QosPolicy> lastPolicyClass,
            QosPolicyCount... policies) {
        this.environment = environment;
        this.totalCount = totalCount;
        this.totalCountChange = totalCountChange;
        this.lastPolicyClass = lastPolicyClass;
        this.policies = Collections
                .synchronizedList(new ArrayList<QosPolicyCount>());

        for (QosPolicyCount p : policies) {
            this.policies.add(p);
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public int getTotalCount() {
        return this.totalCount;
    }

    @Override
    public int getTotalCountChange() {
        return this.totalCountChange;
    }

    @Override
    public Class<? extends QosPolicy> getLastPolicyClass() {
        return this.lastPolicyClass;
    }

    @Override
    public Set<QosPolicyCount> getPolicies() {
        Set<QosPolicyCount> policyCount = new HashSet<QosPolicyCount>();

        for (QosPolicyCount qp : this.policies) {
            policyCount.add(new QosPolicyCountImpl(this.environment, qp
                    .getPolicyClass(), qp.getCount()));
        }
        return policyCount;
    }

}
