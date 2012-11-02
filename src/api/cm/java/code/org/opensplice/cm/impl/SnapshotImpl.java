/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
import org.opensplice.cm.Snapshot;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;
import org.opensplice.cm.meta.MetaType;

/**
 * Implementation of the Snapshot interface.
 * 
 * @date May 18, 2005 
 */
public abstract class SnapshotImpl implements Snapshot{
    protected String id;
    protected boolean freed;
    protected MetaType type = null;
    
    /**
     * Creates a new snapshot
     *
     * @param _id The heap address of the snapshot.
     */
    public SnapshotImpl(String _id){
        id = _id;
        freed = false;
    }
    
    /**
     * Provides access to id.
     * 
     * @return Returns the id.
     */
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
    public Sample read() throws DataTypeUnsupportedException, CMException{
        Sample sample;
        State state;
        int stateValue;
        
        if(freed){
            throw new CMException("Snapshot already freed.");
        }
        try {
            sample = CMFactory.getCommunicator().snapshotRead(this);
            
            if(sample != null){
                state = sample.getState();
                stateValue = state.getValue();
                
                if(state.test(State.VALIDDATA)){
                    state.setValue(stateValue - State.VALIDDATA);
                } else {
                    state.setValue(stateValue + State.VALIDDATA);
                }
            }
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
    public Sample take() throws DataTypeUnsupportedException, CMException{
        Sample sample;
        State state;
        int stateValue;
        
        if(freed){
            throw new CMException("Snapshot already freed.");
        }
        try {
            sample = CMFactory.getCommunicator().snapshotTake(this);
            
            if(sample != null){
                state = sample.getState();
                stateValue = state.getValue();
                
                if(state.test(State.VALIDDATA)){
                    state.setValue(stateValue - State.VALIDDATA);
                } else {
                    state.setValue(stateValue + State.VALIDDATA);
                }
            }
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return sample;
    }
    
    /**
     * Frees the snapshot.
     */
    public synchronized void free(){
        if(!freed){
            freed = true;
            
            try {
                CMFactory.getCommunicator().snapshotFree(this);
            } 
            catch (CMException e) {} 
            catch (CommunicationException e) {}
        }
    }
}
