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
import org.opensplice.cm.NetworkReader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the NetworkReader interface.
 * 
 * @date May 18, 2005
 */
public class NetworkReaderImpl extends ReaderImpl implements NetworkReader {
    /**
     * Constructs a new NetworkReader from the supplied arguments. This function
     * is for internal use only.
     *
     * @param _index
     *            The index of the handle of the Entity.
     * @param _serial
     *            The serial of the handle of the Entity.
     * @param _pointer
     *            The heap address of the Entity.
     * @param _name
     *            The name of the Entity.
     */
    public NetworkReaderImpl(Communicator communicator, long _index,
            long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }

    @Override
    public ReaderSnapshot makeSnapshot() throws CMException {
        throw new CMException("Snapshot of NetworkReader not supported");
    }

    @Override
    public MetaType getDataType() throws CMException {
        throw new CMException(
                "Retrieving content type of NetworkReader not supported");
    }

    @Override
    public Status getStatus() throws CMException {
        throw new CMException("Entity type has no status.");
    }
}
