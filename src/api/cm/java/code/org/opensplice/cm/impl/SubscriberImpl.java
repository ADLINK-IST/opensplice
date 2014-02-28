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
import org.opensplice.cm.DataReader;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
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
     * Constructs a new Subscriber from the supplied arguments. This function
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
    
    /**
     * Makes the Subscriber subscribe to data that is published in the 
     * Partitions that match the supplied expression. Remember that the 
     * Partitions must have been created within the Participant of this 
     * Subscriber prior to calling this function.
     * 
     * @param expression The partition expression to apply.
     * @throws CMException Thrown when Subscriber has already been freed or
     *                     when publish failed.
     */
    public void subscribe(String expression) throws CMException{
        if(this.isFreed()){
            throw new CMException("Subscriber already freed.");
        }
        try {
            getCommunicator().subscriberSubscribe(this, expression);
        } catch (CommunicationException e) {
            throw new CMException("subscribe failed for: " + expression);
        }
    }
    
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

    public DataReader createDataReader(String _name, String view, ReaderQoS qos) throws CMException {
        return new DataReaderImpl(this, _name, view, qos);
    }
}
