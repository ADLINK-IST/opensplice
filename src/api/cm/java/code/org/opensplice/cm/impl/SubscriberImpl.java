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
import org.opensplice.cm.DataReader;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.data.Mask;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.SubscriberQoS;

/**
 * Implementation of the Subscriber interface.
 * 
 * @date May 18, 2005
 */
public class SubscriberImpl extends EntityImpl implements Subscriber{
    /**
     * Constructs a new Subscriber from the supplied arguments. This function is
     * for internal use only and should not be used by API users.
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
    public SubscriberImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }

    /**
     * Creates a new Subscriber from the supplied arguments. The Subscriber
     * is owned by the caller of this constructor.
     * @param participant The Participant to create the Subscriber in.
     * @param name The name of the Subscriber.
     * @param qos The quality of service to apply to the Subscriber.
     *
     * @throws CMException Thrown when Subscriber could not be created.
     */
    public SubscriberImpl(ParticipantImpl participant, String name, SubscriberQoS qos) throws CMException{
        super(participant.getCommunicator(), 0, 0, "", "");
        owner = true;
        SubscriberImpl s;
        try {
            s = (SubscriberImpl)getCommunicator().subscriberNew(participant, name, qos);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(s == null){
            throw new CMException("Subscriber could not be created.");
        }
        this.index = s.index;
        this.serial = s.serial;
        this.name = s.name;
        this.pointer = s.pointer;
        this.enabled = s.enabled;
        s.freed = true;
    }

    @Override
    public void setQoS(QoS qos) throws CMException{
        if(freed){
            throw new CMException("Supplied entity is not available (anymore).");
        } else if(qos instanceof SubscriberQoS){
            try {
                getCommunicator().entitySetQoS(this, qos);
            } catch (CommunicationException ce) {
                throw new CMException(ce.getMessage());
            }
        } else {
            throw new CMException("Supplied entity requires a SubscriberQoS.");
        }
    }

    @Override
    public DataReader createDataReader(String _name, String view, ReaderQoS qos) throws CMException {
        return new DataReaderImpl(this, _name, view, qos);
    }

    @Override
    public void beginAccess() throws CMException {
        if (freed) {
            throw new CMException(
                    "Supplied subscriber is not available (anymore).");
        }
        try {
            getCommunicator().beginAccess(this);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        }
    }

    @Override
    public void endAccess() throws CMException {
        if (freed) {
            throw new CMException(
                    "Supplied subscriber is not available (anymore).");
        }
        try {
            getCommunicator().endAccess(this);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        }
    }

    @Override
    public DataReader[] getDataReaders() throws CMException {
        return this.getDataReaders(Mask.ANY);
    }

    @Override
    public DataReader[] getDataReaders(Mask mask) throws CMException {
        if (freed) {
            throw new CMException(
                    "Supplied subscriber is not available (anymore).");
        }
        try {
            return getCommunicator().subscriberGetDataReaders(this, mask);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        }
    }
}
