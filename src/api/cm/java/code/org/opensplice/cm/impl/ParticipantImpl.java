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
import org.opensplice.cm.Entity;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Partition;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Waitset;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.ParticipantQoS;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.statistics.Statistics;
import org.opensplice.cm.status.Status;

/**
* Implementation of the Participant interface.
*/
public class ParticipantImpl extends EntityImpl implements Participant{
    /**
     * Creates a new participant in the Splice kernel. After calling this
     * function, the user able to participate in the system.
     * @param uri The uri where to connect to.
     * @param timeout The maximum amount of time, the function may keep trying
     *                to create a participant when it fails (in milliseconds).
     * @param name The name of the participant.
     * @param qos The quality of service to apply to the Participant.
     * 
     * @throws CMException Thrown when the C&M API has not been initialised.
     */
    public ParticipantImpl(Communicator communicator, String uri, int timeout, String name, ParticipantQoS qos) throws CMException{
        super(communicator, 0, 0, "", "");
        owner = true;
        ParticipantImpl p;
        try {
            p = (ParticipantImpl)getCommunicator().participantNew(uri, timeout, name, qos);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(p == null){
            throw new CMException("Participant could not be created.");
        }
        this.index = p.index;
        this.serial = p.serial;
        this.name = p.name;
        this.pointer = p.pointer;
        this.enabled = p.enabled;
        p.freed = true;
    }
 
    /** Constructs a new Participant from the supplied arguments. This function
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
    public ParticipantImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name){
        super(communicator, _index, _serial, _pointer, _name);
    }
 
    /**
     * Resolves all topics that are available in the kernel this participant is
     * participating in.
     * 
     * @return An array of all topics in the kernel the participant is
     *         participating in.
     * @throws CMException Thrown when the participant has already been freed 
     *                     or the supplied participants kernel participant 
     *                     could not be claimed.
     */
    public Topic[] resolveAllTopics() throws CMException{
        if(freed){
            throw new CMException("Participant already freed.");
        }
        Topic[] topics;
        try {
            topics = getCommunicator().participantAllTopics(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return topics;
    }
 
    /**
     * Resolves all participants that are available in the kernel this 
     * participant is participating in.
     * 
     * @return An array of all participants in the kernel the participant is
     *         participating in.
     * @throws CMException Thrown when the participant has already been freed 
     *                     or the supplied participants kernel participant 
     *                     could not be claimed.
     */
    public Participant[] resolveAllParticipants() throws CMException{
        if(freed){
            throw new CMException("Participant already freed.");
        }
        Participant[] participants;
        try {
            participants = getCommunicator().participantAllParticipants(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return participants;
    }
 
    /**
     * Resolves all domains that are available in the kernel this participant is
     * participating in.
     * 
     * @return An array of all domains in the kernel the participant is
     *         participating in.
     * @throws CMException Thrown when the participant has already been freed 
     *                     or the supplied participants kernel participant 
     *                     could not be claimed.
     */
    public Partition[] resolveAllDomains() throws CMException{
        if(freed){
            throw new CMException("Entity already freed.");
        }
        Partition[] domains;
        try {
            domains = getCommunicator().participantAllDomains(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return domains;
    }
 
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }
 
    public void setQoS(QoS qos) throws CMException{
        if(freed){
            throw new CMException("Supplied entity is not available (anymore).");
        } else if(qos instanceof ParticipantQoS){
            try {
                getCommunicator().entitySetQoS(this, qos);
            } catch (CommunicationException ce) {
                throw new CMException(ce.getMessage());
            }
        } else {
            throw new CMException("Supplied entity requires a ParticipantQoS.");
        }
    }
 
    /**
     * Registers the supplied data type in the domain<->node combination this 
     * Participant is participating in. Once a type is registered, it is 
     * possible to create a Topic of this type. 
     * 
     * If this type is already known in the domain<->node combination, this
     * call has no effect.
     * 
     * @param type The data type that needs to be registered.
     * @throws CMException Thrown when:
     *                      - The supplied type is not a valid type.
     *                      - A type with the same name but another content then
     *                        the supplied type has already been registered
     *                        in the domain <-> node. 
     */
    public void registerType(MetaType type) throws CMException {
        try {
            getCommunicator().participantRegisterType(this, type);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        }
    }
 
    /**
     * Resolves the topics in the domain this Participant is 
     * participating in that match the supplied expression.
     * 
     * @param topicName The topic name expression that may contain wildcard 
     *                  characters (*,?).
     * @return The list of topics that match the supplied expression.
     * @throws CMException Thrown when:
     *                      - Participant is already freed.
     *                      - Connection with the node is lost.
     */
    public Topic[] findTopic(String topicName) throws CMException {
        Topic[] topics;
     
        if(freed){
            throw new CMException("Entity already freed.");
        }
        try {
            topics = getCommunicator().participantFindTopic(this, topicName);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return topics;
    }

    public Subscriber createSubscriber(String _name, SubscriberQoS qos) throws CMException {
        return new SubscriberImpl(this, _name, qos);
    }

    public Publisher createPublisher(String _name, PublisherQoS qos) throws CMException {
        return new PublisherImpl(this, _name, qos);
    }

    public Partition createPartition(String _name) throws CMException {
        return new PartitionImpl(this, _name);
        
    }

    public Topic createTopic(String _name, String typeName, String keyList, TopicQoS qos) throws CMException {
        return new TopicImpl(this, _name, typeName, keyList, qos);
    }

    public Waitset createWaitset() throws CMException {
        return new WaitsetImpl(this);
    }

	@Override
	public Statistics[] getStatistics(Entity[] entities)  throws CMException {
		
		try {
			return getCommunicator().entityGetStatistics(entities);
		} catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
	}
}
