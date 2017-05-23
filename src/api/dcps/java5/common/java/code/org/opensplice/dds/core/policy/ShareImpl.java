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
import org.opensplice.dds.core.OsplServiceEnvironment;

public class ShareImpl extends QosPolicyImpl implements Share {
    private static final long serialVersionUID = -1731793312659549354L;
    private final String name;

    public ShareImpl(OsplServiceEnvironment environment, String name) {
        super(environment);

        if (name != null) {
            this.name = name;
        } else {
            this.name = "";
        }
    }

    public ShareImpl(OsplServiceEnvironment environment) {
        this(environment, "");
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public String getName() {
        return this.name;
    }

    @Override
    public Share withName(String name) {
        return new ShareImpl(this.environment, name);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Share.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof ShareImpl)) {
            return false;
        }
        return this.name.equals(((ShareImpl) other).name);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.name.hashCode();
    }

}
