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
package org.opensplice.cm.com;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.SortedMap;
import java.util.TreeMap;

import javax.swing.Timer;

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
import org.opensplice.cm.transform.DataTransformerFactory;
import org.opensplice.cm.transform.EntityDeserializer;
import org.opensplice.cm.transform.EntitySerializer;
import org.opensplice.cm.transform.MetaTypeDeserializer;
import org.opensplice.cm.transform.QoSDeserializer;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.SampleDeserializer;
import org.opensplice.cm.transform.SnapshotDeserializer;
import org.opensplice.cm.transform.SnapshotSerializer;
import org.opensplice.cm.transform.StatisticsDeserializer;
import org.opensplice.cm.transform.StatusDeserializer;
import org.opensplice.cm.transform.StorageDeserializer;
import org.opensplice.cm.transform.StorageSerializer;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cm.transform.UserDataSerializer;

/**
 * SOAP implementation of the Control & Monitoring communication interface
 * (Communicator). When this communicator is plugged into the Control &
 * Monitoring API, communication with SPLICE-DDS domains on remote nodes is
 * possible if the Control & Monitoring SOAP Service is running on that node/
 * domain combination.
 * 
 * @date Jan 17, 2005
 */
public class SOAPCommunicator implements Communicator, ActionListener {
    private SOAPConnectionPool connectionPool;
    private String url = null;
    private EntityDeserializer entityDeserializer;
    private MetaTypeDeserializer typeDeserializer;
    private EntitySerializer entitySerializer;
    private StatusDeserializer statusDeserializer;
    private SnapshotDeserializer snapshotDeserializer;
    private SnapshotSerializer snapshotSerializer;
    private SampleDeserializer untypedSampleDeserializer;
    private UserDataSerializer userDataSerializer;
    private QoSDeserializer qosDeserializer;
    private StorageDeserializer storageDeserializer;
    private StorageSerializer storageSerializer;
    private Timer updateLease;
    private SOAPMessage leaseRequest;
    private QoSSerializer qosSerializer;
    private StatisticsDeserializer statisticsDeserializer;
    private boolean initialized;
    private boolean connectionAlive;

    /**
     * Creates a new SOAP communication handler for the Control & Monitoring
     * API.
     * 
     * @throws Exception
     *             Thrown when: - The Java SOAP extensions are not available.
     */
    public SOAPCommunicator() throws CommunicationException {
        initialized = false;
        connectionAlive = false;

        try {
            entityDeserializer = DataTransformerFactory.getEntityDeserializer(this,
                                                DataTransformerFactory.XML);
            typeDeserializer = DataTransformerFactory.getMetaTypeDeserializer(
                                                DataTransformerFactory.XML);
            entitySerializer = DataTransformerFactory.getEntitySerializer(
                                                DataTransformerFactory.XML);
            statusDeserializer = DataTransformerFactory.getStatusDeserializer(
                                                DataTransformerFactory.XML);
            snapshotDeserializer = DataTransformerFactory.getSnapshotDeserializer(this,
                                                DataTransformerFactory.XML);
            snapshotSerializer = DataTransformerFactory.getSnapshotSerializer(
                                                DataTransformerFactory.XML);
            untypedSampleDeserializer = DataTransformerFactory.getUntypedSampleDeserializer(
                                                DataTransformerFactory.XML);
            userDataSerializer = DataTransformerFactory.getUserDataSerializer(
                                                DataTransformerFactory.XML);
            qosDeserializer = DataTransformerFactory.getQoSDeserializer(
                                                DataTransformerFactory.XML);
            qosSerializer = DataTransformerFactory.getQoSSerializer(
                                                DataTransformerFactory.XML);
            statisticsDeserializer = DataTransformerFactory.getStatisticsDeserializer(
                                                DataTransformerFactory.XML);
            storageDeserializer = DataTransformerFactory.getStorageDeserializer(
                                                DataTransformerFactory.XML);
            storageSerializer = DataTransformerFactory.getStorageSerializer(
                                                DataTransformerFactory.XML);

            updateLease = new Timer(5*1000, this);
            updateLease.setRepeats(false);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("dummy", "dummy");
            leaseRequest = this.createRequest("updateLease", members);
        } catch (Exception e) {
            throw new CommunicationException(e.getMessage());
        }
    }

    @Override
    public void initialise(String _url) throws CommunicationException {
        SOAPConnection connection = null;
        try {
            connectionPool = SOAPConnectionPool.getInstance();
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("dummy", "dummy");
            SOAPMessage request = this.createRequest("initialise", members);
            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, _url);
            url = _url;
            String result = this.getResponse(response);

            if((result == null) || (!(result.equals("<result>OK</result>")))){
                throw new CommunicationException("Could not initialise.");
            }
            initialized = true;
            connectionAlive = true;
            updateLease.start();
        } catch (UnsupportedOperationException e) {
            throw new CommunicationException(e.getMessage());
        } catch (SOAPException e) {
            this.connectionAlive = false;
            this.checkConnection();
        } finally {
            if (connectionPool != null) {
                connectionPool.releaseConnection(connection);
            }
        }
    }

    @Override
    public void detach() throws CommunicationException {
        if(connectionPool == null){
            initialized = false;
            connectionAlive = false;
            throw new CommunicationException("No current connection");
        }
        SOAPConnection connection = null;
        try {
            if(this.initialized && this.connectionAlive){
                initialized = false;
                connectionAlive = false;
                SortedMap<String, String> members = new TreeMap<String, String>();
                members.put("dummy", "dummy");
                SOAPMessage request = this.createRequest("detach", members);

                connection = connectionPool.acquireConnection();
                SOAPMessage response = connection.call(request, url);
                connectionPool.releaseConnection(connection);
                String result = this.getResponse(response);

                if((result == null) || (!(result.equals("<result>OK</result>")))){
                    throw new CommunicationException("Could not detach.");
                }
                updateLease.stop();
                connectionPool.closeConnections();
                connectionPool = null;
            }
        } catch (UnsupportedOperationException e) {
            throw new CommunicationException(e.getMessage());
        } catch (SOAPException e) {
            this.connectionAlive = false;
            this.checkConnection();
        } finally {
            if (connectionPool != null) {
                connectionPool.releaseConnection(connection);
                updateLease.stop();
            }
        }
    }

    @Override
    public void entityFree(Entity entity) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            SOAPMessage request = this.createRequest("entityFree", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            this.getEmptyResponse(result);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException(te.getMessage());
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public Entity[] entityOwnedEntities(Entity entity, EntityFilter filter) throws CommunicationException {
        Entity[] e = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            members.put("filter", EntityFilter.getString(filter));
            SOAPMessage request = this.createRequest("entityOwnedEntities", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String entities = this.getResponse(result);
            e = entityDeserializer.deserializeEntityList(entities);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve owned entities.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return e;

    }

    @Override
    public Entity[] entityDependantEntities(Entity entity, EntityFilter filter) throws CommunicationException {
        Entity[] e = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            members.put("filter", EntityFilter.getString(filter));
            SOAPMessage request = this.createRequest("entityDependantEntities", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String entities = this.getResponse(result);
            e = entityDeserializer.deserializeEntityList(entities);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve dependant entities.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return e;

    }

    @Override
    public Participant participantNew(String uri, int timeout, String name, ParticipantQoS qos) throws CommunicationException {
        Participant participant = null;
        String xmlQos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("uri", uri);
            members.put("timeout", Integer.toString(timeout));
            members.put("name", name);
            members.put("qos", xmlQos);
            SOAPMessage request = this.createRequest("participantNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            participant = (Participant)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create participant");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return participant;
    }

    @Override
    public Participant[] participantAllParticipants(Participant p) throws CommunicationException {
        Participant[] result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String entities = null;
            String xmlEntity = entitySerializer.serializeEntity(p);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlEntity);
            SOAPMessage request = this.createRequest("participantAllParticipants", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage soapResult = connection.call(request, url);

            entities = this.getResponse(soapResult);
            Entity[] entityResult = entityDeserializer.deserializeEntityList(entities);
            result = new Participant[entityResult.length];

            for(int i=0; i<entityResult.length; i++){
                result[i] = (Participant)(entityResult[i]);
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve all partiticipants.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public Topic[] participantAllTopics(Participant p) throws CommunicationException {
        Topic[] result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String entities = null;
            String xmlEntity = entitySerializer.serializeEntity(p);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlEntity);
            SOAPMessage request = this.createRequest("participantAllTopics", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage soapResult = connection.call(request, url);

            entities = this.getResponse(soapResult);
            Entity[] entityResult = entityDeserializer.deserializeEntityList(entities);
            result = new Topic[entityResult.length];

            for(int i=0; i<entityResult.length; i++){
                result[i] = (Topic)(entityResult[i]);
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve all topics");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public Partition[] participantAllDomains(Participant p) throws CommunicationException {
        Partition[] result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String entities = null;
            String xmlEntity = entitySerializer.serializeEntity(p);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlEntity);
            SOAPMessage request = this.createRequest("participantAllDomains", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage soapResult = connection.call(request, url);

            entities = this.getResponse(soapResult);
            Entity[] entityResult = entityDeserializer.deserializeEntityList(entities);
            result = new Partition[entityResult.length];

            for(int i=0; i<entityResult.length; i++){
                result[i] = (Partition)(entityResult[i]);
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve all partitions");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public Topic[] participantFindTopic(Participant participant, String topicName) throws CommunicationException {
        Topic[] result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String entities = null;
            String xmlEntity = entitySerializer.serializeEntity(participant);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlEntity);
            members.put("topicName", topicName);
            SOAPMessage request = this.createRequest("participantFindTopic", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage soapResult = connection.call(request, url);

            entities = this.getResponse(soapResult);
            Entity[] entityResult = entityDeserializer.deserializeEntityList(entities);
            result = new Topic[entityResult.length];

            for(int i=0; i<entityResult.length; i++){
                result[i] = (Topic)(entityResult[i]);
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not find topic.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public MetaType topicGetDataType(Topic topic) throws CommunicationException, DataTypeUnsupportedException {
        MetaType result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(topic);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("topic", xmlEntity);
            SOAPMessage request = this.createRequest("topicDataType", members);
            updateLease.restart();

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlType = this.getResponse(response);
            result = typeDeserializer.deserializeMetaType(xmlType);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve topic data type.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;

    }

    @Override
    public MetaType readerGetDataType(Reader reader) throws CommunicationException, DataTypeUnsupportedException {
        MetaType result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(reader);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("reader", xmlEntity);
            SOAPMessage request = this.createRequest("readerDataType", members);
            updateLease.restart();

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlType = this.getResponse(response);
            result = typeDeserializer.deserializeMetaType(xmlType);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve topic data type of reader.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public MetaType writerGetDataType(Writer writer) throws CommunicationException, DataTypeUnsupportedException {
        MetaType result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            SOAPMessage request = this.createRequest("writerDataType", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlType = this.getResponse(response);
            result = typeDeserializer.deserializeMetaType(xmlType);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve topic data type of writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public ServiceState serviceGetState(Service service) throws CommunicationException {
        ServiceState state = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(service);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("service", xmlEntity);
            SOAPMessage request = this.createRequest("serviceGetState", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlState = this.getResponse(response);
            state = (ServiceState)(entityDeserializer.deserializeEntity(xmlState));
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve service state.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return state;

    }

    @Override
    public String getVersion() throws CommunicationException, CMException {
        String version = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("dummy", "dummy");
            SOAPMessage request = this.createRequest("getVersion", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            version = this.getResponse(response);
        } catch (SOAPException se) {
            if (se.getFaultCode() == SOAPException.SOAP_Client) {
                if (se.getFaultString().contains("not implemented")) {
                    version = "N.A.";
                } else {
                    throw new CMException(se.getMessage());
                }
            } else {
                throw new CMException(se.getMessage());
            }
        } finally {
            connectionPool.releaseConnection(connection);
        }

        if (version == null) {
            version = "N.A.";
        }

        return version;
    }

    @Override
    public Sample readerRead(Reader reader) throws CommunicationException, DataTypeUnsupportedException {
        Sample result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("reader", xmlEntity);
            SOAPMessage request = this.createRequest("readerRead", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSample = this.getResponse(response);
            result = untypedSampleDeserializer.deserializeSample(xmlSample, reader.getDataType());
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not read sample.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public Sample readerTake(Reader reader) throws CommunicationException, DataTypeUnsupportedException {
        Sample result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("reader", xmlEntity);
            SOAPMessage request = this.createRequest("readerTake", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSample = this.getResponse(response);
            result = untypedSampleDeserializer.deserializeSample(xmlSample, reader.getDataType());
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not take sample.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public Sample readerReadNext(Reader reader, GID instanceGID) throws CommunicationException, DataTypeUnsupportedException {
        Sample result = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("reader", xmlEntity);
            members.put("localId", Long.toString(instanceGID.getLocalId()));
            members.put("systemId", Long.toString(instanceGID.getSystemId()));

            SOAPMessage request = this.createRequest("readerReadNext", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSample = this.getResponse(response);
            result = untypedSampleDeserializer.deserializeSample(xmlSample, reader.getDataType());
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not readNext sample.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return result;
    }

    @Override
    public Status entityGetStatus(Entity entity) throws CommunicationException {
        Status status = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            SOAPMessage request = this.createRequest("entityGetStatus", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlStatus = this.getResponse(response);
            status = statusDeserializer.deserializeStatus(xmlStatus);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve entity status.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return status;
    }

    @Override
    public ReaderSnapshot readerSnapshotNew(Reader reader) throws CommunicationException {
        ReaderSnapshot rs = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(reader);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("reader", xmlEntity);
            SOAPMessage request = this.createRequest("readerSnapshotNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSnapshot = this.getResponse(response);
            rs = snapshotDeserializer.deserializeReaderSnapshot(xmlSnapshot, reader);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create a snapshot of the supplied reader.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return rs;
    }

    @Override
    public WriterSnapshot writerSnapshotNew(Writer writer) throws CommunicationException {
        WriterSnapshot ws = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            SOAPMessage request = this.createRequest("writerSnapshotNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSnapshot = this.getResponse(response);
            ws = snapshotDeserializer.deserializeWriterSnapshot(xmlSnapshot, writer);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create a snapshot of the supplied writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return ws;

    }

    @Override
    public void snapshotFree(Snapshot snapshot) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlSnapshot = snapshotSerializer.serializeSnapshot(snapshot);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("snapshot", xmlSnapshot);
            SOAPMessage request = this.createRequest("snapshotFree", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            this.getEmptyResponse(response);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not free snapshot.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public Sample snapshotRead(Snapshot snapshot) throws CommunicationException, DataTypeUnsupportedException {
        Sample s = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlSnapshot = snapshotSerializer.serializeSnapshot(snapshot);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("snapshot", xmlSnapshot);
            SOAPMessage request = this.createRequest("snapshotRead", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSample = this.getResponse(response);
            s = untypedSampleDeserializer.deserializeSample(xmlSample, snapshot.getUserDataType());
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not read from snapshot.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return s;
    }

    @Override
    public Sample snapshotTake(Snapshot snapshot) throws CommunicationException, DataTypeUnsupportedException {
        Sample s = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlSnapshot = snapshotSerializer.serializeSnapshot(snapshot);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("snapshot", xmlSnapshot);
            SOAPMessage request = this.createRequest("snapshotTake", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlSample = this.getResponse(response);
            s = untypedSampleDeserializer.deserializeSample(xmlSample, snapshot.getUserDataType());
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not take from snapshot.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return s;
    }

    @Override
    public void writerWrite(Writer writer, UserData data) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            members.put("userData", xmlData);
            SOAPMessage request = this.createRequest("writerWrite", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Could not write data.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not write data with suppplied writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void writerDispose(Writer writer, UserData data) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            members.put("userData", xmlData);
            SOAPMessage request = this.createRequest("writerDispose", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Could not dispose data.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not dispose data with suppplied writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void writerWriteDispose(Writer writer, UserData data) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            members.put("userData", xmlData);
            SOAPMessage request = this.createRequest("writerWriteDispose", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Could not writeDspose data.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not writeDispose data with suppplied writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void writerRegister(Writer writer, UserData data) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            members.put("userData", xmlData);
            SOAPMessage request = this.createRequest("writerRegister", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Could not register instance.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not register instance with suppplied writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void writerUnregister(Writer writer, UserData data) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("writer", xmlEntity);
            members.put("userData", xmlData);
            SOAPMessage request = this.createRequest("writerUnregister", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Could not unregister instance.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not unregister instance with suppplied writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public Publisher publisherNew(Participant p, String name, PublisherQoS qos) throws CommunicationException {
        Publisher entity = null;
        String xmlQos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlParticipant = entitySerializer.serializeEntity(p);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlParticipant);
            members.put("name", name);
            members.put("qos", xmlQos);
            SOAPMessage request = this.createRequest("publisherNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (Publisher)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create publisher.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public Subscriber subscriberNew(Participant p, String name, SubscriberQoS qos) throws CommunicationException {
        Subscriber entity = null;
        String xmlQos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlParticipant = entitySerializer.serializeEntity(p);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlParticipant);
            members.put("name", name);
            members.put("qos", xmlQos);
            SOAPMessage request = this.createRequest("subscriberNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (Subscriber)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create subscriber.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public Partition partitionNew(Participant p, String name) throws CommunicationException {
        Partition entity = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlParticipant = entitySerializer.serializeEntity(p);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlParticipant);
            members.put("name", name);
            SOAPMessage request = this.createRequest("domainNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (Partition)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create partition.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public void publisherPublish(Publisher p, String expression) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlPublisher = entitySerializer.serializeEntity(p);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("publisher", xmlPublisher);
            members.put("expression", expression);

            SOAPMessage request = this.createRequest("publisherPublish", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Publication failed.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Publication failed.");
        } finally {
            connectionPool.releaseConnection(connection);
        }

    }

    @Override
    public void subscriberSubscribe(Subscriber p, String expression) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlSubscriber = entitySerializer.serializeEntity(p);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("subscriber", xmlSubscriber);
            members.put("expression", expression);

            SOAPMessage request = this.createRequest("subscriberSubscribe", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlResult = this.getResponse(response);

            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Subscription failed.");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Subscription failed.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public Writer writerNew(Publisher p, String name, Topic t, WriterQoS qos) throws CommunicationException {
        Writer entity = null;
        String xmlQos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlPublisher = entitySerializer.serializeEntity(p);
            String xmlTopic = entitySerializer.serializeEntity(t);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("publisher", xmlPublisher);
            members.put("name", name);
            members.put("topic", xmlTopic);
            members.put("qos", xmlQos);

            SOAPMessage request = this.createRequest("writerNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (Writer)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create writer.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public DataReader dataReaderNew(Subscriber s, String name, String viewExpression, ReaderQoS qos) throws CommunicationException {
        DataReader entity = null;
        String xmlQos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlSubscriber = entitySerializer.serializeEntity(s);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("subscriber", xmlSubscriber);
            members.put("name", name);
            members.put("view", viewExpression);
            members.put("qos", xmlQos);

            SOAPMessage request = this.createRequest("dataReaderNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (DataReader)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create datareader.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public void dataReaderWaitForHistoricalData(DataReader dr, Time time) throws CommunicationException{
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlDataReader = entitySerializer.serializeEntity(dr);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("dataReader", xmlDataReader);
            members.put("seconds", Integer.toString(time.sec));
            members.put("nanoseconds", Integer.toString(time.nsec));

            SOAPMessage request = this.createRequest("dataReaderWaitForHistoricalData", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException("Wait for historical data failed: " + result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("dataReaderWaitForHistoricalData failed.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public Query queryNew(Reader source, String name, String expression) throws CommunicationException {
        Query entity = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlReader = entitySerializer.serializeEntity(source);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("reader", xmlReader);
            members.put("name", name);
            members.put("expression", expression);

            SOAPMessage request = this.createRequest("queryNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (Query)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create query.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public Topic topicNew(Participant p, String name, String typeName, String keyList, TopicQoS qos) throws CommunicationException {
        Topic entity = null;
        String xmlQos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlParticipant = entitySerializer.serializeEntity(p);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlParticipant);
            members.put("name", name);
            members.put("typeName", typeName);
            members.put("keyList", keyList);
            members.put("qos", xmlQos);

            SOAPMessage request = this.createRequest("topicNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlEntity = this.getResponse(result);
            entity = (Topic)entityDeserializer.deserializeEntity(xmlEntity);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create topic.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entity;
    }

    @Override
    public Waitset waitsetNew(Participant participant) throws CommunicationException {
        Waitset waitset = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(participant);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlEntity);

            SOAPMessage request = this.createRequest("waitsetNew", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage result = connection.call(request, url);

            String xmlWaitset= this.getResponse(result);
            waitset = (Waitset)entityDeserializer.deserializeEntity(xmlWaitset);

        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not create waitset.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return waitset;
    }

    @Override
    public QoS entityGetQoS(Entity entity) throws CommunicationException {
        QoS qos = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            SOAPMessage request = this.createRequest("entityGetQos", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String xmlQos = this.getResponse(response);
            qos = qosDeserializer.deserializeQoS(xmlQos);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Could not resolve qos.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return qos;
    }

    @Override
    public void entitySetQoS(Entity entity, QoS qos) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String xmlQoS = qosSerializer.serializeQoS(qos);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            members.put("qos", xmlQoS);
            SOAPMessage request = this.createRequest("entitySetQos", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch(TransformationException te){
            throw new CommunicationException("Applying new qos failed.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void participantRegisterType(Participant participant, MetaType type) throws CommunicationException{
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(participant);
            String xmlType = type.toXML();

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("participant", xmlEntity);
            members.put("type", xmlType);

            SOAPMessage request = this.createRequest("participantRegisterType", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not register type");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public Statistics entityGetStatistics(Entity entity) throws CommunicationException {
        Statistics statistics = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);

            SOAPMessage request = this.createRequest("entityStatistics", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if((result != null) &&(!("".equals(result)))){
                statistics = statisticsDeserializer.deserializeStatistics(result, entity);
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not get statistics.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return statistics;
    }

    @Override
    public Statistics[] entityGetStatistics(Entity[] entities) throws CommunicationException {
        Statistics[] statistics = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntities = entitySerializer.serializeEntities(entities);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entities", xmlEntities);

            SOAPMessage request = this.createRequest("entitiesStatistics", members);
            
            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if((result != null) &&(!("".equals(result)))){
                statistics = statisticsDeserializer.deserializeStatistics(result, entities);	
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not get statistics.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return statistics;
    }
    
    @Override
    public void entityResetStatistics(Entity entity, String fieldName) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);
            members.put("fieldName", fieldName);

            SOAPMessage request = this.createRequest("entityResetStatistics", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not reset statistics.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void entityEnable(Entity entity) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("entity", xmlEntity);

            SOAPMessage request = this.createRequest("entityEnable", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not enable entity.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
    }

    @Override
    public void waitsetAttach(Waitset waitset, Entity entity) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String xmlEntity  = entitySerializer.serializeEntity(entity);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("waitset", xmlWaitset);
            members.put("entity", xmlEntity);

            SOAPMessage request = this.createRequest("waitsetAttach", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not attach entity to waitset.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return;
    }

    @Override
    public void waitsetDetach(Waitset waitset, Entity entity) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String xmlEntity  = entitySerializer.serializeEntity(entity);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("waitset", xmlWaitset);
            members.put("entity", xmlEntity);

            SOAPMessage request = this.createRequest("waitsetDetach", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not attach entity to waitset.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return;
    }

    @Override
    public Entity[] waitsetWait(Waitset waitset) throws CommunicationException{
        Entity[] entities = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("waitset", xmlWaitset);

            SOAPMessage request = this.createRequest("waitsetWait", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            Entity[] entityResult = entityDeserializer.deserializeEntityList(result);

            if(entityResult != null){
                entities = new Entity[entityResult.length];

                for(int i=0; i<entityResult.length; i++){
                    entities[i] = entityResult[i];
                }
            } else {
                throw new CommunicationException("waitsetWait failed (2)");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("waitsetWait failed.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entities;
    }

    @Override
    public Entity[] waitsetTimedWait(Waitset waitset, Time time) throws CommunicationException{
        Entity[] entities = null;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("waitset", xmlWaitset);
            members.put("seconds", Integer.toString(time.sec));
            members.put("nanoseconds", Integer.toString(time.nsec));

            SOAPMessage request = this.createRequest("waitsetTimedWait", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            Entity[] entityResult = entityDeserializer.deserializeEntityList(result);

            if(entityResult != null){
                entities = new Entity[entityResult.length];

                for(int i=0; i<entityResult.length; i++){
                    entities[i] = entityResult[i];
                }
            } else {
                throw new CommunicationException("waitsetTimedWait failed (2)");
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("waitsetTimedWait failed.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return entities;
    }

    @Override
    public int waitsetGetEventMask(Waitset waitset) throws CommunicationException {
        int mask = 0;
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("waitset", xmlWaitset);

            SOAPMessage request = this.createRequest("waitsetGetEventMask", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);
            mask = Integer.parseInt(result);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve event mask of waitset.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return mask;
    }

    @Override
    public void waitsetSetEventMask(Waitset waitset, int mask) throws CommunicationException {
        this.checkConnection();
        SOAPConnection connection = null;
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("waitset", xmlWaitset);
            members.put("mask", Integer.toString(mask));

            SOAPMessage request = this.createRequest("waitsetSetEventMask", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            String result = this.getResponse(response);

            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not set event mask of waitset.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return;
    }

    @Override
    public Object storageOpen(String attrs) throws CommunicationException {
        Object storage = null;
        String result;
        SOAPConnection connection = null;
        this.checkConnection();
        try{
            /* attrs is already XML, so doesn't need to be serialized */
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("attrs", attrs);

            SOAPMessage request = this.createRequest("storageOpen", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            result = this.getResponse(response);

            Result r = storageDeserializer.deserializeOpenResult_Result(result);

            if(!r.equals(Result.SUCCESS)){
                /* TODO: create StorageException or something like that */
            } else {
                storage = storageDeserializer.deserializeOpenResult_Storage(result);
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not deserialize storageOpen result.");
        } finally {
            connectionPool.releaseConnection(connection);
        }

        return storage;
    }

    @Override
    public Result storageClose(Object storage) throws CommunicationException{
        Result r = Result.ERROR;
        String result;
        SOAPConnection connection = null;
        this.checkConnection();
        try{
            String xmlStorage = storageSerializer.serializeStorage(storage);
            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("storage", xmlStorage);

            SOAPMessage request = this.createRequest("storageClose", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            result = this.getResponse(response);

            r = storageDeserializer.deserializeStorageResult(result);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not deserialize storageClose result.");
        } finally {
            connectionPool.releaseConnection(connection);
        }

        return r;
    }

    @Override
    public Result storageAppend(Object storage, UserData data) throws CommunicationException {
        Result r = Result.ERROR;
        String result;
        SOAPConnection connection = null;
        this.checkConnection();
        try{
            String xmlStorage = storageSerializer.serializeStorage(storage);
            String xmlType = data.getUserDataType().toXML();
            String xmlData = userDataSerializer.serializeUserData(data);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("storage", xmlStorage);
            members.put("metadata", xmlType);
            members.put("data", xmlData);

            SOAPMessage request = this.createRequest("storageAppend", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            result = this.getResponse(response);

            r = storageDeserializer.deserializeStorageResult(result);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not deserialize storageAppend result.");
        } finally {
            connectionPool.releaseConnection(connection);
        }

        return r;
    }

    @Override
    public UserData storageRead(Object storage) throws CommunicationException {
        UserData data = null;
        String result;
        SOAPConnection connection = null;
        this.checkConnection();
        try{
            String xmlStorage = storageSerializer.serializeStorage(storage);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("storage", xmlStorage);

            SOAPMessage request = this.createRequest("storageRead", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            result = this.getResponse(response);

            Result r = storageDeserializer.deserializeReadResult_Result(result);
            if(!r.equals(Result.SUCCESS)){
                /* TODO: create StorageException or something like that */
            } else {
                /* Retrieve the metadata. TODO: Only resolve if type unkown; i.e.
                 * cache the output. */
                String typeName = storageDeserializer.deserializeReadResult_DataTypeName(result);
                if (typeName != null) {
                    MetaType type = storageGetType(storage, typeName);
                    data = storageDeserializer.deserializeReadResult_Data(result, type);
                } /* Else read returned NULL */
            }
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not deserialize storageRead result.");
        } finally {
            connectionPool.releaseConnection(connection);
        }

        return data;
    }

    @Override
    public MetaType storageGetType(Object storage, String typeName) throws CommunicationException {
        MetaType meta = null;
        String result;
        SOAPConnection connection = null;
        this.checkConnection();
        try{
            String xmlStorage = storageSerializer.serializeStorage(storage);
            String xmlTypeName = storageSerializer.serializeTypeName(typeName);

            SortedMap<String, String> members = new TreeMap<String, String>();
            members.put("storage", xmlStorage);
            members.put("typeName", xmlTypeName);

            SOAPMessage request = this.createRequest("storageGetType", members);

            connection = connectionPool.acquireConnection();
            SOAPMessage response = connection.call(request, url);

            result = this.getResponse(response);

            meta = storageDeserializer.deserializeGetTypeResult_Metadata(result);
        } catch (SOAPException se) {
            this.connectionAlive = false;
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not deserialize storageGetType result.");
        } catch (DataTypeUnsupportedException e){
            throw new CommunicationException("Datatype returned by storage not supported.");
        } finally {
            connectionPool.releaseConnection(connection);
        }
        return meta;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if(e.getSource().equals(updateLease)){
            SOAPConnection connection = null;
            try {
                this.checkConnection();
                connection = connectionPool.acquireConnection();
                SOAPMessage response = connection.call(leaseRequest, url);

                String result = this.getResponse(response);

                if((result != null) && ((result.equals("<result>OK</result>")))){
                    updateLease.restart();
                }
            }
            catch (SOAPException se) {
                this.connectionAlive = false;
            }
            catch (CommunicationException ce) {
                this.connectionAlive = false;
            } finally {
                connectionPool.releaseConnection(connection);
            }
        }
    }

    private SOAPMessage createRequest(String method, SortedMap<String, String> members){
        SOAPRequest message;
        String value, key;

        message = new SOAPRequest();
        message.setMethod(method);

        if(members != null){
            for (Iterator<Entry<String, String>> it = members.entrySet().iterator(); it.hasNext();) {
                Map.Entry<String, String> entry = it.next();
                key = entry.getKey();
                value = entry.getValue();
                message.addBodyParameter(key, value);
            }
        }
        message.saveChanges();

        return message;
    }

    private String getResponse(SOAPMessage response) throws CommunicationException {
        String result = null;

        if(response == null){
            throw new CommunicationException("Empty response received.");
        }
        updateLease.restart();
        result = ((SOAPResponse)response).getBodyContent();

        return result;
    }

    private void getEmptyResponse(SOAPMessage response) throws CommunicationException {
        if(response == null){
            throw new CommunicationException("null input.");
        }
        updateLease.restart();
    }

    private void checkConnection() throws CommunicationException{
        if(this.initialized && !this.connectionAlive){
            throw new ConnectionLostException();
        } else if (!this.initialized){
           throw new CommunicationException("Connection has been closed already");
        }
    }
}

