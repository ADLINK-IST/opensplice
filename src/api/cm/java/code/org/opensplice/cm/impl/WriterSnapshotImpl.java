/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
     * @param _id The heap address of the snapshot.
     * @param _reader The Writer, which contents are in the snapshot.
     */
    public WriterSnapshotImpl(Communicator communicator, String _id, Writer _writer) {
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
    public MetaType getUserDataType() throws DataTypeUnsupportedException, CMException {
        if(type == null){
            type = writer.getDataType();
        }
        return type;
    }

    /**
     * Provides access to writer.
     * 
     * @return Returns the writer.
     */
    public Writer getWriter() {
        return writer;
    }
}
