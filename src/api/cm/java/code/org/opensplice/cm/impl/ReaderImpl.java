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
import org.opensplice.cm.Query;
import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;

/**
 * Implementation of the Reader interface.
 * 
 * @date May 18, 2005
 */
public abstract class ReaderImpl extends EntityImpl implements Reader{
    private MetaType dataType = null;

    /**
     * Constructs a new Reader from the supplied arguments. This function is for
     * internal use only and should not be used by API users.
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
     */
    public ReaderImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }

    /**
     * Makes a snapshot of the current contents of the reader database.
     * 
     * @return The snapshot of the reader database.
     * @throws CMException
     *             Thrown when the reader is not available or when snapshot
     *             could not be created.
     */
    @Override
    public ReaderSnapshot makeSnapshot() throws CMException{
        if(freed){
            throw new CMException("Reader has already been freed.");
        }
        ReaderSnapshot snapshot;
        try {
            snapshot = getCommunicator().readerSnapshotNew(this);
        } catch (CommunicationException e) {
            throw new CMException("Snapshot of '" + this.toString() +
                    "' could not be created.");
        }
        return snapshot;
    }

    /**
     * Provides access to the userData type of the contents of the reader
     * database.
     * 
     * @return The userData type of the data in the database.
     * @throws CMException
     *             Thrown when: - Reader is already freed. - Connection with
     *             node is lost.
     * @throws DataTypeUnsupportedException
     *             Thrown when the data type of the Topic is not supported.
     */
    @Override
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException{
        if(freed){
            throw new CMException("Reader has already been freed.");
        }
        if(dataType == null){
            try {
                dataType = getCommunicator().readerGetDataType(this);
            }
            catch (CommunicationException e) {
                throw new CMException("Reader not available.");
            }
        }
        return dataType;
    }

    /**
     * Reads data from Reader database.
     * 
     * @return The read Sammple, or null of no data was available.
     * @throws CMException
     *             Thrown when: - Reader is already freed. - Connection with
     *             node is lost.
     * @throws DataTypeUnsupportedException
     *             Thrown when the data type of the Topic is not supported.
     */
    @Override
    public Sample read() throws DataTypeUnsupportedException, CMException{
        Sample result = null;

        if(!(this.isFreed())){
            try {
                result = getCommunicator().readerRead(this);
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            }
        } else {
            throw new CMException("Reader already freed.");
        }
        return result;
    }

    /**
     * Takes data from Reader database.
     * 
     * @return The taken Sammple, or null of no data was available.
     * @throws CMException
     *             Thrown when: - Reader is already freed. - Connection with
     *             node is lost.
     * @throws DataTypeUnsupportedException
     *             Thrown when the data type of the Topic is not supported.
     */
    @Override
    public Sample take() throws DataTypeUnsupportedException, CMException{
        Sample result = null;

        if(!(this.isFreed())){
            try {
                result = getCommunicator().readerTake(this);
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            }
        } else {
            throw new CMException("Reader already freed.");
        }
        return result;
    }

    /**
     * Reads data from Reader database.
     * 
     * @return The read Sammple, or null of no data was available.
     * @throws CMException
     *             Thrown when: - Reader is already freed. - Connection with
     *             node is lost.
     * @throws DataTypeUnsupportedException
     *             Thrown when the data type of the Topic is not supported.
     */
    @Override
    public Sample readNext(GID instanceHandle) throws DataTypeUnsupportedException, CMException{
        Sample result = null;

        if(!(this.isFreed())){
            try {
                result = getCommunicator().readerReadNext(this, instanceHandle);
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            }
        } else {
            throw new CMException("Reader already freed.");
        }
        return result;
    }

    @Override
    public Query createQuery(String _name, String expression) throws CMException {
        return new QueryImpl(this, _name, expression);
    }
}
