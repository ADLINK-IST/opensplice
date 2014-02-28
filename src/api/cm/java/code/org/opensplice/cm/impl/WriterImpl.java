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
import org.opensplice.cm.CMFactory;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.WriterQoS;

/**
 * Implementation of the Writer interface.
 * 
 * @date May 18, 2005 
 */
public class WriterImpl extends EntityImpl implements Writer{
    private MetaType dataType = null;
    /** 
     * Constructs a new Domain from the supplied arguments. This function
     * is for internal use only and should not be used by API users.
     * 
     * @param _index The index of the handle of the kernel entity that is
     *               associated with this entity.
     * @param _serial The serial of the handle of the kernel entity that is
     *                associated with this entity.
     * @param _pointer The address of the user layer entity that is associated
     *                 with this entity.
     * @param _name The name of the kernel entity that is associated with this
     *              entity.
     */
    public WriterImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }
    
    /**
     * Creates a new Writer from the supplied arguments. The Writer is owned by
     * the caller of this constructor.
     * @param publisher The Publisher to attach the Writer to. 
     * @param name The name of the Writer.
     * @param topic The Topic that the Writer needs to write.
     * @param qos The quality of service to apply to the Writer.
     *
     * @throws CMException Thrown when the Writer could not be created.
     */
    public WriterImpl(PublisherImpl publisher, String name, Topic topic, WriterQoS qos) throws CMException{
        super(publisher.getCommunicator(), 0, 0, "", "");
        owner = true;
        WriterImpl w;
        
        try {
            w = (WriterImpl)getCommunicator().writerNew(publisher, name, topic, qos);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(w == null){
            throw new CMException("Writer could not be created.");
        }
        this.index = w.index;
        this.serial = w.serial;
        this.name = w.name;
        this.pointer = w.pointer;
        this.enabled = w.enabled;
        w.freed = true;
    }
    
    /**
     * Makes a snapshot of the current contents of the writer history.
     * 
     * @return The snapshot of the writer history.
     * @throws CMException Thrown when the writer is not available or when 
     *                     snapshot could not be created.
     */
    @Override
    public WriterSnapshot makeSnapshot() throws CMException{
        if(freed){
            throw new CMException("Reader has already been freed.");
        }
        WriterSnapshot snapshot;
        try {
            snapshot = getCommunicator().writerSnapshotNew(this);
        } catch (CommunicationException e) {
            throw new CMException("Snapshot of '" + this.toString() + 
                                        "' could not be created.");
        }
        return snapshot;
    }
    
    /**
     * Writes the supplied data in the Splice system.
     * 
     * @param data The data to write.
     * @throws CMException Thrown when the writer is not available anymore.
     */
    @Override
    public void write(UserData data) throws CMException{
        if(freed){
            throw new CMException("Writer has already been freed.");
        }
        try {
            getCommunicator().writerWrite(this, data);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }
    
    /**
     * Disposes the supplied data in the Splice system.
     * 
     * @param data The data to dispose.
     * @throws CMException Thrown when the writer is not available anymore.
     */
    @Override
    public void dispose(UserData data) throws CMException{
        if(freed){
            throw new CMException("Writer has already been freed.");
        }
        if (data.getUserData().isEmpty()) {
            throw new CMException("No instance to dispose");
        }
        try {
            getCommunicator().writerDispose(this, data);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }
    
    /**
     * WriteDisposes the supplied data in the Splice system.
     * 
     * @param data The data to writeDispose.
     * @throws CMException Thrown when the writer is not available anymore.
     */
    @Override
    public void writeDispose(UserData data) throws CMException{
        if(freed){
            throw new CMException("Writer has already been freed.");
        }
        try {
            getCommunicator().writerWriteDispose(this, data);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }
    
    /**
     * Registers the supplied data as instance in the Splice system.
     * 
     * @param data The data to register.
     * @throws CMException Thrown when the writer is not available anymore.
     */
    @Override
    public void register(UserData data) throws CMException{
        if(freed){
            throw new CMException("Writer has already been freed.");
        }
        try {
            getCommunicator().writerRegister(this, data);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }
    
    /**
     * Unregisters the supplied data as instance in the Splice system.
     * 
     * @param data The data to unregister.
     * @throws CMException Thrown when the writer is not available anymore.
     */
    @Override
    public void unregister(UserData data) throws CMException{
        if(freed){
            throw new CMException("Writer has already been freed.");
        }
        try {
            getCommunicator().writerUnregister(this, data);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }
    
    /**
     * Provides access to the userData type of the data the Writer writes.
     * 
     * @return The userData type of the data that is written by the Writer.
     * @throws CMException Thrown when:
     *                      - Writer is already freed.
     *                      - Connection with node is lost.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported.
     */
    @Override
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException{
        if(freed){
            throw new CMException("Writer has already been freed.");
        }
        if(dataType == null){
            try {
                dataType = getCommunicator().writerGetDataType(this);
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            }
        }
        return dataType;
    }
    
    @Override
    public void setQoS(QoS qos) throws CMException{
        if(freed){
            throw new CMException("Supplied entity is not available (anymore).");
        } else if(qos instanceof WriterQoS){
            try {
                getCommunicator().entitySetQoS(this, qos);
            } catch (CommunicationException ce) {
                throw new CMException(ce.getMessage());
            }
        } else {
            throw new CMException("Supplied entity requires a WriterQoS.");
        }
    }
}
