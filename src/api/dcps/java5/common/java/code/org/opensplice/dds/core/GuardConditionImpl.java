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

import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.ServiceEnvironment;

public class GuardConditionImpl extends GuardCondition implements org.opensplice.dds.core.Condition<DDS.GuardCondition> {
    private final OsplServiceEnvironment environment;
    private final DDS.GuardCondition oldGuardCondition;


    public GuardConditionImpl(OsplServiceEnvironment environment){
        this.environment = environment;
        this.oldGuardCondition = new DDS.GuardCondition();
    }

    @Override
    public boolean getTriggerValue() {
        return this.oldGuardCondition.get_trigger_value();
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public void setTriggerValue(boolean value) {
        this.oldGuardCondition.set_trigger_value(value);
    }

    @Override
    public DDS.GuardCondition getOldCondition() {
        return this.oldGuardCondition;
    }

    @Override
    public String toString() {
        return "GuardCondition (" + Integer.toHexString(hashCode()) + ")";
    }
}
