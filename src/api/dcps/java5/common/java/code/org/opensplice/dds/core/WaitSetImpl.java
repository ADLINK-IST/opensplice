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

import java.util.Collection;
import java.util.Collections;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Condition;
import org.omg.dds.core.Duration;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.WaitSet;

public class WaitSetImpl extends WaitSet {
    private final OsplServiceEnvironment environment;
    private final DDS.WaitSet oldWaitSet;
    private ConcurrentHashMap<DDS.Condition, org.omg.dds.core.Condition> conditions;

    public WaitSetImpl(OsplServiceEnvironment environment) {
        this.environment = environment;
        this.oldWaitSet = new DDS.WaitSet();
        this.conditions = new ConcurrentHashMap<DDS.Condition, org.omg.dds.core.Condition>();
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public void waitForConditions() {
        DDS.ConditionSeqHolder holder = new DDS.ConditionSeqHolder();
        int rc = this.oldWaitSet._wait(holder, DDS.DURATION_INFINITE.value);

        Utilities.checkReturnCode(rc, this.environment,
                "Waitset.waitForConditions() failed.");

        return;
    }

    @Override
    public void waitForConditions(Collection<Condition> activeConditions) {
        if (activeConditions == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Collection<Condition> (null) provided.");
        }

        DDS.ConditionSeqHolder holder = new DDS.ConditionSeqHolder();
        int rc = this.oldWaitSet._wait(holder, DDS.DURATION_INFINITE.value);

        Utilities.checkReturnCode(rc, this.environment,
                "Waitset.waitForConditions() failed.");

        activeConditions.clear();

        for (DDS.Condition cond : holder.value) {
            activeConditions.add(this.conditions.get(cond));
        }
    }

    @Override
    public void waitForConditions(Duration timeout) throws TimeoutException {
        if (timeout == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Duration (null) provided.");
        }
        DDS.ConditionSeqHolder holder = new DDS.ConditionSeqHolder();
        DDS.Duration_t oldTimeout = Utilities
                .convert(this.environment, timeout);

        int rc = this.oldWaitSet._wait(holder, oldTimeout);

        Utilities.checkReturnCodeWithTimeout(rc, this.environment,
                "Waitset.waitForConditions() failed.");
    }

    @Override
    public void waitForConditions(long timeout, TimeUnit unit)
            throws TimeoutException {
        this.waitForConditions(this.environment.getSPI().newDuration(timeout,
                unit));
    }

    @Override
    public void waitForConditions(Collection<Condition> activeConditions,
            Duration timeout) throws TimeoutException {

        if (activeConditions == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Collection<Condition> (null) provided.");

        }
        DDS.ConditionSeqHolder holder = new DDS.ConditionSeqHolder();
        DDS.Duration_t oldTimeout = Utilities
                .convert(this.environment, timeout);

        int rc = this.oldWaitSet._wait(holder, oldTimeout);

        Utilities.checkReturnCode(rc, this.environment,
                "Waitset.waitForConditions() failed.");

        activeConditions.clear();

        for (DDS.Condition cond : holder.value) {
            activeConditions.add(this.conditions.get(cond));
        }

    }

    @Override
    public void waitForConditions(Collection<Condition> activeConditions,
            long timeout, TimeUnit unit) throws TimeoutException {
        this.waitForConditions(activeConditions, this.environment.getSPI()
                .newDuration(timeout, unit));

    }

    @Override
    public void attachCondition(Condition cond) {
        org.opensplice.dds.core.Condition<?> c;

        if (cond == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Condition (null) provided.");
        }
        try {
            c = (org.opensplice.dds.core.Condition<?>) cond;
        } catch (ClassCastException cce) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Attaching non-OpenSplice Condition implementation is not supported.");
        }
        DDS.Condition old = c.getOldCondition();
        int rc = this.oldWaitSet.attach_condition(old);
        Utilities.checkReturnCode(rc, this.environment,
                "Attaching condition failed.");

        this.conditions.put(old, cond);

    }

    @Override
    public void detachCondition(Condition cond) {
        org.opensplice.dds.core.Condition<?> c;

        if (cond == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal Condition (null) provided.");
        }
        try {
            c = (org.opensplice.dds.core.Condition<?>) cond;
        } catch (ClassCastException cce) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Detaching non-OpenSplice Condition implementation is not supported.");
        }
        DDS.Condition old = c.getOldCondition();
        int rc = this.oldWaitSet.detach_condition(old);
        Utilities.checkReturnCode(rc, this.environment,
                "Detaching condition failed.");

        this.conditions.remove(old);

    }

    @Override
    public Collection<Condition> getConditions() {
        return Collections.unmodifiableCollection(conditions.values());
    }

    @Override
    public String toString() {
        return "WaitSet (" + Integer.toHexString(this.hashCode()) + ")";
    }
}
