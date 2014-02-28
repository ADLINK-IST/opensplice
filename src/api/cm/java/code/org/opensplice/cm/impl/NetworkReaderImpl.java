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
     * Constructs a new NetworkReader from the supplied arguments. This
     * function is for internal use only.
     *
     * @param _index The index of the handle of the Entity.
     * @param _serial The serial of the handle of the Entity.
     * @param _pointer The heap address of the Entity.
     * @param _name The name of the Entity.
     */
    public NetworkReaderImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }

    public ReaderSnapshot makeSnapshot() throws CMException{
        throw new CMException("Snapshot of NetworkReader not supported");
    }
    
    public MetaType getDataType() throws CMException{
        throw new CMException("Retrieving content type of NetworkReader not supported");
    }
    
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }
}
