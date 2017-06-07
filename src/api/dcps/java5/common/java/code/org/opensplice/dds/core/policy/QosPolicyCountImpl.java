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

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.QosPolicy;
import org.omg.dds.core.policy.QosPolicyCount;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class QosPolicyCountImpl implements QosPolicyCount {
    private static final long serialVersionUID = -5717335162269137308L;
    private final transient OsplServiceEnvironment environment;
    private final Class<? extends QosPolicy> policy;
    private final int count;

    public QosPolicyCountImpl(OsplServiceEnvironment environment, Class<? extends QosPolicy> policy, int count){
        this.environment = environment;
        this.policy = policy;
        this.count = count;
    }
    
    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return this.policy;
    }

    @Override
    public int getCount() {
        return this.count;
    }

}
