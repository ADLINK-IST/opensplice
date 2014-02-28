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
import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;

/**
 * Implementation of the ReaderSnapshot interface.
 * 
 * @date May 18, 2005 
 */
public class ReaderSnapshotImpl extends SnapshotImpl implements ReaderSnapshot{
    /**
     * The reader, which contents were inserted in the snapshot.
     */
    private Reader reader;
    
    /**
     * Creates a new ReaderSnapshot from the supplied arguments. This
     * constructor is for internal use only.  
     *
     * @param _id The heap address of the snapshot.
     * @param _reader The Reader, which contents are in the snapshot.
     */
    public ReaderSnapshotImpl(Communicator communicator, String _id, Reader _reader){
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
    public MetaType getUserDataType() throws DataTypeUnsupportedException, CMException{
        if(type == null){
            type = reader.getDataType();
        }
        return type;
    }
    
    /**
     * Provides access to reader.
     * 
     * @return Returns the reader.
     */
    public Reader getReader() {
        return reader;
    }
}
