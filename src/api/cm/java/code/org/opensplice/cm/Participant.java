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
package org.opensplice.cm;

import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.statistics.Statistics;

/**
 * Represents a participant in SPLICE-DDS. This interface offers the entrance
 * the Control & Monitoring facilities. By creating a new participant, it is
 * possible to resolve information from SPLICE-DDS and modify (parts of) this 
 * information.
 */
public interface Participant extends Entity {
    /**
     * Resolves all topics that are available in the kernel this participant is
     * participating in.
     * 
     * @return An array of all topics in the kernel the participant is
     *         participating in.
     * @throws CMException hrown when: 
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Communication with SPLICE failed.
     */
    public Topic[] resolveAllTopics() throws CMException;
    
    /**
     * Resolves all participants that are available in the kernel this 
     * participant is participating in.
     * 
     * @return An array of all participants in the kernel the participant is
     *         participating in.
     * @throws CMException hrown when: 
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Communication with SPLICE failed.
     */
    public Participant[] resolveAllParticipants() throws CMException;
    
    /**
     * Resolves all domains that are available in the kernel this participant is
     * participating in.
     * 
     * @return An array of all domains in the kernel the participant is
     *         participating in.
     * @throws CMException Thrown when: 
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Communication with SPLICE failed.
     */
    public Partition[] resolveAllDomains() throws CMException;
    
    
    
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
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Communication with SPLICE failed.
     *                      - The supplied type is not a valid type.
     *                      - A type with the same name but another content then
     *                        the supplied type has already been registered
     *                        in the domain <-> node. 
     */
    public void registerType(MetaType type) throws CMException;
    
    /**
     * Resolves the topics in the domain this Participant is 
     * participating in that match the supplied expression.
     * 
     * @param topicName The topic name expression that may contain wildcard 
     *                  characters (*,?).
     * @return The list of topics that match the supplied expression.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Communication with SPLICE failed.
     */
    public Topic[] findTopic(String topicName) throws CMException;
   
    /**
     * Creates a new Subscriber for this Participant.
     * 
     * @param name The name of the Subscriber.
     * @param qos The QoS policies for the Subscriber.
     * @return The newly created Subscriber.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public Subscriber createSubscriber(String name, SubscriberQoS qos) throws CMException;
    
    /**
     * Creates a new Publisher for this Participant.
     * 
     * @param name The name of the Publisher.
     * @param qos The QoS policies for the Publisher.
     * @return The newly created Publisher.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public Publisher createPublisher(String name, PublisherQoS qos) throws CMException;
    
    /**
     * Creates a new Partition for this Participant.
     * 
     * @param name The name of the Partition.
     * @return The newly created Partition.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public Partition createPartition(String name) throws CMException;
    
    /**
     * Creates a new Topic for this Participant. The type of the Topic must be
     * known in SPLICE. This can be achieved by calling the 
     * <code>registerType</code> function of the Participant.
     * 
     * @param name The name of the Topic.
     * @param typeName The type name of the Topic.
     * @param keyList The key list of the Topic.
     * @param qos The QoS policies of the Topic.
     * @return The newly created Topic.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Participant is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     *                      - Topic type is unknown in SPLICE.
     */
    public Topic createTopic(String name, String typeName, String keyList, TopicQoS qos) throws CMException;
    
    public Waitset createWaitset() throws CMException;
    
    public Statistics[] getStatistics(Entity[] entities) throws CMException ;
}
