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

import org.omg.dds.core.policy.EntityFactory;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class EntityFactoryImpl extends QosPolicyImpl implements EntityFactory {
    private static final long serialVersionUID = -775163554877034009L;
    private final boolean autoEnableCreatedEntities;

    public EntityFactoryImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.autoEnableCreatedEntities = true;
    }

    public EntityFactoryImpl(OsplServiceEnvironment environment,
            boolean autoEnableCreatedEntities) {
        super(environment);
        this.autoEnableCreatedEntities = autoEnableCreatedEntities;
    }

    @Override
    public boolean isAutoEnableCreatedEntities() {
        return this.autoEnableCreatedEntities;
    }

    @Override
    public EntityFactory withAutoEnableCreatedEntities(
            boolean autoEnableCreatedEntities) {
        return new EntityFactoryImpl(this.environment,
                autoEnableCreatedEntities);
    }

    @Override
    public String toString(){
        return "autoEnableCreatedEntities = " + 
                (this.autoEnableCreatedEntities==true?"true":"false");
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return EntityFactory.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof EntityFactoryImpl)) {
            return false;
        }
        return (this.autoEnableCreatedEntities == ((EntityFactoryImpl) other).autoEnableCreatedEntities);
    }

    @Override
    public int hashCode() {
        return 31 * 17 + (this.autoEnableCreatedEntities ? 1 : 0);
    }
}
