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

import org.omg.dds.core.ServiceEnvironment;
import org.opensplice.dds.core.DDSExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public abstract class QosPolicyImpl implements QosPolicy {
	private static final long serialVersionUID = -1576883735576551398L;
	protected OsplServiceEnvironment environment;

	public QosPolicyImpl(OsplServiceEnvironment environment){
		this.environment = environment;

        if (this.environment == null) {
            throw new DDSExceptionImpl(null, "Supplied Environment is invalid.");
        }
	}

	@Override
	public ServiceEnvironment getEnvironment() {
		return this.environment;
	}

    public abstract Class<? extends org.omg.dds.core.policy.QosPolicy> getPolicyClass();

    @Override
    public abstract boolean equals(Object other);

    @Override
    public abstract int hashCode();

}
