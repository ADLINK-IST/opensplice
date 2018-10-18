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
package org.opensplice.dds.sub;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.sub.InstanceState;
import org.omg.dds.sub.SampleState;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.Subscriber.DataState;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.UnsupportedOperationExceptionImpl;
import org.omg.dds.sub.ViewState;

public class DataStateImpl implements Subscriber.DataState {
    private OsplServiceEnvironment environment;
    private HashSet<SampleState> sampleState;
    private HashSet<ViewState> viewState;
    private HashSet<InstanceState> instanceState;

    public DataStateImpl(OsplServiceEnvironment environment,
            Collection<SampleState> sampleState,
            Collection<ViewState> viewState,
            Collection<InstanceState> instanceState) {
        this.environment = environment;
        this.sampleState = new HashSet<SampleState>(sampleState);
        this.viewState = new HashSet<ViewState>(viewState);
        this.instanceState = new HashSet<InstanceState>(instanceState);
    }

    public DataStateImpl(OsplServiceEnvironment environment) {
        this.environment = environment;
        this.sampleState = new HashSet<SampleState>();
        this.viewState = new HashSet<ViewState>();
        this.instanceState = new HashSet<InstanceState>();
    }

    public static DataStateImpl getAnyStateDataState(OsplServiceEnvironment env) {
        return (DataStateImpl) new DataStateImpl(env).withAnySampleState()
                .withAnyViewState().withAnyInstanceState();
    }

    public static SampleState getSampleStateFromOld(OsplServiceEnvironment env,
            int state) {
        switch (state) {
        case DDS.READ_SAMPLE_STATE.value:
            return SampleState.READ;
        case DDS.NOT_READ_SAMPLE_STATE.value:
            return SampleState.NOT_READ;
        default:
            throw new IllegalArgumentExceptionImpl(env, "Invalid SampleState");
        }
    }

    public static ViewState getViewStateFromOld(OsplServiceEnvironment env,
            int state) {
        switch (state) {
        case DDS.NEW_VIEW_STATE.value:
            return ViewState.NEW;
        case DDS.NOT_NEW_VIEW_STATE.value:
            return ViewState.NOT_NEW;
        default:
            throw new IllegalArgumentExceptionImpl(env, "Invalid ViewState");
        }
    }

    public static InstanceState getInstanceStateFromOld(
            OsplServiceEnvironment env, int state) {
        switch (state) {
        case DDS.ALIVE_INSTANCE_STATE.value:
            return InstanceState.ALIVE;
        case DDS.NOT_ALIVE_DISPOSED_INSTANCE_STATE.value:
            return InstanceState.NOT_ALIVE_DISPOSED;
        case DDS.NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.value:
            return InstanceState.NOT_ALIVE_NO_WRITERS;
        default:
            throw new IllegalArgumentExceptionImpl(env, "Invalid InstanceState");
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public Set<SampleState> getSampleStates() {
        return Collections.unmodifiableSet(this.sampleState);
    }

    @Override
    public Set<ViewState> getViewStates() {
        return Collections.unmodifiableSet(this.viewState);
    }

    @Override
    public Set<InstanceState> getInstanceStates() {
        return Collections.unmodifiableSet(this.instanceState);
    }

    @Override
    public DataState with(SampleState state) {
        HashSet<SampleState> s = new HashSet<SampleState>(this.sampleState);
        s.add(state);

        return new DataStateImpl(this.environment, s, this.viewState,
                this.instanceState);
    }

    @Override
    public DataState with(ViewState state) {
        HashSet<ViewState> s = new HashSet<ViewState>(this.viewState);
        s.add(state);

        return new DataStateImpl(this.environment, this.sampleState, s,
                this.instanceState);
    }

    @Override
    public DataState with(InstanceState state) {
        HashSet<InstanceState> s = new HashSet<InstanceState>(
                this.instanceState);
        s.add(state);

        return new DataStateImpl(this.environment, this.sampleState,
                this.viewState, s);
    }

    @Override
    public DataState withAnySampleState() {
        HashSet<SampleState> s = new HashSet<SampleState>(this.sampleState);

        s.add(SampleState.READ);
        s.add(SampleState.NOT_READ);

        return new DataStateImpl(this.environment, s, this.viewState,
                this.instanceState);
    }

    @Override
    public DataState withAnyViewState() {
        HashSet<ViewState> s = new HashSet<ViewState>(this.viewState);

        s.add(ViewState.NEW);
        s.add(ViewState.NOT_NEW);

        return new DataStateImpl(this.environment, this.sampleState, s,
                this.instanceState);
    }

    @Override
    public DataState withAnyInstanceState() {
        HashSet<InstanceState> s = new HashSet<InstanceState>(
                this.instanceState);

        s.add(InstanceState.ALIVE);
        s.add(InstanceState.NOT_ALIVE_DISPOSED);
        s.add(InstanceState.NOT_ALIVE_NO_WRITERS);

        return new DataStateImpl(this.environment, this.sampleState,
                this.viewState, s);
    }

    @Override
    public DataState withNotAliveInstanceStates() {
        HashSet<InstanceState> s = new HashSet<InstanceState>(
                this.instanceState);

        s.remove(InstanceState.ALIVE);
        s.add(InstanceState.NOT_ALIVE_DISPOSED);
        s.add(InstanceState.NOT_ALIVE_NO_WRITERS);

        return new DataStateImpl(this.environment, this.sampleState,
                this.viewState, s);
    }

    public int getOldSampleState() {
        int result;

        boolean read = this.sampleState.contains(SampleState.READ);
        boolean notRead = this.sampleState.contains(SampleState.NOT_READ);

        if (read && notRead) {
            result = DDS.ANY_SAMPLE_STATE.value;
        } else if (read) {
            result = DDS.READ_SAMPLE_STATE.value;
        } else if (notRead) {
            result = DDS.NOT_READ_SAMPLE_STATE.value;
        } else {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Incomplete DataState: no SampleState set.");
        }
        return result;
    }

    public void withOldSampleState(int state) {
        switch (state) {
        case DDS.ANY_INSTANCE_STATE.value:
            this.sampleState.add(SampleState.READ);
            this.sampleState.add(SampleState.NOT_READ);
            break;
        case DDS.READ_SAMPLE_STATE.value:
            this.sampleState.add(SampleState.READ);
            break;
        case DDS.NOT_READ_SAMPLE_STATE.value:
            this.sampleState.add(SampleState.NOT_READ);
            break;
        default:
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid SampleState");
        }
    }

    public int getOldViewState() {
        int result;

        boolean _new = this.viewState.contains(ViewState.NEW);
        boolean notNew = this.viewState.contains(ViewState.NOT_NEW);

        if (_new && notNew) {
            result = DDS.ANY_VIEW_STATE.value;
        } else if (_new) {
            result = DDS.NEW_VIEW_STATE.value;
        } else if (notNew) {
            result = DDS.NOT_NEW_VIEW_STATE.value;
        } else {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Incomplete DataState: no ViewState set.");
        }
        return result;
    }

    public void withOldViewState(int state) {
        switch (state) {
        case DDS.ANY_VIEW_STATE.value:
            this.viewState.add(ViewState.NEW);
            this.viewState.add(ViewState.NOT_NEW);
            break;
        case DDS.NEW_VIEW_STATE.value:
            this.viewState.add(ViewState.NEW);
            break;
        case DDS.NOT_NEW_VIEW_STATE.value:
            this.viewState.add(ViewState.NOT_NEW);
            break;
        default:
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid ViewState");
        }
    }

    public int getOldInstanceState() {
        int result = 0;

        boolean alive = this.instanceState.contains(InstanceState.ALIVE);
        boolean disposed = this.instanceState
                .contains(InstanceState.NOT_ALIVE_DISPOSED);
        boolean noWriters = this.instanceState
                .contains(InstanceState.NOT_ALIVE_NO_WRITERS);

        if (alive) {
            result |= DDS.ALIVE_INSTANCE_STATE.value;
        }
        if (disposed) {
            result |= DDS.NOT_ALIVE_DISPOSED_INSTANCE_STATE.value;
        }
        if (noWriters) {
            result |= DDS.NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.value;
        }
        
        if (alive && disposed && noWriters) {
            result = DDS.ANY_INSTANCE_STATE.value;
        }
        if (result == 0) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Incomplete DataState: no InstanceState set.");
        }
        return result;
    }

    public void withOldInstanceState(int state) {
        switch (state) {
        case DDS.ANY_INSTANCE_STATE.value:
            this.instanceState.add(InstanceState.ALIVE);
            this.instanceState.add(InstanceState.NOT_ALIVE_DISPOSED);
            this.instanceState.add(InstanceState.NOT_ALIVE_NO_WRITERS);
            break;
        case DDS.NOT_ALIVE_INSTANCE_STATE.value:
            this.instanceState.remove(InstanceState.ALIVE);
            this.instanceState.add(InstanceState.NOT_ALIVE_DISPOSED);
            this.instanceState.add(InstanceState.NOT_ALIVE_NO_WRITERS);
            break;
        case DDS.ALIVE_INSTANCE_STATE.value:
            this.instanceState.add(InstanceState.ALIVE);
            break;
        case DDS.NOT_ALIVE_DISPOSED_INSTANCE_STATE.value:
            this.instanceState.add(InstanceState.NOT_ALIVE_DISPOSED);
            break;
        case DDS.NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.value:
            this.instanceState.add(InstanceState.NOT_ALIVE_NO_WRITERS);
            break;
        default:
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid InstanceState");
        }
    }

    public static DataStateImpl any(OsplServiceEnvironment environment) {
        return (DataStateImpl) new DataStateImpl(environment)
                .withAnySampleState().withAnyViewState().withAnyInstanceState();
    }

    @SuppressWarnings("unchecked")
    @Override
    public DataStateImpl clone() {
        try {
            DataStateImpl cloned = (DataStateImpl) super.clone();

            cloned.environment = this.environment;
            cloned.instanceState = (HashSet<InstanceState>) this.instanceState
                    .clone();
            cloned.sampleState = (HashSet<SampleState>) this.sampleState
                    .clone();
            cloned.viewState = (HashSet<ViewState>) this.viewState.clone();

            return cloned;
        } catch (CloneNotSupportedException e) {
            throw new UnsupportedOperationExceptionImpl(this.environment,
                    "Cloning of DataState not supported.");
        }
    }

    @Override
    public boolean equals(Object other) {
        if (other == this) {
            return true;
        }
        if (!(other instanceof DataStateImpl)) {
            return false;
        }
        DataStateImpl d = (DataStateImpl) other;

        if (this.instanceState.size() != d.instanceState.size()) {
            return false;
        }
        if (!d.instanceState.containsAll(this.instanceState)) {
            return false;
        }
        if (this.sampleState.size() != d.sampleState.size()) {
            return false;
        }
        if (!d.sampleState.containsAll(this.sampleState)) {
            return false;
        }
        if (this.viewState.size() != d.viewState.size()) {
            return false;
        }
        if (!d.viewState.containsAll(this.viewState)) {
            return false;
        }
        return true;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        for (InstanceState s : this.instanceState) {
            result = prime * result + s.hashCode();
        }
        for (SampleState s : this.sampleState) {
            result = prime * result + s.hashCode();
        }
        for (ViewState s : this.viewState) {
            result = prime * result + s.hashCode();
        }
        return result;
    }
}
