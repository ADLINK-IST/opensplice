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
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
import org.opensplice.cm.ServiceState;
import org.opensplice.cm.ServiceStateKind;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the ServiceState interface.
 * 
 * @date May 18, 2005
 */
public class ServiceStateImpl extends EntityImpl implements ServiceState {
    private String stateName;
    private ServiceStateKind kind;

    /**
     * Constructs a new Service from the supplied arguments. This function is
     * for internal use only and should not be used by API users.
     * 
     * @param _index
     *            The index of the handle of the kernel entity that is
     *            associated with this entity.
     * @param _serial
     *            The serial of the handle of the kernel entity that is
     *            associated with this entity.
     * @param _pointer
     *            The address of the user layer entity that is associated with
     *            this entity.
     * @param _name
     *            The name of the kernel entity that is associated with this
     *            entity.
     * @param _stateName
     *            The name of the state.
     * @param _kind
     *            The state kind of the state.
     */
    public ServiceStateImpl(Communicator communicator, long _index,
            long _serial, String _pointer, String _name, String _stateName,
            ServiceStateKind _kind) {
        super(communicator, _index, _serial, _pointer, _name);
        stateName = _stateName;
        kind = _kind;
    }

    /**
     * Resolves the name of the state.
     * 
     * @return The name of the state.
     */
    @Override
    public String getStateName() {
        return stateName;
    }

    /**
     * Resolves the kind of the state.
     * 
     * @return The kind of the state.
     */
    @Override
    public ServiceStateKind getServiceStateKind() {
        return kind;
    }

    /**
     * Frees the entity, by deregistering it from the factory.
     */
    @Override
    public synchronized void free() {
        /* do nothing. */
    }

    @Override
    public Status getStatus() throws CMException {
        throw new CMException("Entity type has no status.");
    }
}
