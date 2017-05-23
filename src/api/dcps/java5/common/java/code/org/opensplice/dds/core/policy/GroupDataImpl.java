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

import java.util.Arrays;

import org.omg.dds.core.policy.GroupData;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class GroupDataImpl extends QosPolicyImpl implements GroupData {
    private static final long serialVersionUID = -5108639314694283781L;
    private final byte[] value;

    public GroupDataImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.value = new byte[0];
    }

    public GroupDataImpl(OsplServiceEnvironment environment, byte[] value) {
        super(environment);

        if(value != null){
            this.value = value.clone();
        } else {
            this.value = new byte[0];
        }
    }

    @Override
    public byte[] getValue() {
        if (this.value == null) {
            return null;
        }
        return this.value.clone();
    }

    @Override
    public GroupData withValue(byte[] value, int offset, int length) {
        return new GroupDataImpl(this.environment, value);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return GroupData.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof GroupDataImpl)) {
            return false;
        }
        return Arrays.equals(this.value, ((GroupDataImpl) other).value);
    }

    @Override
    public int hashCode(){
        final int prime = 31;
        int result = 17;

        for (byte b : this.value) {
            result = prime * result + b;
        }
        return result;
    }
}
