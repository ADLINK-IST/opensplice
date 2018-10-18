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

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class ViewKeysImpl extends QosPolicyImpl implements ViewKeys {
    private static final long serialVersionUID = -4786321587154092502L;
    private final HashSet<String> keyList;

    public ViewKeysImpl(OsplServiceEnvironment environment, String... keyValue) {
        super(environment);
        this.keyList = new HashSet<String>();

        for (String k : keyValue) {
            this.keyList.add(k);
        }
    }

    public ViewKeysImpl(OsplServiceEnvironment environment,
            Collection<String> keyValue) {
        super(environment);
        this.keyList = new HashSet<String>();

        if (keyValue != null) {
            for (String k : keyValue) {
                this.keyList.add(k);
            }
        }
    }

    @Override
    public Set<String> getKey() {
        return Collections.unmodifiableSet(this.keyList);
    }

    @Override
    public ViewKeys withKey(Collection<String> keyList) {
        return new ViewKeysImpl(this.environment, keyList);
    }

    @Override
    public ViewKeys withKey(String keyList) {
        return new ViewKeysImpl(this.environment, keyList);
    }

    @Override
    public ViewKeys withKey(String... keyList) {
        return new ViewKeysImpl(this.environment, keyList);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return ViewKeys.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof ViewKeysImpl)) {
            return false;
        }
        ViewKeysImpl v = (ViewKeysImpl) other;

        if (v.keyList.size() != this.keyList.size()) {
            return false;
        }
        return v.keyList.containsAll(this.keyList);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        for (String key : this.keyList) {
            result = prime * result + key.hashCode();
        }
        return result;
    }
}
