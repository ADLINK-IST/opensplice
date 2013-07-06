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

import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.statistics.Statistics;
import org.opensplice.cm.status.Status;

/**
 * Represents an entity in the Splice kernel. This class has been defined
 * abstract, because only descendants of this class may actually exist.
 * 
 * The communication to Splice is done using a specific communication protocol.
 * To be able to communicate with Splice, the entity is serialized to and
 * deserialized from XML. XML represents the entity in a language independant
 * way and is therefore suitable to be transported over all kinds of 
 * communication protocols.
 *
 * The model looks like:
 @verbatim
     -------------------
     |   Java Entity   |       
     -------------------
           ^   |
           |   v
     -------------------
     |    XML entity   |
     -------------------
           ^   |
           |   v
     -------------------
     |   User entity   |
     ------------------- 
 @endverbatim
 */
public interface Entity {
    /**
     * Provides access to the entity's index.
     * 
     * @return The index of the entity.
     */
    public long getIndex();

    /**
     * Provides access to the entity's name.
     * 
     * @return The name of the entity.
     */
    public String getName();
    
    /**
     * Provides access to the entity's serial.
     * 
     * @return The serial of the entity.
     */
    public long getSerial();
    
    /**
     * Checks if this application owns this Entity or only owns a proxy.
     * 
     * @return true if the application is owned, false otherwise.
     */
    public boolean isOwner();
    
    /**
     * Provides access to the address of the user entity that is associated
     * with this entity. 
     * 
     * @return The address of the user entity.
     */
    public String getPointer();
    
    /**
     * Provides access to the status of the Entity. Not all Entity types have
     * a status. Entity types that have a status are:
     * - Subscriber
     * - Topic
     * - Partition
     * - DataReader
     * - Writer
     * 
     * @return The status of the Entity.
     * @throws CMException Thrown when:
     *                     - The Entity is not available anymore.
     *                     - The Entity has no status.
     */
    public Status getStatus() throws CMException;
    
    /**
     * Provides access to the Quality of Service of the Entity. Not all Entity
     * types have a Quality of Service. Entity types that have a Quality of
     * Service are:
     * - Participant
     * - Topic
     * - Publisher
     * - Subscriber
     * - DataReader
     * - Writer
     * 
     * @return The Quality of Service of the Entity. (QoS)
     * @throws CMException Throw when:
     *                     - The Entity is not available anymore
     *                     - The Entity has no QoS.
     */
    public QoS getQoS() throws CMException;
    
    /**
     * Applies the supplied quality of service to the Entity. Not all Entity
     * types have a Quality of Service. Entity types that have a Quality of
     * Service are:
     * - Participant
     * - Topic
     * - Publisher
     * - Subscriber
     * - DataReader
     * - Writer
     * 
     * The supplied QoS type must match with associated the entity type:
     * - Participant -> ParticipantQoS
     * - Topic -> TopicQoS
     * - Publisher -> PublisherQoS
     * - Subscriber -> SubscriberQoS
     * - DataReader -> ReaderQoS
     * - Writer -> WriterQoS 
     * 
     * @param qos The quality of service to apply to the Entity.
     * @throws CMException Thrown when:
     *                     - The Entity is not available.
     *                     - Entity type has no QoS.
     *                     - Entity type does not match QoS type.
     *                     - Supplied QoS cannot be applied to the Entity,
     *                       because (parts of) it are immutable.  
     */
    public void setQoS(QoS qos) throws CMException;
    
    /**
     * Provides access to the enabled flag. 
     * 
     * @return Whether the entity is currently enabled.
     */
    public boolean isEnabled();
    
    /**
     * Enables the entity. If the entity is already enabled, this call has
     * no effect.
     * 
     * @throws CMException Thrown when the entity could not be enabled.
     */
    public void enable() throws CMException;
        
    /**
     * Provides access to the statistics of this entity.
     * 
     * @return The statistics of this entity or null if the entity has no
     *         statistics.
     * @throws CMException Thrown when the statistics could not be resolved.
     */
    public Statistics getStatistics() throws CMException;
    
    /**
     * Resets (a part of) the statistics of this entity. 
     * 
     * @param fieldName The fieldName of the statistics field to reset or null
     *                  if the complete statistics must be resetted.
     * @throws CMException Thrown when the statistics could not be resetted.
     */
    public void resetStatistics(String fieldName) throws CMException;
    
    /**
     * Provides access to the entities that are owned by this entity and match
     * the supplied filter. 
     * 
     * Possible relations are:
     * - Participant    -> Publisher
     * - Participant    -> Subscriber
     * - Participant    -> View
     * - Publisher      -> Writer
     * - Publisher      -> Partition
     * - Subscriber     -> Reader
     * - Subscriber     -> Partition
     * - Writer         -> Topic
     * - DataReader     -> Query
     * - DataReader     -> Description
     * - View           -> Query
     * - View           -> Description
     * - Join           -> Description
     * - Queue          -> Topic
     * - Queue          -> Query
     * - Query          -> Query
     * 
     * @param filter The filter, which the owned entities must match.
     * @return An array of entities that are owned by this entity and match the
     *         supplied filter.
     * @throws CMException This exception is thrown when this entity or its
     *                     associated kernel entity has been freed.
     */
    public Entity[] getOwnedEntities(EntityFilter filter) throws CMException;
    
    /**
     * Provides access to the entities which this entity depends on.
     * 
     * Possible relations are:
     * - DataReader     -> Subscriber
     * - Queue          -> Subscriber
     * - Query          -> Reader
     * - Writer         -> Publisher
     * - View           -> Participant
     * - Publisher      -> Participant
     * - Subscriber     -> Participant
     * - Partition      -> Publisher
     * - Partition      -> Subscriber
     * - View           -> View
     * - View           -> DataReader
     * - Topic          -> View
     * - Topic          -> Reader
     * - Topic          -> Writer
     * 
     * Relations that have not been implemented, are:
     * - Join           -> Join
     * - Join           -> View
     * - Join           -> DataReader
     * - View           -> Join
     * - Topic          -> Join
     * 
     * @param filter The filter, which the owned entities must match.
     * @return An array of entities which this entity depends on and that match
     *         the supplied filter.
     * @throws CMException This exception is thrown when this entity or its
     *                     associated kernel entity has been freed.
     */
    public Entity[] getDependantEntities(EntityFilter filter) throws CMException;
    
    /**
     * Creates a String representation of the Entity which is the name
     * of the entity;
     * 
     * @return The name of the Entity.
     */
    public String toString();
    
    /**
     * Creates an extended String representation of the entity. The 
     * representation looks like:
     * 
     * "Classname : name" if name != null and like:
     * "Classname" if name == null.
     * 
     * Using this representation the toString method also creates the correct
     * String representation for descendants of this class.
     * 
     * @return The String representation of the Entity.
     */
    public String toStringExtended();
    
    /**
     * Checks whether the entity has been freed.
     * 
     * @return true if the entity has already been freed, false otherwise.
     */
    public boolean isFreed();
        
    /**
     * Frees the entity. This is done by freeing the XML entity, user entity
     * and when the user entity is owner of the kernel entity, the kernel 
     * entity.
     */
    public void free();
    
    /**
     * Returns a hash code value for the object. This method throws an
     * UnsupportedOperationException.
     */
    public int hashCode();
    
    /**
     * Checks whether the supplied object equals this object. An object is 
     * equal when it is an Entity with the same index and serial as this object.
     * 
     * @return true when equal, false otherwise.
     */
    public boolean equals(Object obj);
}
