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
/**
 * Contains the communication interface for the Java C&M API. The purpose of
 * this package is to provide an interface to Splice independant of the
 * communication protocol. By using this package multiple communication handlers
 * can be plugged into the C&M API.
 */
package org.opensplice.cm.com;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataReader;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Partition;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Query;
import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Service;
import org.opensplice.cm.ServiceState;
import org.opensplice.cm.Snapshot;
import org.opensplice.cm.Storage.Result;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.Time;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Waitset;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;
import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.ParticipantQoS;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.qos.WriterQoS;
import org.opensplice.cm.statistics.Statistics;
import org.opensplice.cm.status.Status;

/**
 * Communicator class for the Control & Monitoring API. By using this interface
 * it is possible to use several communication protocols without altering the
 * code of the actual Control & Monitoring API.
 * 
 * All communication handlers must implement this interface.
 */
public interface Communicator {

    /**
     * Initialises the Communicator to be able to communicate with the node the
     * Java C&M API has to get its information from.
     * 
     * @param The
     *            URL of the node to initialise.
     * @throws CommunicationException
     *             Thrown when initialization failed.
     */
    public void initialise(String url) throws CommunicationException;

    /**
     * Detaches the Control & Monitoring API.
     * 
     * @throws CommunicationException
     *             Thrown when detach failed.
     */
    public void detach() throws CommunicationException;

    /**
     * Frees the supplied entity. Both the XML and user layer entity will be
     * freed.
     * 
     * @param entity
     *            The entity that must be freed.
     * @throws CommunicationException
     *             Thrown when free failed.
     */
    public void entityFree(Entity entity) throws CommunicationException;

    /**
     * Resolves all entities that the supplied entity owns and match the
     * supplied filter.
     * 
     * @param entity
     *            The entity, which owned entities must be resolved.
     * @param filter
     *            The filter, which must be matched by every entity that is
     *            returned.
     * @return An array of all entities that are owned by the supplied entity
     *         and match the supplied filter. When the supplied entity is not
     *         available, null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public Entity[] entityOwnedEntities(Entity entity, EntityFilter filter) throws CommunicationException;

    /**
     * Resolves all entities that the supplied entity depends on and match the
     * supplied filter.
     * 
     * @param entity
     *            The entity, which dependant entities must be resolved.
     * @param filter
     *            The filter, which must be matched by every entity that is
     *            returned.
     * @return An array of all entities that the supplied entity depends on and
     *         match the supplied filter. When the supplied entity is not
     *         available, null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public Entity[] entityDependantEntities(Entity entity, EntityFilter filter) throws CommunicationException;

    /**
     * Resolves the QoS of the supplied Entity.
     * 
     * @param entity
     *            The Entity where to resolve to QoS of.
     * @return The Entity QoS or null if: - The Entity has no QoS - The Entity
     *         is not available (anymore)
     */
    public QoS entityGetQoS(Entity entity) throws CommunicationException;

    /**
     * Applies the supplied QoS to the supplied Entity.
     * 
     * @param entity
     *            The Entity to apply the supplied QoS to.
     * @param qos
     *            The QoS to apply to the supplied Entity.
     * @throws CommunicationException
     *             Thrown when: - The Entity has no QoS. - The Entity is not
     *             available. - The QoS or parts of it are immutable. - The
     *             communication with SPLICE failed.
     */
    public void entitySetQoS(Entity entity, QoS qos) throws CommunicationException;

    /**
     * Resolves the current status of the supplied Entity.
     * 
     * @param entity
     *            The Entity to resolve the current status from.
     * @return The current status of the Entity, or null if the Entity has no
     *         status.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public Status entityGetStatus(Entity entity) throws CommunicationException;

    /**
     * Provides access to the statistics of the supplied Entity.
     * 
     * @param entity
     *            The entity, which statistics must be resolved.
     * @return The statistics of the Entity or null if it has none.
     * @throws CommunicationException
     *             Thrown when: - The Entity is not available. - The
     *             communication with SPLICE failed.
     */
    public Statistics entityGetStatistics(Entity entity) throws CommunicationException;
    
    /**
     * Provides access to the statistics of the supplied Entities.
     * 
     * @param entities
     *            The entities, which statistics must be resolved.
     * @return The statistics of the Entities or null if they have none.
     * @throws CommunicationException
     *             Thrown when: - The Entities are not available. - The
     *             communication with SPLICE failed.
     */
	public Statistics[] entityGetStatistics(Entity[] entities)	throws CommunicationException;
	
    /**
     * Resets (a part of) the statistics of the supplied entity.
     * 
     * @param entity
     *            The entity, which statistics to reset.
     * @param fieldName
     *            The field within the statistics to reset, or null if all
     *            statistics must be reset.
     * @throws CommunicationException
     *             Thrown when: - The Entity is not available. - The
     *             communication with SPLICE failed. - The supplied fieldName
     *             does not exist.
     */
    public void entityResetStatistics(Entity entity, String fieldName) throws CommunicationException;

    /**
     * Enables the supplied entity. If the supplied entity is already enabled,
     * this call has no effect.
     * 
     * @param entity
     *            The entity to enable.
     * @throws CommunicationException
     *             Thrown when: - The Entity is not available. - The
     *             communication with SPLICE failed.
     */
    public void entityEnable(Entity entity) throws CommunicationException;

    /**
     * Creates a Participant in the kernel that matches the supplied URI.
     * 
     * @param uri
     *            The URI which must be opened to get access to the kernel where
     *            the Participant must be created in.
     * @param timeout
     *            The maximum amount of time this function may keep trying to
     *            create the participant when creation fails (in milliseconds).
     * @param name
     *            The name of the Participant.
     * @param qos
     *            The quality of service for the Participant.
     * 
     * @return The newly created Participant or null if it could not be created.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Participant participantNew(String uri, int timeout, String name, ParticipantQoS qos) throws CommunicationException;

    /**
     * Resolves all participants in the kernel the supplied participant is
     * participating in.
     * 
     * @param p
     *            The Participant which kernel must be resolved.
     * @return An array of all participants that participate in the same kernel
     *         as the supplied participant. When the supplied participant is not
     *         available null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public Participant[] participantAllParticipants(Participant p) throws CommunicationException;

    /**
     * Resolves all topics in the kernel the supplied participant is
     * participating in.
     * 
     * @param p
     *            The Participant which kernel must be resolved.
     * @return An array of all topics that are located in the same kernel as the
     *         supplied participant. When the supplied participant is not
     *         available null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public Topic[] participantAllTopics(Participant p) throws CommunicationException;

    /**
     * Resolves all domains in the kernel the supplied participant is
     * participating in.
     * 
     * @param p
     *            The Participant which kernel must be resolved.
     * @return An array of all domains that are located in the same kernel as
     *         the supplied participant. When the supplied participant is not
     *         available null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public Partition[] participantAllDomains(Participant p) throws CommunicationException;

    /**
     * Resolves the topics in the domain this Participant is participating in
     * that match the supplied expression.
     * 
     * @param participant
     *            The participant that determines the domain and node to look
     *            in.
     * @param topicName
     *            The topic name expression that may contain wildcard characters
     *            (*,?).
     * @return The list of Topic entities that match the supplied expression.
     * @throws CommunicationException
     *             Thrown when the find failed.
     */
    public Topic[] participantFindTopic(Participant participant, String topicName) throws CommunicationException;

    /**
     * Registers the supplied data type in the domain<->node combination this
     * Participant is participating in. Once a type is registered, it is
     * possible to create a Topic of this type.
     * 
     * If this type is already known in the domain<->node combination, this call
     * has no effect.
     * 
     * @param participant
     *            The participant that determines the domain and node where the
     *            data type needs to be registered.
     * @param type
     *            The data type that needs to be registered.
     * @throws CommunicationException
     *             Throw when: - Connection with the node is lost. - The
     *             supplied type is not a valid type. - A type with the same
     *             name but another content then the supplied type has already
     *             been registered in the domain <-> node.
     */
    public void participantRegisterType(Participant participant, MetaType type) throws CommunicationException;

    /**
     * Creates a new Publisher in the supplied Participant.
     * 
     * @param p
     *            The Participant where to create the Publisher in.
     * @param name
     *            The name of the Publisher.
     * @param qos
     *            The quality of service for the Publisher.
     * 
     * @return The newly created Publisher, or null if creation failed.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Publisher publisherNew(Participant p, String name, PublisherQoS qos) throws CommunicationException;

    /**
     * Makes the supplied Publisher publish its data in the Partitions that
     * match the supplied expression. The Partitions concerned must be created
     * prior to calling the function or else calling this function has no
     * effect.
     * 
     * @param p
     *            The Publisher that needs to publish data.
     * @param expression
     *            The Partition expression.
     * @throws CommunicationException
     *             Thrown when publication failed.
     */
    public void publisherPublish(Publisher p, String expression) throws CommunicationException;

    /**
     * Creates a new Subscriber in the supplied Participant.
     * 
     * @param p
     *            The Participant where to create the Subscriber in.
     * @param name
     *            The name of the Subscriber.
     * @param qos
     *            The quality of service for the Subscriber.
     * 
     * @return The newly created Subscriber.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Subscriber subscriberNew(Participant p, String name, SubscriberQoS qos) throws CommunicationException;

    /**
     * Makes the supplied Subscriber read its data from the Partitions that
     * match the supplied expression. The Partitions concerned must be created
     * prior to calling the function or else calling this function has no
     * effect.
     * 
     * @param p
     *            The Subscribers that needs to read data.
     * @param expression
     *            The Partition expression.
     * @throws CommunicationException
     *             Thrown when subscription failed.
     */
    public void subscriberSubscribe(Subscriber p, String expression) throws CommunicationException;

    /**
     * Creates a new Partition in the supplied Participant.
     * 
     * @param p
     *            The Participant where to create the Partition in.
     * @param name
     *            The name of the Publisher.
     * @return The newly created Partition.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Partition partitionNew(Participant p, String name) throws CommunicationException;

    /**
     * Creates a new Topic in the supplied Participant.
     * 
     * @param p
     *            The Participant where to create the Topic in.
     * @param name
     *            The name of the Topic.
     * @param typeName
     *            The type name of the Topic.
     * @param keyList
     *            The keyList of the Topic.
     * @param qos
     *            The quality of service for Topic
     * 
     * @return The newly created Topic.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Topic topicNew(Participant p, String name, String typeName, String keyList, TopicQoS qos) throws CommunicationException;

    /**
     * Resolves the data type of the supplied Topic.
     * 
     * @param topic
     *            The topic, which data type must be resolved.
     * @return The data type of the supplied Topic. If the Topic is not
     *         available, null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public MetaType topicGetDataType(Topic topic) throws CommunicationException, DataTypeUnsupportedException;

    /**
     * Resolves the data type of data in the database of the supplied Reader.
     * 
     * @param reader
     *            The reader, which data type must be resolved.
     * @return The data type of the supplied Reader. If the Reader is not
     *         available, null is returned.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public MetaType readerGetDataType(Reader reader) throws CommunicationException, DataTypeUnsupportedException;

    /**
     * Reads a Sample from the supplied Reader.
     * 
     * @param reader
     *            The Reader to read data from.
     * @return The read Sample, or null if no data is available.
     * @throws CommunicationException
     *             Thrown when read failed.
     * @throws DataTypeUnsupportedException
     */
    public Sample readerRead(Reader reader) throws CommunicationException, DataTypeUnsupportedException;

    /**
     * Takes a Sample from the supplied Reader.(Read and remove)
     * 
     * @param reader
     *            The Reader to take data from.
     * @return The taken Sample, or null if no data is available.
     * @throws CommunicationException
     *             Thrown when take failed.
     * @throws DataTypeUnsupportedException
     */
    public Sample readerTake(Reader reader) throws CommunicationException, DataTypeUnsupportedException;

    /**
     * Reads the next sample in the reader database.
     * 
     * @param reader
     *            The reader to read from.
     * @param instanceGID
     *            The instance id of the previous instance.
     * @return The read sample.
     * @throws CommunicationException
     *             Thrown when: - The Reader is not available. - The
     *             communication with SPLICE failed.
     * @throws DataTypeUnsupportedException
     *             Thrown when the Topic data type is not supported.
     */
    public Sample readerReadNext(Reader reader, GID instanceGID) throws CommunicationException, DataTypeUnsupportedException;

    /**
     * Creates a new DataReader in the supplied Subscriber.
     * 
     * @param s
     *            The Subscriber where to attach the DataReader to.
     * @param name
     *            The name of the DataReader.
     * @param view
     *            The view expression for the DataReader.
     * @param qos
     *            The quality of service for the DataReader.
     * 
     * @return The newly created DataReader.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public DataReader dataReaderNew(Subscriber s, String name, String viewExpression, ReaderQoS qos) throws CommunicationException;

    public void dataReaderWaitForHistoricalData(DataReader d, Time maxWaitTime) throws CommunicationException;

    /**
     * Creates a new Query in the supplied Reader.
     * 
     * @param source
     *            The Reader where to attach the Query to.
     * @param name
     *            The name of the Query.
     * @param expression
     *            The Query expression.
     * @return The newly created Query.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Query queryNew(Reader source, String name, String expression) throws CommunicationException;

    /**
     * Creates a new Writer in the supplied Publisher.
     * 
     * @param p
     *            The Publisher where to attach the Writer to.
     * @param name
     *            The name of the Writer.
     * @param t
     *            The Topic that the Writer must write.
     * @param qos
     *            The quality of service for the Writer.
     * 
     * @return The newly created Writer.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public Writer writerNew(Publisher p, String name, Topic t, WriterQoS qos) throws CommunicationException;

    /**
     * Resolves the userData type of the data the supplied Writer writes.
     * 
     * @param writer
     *            The writer to resolve the userData type of.
     * @return The userData type.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public MetaType writerGetDataType(Writer writer) throws CommunicationException , DataTypeUnsupportedException;

    /**
     * Writes data in the Splice system.
     * 
     * @param writer
     *            The writer where to inject the data into.
     * @param data
     *            The data to inject.
     * @throws CommunicationException
     *             Thrown when write failed.
     */
    public void writerWrite(Writer writer, UserData data) throws CommunicationException;

    /**
     * Disposes data in the Splice system.
     * 
     * @param writer
     *            The writer where to dispose the data from.
     * @param data
     *            The data to dispose.
     * @throws CommunicationException
     *             Thrown when dispose failed.
     */
    public void writerDispose(Writer writer, UserData data) throws CommunicationException;

    /**
     * WriteDisposes data in the Splice system.
     * 
     * @param writer
     *            The writer where to writeDispose the data from.
     * @param data
     *            The data to writeDispose.
     * @throws CommunicationException
     *             Thrown when writeDispose failed.
     */
    public void writerWriteDispose(Writer writer, UserData data) throws CommunicationException;

    /**
     * Registers data in the Splice system.
     * 
     * @param writer
     *            The writer where to register the data.
     * @param data
     *            The data to register.
     * @throws CommunicationException
     *             Thrown when register failed.
     */
    public void writerRegister(Writer writer, UserData data) throws CommunicationException;

    /**
     * Unregisters data in the Splice system.
     * 
     * @param writer
     *            The writer where to unregister the data.
     * @param data
     *            The data to unregister.
     * @throws CommunicationException
     *             Thrown when unregister failed.
     */
    public void writerUnregister(Writer writer, UserData data) throws CommunicationException;

    /**
     * Resolves the current state of the supplied Service.
     * 
     * @param service
     *            The Service, which state must be resolved.
     * @return The current state of the supplied Service, or null if the
     *         supplied Service is not available.
     * @throws CommunicationException
     *             Thrown when resolve failed.
     */
    public ServiceState serviceGetState(Service service) throws CommunicationException;

    /**
     * Resolves the current remote version of the CM API.
     * 
     * @return The current version of the CM API if an uri is supplied the
     *         current version of the remote system will be given
     * @throws CommunicationException
     *             Thrown when resolve failed.
     * @throws CMException
     */
    public String getVersion() throws CommunicationException, CMException;

    /**
     * Creates a snapshot of the database of the supplied Reader.
     * 
     * @param reader
     *            The Reader to make a snapshot of.
     * @return The created snapshot.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public ReaderSnapshot readerSnapshotNew(Reader reader) throws CommunicationException;

    /**
     * Creates a snapshot of the history of the supplied Writer.
     * 
     * @param writer
     *            The Writer to make a snapshot of.
     * @return The created snapshot.
     * @throws CommunicationException
     *             Thrown when creation failed.
     */
    public WriterSnapshot writerSnapshotNew(Writer writer) throws CommunicationException;

    /**
     * Frees the supplied snapshot.
     * 
     * @param snapshot
     *            The snapshot to free.
     * @throws CommunicationException
     *             Thrown when free failed.
     */
    public void snapshotFree(Snapshot snapshot) throws CommunicationException;

    /**
     * Reads a Sample from the supplied Snapshot.
     * 
     * @param snapshot
     *            The Snapshot to read data from.
     * @return The read Sample, or null if no data is available.
     * @throws CommunicationException
     *             Thrown when read failed.
     * @throws DataTypeUnsupportedException
     */
    public Sample snapshotRead(Snapshot snapshot) throws CommunicationException, DataTypeUnsupportedException;

    /**
     * Takes a Sample from the supplied Snapshot.
     * 
     * @param snapshot
     *            The Snapshot to take data from.
     * @return The taken Sample, or null if no data is available.
     * @throws CommunicationException
     *             Thrown when take failed.
     * @throws DataTypeUnsupportedException
     */
    public Sample snapshotTake(Snapshot snapshot) throws CommunicationException, DataTypeUnsupportedException;

    public Waitset waitsetNew(Participant participant) throws CommunicationException;

    public void waitsetAttach(Waitset waitset, Entity entity) throws CommunicationException;

    public void waitsetDetach(Waitset waitset, Entity entity) throws CommunicationException;

    public Entity[] waitsetWait(Waitset waitset) throws CommunicationException;

    public Entity[] waitsetTimedWait(Waitset waitset, Time time) throws CommunicationException;

    public int waitsetGetEventMask(Waitset waitset) throws CommunicationException;

    public void waitsetSetEventMask(Waitset waitset, int mask) throws CommunicationException;

    /**
     *
     * @param attrs
     * @return Opaque representation of the opened storage
     * @throws CommunicationException
     */
    public Object storageOpen(String attrs) throws CommunicationException;
    public Result storageClose(Object storage) throws CommunicationException;
    public Result storageAppend(Object storage, UserData data) throws CommunicationException;
    public UserData storageRead(Object storage) throws CommunicationException;
    public MetaType storageGetType(Object storage, String typeName) throws CommunicationException;
}
