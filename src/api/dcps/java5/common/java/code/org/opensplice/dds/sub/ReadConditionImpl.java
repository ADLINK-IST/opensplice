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
package org.opensplice.dds.sub;

import java.util.Collection;
import java.util.Set;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.InstanceState;
import org.omg.dds.sub.ReadCondition;
import org.omg.dds.sub.SampleState;
import org.omg.dds.sub.ViewState;
import org.opensplice.dds.core.Condition;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.Utilities;

public class ReadConditionImpl<TYPE> implements ReadCondition<TYPE>,
        Condition<DDS.ReadCondition> {
    protected final OsplServiceEnvironment environment;
    protected DDS.ReadCondition old;
    protected final AbstractDataReader<TYPE> parent;
    protected final DataStateImpl state;

    public ReadConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent,
            Collection<SampleState> sampleState,
            Collection<ViewState> viewState,
            Collection<InstanceState> instanceState) {
        this(environment, parent, new DataStateImpl(environment, sampleState,
                viewState, instanceState), true);
    }

    public ReadConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent, DataStateImpl state) {
        this(environment, parent, state, true);
    }

    public ReadConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent) {
        this(environment, parent, DataStateImpl.any(environment), true);
    }

    public ReadConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent, boolean setupCondition) {
        this(environment, parent, DataStateImpl.any(environment),
                setupCondition);
    }

    public ReadConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent, DataStateImpl state,
            boolean setupCondition) {
        this.environment = environment;
        this.parent = parent;
        this.state = state;

        if (state == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Illegal DataState (null) provided.");
        }

        if (setupCondition) {
            this.old = parent.getOld().create_readcondition(
                    this.state.getOldSampleState(),
                    this.state.getOldViewState(),
                    this.state.getOldInstanceState());

            if (this.old == null) {
                Utilities.throwLastErrorException(this.environment);
            }
        }
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
    public Set<SampleState> getSampleStates() {
        return this.state.getSampleStates();
    }

    @Override
    public Set<ViewState> getViewStates() {
        return this.state.getViewStates();
    }

    @Override
    public Set<InstanceState> getInstanceStates() {
        return this.state.getInstanceStates();
    }

    @Override
    public DataReader<TYPE> getParent() {
        return this.parent;
    }

    public DDS.ReadCondition getOld() {
        return this.old;
    }

    @Override
    public void close() {
        this.parent.destroyReadCondition(this);
    }
    
    public DataStateImpl getState(){
        return this.state.clone();
    }

    @Override
    public DDS.ReadCondition getOldCondition() {
        return this.old;
    }

}
