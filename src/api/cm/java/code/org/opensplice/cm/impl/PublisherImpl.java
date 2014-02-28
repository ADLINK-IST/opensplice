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
     * Constructs a new Publisher from the supplied arguments. This function
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
    public PublisherImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }
    
    /**
     * Constructs a new Publisher from the supplied arguments. The Publisher
     * is owner by the caller of this constructor.
     * @param participant The Participant to create the Publisher in.
     * @param name The name of the Publisher.
     * @param qos The quality of service to apply to the Publisher.
     *  
     * @throws CMException Thrown when the publisher could not be created.
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
    
    /**
     * Makes the Publisher publish data in the Partitions that match the 
     * supplied expression. Remember that the Partitions must have been created 
     * within the Participant of this Publisher prior to calling this function.
     * 
     * @param expression The partition expression to apply.
     * @throws CMException Thrown when Publisher has already been freed or
     *                     when publish failed.
     */
    public void publish(String expression) throws CMException{
        if(this.isFreed()){
            throw new CMException("Publisher already freed.");
        }
        try {
            getCommunicator().publisherPublish(this, expression);
        } catch (CommunicationException e) {
            throw new CMException("publish failed for: " + expression);
        }
    }
    
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }
    
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

    public Writer createWriter(String _name, Topic topic, WriterQoS qos) throws CMException {
        return new WriterImpl(this, _name, topic, qos);
    }
}
