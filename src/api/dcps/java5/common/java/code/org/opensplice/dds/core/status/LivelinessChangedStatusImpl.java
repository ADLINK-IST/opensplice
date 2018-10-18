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
package org.opensplice.dds.core.status;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.status.LivelinessChangedStatus;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class LivelinessChangedStatusImpl extends LivelinessChangedStatus {
    private static final long serialVersionUID = -8177984046769155908L;
    private final transient OsplServiceEnvironment environment;
    private final int aliveCount;
    private final int notAliveCount;
    private final int aliveCountChange;
    private final int notAliveCountChange;
    private final InstanceHandle lastPublicationHandle;

    public LivelinessChangedStatusImpl(OsplServiceEnvironment environment,
            int aliveCount, int notAliveCount, int aliveCountChange,
            int notAliveCountChange, InstanceHandle lastPublicationHandle) {
        this.environment = environment;
        this.aliveCount = aliveCount;
        this.notAliveCount = notAliveCount;
        this.aliveCountChange = aliveCountChange;
        this.notAliveCountChange = notAliveCountChange;
        this.lastPublicationHandle = lastPublicationHandle;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public int getAliveCount() {
        return this.aliveCount;
    }

    @Override
    public int getNotAliveCount() {
        return this.notAliveCount;
    }

    @Override
    public int getAliveCountChange() {
        return this.aliveCountChange;
    }

    @Override
    public int getNotAliveCountChange() {
        return this.notAliveCountChange;
    }

    @Override
    public InstanceHandle getLastPublicationHandle() {
        return this.lastPublicationHandle;
    }

}
