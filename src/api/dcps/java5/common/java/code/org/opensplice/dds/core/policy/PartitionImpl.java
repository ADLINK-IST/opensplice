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

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class PartitionImpl extends QosPolicyImpl implements Partition {
    private static final long serialVersionUID = 3060990234546666051L;
    private final HashSet<String> name;

    public PartitionImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.name = new HashSet<String>();
        this.name.add("");
    }

    public PartitionImpl(OsplServiceEnvironment environment,
            Collection<String> name) {
        super(environment);

        if (name == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied name is invalid.");
        }
        this.name = new HashSet<String>(name);

        if(this.name.size() == 0){
            this.name.add("");
        }
    }

    public PartitionImpl(OsplServiceEnvironment environment,
            String... name) {
        super(environment);
        this.name = new HashSet<String>();

        for(String partition: name){
            this.name.add(partition);
        }
        if(this.name.size() == 0){
            this.name.add("");
        }
    }

    @Override
    public Set<String> getName() {
        return Collections.unmodifiableSet(this.name);
    }

    @Override
    public Partition withName(Collection<String> name) {
        return new PartitionImpl(this.environment, name);
    }

    @Override
    public Partition withName(String... names) {
        return new PartitionImpl(this.environment, names);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Partition.class;
    }

    @Override
    public Partition withName(String name) {
        return new PartitionImpl(this.environment, name);
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof PartitionImpl)) {
            return false;
        }
        PartitionImpl p = (PartitionImpl) other;

        if (this.name.size() != p.name.size()) {
            return false;
        }
        if (!this.name.containsAll(p.name)) {
            return false;
        }
        return true;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        for (String name : this.name) {
            result = prime * result + name.hashCode();
        }
        return result;
    }
}
