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
import org.opensplice.cm.Snapshot;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;

/**
 * Implementation of the Snapshot interface.
 * 
 * @date May 18, 2005
 */
public abstract class SnapshotImpl implements Snapshot {
    protected String id;
    protected boolean freed;
    protected MetaType type = null;

    private final Communicator communicator;

    /**
     * Creates a new snapshot
     *
     * @param _id The heap address of the snapshot.
     */
    public SnapshotImpl(Communicator communicator, String _id){
        if(communicator == null) {
            throw new IllegalArgumentException("The communicator parameter can not be null.");
        }
        id = _id;
        freed = false;
        this.communicator = communicator;
    }

    /**
     * Provides access to id.
     * 
     * @return Returns the id.
     */
    @Override
    public String getId() {
        return id;
    }

    /**
     * Reads a Sample from the snapshot.
     * 
     * @return The read sample, or null if no Sample is available.
     * @throws CMException
     * @throws DataTypeUnsupportedException
     */
    @Override
    public Sample read() throws DataTypeUnsupportedException, CMException{
        Sample sample;

        if(freed){
            throw new CMException("Snapshot already freed.");
        }
        try {
            sample = getCommunicator().snapshotRead(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return sample;
    }

    /**
     * Takes a Sample from the snapshot.
     * 
     * @return The taken sample, or null if no Sample is available.
     * @throws CMException
     * @throws DataTypeUnsupportedException
     */
    @Override
    public Sample take() throws DataTypeUnsupportedException, CMException{
        Sample sample;

        if(freed){
            throw new CMException("Snapshot already freed.");
        }
        try {
            sample = getCommunicator().snapshotTake(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return sample;
    }

    /**
     * Frees the snapshot.
     */
    @Override
    public synchronized void free(){
        if(!freed){
            freed = true;

            try {
                getCommunicator().snapshotFree(this);
            } catch (CMException e) {
            }
            catch (CommunicationException e) {}
        }
    }

    protected Communicator getCommunicator() throws CMException {
        return communicator;
    }
}
