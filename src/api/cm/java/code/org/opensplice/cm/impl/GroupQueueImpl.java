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
import org.opensplice.cm.GroupQueue;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the GroupQueue interface.
 * 
 * @date May 18, 2005 
 */
public class GroupQueueImpl extends ReaderImpl implements GroupQueue {
    /**
     * Creates a new group queue from the supplied arguments. This constructor
     * is for internal use only.
     *
     * @param _index The index of the handle of the entity.
     * @param _serial The serial of the handle of the entity.
     * @param _pointer The heap address of the entity.
     * @param _name The name of the entity.
     */
    public GroupQueueImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }
    
    public ReaderSnapshot makeSnapshot() throws CMException{
        throw new CMException("Snapshot of GroupQueue not supported.");
    }
    
    public MetaType getDataType() throws CMException{
        throw new CMException("Retrieving content type of GroupQueue not supported");
    }
    
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }
}
