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
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;

/**
 * Implementation of the ReaderSnapshot interface.
 * 
 * @date May 18, 2005
 */
public class ReaderSnapshotImpl extends SnapshotImpl implements ReaderSnapshot {
    /**
     * The reader, which contents were inserted in the snapshot.
     */
    private Reader reader;

    /**
     * Creates a new ReaderSnapshot from the supplied arguments. This
     * constructor is for internal use only.
     *
     * @param _id
     *            The heap address of the snapshot.
     * @param _reader
     *            The Reader, which contents are in the snapshot.
     */
    public ReaderSnapshotImpl(Communicator communicator, String _id,
            Reader _reader) {
        super(communicator, _id);
        reader = _reader;
    }

    /**
     * Provides access to userDataType.
     * 
     * @return Returns the userDataType.
     * @throws CMException
     * @throws DataTypeUnsupportedException
     */
    @Override
    public MetaType getUserDataType() throws DataTypeUnsupportedException,
            CMException {
        if (type == null) {
            type = reader.getDataType();
        }
        return type;
    }

    /**
     * Provides access to reader.
     * 
     * @return Returns the reader.
     */
    @Override
    public Reader getReader() {
        return reader;
    }
}
