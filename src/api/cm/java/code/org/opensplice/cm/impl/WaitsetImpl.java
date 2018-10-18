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
import org.opensplice.cm.Entity;
import org.opensplice.cm.Event;
import org.opensplice.cm.Time;
import org.opensplice.cm.Waitset;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the GroupQueue interface.
 * 
 * @date May 18, 2005
 */
public class WaitsetImpl extends EntityImpl implements Waitset {
    /**
     * Creates a new group queue from the supplied arguments. This constructor
     * is for internal use only.
     *
     * @param _index The index of the handle of the entity.
     * @param _serial The serial of the handle of the entity.
     * @param _pointer The heap address of the entity.
     * @param _name The name of the entity.
     */
    public WaitsetImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }

    public WaitsetImpl(ParticipantImpl participant) throws CMException {
        super(participant.getCommunicator(), 0, 0, "", "");

        WaitsetImpl w;

        try {
            w = (WaitsetImpl)getCommunicator().waitsetNew(participant);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(w == null){
            throw new CMException("Waitset could not be created.");
        }
        this.index = w.index;
        this.serial = w.serial;
        this.name = w.name;
        this.pointer = w.pointer;
        this.enabled = w.enabled;
        w.freed = true;
    }

    @Override
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }

    @Override
    public QoS getQoS() throws CMException{
        throw new CMException("Entity type has no QoS.");
    }

    @Override
    public void attach(Entity entity) throws CMException {
        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            getCommunicator().waitsetAttach(this, entity);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return;
    }

    @Override
    public void detach(Entity entity) throws CMException {
        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            getCommunicator().waitsetDetach(this, entity);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return;
    }

    @Override
    public Entity[] _wait() throws CMException {
        Entity[] entities = null;

        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            entities = getCommunicator().waitsetWait(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return entities;
    }

    @Override
    public Entity[] timedWait(Time time) throws CMException {
        Entity[] entities = null;

        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            entities = getCommunicator().waitsetTimedWait(this, time);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return entities;
    }

    @Override
    public int getEventMask() throws CMException {
        int result = Event.UNDEFINED;

        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            result = getCommunicator().waitsetGetEventMask(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return result;
    }

    @Override
    public void setEventMask(int mask) throws CMException {
        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            getCommunicator().waitsetSetEventMask(this, mask);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }

}
