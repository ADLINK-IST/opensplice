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
package org.opensplice.dds.core;

import java.util.Arrays;
import java.util.Collection;
import java.util.Set;

import org.omg.dds.core.Entity;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.Status;
import org.opensplice.dds.core.status.StatusConverter;

public class StatusConditionImpl<T extends Entity<?, ?>> implements
        StatusCondition<T>, Condition<DDS.StatusCondition> {
    private final DDS.StatusCondition old;
    private final T parent;
    private final OsplServiceEnvironment environment;

    public StatusConditionImpl(OsplServiceEnvironment environment,
            DDS.StatusCondition oldCondition, T parent) {
        this.old = oldCondition;
        this.parent = parent;
        this.environment = environment;
    }

    @Override
    public boolean getTriggerValue() {
        return this.old.get_trigger_value();
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public T getParent() {
        return this.parent;
    }

    @Override
    public Set<Class<? extends Status>> getEnabledStatuses() {
        return StatusConverter.convertMask(this.environment,
                this.old.get_enabled_statuses());
    }

    @Override
    public void setEnabledStatuses(Collection<Class<? extends Status>> statuses) {
        int rc = this.old.set_enabled_statuses(StatusConverter.convertMask(
                this.environment, statuses));

        Utilities.checkReturnCode(rc, this.environment,
                "StatusCondition.withEnabledStatuses() failed.");
    }

    @Override
    public void setEnabledStatuses(Class<? extends Status>... statuses) {
        this.setEnabledStatuses(Arrays.asList(statuses));

    }

    @Override
    public DDS.StatusCondition getOldCondition() {
        return this.old;
    }

    @Override
    public String toString(){
        return "StatusCondition (" + Integer.toHexString(this.hashCode()) + ")";
    }
}
