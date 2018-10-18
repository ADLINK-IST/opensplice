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
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.WriterQoS;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the Publisher interface.
 * 
 * @date May 18, 2005
 */
public class PublisherImpl extends EntityImpl implements Publisher{
    /**
     * Constructs a new Publisher from the supplied arguments. This function is
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
    public PublisherImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }

    /**
     * Constructs a new Publisher from the supplied arguments. The Publisher is
     * owner by the caller of this constructor.
     * 
     * @param participant
     *            The Participant to create the Publisher in.
     * @param name
     *            The name of the Publisher.
     * @param qos
     *            The quality of service to apply to the Publisher.
     * 
     * @throws CMException
     *             Thrown when the publisher could not be created.
     */
    public PublisherImpl(ParticipantImpl participant, String name, PublisherQoS qos) throws CMException{
        super(participant.getCommunicator(), 0, 0, "", "");
        owner = true;
        PublisherImpl p;
        try {
            p = (PublisherImpl)getCommunicator().publisherNew(participant, name, qos);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(p == null){
            throw new CMException("Publisher could not be created.");
        }
        this.index = p.index;
        this.serial = p.serial;
        this.name = p.name;
        this.pointer = p.pointer;
        this.enabled = p.enabled;
        p.freed = true;
    }

    @Override
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }

    @Override
    public void setQoS(QoS qos) throws CMException{
        if(freed){
            throw new CMException("Supplied entity is not available (anymore).");
        } else if(qos instanceof PublisherQoS){
            try {
                getCommunicator().entitySetQoS(this, qos);
            } catch (CommunicationException ce) {
                throw new CMException(ce.getMessage());
            }
        } else {
            throw new CMException("Supplied entity requires a PublisherQoS.");
        }
    }

    @Override
    public Writer createWriter(String _name, Topic topic, WriterQoS qos) throws CMException {
        return new WriterImpl(this, _name, topic, qos);
    }

    @Override
    public void beginCoherentChanges() throws CMException {
        if (freed) {
            throw new CMException(
                    "Supplied publisher is not available (anymore).");
        }
        try {
            getCommunicator().beginCoherentChanges(this);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        }
    }

    @Override
    public void endCoherentChanges() throws CMException {
        if (freed) {
            throw new CMException(
                    "Supplied publisher is not available (anymore).");
        }
        try {
            getCommunicator().endCoherentChanges(this);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        }
    }
}
