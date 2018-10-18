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
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;

/**
 * Implementation of the WriterSnapshot interface.
 * 
 * @date May 18, 2005
 */
public class WriterSnapshotImpl extends SnapshotImpl implements WriterSnapshot {
    private Writer writer;

    /**
     * Creates a new WriterSnapshot from the supplied arguments. This
     * constructor is for internal use only.
     *
     * @param _id
     *            The heap address of the snapshot.
     * @param _reader
     *            The Writer, which contents are in the snapshot.
     */
    public WriterSnapshotImpl(Communicator communicator, String _id,
            Writer _writer) {
        super(communicator, _id);
        writer = _writer;
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
            type = writer.getDataType();
        }
        return type;
    }

    /**
     * Provides access to writer.
     * 
     * @return Returns the writer.
     */
    @Override
    public Writer getWriter() {
        return writer;
    }
}
