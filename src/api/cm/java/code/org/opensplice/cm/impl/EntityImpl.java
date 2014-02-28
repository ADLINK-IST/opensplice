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
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.statistics.Statistics;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the Entity interface.
 */
public abstract class EntityImpl implements Entity, Comparable{
 /**
  * Creates a new entity and registers it with the factory. This function
  * is for internal use only.
  * @param _index The index of the handle of the kernel entity that is 
  *               associated with this entity.
  * @param _serial The index of the handle of the kernel entity that is
  *                associated with this entity.
  * @param _pointer The address of the user layer entity that is associated
  *                 with this entity.
  * @param _name The name of the kernel entity that is associated with this
  *              entity.
  * @param _enabled Whether the entity is enabled.
  */
 protected EntityImpl(Communicator _communicator, long _index, long _serial, String _pointer, String _name){
	 if(_communicator == null) {
		 throw new IllegalArgumentException("The communicator can not be null.");
	 }
	 
	 communicator = _communicator;
   index   = _index;
     serial  = _serial;
     pointer = _pointer;
     name    = _name;
     freed   = false;
     owner   = false;
 }
 
 /**
  * Provides access to the entity's index.
  * 
  * @return The index of the entity.
  */
 public long getIndex() {
     return index;
 }

 /**
  * Provides access to the entity's name.
  * 
  * @return The name of the entity.
  */
 public String getName() {
     return name;
 }
 
 /**
  * Provides access to the entity's serial.
  * 
  * @return The serial of the entity.
  */
 public long getSerial() {
     return serial;
 }
 
 /**
  * Checks if this application owns this Entity or only owns a proxy.
  * 
  * @return true if the application is owned, false otherwise.
  */
 public boolean isOwner(){
     return owner;
 }
 
 /**
  * Provides access to the address of the user entity that is associated
  * with this entity. 
  * 
  * @return The address of the user entity.
  */
 public String getPointer(){
     return pointer;
 }
 
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
 public Status getStatus() throws CMException{
     Status status;
     
     if(freed){
         throw new CMException("Supplied entity is not available (anymore).");
     }
     try {
         status = getCommunicator().entityGetStatus(this);
     } catch (CommunicationException e) {
         throw new CMException(e.getMessage());
     }
     return status;
 }
 
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
 public QoS getQoS() throws CMException{
     QoS qoS;
     
     if(freed){
         throw new CMException("Supplied entity is not available (anymore).");
     }
     try {
         qoS = getCommunicator().entityGetQoS(this);
     } catch (CommunicationException e) {
         throw new CMException(e.getMessage());
     }
     return qoS;
 }
 
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
 public void setQoS(QoS qos) throws CMException{
     throw new CMException("Entity type has no QoS.");
 }
 
 /**
  * Provides access to the enabled flag. 
  * 
  * @return Whether the entity is currently enabled.
  */
 public boolean isEnabled(){
     return this.enabled;
 }
 
 /**
  * Enables the entity. If the entity is already enabled, this call has
  * no effect.
  * 
  * @throws CMException Thrown when the entity could not be enabled.
  */
 public void enable() throws CMException {
     if(freed){
         throw new CMException("Entity already freed.");
     }
     try {
         getCommunicator().entityEnable(this);
         this.enabled = true;
     } catch (CommunicationException ce) {
         throw new CMException(ce.getMessage());
     }
 }
 
 /**
  * Sets the enabled flag to the desired value, but does not actually enable
  * or disable the entity.
  * 
  * @param enabled Whether or not the entity is enabled.
  */
 public void setEnabled(boolean enabled){
     this.enabled = enabled;
 }
 
 /**
  * Provides access to the statistics of this entity.
  * 
  * @return The statistics of this entity or null if the entity has no
  *         statistics.
  * @throws CMException Thrown when the statistics could not be resolved.
  */
 public Statistics getStatistics() throws CMException{
     Statistics s = null;
     
     if(freed){
         throw new CMException("Supplied entity is not available (anymore).");
     }
     try {
         s = getCommunicator().entityGetStatistics(this);
     } catch(CommunicationException ce){
         throw new CMException(ce.getMessage());
     }
     return s;
 }
 
 /**
  * Resets (a part of) the statistics of this entity. 
  * 
  * @param fieldName The fieldName of the statistics field to reset or null
  *                  if the complete statistics must be resetted.
  * @throws CMException Thrown when the statistics could not be resetted.
  */
 public void resetStatistics(String fieldName) throws CMException{
     if(freed){
         throw new CMException("Supplied entity is not available (anymore).");
     }
     try {
         getCommunicator().entityResetStatistics(this, fieldName);
     } catch (CommunicationException e) {
         throw new CMException(e.getMessage());
     }
 }
 
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
 public Entity[] getOwnedEntities(EntityFilter filter) throws CMException{
     if(freed){
         throw new CMException("Entity already freed.");
     }
     Entity[] result;
     try {
         result = getCommunicator().entityOwnedEntities(this, filter);
     } catch (CommunicationException e) {
         throw new CMException(e.getMessage());
     }
     return result;
 }
 
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
 public Entity[] getDependantEntities(EntityFilter filter) throws CMException{
     Entity[] result;
     
     if(freed){
         throw new CMException("Entity already freed.");
     }
     try {
         result = getCommunicator().entityDependantEntities(this, filter);
     } catch (CommunicationException e) {
         throw new CMException(e.getMessage());
     }
     return result;
 }
 
 /**
  * Creates a String representation of the Entity which is the name
  * of the entity;
  * 
  * @return The name of the Entity.
  */
 public String toString(){
     return name;
 }
 
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
 public String toStringExtended(){
     String result = null;
     result = this.getClass().getName();
     String pckg = this.getClass().getPackage().getName();
     result = result.substring(pckg.length()+1, result.length()-4);
     
     if(name != null){
         result += ": " + name;
     }
     
     return result;
 }
 
 /**
  * Checks whether the entity has been freed.
  * 
  * @return true if the entity has already been freed, false otherwise.
  */
 public boolean isFreed(){
     return freed;
 }
     
 /**
  * Frees the entity. This is done by freeing the XML entity, user entity
  * and when the user entity is owner of the kernel entity, the kernel 
  * entity.
  */
 public synchronized void free(){
     if(!freed){
         freed = true;
         
         try {
             getCommunicator().entityFree(this);
         } 
         catch (CMException e) {} 
         catch (CommunicationException e) {}
     }
 }
 /**
  * Returns a hash code value for the object. This method throws an
  * UnsupportedOperationException.
  */
 public int hashCode(){
     throw new UnsupportedOperationException();
 }
 
 /**
  * Checks whether the supplied object equals this object. An object is 
  * equal when it is an Entity with the same index and serial as this object.
  * 
  * @return true when equal, false otherwise.
  */
 public boolean equals(Object obj){
     if(obj instanceof Entity){
         Entity e = (Entity)obj;
         
         if((e.getIndex() == index) &&
            (e.getSerial() == serial))
         {
             return true;
         }
     }
     return false;
 }
 
 public int compareTo(Object obj){
     int result;
     
     if(this.equals(obj)){
         result = 0;
     } else if(obj instanceof EntityImpl){
         result = this.toStringExtended().compareTo(((EntityImpl)obj).toStringExtended());
     } else {
         result = -1;
     }
     return result;
 }

  protected Communicator getCommunicator() throws CMException {
    return communicator;
  }
     
 /** 
  * The index of the handle of the kernel entity that is associated with
  * this entity.
  */
 protected long index;
 
 /** 
  * The serial of the handle of the kernel entity that is associated with
  * this entity.
  */
 protected long serial;
 
 /** 
  * The pointer to the user layer entity that is associated with this
  * entity. This is used to resolve and claim the entity when actions are
  * performed on this entity.
  */
 protected String pointer;
 
 /**
  * The name of the kernel layer entity that is associated with this entity.
  */
 protected String name;
 
 /**
  * Determines whether the entity has been freed. If it is true, entity 
  * actions will result in an CMException being thrown.
  */
 protected boolean freed;
 
 /**
  * Whether or not the application owns the entity.
  */
 protected boolean owner;
 
 /**
  * Whether the entity is enabled.
  */
 protected boolean enabled;

 /**
  * The communicator used by the entity. It should never be null.
  */
 private final Communicator communicator;
 
}
