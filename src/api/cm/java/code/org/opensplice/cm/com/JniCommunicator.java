/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.cm.com;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataReader;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Event;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Partition;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Query;
import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Service;
import org.opensplice.cm.ServiceState;
import org.opensplice.cm.Snapshot;
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
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cm.transform.UserDataSerializer;

/**
 * JNI implementation of the Control & Monitoring communication interface 
 * (Communicator). When this communicator is plugged into the Control & 
 * Monitoring API, communication with Splice is only possible on the same node
 * the API is running.
 */
public class JniCommunicator implements Communicator {
    private EntityDeserializer entityDeserializer;
    private MetaTypeDeserializer typeDeserializer;
    private EntitySerializer entitySerializer;
    private StatusDeserializer statusDeserializer;
    private SnapshotDeserializer snapshotDeserializer;
    private SnapshotSerializer snapshotSerializer;
    private SampleDeserializer untypedSampleDeserializer;
    private UserDataSerializer userDataSerializer;
    private QoSDeserializer qosDeserializer;
    private QoSSerializer qosSerializer;
    private StatisticsDeserializer statisticsDeserializer;
    private boolean initialized;
    private static boolean connectionAlive;
    
    /**
     * Creates a new JNI communication handler for the Control & Monitoring API.
     * 
     * @throws CMException Thrown when the C JNI library cannot be loaded
     *                     (libcmjni) or one of the XML (de)serializers cannot 
     *                     be initialised.
     */
    public JniCommunicator() throws CommunicationException{
        try{                        
            System.loadLibrary("cmjni");
        }
        catch (UnsatisfiedLinkError e) {
            System.err.println("Could not load JNI library 'cmjni'. Reason:\n" +
                     e.getMessage());
            throw new CommunicationException("Could not load JNI library 'cmjni': " + e.getMessage());
        }
        
        initialized = false;
        JniCommunicator.connectionAlive = false;
        entityDeserializer = DataTransformerFactory.getEntityDeserializer(
                                DataTransformerFactory.XML);
        typeDeserializer = DataTransformerFactory.getMetaTypeDeserializer(
                                DataTransformerFactory.XML);
        entitySerializer = DataTransformerFactory.getEntitySerializer(
                                DataTransformerFactory.XML);
        statusDeserializer = DataTransformerFactory.getStatusDeserializer(
                                DataTransformerFactory.XML);
        snapshotDeserializer = DataTransformerFactory.getSnapshotDeserializer(
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
    }
    
    public synchronized void initialise(String url) throws CommunicationException{
        String result = jniInitialise();
        
        if((result == null) || (!(result.equals("<result>OK</result>")))){
            throw new CommunicationException("Could not initialise.");
        }
        initialized = true;
        JniCommunicator.connectionAlive = true;
    }

    public void detach() throws CommunicationException{
        String result = jniDetach();
        initialized = false;
        JniCommunicator.connectionAlive = false;
        
        if((result == null) || (!(result.equals("<result>OK</result>")))){
            throw new CommunicationException("Could not detach.");
        }
    }

    public Entity[] entityOwnedEntities(Entity entity, EntityFilter filter) throws CommunicationException {
        Entity[] result = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String xmlEntities = this.jniGetOwnedEntities(xmlEntity, EntityFilter.getString(filter));
            this.checkConnection();
            result = entityDeserializer.deserializeEntityList(xmlEntities);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve owned entities.");
        }
        return result;
    }

    public Entity[] entityDependantEntities(Entity entity, EntityFilter filter) throws CommunicationException {
        Entity[] result = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String xmlEntities = this.jniGetDependantEntities(xmlEntity, EntityFilter.getString(filter));
            this.checkConnection();
            result = entityDeserializer.deserializeEntityList(xmlEntities);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve dependant entities.");
        }
        return result;
    }
    
    public MetaType topicGetDataType(Topic topic) throws CommunicationException, DataTypeUnsupportedException{
        MetaType result = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(topic);
            String xmlType = this.jniGetTopicDataType(xmlEntity);
            this.checkConnection();
            result = typeDeserializer.deserializeMetaType(xmlType);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve topic data type.");
        }
        return result;
    }
    
    public MetaType readerGetDataType(Reader reader) throws CommunicationException, DataTypeUnsupportedException{
        MetaType result = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            String xmlType = this.jniReaderDataType(xmlEntity);
            this.checkConnection();
            result = typeDeserializer.deserializeMetaType(xmlType);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve topic data type of reader.");
        }
        return result;
    }
    
    public Sample readerRead(Reader reader) throws CommunicationException, DataTypeUnsupportedException {
        Sample result = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            String xmlSample = this.jniReaderRead(xmlEntity);
            this.checkConnection();
            result = untypedSampleDeserializer.deserializeSample(xmlSample, reader.getDataType());
        } catch (CMException e) {
            throw new CommunicationException(e.getMessage());
        } catch (TransformationException e) {
            throw new CommunicationException("Could not read sample.");
        }
        
        return result;
    }

    public Sample readerTake(Reader reader) throws CommunicationException, DataTypeUnsupportedException {
        Sample result = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            String xmlSample = this.jniReaderTake(xmlEntity);
            this.checkConnection();
            result = untypedSampleDeserializer.deserializeSample(xmlSample, reader.getDataType());
        } catch (CMException e) {
            throw new CommunicationException(e.getMessage());
        } catch (TransformationException e) {
            throw new CommunicationException("Could not take sample.");
        }
        return result;
    }
    
    public Sample readerReadNext(Reader reader, GID instanceGID) throws CommunicationException, DataTypeUnsupportedException {
        Sample result = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(reader);
            String xmlSample = this.jniReaderReadNext(xmlEntity,
                                                        Long.toString(instanceGID.getLocalId()),
                                                        Long.toString(instanceGID.getSystemId()));
            this.checkConnection();
            result = untypedSampleDeserializer.deserializeSample(xmlSample, reader.getDataType());
        } catch (CMException e) {
            throw new CommunicationException(e.getMessage());
        } catch (TransformationException e) {
            throw new CommunicationException("Could not read next sample.");
        }
        return result;
    }
    
    public void entityFree(Entity entity) throws CommunicationException {
        String xmlEntity;
        this.checkConnection();
        
        try {
            xmlEntity = entitySerializer.serializeEntity(entity);
        } catch (TransformationException e) {
            throw new CommunicationException(e.getMessage());
        }
        this.jniEntityFree(xmlEntity);
        this.checkConnection();
    }
        
    public Participant participantNew(String uri, int timeout, String name, ParticipantQoS qos) throws CommunicationException {
        Participant participant;
        String xmlQos = null;
        this.checkConnection();
        
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlEntity = this.jniCreateParticipant(uri, timeout, name, xmlQos);
            this.checkConnection();
            participant = (Participant)(entityDeserializer.deserializeEntity(xmlEntity));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create participant");
        }
        return participant;
    }
    
    public Participant[] participantAllParticipants(Participant p) throws CommunicationException {
        Participant[] result = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlEntities = this.jniParticipantAllParticipants(xmlEntity);
            this.checkConnection();
            Entity[] entityResult = entityDeserializer.deserializeEntityList(xmlEntities);
            
            if(entityResult != null){
                result = new Participant[entityResult.length];
                
                for(int i=0; i<entityResult.length; i++){
                    result[i] = (Participant)(entityResult[i]);
                }
            } else {
                throw new CommunicationException("Could not resolve all participants");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve all participants");
        }
        return result;
    }
    
    public Topic[] participantAllTopics(Participant p) throws CommunicationException {
        Topic[] result = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlEntities = this.jniParticipantAllTopics(xmlEntity);
            this.checkConnection();
            Entity[] entityResult = entityDeserializer.deserializeEntityList(xmlEntities);
                        
            if(entityResult != null){
                result = new Topic[entityResult.length];
                
                for(int i=0; i<entityResult.length; i++){
                    result[i] = (Topic)(entityResult[i]);
                }
            } else {
                throw new CommunicationException("Could not resolve all topics");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve all topics");
        }   
        return result;
    }
   
    public Partition[] participantAllDomains(Participant p) throws CommunicationException {
        Partition[] result = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlEntities = this.jniParticipantAllDomains(xmlEntity);
            this.checkConnection();
            Entity[] entityResult = entityDeserializer.deserializeEntityList(xmlEntities);
            
            if(entityResult != null){
                result = new Partition[entityResult.length];
                
                for(int i=0; i<entityResult.length; i++){
                    result[i] = (Partition)(entityResult[i]);
                }
            } else {
                throw new CommunicationException("Could not resolve all partitions");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve all partitions");
        }
        return result;
    }
    
    public Topic[] participantFindTopic(Participant participant, String topicName) throws CommunicationException {
        Topic[] result = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(participant);
            String xmlEntities = this.jniParticipantFindTopic(xmlEntity, topicName);
            this.checkConnection();
            Entity[] entityResult = entityDeserializer.deserializeEntityList(xmlEntities);
            
            if(entityResult != null){
                result = new Topic[entityResult.length];
                
                for(int i=0; i<entityResult.length; i++){
                    result[i] = (Topic)(entityResult[i]);
                }
            } else {
                throw new CommunicationException("Could not find topic.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not find topic.");
        }
        return result;    
    }
    
    public ServiceState serviceGetState(Service service) throws CommunicationException {
        ServiceState state;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(service);
            String xmlState = this.jniGetServiceState(xmlEntity);
            this.checkConnection();
            state = (ServiceState)(entityDeserializer.deserializeEntity(xmlState));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve service state.");
        }
        return state;
    }
    
    public Status entityGetStatus(Entity entity) throws CommunicationException {
        Status status;
        this.checkConnection();
        
        try{ 
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String xmlStatus = this.jniEntityStatus(xmlEntity);
            this.checkConnection();
            status = statusDeserializer.deserializeStatus(xmlStatus);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve entity status.");
        }
        return status;
    }
    
    public ReaderSnapshot readerSnapshotNew(Reader reader) throws CommunicationException{
        ReaderSnapshot rs = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(reader);
            String xmlSnapshot = this.jniReaderSnapshotNew(xmlEntity);
            this.checkConnection();
            rs = snapshotDeserializer.deserializeReaderSnapshot(xmlSnapshot, reader);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create a snapshot of the supplied reader.");
        }
        return rs;
    }
    
    public WriterSnapshot writerSnapshotNew(Writer writer) throws CommunicationException{
        WriterSnapshot rs = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlSnapshot = this.jniWriterSnapshotNew(xmlEntity);
            this.checkConnection();
            
            rs = snapshotDeserializer.deserializeWriterSnapshot(xmlSnapshot, writer);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create a snapshot of the supplied writer.");
        }
        return rs;
    }
    
    public void snapshotFree(Snapshot snapshot) throws CommunicationException{
        try{
            String xmlSnapshot = snapshotSerializer.serializeSnapshot(snapshot);
            this.jniSnapshotFree(xmlSnapshot);
            this.checkConnection();
        } catch (TransformationException e) {
            throw new CommunicationException("Could not free snapshot.");
        }
    }
    
    public Sample snapshotRead(Snapshot snapshot) throws CommunicationException, DataTypeUnsupportedException{
        Sample s = null;
        this.checkConnection();
        
        try{
            String xmlSnapshot = snapshotSerializer.serializeSnapshot(snapshot);
            String xmlSample = this.jniSnapshotRead(xmlSnapshot);
            this.checkConnection();
            s = untypedSampleDeserializer.deserializeSample(xmlSample, snapshot.getUserDataType());
            
        } catch (TransformationException e) {
            throw new CommunicationException("Could not read from snapshot.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        }  
        return s;
    }
    
    public Sample snapshotTake(Snapshot snapshot) throws CommunicationException, DataTypeUnsupportedException{
        Sample s = null;
        this.checkConnection();
        
        try{
            String xmlSnapshot = snapshotSerializer.serializeSnapshot(snapshot);
            String xmlSample = this.jniSnapshotTake(xmlSnapshot);
            this.checkConnection();
            s = untypedSampleDeserializer.deserializeSample(xmlSample, snapshot.getUserDataType());
        } catch (TransformationException e) {
            throw new CommunicationException("Could not take from snapshot.");
        } catch (CMException ce) {
            throw new CommunicationException(ce.getMessage());
        } 
        return s;
    }
    
    public MetaType writerGetDataType(Writer writer) throws CommunicationException, DataTypeUnsupportedException {
        MetaType result = null;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlType = this.jniWriterDataType(xmlEntity);
            this.checkConnection();
            result = typeDeserializer.deserializeMetaType(xmlType);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve topic data type of writer.");
        }
        return result;
    }

    public void writerWrite(Writer writer, UserData data) throws CommunicationException {
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            String success = this.jniWriterWrite(xmlEntity, xmlData);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(success))){
                throw new CommunicationException("Write failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not write data with suppplied writer.");
        }
    }
    
    public void writerDispose(Writer writer, UserData data) throws CommunicationException{
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            String success = this.jniWriterDispose(xmlEntity, xmlData);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(success))){
                throw new CommunicationException("Dispose failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not dispose data with suppplied writer.");
        }
    }
    
    public void writerWriteDispose(Writer writer, UserData data) throws CommunicationException{
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            String success = this.jniWriterWriteDispose(xmlEntity, xmlData);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(success))){
                throw new CommunicationException("WriteDispose failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not writeDispose data with suppplied writer.");
        }
    }
    
    public void writerRegister(Writer writer, UserData data) throws CommunicationException{
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            String success = this.jniWriterRegister(xmlEntity, xmlData);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(success))){
                throw new CommunicationException("Register failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not register instance with suppplied writer.");
        }
    }
    
    public void writerUnregister(Writer writer, UserData data) throws CommunicationException{
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(writer);
            String xmlData = userDataSerializer.serializeUserData(data);
            String success = this.jniWriterUnregister(xmlEntity, xmlData);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(success))){
                throw new CommunicationException("Unregister failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not unregister instance with suppplied writer.");
        }
    }
    
    public Publisher publisherNew(Participant p, String name, PublisherQoS qos) throws CommunicationException {
        Publisher pub;
        String xmlQos = null;
        this.checkConnection();
        
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlPub = this.jniPublisherNew(xmlEntity, name, xmlQos);
            this.checkConnection();
            pub = (Publisher)(entityDeserializer.deserializeEntity(xmlPub));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create publisher.");
        }
        return pub;
    }

    public Subscriber subscriberNew(Participant p, String name, SubscriberQoS qos) throws CommunicationException {
        Subscriber sub;
        String xmlQos = null;
        this.checkConnection();
        
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlSub = this.jniSubscriberNew(xmlEntity, name, xmlQos);
            this.checkConnection();
            sub = (Subscriber)(entityDeserializer.deserializeEntity(xmlSub));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create subscriber.");
        }
        return sub;
    }

    public Partition partitionNew(Participant p, String name) throws CommunicationException {
        Partition dom;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlDom = this.jniDomainNew(xmlEntity, name);
            this.checkConnection();
            dom = (Partition)(entityDeserializer.deserializeEntity(xmlDom));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create partition.");
        }
        return dom;
    }

    public void publisherPublish(Publisher p, String expression) throws CommunicationException {
        this.checkConnection();
        
        try{ 
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlResult = this.jniPublisherPublish(xmlEntity, expression);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Publication failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Publication failed");
        }
        
    }

    public void subscriberSubscribe(Subscriber s, String expression) throws CommunicationException {
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(s);
            String xmlResult = this.jniSubscriberSubscribe(xmlEntity, expression);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(xmlResult))){
                throw new CommunicationException("Subscription failed.");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Subscription failed.");
        }
    }

    public Writer writerNew(Publisher p, String name, Topic t, WriterQoS qos) throws CommunicationException {
        Writer wri;
        String xmlQos = null;
        this.checkConnection();
        
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlTopic = entitySerializer.serializeEntity(t);
            String xmlResult = this.jniCreateWriter(xmlEntity, name, xmlTopic, xmlQos);
            this.checkConnection();
            wri = (Writer)(entityDeserializer.deserializeEntity(xmlResult));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create writer.");
        }
        return wri;
    }

    public DataReader dataReaderNew(Subscriber s, String name, String viewExpression, ReaderQoS qos) throws CommunicationException {
        DataReader dr;
        String xmlQos = null;
        this.checkConnection();
        
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlSub = entitySerializer.serializeEntity(s);
            String xmlResult = this.jniCreateDataReader(xmlSub, name, viewExpression, xmlQos);
            this.checkConnection();
            dr = (DataReader)(entityDeserializer.deserializeEntity(xmlResult));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create datareader.");
        }
        return dr;
    }
    
    public void dataReaderWaitForHistoricalData(DataReader d, Time time) throws CommunicationException {
        this.checkConnection();
        
        try{
            String xmlDataReader = entitySerializer.serializeEntity(d);
            String result = this.jniDataReaderWaitForHistoricalData(xmlDataReader, time.sec, time.nsec);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException("Wait for historical data failed: " + result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not wait for historical data.");
        }
    }

    public Query queryNew(Reader source, String name, String expression) throws CommunicationException {
        Query q;
        this.checkConnection();
        
        try{
            String xmlRea = entitySerializer.serializeEntity(source);
            String xmlResult = this.jniQueryNew(xmlRea, name, expression);
            this.checkConnection();
            q = (Query)(entityDeserializer.deserializeEntity(xmlResult));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create query.");
        }
        return q;
    }

    public Topic topicNew(Participant p, String name, String typeName, String keyList, TopicQoS qos) throws CommunicationException {
        Topic top;
        String xmlQos = null;
        this.checkConnection();
        
        try{
            if(qos != null){
                xmlQos = qosSerializer.serializeQoS(qos);
            }
            String xmlEntity = entitySerializer.serializeEntity(p);
            String xmlTop = this.jniTopicNew(xmlEntity, name, typeName, keyList, xmlQos);
            this.checkConnection();
            top = (Topic)(entityDeserializer.deserializeEntity(xmlTop));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create topic.");
        }
        return top;
    }
    
    public Waitset waitsetNew(Participant participant) throws CommunicationException {
        Waitset waitset = null;
        
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(participant);
            String xmlWaitset = this.jniWaitsetNew(xmlEntity);
            this.checkConnection();
            waitset = (Waitset)(entityDeserializer.deserializeEntity(xmlWaitset));
        } catch (TransformationException e) {
            throw new CommunicationException("Could not create waitset.");
        }
        return waitset;
    }
    
    public QoS entityGetQoS(Entity entity) throws CommunicationException {
        QoS result;
        this.checkConnection();
        
        try{
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String xmlQoS = this.jniEntityQoS(xmlEntity);
            this.checkConnection();
            result = qosDeserializer.deserializeQoS(xmlQoS);
        } catch (TransformationException e) {
            throw new CommunicationException("Could not resolve qos.");
        }
        return result;
        
    }
    
    public void entitySetQoS(Entity entity, QoS qos) throws CommunicationException {
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String xmlQoS = qosSerializer.serializeQoS(qos);
            String result = this.jniEntitySetQoS(xmlEntity, xmlQoS);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Applying new qos failed.");
        }
    }
    
    public void participantRegisterType(Participant participant, MetaType type) throws CommunicationException{
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(participant);
            String xmlType = type.toXML();
            String result = this.jniRegisterType(xmlEntity, xmlType);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not register type.");
        }
    }
    
    public Statistics entityGetStatistics(Entity entity) throws CommunicationException {
        Statistics statistics = null;
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String result = this.jniEntityGetStatistics(xmlEntity);
            this.checkConnection();
            
            if(result != null){
                statistics = statisticsDeserializer.deserializeStatistics(result, entity);
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not reset statistics.");
        }
        return statistics;
    }

    public void entityResetStatistics(Entity entity, String fieldName) throws CommunicationException {
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String result = this.jniEntityResetStatistics(xmlEntity, fieldName);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not reset statistics.");
        }
    }
    
    public void entityEnable(Entity entity) throws CommunicationException {
        this.checkConnection();
        
        try {
            String xmlEntity = entitySerializer.serializeEntity(entity);
            String result = this.jniEntityEnable(xmlEntity);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not enable entity.");
        }
    }
    
    public void waitsetAttach(Waitset waitset, Entity entity) throws CommunicationException {
        this.checkConnection();
        
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String xmlEntity  = entitySerializer.serializeEntity(entity);
            String result = this.jniWaitsetAttach(xmlWaitset, xmlEntity);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not attach Entity to Waitset.");
        }
        return;
    }
    
    public void waitsetDetach(Waitset waitset, Entity entity) throws CommunicationException {
        this.checkConnection();
        
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String xmlEntity  = entitySerializer.serializeEntity(entity);
            String result = this.jniWaitsetDetach(xmlWaitset, xmlEntity);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not detach Entity from Waitset.");
        }
        return;
    }
    
    public Entity[] waitsetWait(Waitset waitset) throws CommunicationException{
        Entity[] result = null;
        this.checkConnection();
        
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String xmlEntities = this.jniWaitsetWait(xmlWaitset);
            this.checkConnection();
            
            Entity[] entityResult = entityDeserializer.deserializeEntityList(xmlEntities);
            
            if(entityResult != null){
                result = new Entity[entityResult.length];
                
                for(int i=0; i<entityResult.length; i++){
                    result[i] = entityResult[i];
                }
            } else {
                throw new CommunicationException("waitsetWait failed (2)");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("waitsetWait failed");
        }
        return result;
    }
    
    public Entity[] waitsetTimedWait(Waitset waitset, Time time) throws CommunicationException {
        Entity[] result = null;
        this.checkConnection();
        
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String xmlEntities = this.jniWaitsetTimedWait(xmlWaitset, time.sec, time.nsec);
            this.checkConnection();
            
            Entity[] entityResult = entityDeserializer.deserializeEntityList(xmlEntities);
            
            if(entityResult != null){
                result = new Entity[entityResult.length];
                
                for(int i=0; i<entityResult.length; i++){
                    result[i] = entityResult[i];
                }
            } else {
                throw new CommunicationException("waitsetTimedWait failed (2)");
            }
        } catch (TransformationException e) {
            throw new CommunicationException("waitsetTimedWait failed");
        }
        return result;
    }
    
    public int waitsetGetEventMask(Waitset waitset) throws CommunicationException {
        int result = Event.UNDEFINED;
        this.checkConnection();
        
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            result = this.jniWaitsetGetEventMask(xmlWaitset);
            this.checkConnection();
            
        } catch (TransformationException e) {
            throw new CommunicationException("Could not attach Entity to Waitset.");
        }
        return result;
    }

    public void waitsetSetEventMask(Waitset waitset, int mask) throws CommunicationException {
        this.checkConnection();
        
        try{
            String xmlWaitset = entitySerializer.serializeEntity(waitset);
            String result = this.jniWaitsetSetEventMask(xmlWaitset, mask);
            this.checkConnection();
            
            if(!("<result>OK</result>".equals(result))){
                throw new CommunicationException(result.substring(8, result.length() -9));
            }
        } catch (TransformationException e) {
            throw new CommunicationException("Could not attach Entity to Waitset.");
        }
    }
    
    private void checkConnection() throws CommunicationException{
        if(this.initialized && !JniCommunicator.connectionAlive){
            throw new ConnectionLostException();
        }
    }
    
    
    /*Init/detach.*/
    private native String   jniInitialise();
    private native String   jniDetach();
    
    /*Entity functions.*/
    private native void     jniEntityFree(String xmlEntity);
    private native String   jniEntityStatus(String xmlEntity);
    private native String   jniEntityQoS(String xmlEntity);
    private native String   jniEntitySetQoS(String xmlEntity, String xmlQoS);
    private native String   jniGetOwnedEntities(String xmlEntity, String filter);
    private native String   jniGetDependantEntities(String xmlEntity, String filter);
    private native String   jniEntityResetStatistics(String xmlEntity, String fieldName);
    private native String   jniEntityGetStatistics(String xmlEntity);
    private native String   jniEntityEnable(String xmlEntity);
    
    /*Participant functions.*/
    private native String   jniCreateParticipant(String kernelURI, int timeout, String name, String qos);
    private native String   jniParticipantAllParticipants(String xmlParticipant);
    private native String   jniParticipantAllTopics(String xmlTopic);
    private native String   jniParticipantAllDomains(String xmlDomain);
    private native String   jniParticipantFindTopic(String xmlParticipant, String topicName);
    private native String   jniRegisterType(String participant, String type);
    
    private native String   jniPublisherNew(String participant, String name, String qos);
    private native String   jniSubscriberNew(String participant, String name, String qos);
    private native String   jniDomainNew(String participant, String name);
    private native String   jniTopicNew(String participant, String name, String typeName, String keyList, String qos);
        
    /*Topic functions.*/
    private native String   jniGetTopicDataType(String xmlEntity);
    
    /*Service functions.*/
    private native String   jniGetServiceState(String xmlService);
    
    /*Reader functions.*/
    private native String   jniReaderSnapshotNew(String reader);
    private native String   jniReaderDataType(String reader);
    private native String   jniReaderTake(String reader);
    private native String   jniReaderRead(String reader);
    private native String   jniReaderReadNext(String reader, String localId, String systemId);
    private native String   jniQueryNew(String reader, String name, String expression);
    
    /*Snapshot functions.*/
    private native void     jniSnapshotFree(String snapshot);
    private native String   jniSnapshotRead(String snapshot);
    private native String   jniSnapshotTake(String snapshot);

    /*Writer functions. */
    private native String   jniWriterSnapshotNew(String writer);
    private native String   jniWriterDataType(String writer);
    private native String   jniWriterWrite(String writer, String data);
    private native String   jniWriterDispose(String writer, String data);
    private native String   jniWriterWriteDispose(String writer, String data);
    private native String   jniWriterRegister(String writer, String data);
    private native String   jniWriterUnregister(String writer, String data);
    
    /*Publisher functions*/
    private native String   jniPublisherPublish(String publisher, String expr);
    private native String   jniCreateWriter(String publisher, String name, String topic, String qos);
    
    /*Subscriber functions*/
    private native String   jniSubscriberSubscribe(String subscriber, String expr);
    private native String   jniCreateDataReader(String subscriber, String name, String view, String qos);
 
    /*Waitset functions*/
    private native String   jniWaitsetNew(String participant);
    private native String   jniWaitsetAttach(String waitset, String entity);
    private native String   jniWaitsetDetach(String waitset, String entity);
    private native String   jniWaitsetWait(String waitset);
    private native String   jniWaitsetTimedWait(String waitset, int sec, int nsec);    
    private native int      jniWaitsetGetEventMask(String waitset);
    private native String   jniWaitsetSetEventMask(String waitset, int mask);
    
    private native String   jniDataReaderWaitForHistoricalData(String dataReader, int sec, int nsec);
}
