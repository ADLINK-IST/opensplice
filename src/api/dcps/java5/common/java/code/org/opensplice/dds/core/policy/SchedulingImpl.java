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

import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.Scheduling.ListenerScheduling;
import org.opensplice.dds.core.policy.Scheduling.WatchdogScheduling;

public class SchedulingImpl extends QosPolicyImpl implements
        ListenerScheduling, WatchdogScheduling {
    private static final long serialVersionUID = -4704717290713490662L;
    private final int priority;
    private final SchedulingKind schedulingKind;
    private final SchedulingClass schedulingClass;

    public SchedulingImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.priority = 0;
        this.schedulingKind = SchedulingKind.ABSOLUTE;
        this.schedulingClass = SchedulingClass.DEFAULT;
    }

    public SchedulingImpl(OsplServiceEnvironment environment,
            SchedulingClass schedulingClass, SchedulingKind schedulingKind,
            int priority) {
        super(environment);

        if (schedulingClass == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Invalid scheduling class supplied.");
        }
        if (schedulingKind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Invalid scheduling kind supplied.");
        }
        this.schedulingClass = schedulingClass;
        this.schedulingKind = schedulingKind;
        this.priority = priority;
    }

    @Override
    public int getPriority() {
        return this.priority;
    }

    @Override
    public SchedulingKind getKind() {
        return this.schedulingKind;
    }

    @Override
    public SchedulingClass getSchedulingClass() {
        return this.schedulingClass;
    }

    @Override
    public Scheduling withPriority(int priority) {
        return new SchedulingImpl(this.environment, this.schedulingClass,
                this.schedulingKind, priority);
    }

    @Override
    public Scheduling withKind(SchedulingKind schedulingKind) {
        return new SchedulingImpl(this.environment, this.schedulingClass,
                schedulingKind, this.priority);
    }

    @Override
    public Scheduling withSchedulingClass(SchedulingClass schedulingClass) {
        return new SchedulingImpl(this.environment, schedulingClass,
                this.schedulingKind, this.priority);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return Scheduling.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof SchedulingImpl)) {
            return false;
        }
        SchedulingImpl s = (SchedulingImpl) other;

        if (s.priority != this.priority) {
            return false;
        }
        if (s.schedulingClass != this.schedulingClass) {
            return false;
        }
        return (s.schedulingKind == this.schedulingKind);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + this.priority;
        result = prime * result + this.schedulingClass.hashCode();

        return prime * result + this.schedulingKind.hashCode();
    }

}
