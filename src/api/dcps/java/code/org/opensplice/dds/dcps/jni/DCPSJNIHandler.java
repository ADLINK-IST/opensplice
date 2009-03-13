package org.opensplice.dds.dcps.jni;

import org.opensplice.dds.dcps.*;
import org.opensplice.dds.dcps.policy.*;


/**@class DCPSJNIHandler
 * @brief The JNI implementation of the communication handler.
 * 
 * This object contains native function calls, that have been
 * implemented in C.
 */
public class DCPSJNIHandler implements DCPSCommunicationHandler{
    
    /**@brief Creates a new communication handler by loading the 
     * dynamicly linked C library
     */
    public DCPSJNIHandler(){
        System.loadLibrary("dcpsjni");
    }
    
    /**@brief This operation initializes the DomainParticipantFactory
     * 
     * @return 0 if succeeded an integer from 1 to 7 otherwise.
     */
    public int init_participant_factory(){
        return jniGetParticipantFactoryInstance();
    }
    
    /**@brief This operation deinitializes the DomainParticipantFactory
     * 
     * @return 0 if succeeded an integer from 1 to 7 otherwise.
     */
    public int delete_participant_factory(){
        return jniDeleteParticipantFactoryInstance();
    }
    
    /**@brief This operation creates a DomainParticipant
     * 
     * @param _dfp The DomainParticipantFactory singleton instance.
     * @param _domainId The domainId to participate in.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DomainParticipant.
     * 
     * @return The newly created DomainParticipant. In case of failure, null is returned.
     */
    public DomainParticipant create_participant(DomainParticipantFactory _dfp, 
                                                long _domainId, 
                                                DomainParticipantQos _qos, 
                                                DomainParticipantListener _listener){
        return jniCreateParticipant(_dfp, _domainId, _qos, _listener);
    }
    
    /**@brief Deletes the DomainParticipant
     * 
     * @param dp The DomainParticipant to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */   
    public int delete_participant(DomainParticipant dp){
        return jniDeleteParticipant(dp); 
    }
    
    /**@brief Creates a Publisher
     * 
     * @param _p The DomainParticipant that holds the Publisher.
     * @param _qos The desired QoS policies.
     * @param _listener The PublisherListener that will be attached to the Publisher.
     * 
     * @return The newly created Publisher. If the operation fails it returns null.
     */ 
    public Publisher create_publisher(  DomainParticipant _p, 
                                        PublisherQos _qos, 
                                        PublisherListener _listener){
        return jniCreatePublisher(_p, _qos, _listener);                                
    }
    
    /**@brief Deletes a Publisher.
     * 
     * @param _pub The Publisher to delete.
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */ 
    public int delete_publisher(Publisher _pub){
        return jniDeletePublisher(_pub);
    }
    
    /**@brief Creates a Subscriber.
     * 
     * @param _p The DomainParticipant that holds the Subscriber.
     * @param _qos The QoS policies.
     * @param _listener The listener that will be attached to the Subscriber. 
     * 
     * @return The newly created Subscriber. If the operation fails, null is returned.
     */
    public Subscriber create_subscriber(    DomainParticipant _p, 
                                            SubscriberQos _qos, 
                                            SubscriberListener _listener){
        return jniCreateSubscriber(_p, _qos, _listener);
    }
    
    /**@brief Deletes a Subscriber.
     * 
     * @param _subscriber The Subscriber to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */
    public int delete_subscriber(Subscriber _subscriber){
        return jniDeleteSubscriber(_subscriber);
    }
    
    /**@brief Creates a Topic.
     * 
     * @param _p The DomainParticipant that holds the Topic.
     * @param _name The name of the Topic.
     * @param _typeName The name of the type of the Topic.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the Topic.
     * 
     * @return The newly created Topic. If the operation fails, null is returned.
     */
    public Topic create_topic(  DomainParticipant _p, 
                                String _name, 
                                String _typeName, 
                                TopicQos _qos, 
                                TopicListener _listener){
        return jniCreateTopic(_p, _name, _typeName, _qos, _listener);                                
    }
    
    /**@brief Deletes a Topic.
     * 
     * @param _t The Topic to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */         
    public int delete_topic(Topic _t){
        return jniDeleteTopic(_t);
    }
    
    /**@brief Creates a DataWriter.
     * 
     * @param _p The Publisher that holds the DataWriter.
     * @param _topic The Topic where the DataWriter writes instances of.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataWriter.
     * 
     * @return The newly created DataWriter. If the operation fails, null is returned.
     */
    public DataWriter create_datawriter(Publisher _p, 
                                        Topic _topic, 
                                        DataWriterQos _qos, 
                                        DataWriterListener _listener){
        return jniCreateDataWriter(_p, _topic, _qos, _listener);                                        
    }
    
    /**@brief Deletes a DataWriter.
     * 
     * @param _writer The DataWriter to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */      
    public int delete_datawriter(DataWriter _writer){
        return jniDeleteDataWriter(_writer);
    }
    
    /**@brief Creates a DataReader.
     * 
     * @param _s The Subscriber that holds the DataReader.
     * @param _td The associated TopicDescription.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataReader.
     * 
     * @return The newly created DataWriter. If the operation fails, null is returned.
     */
    public DataReader create_datareader(    Subscriber _s,
                                            TopicDescription _td,
                                            DataReaderQos _qos,
                                            DataReaderListener _listener){
        return jniCreateDataReader(_s, _td, _qos, _listener);
    }
    
    /**@brief Deletes a DataReader.
     * 
     * @param _reader The DataReader to delete.
     * 
     * @return 0 if succeeeded, an integer from 1 to 7 otherwise.
     */
    public int delete_datareader(DataReader _reader){
        return jniDeleteDataReader(_reader);
    }
    
    /**@brief Recursively deletes all contained entities of a DomainParticipant.
     * 
     * @param _p The DomainParticipant where to delete all the contained entities from.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */
    public int delete_participant_entities(DomainParticipant _p){
        return jniDeleteParticipantEntities(_p);
    }
    
    /**@brief Reads one sample from the supplied DataReader in XML format.
     * 
     * @param _r The DataReader to read from.
     * @see org.opensplice.dds.dcps.jni.DCPSCommunicationHandler#reader_read(org.opensplice.dds.dcps.DataReader)
     * @return The XML representation of the read sample.
     */
    public String reader_read(DataReader _r){
        return jniReaderRead(_r);
    }
    
    /**@brief Takes one sample from the supplied DataReader in XML format.
     * 
     * @param _r The DataReader to take from.
     * @see org.opensplice.dds.dcps.jni.DCPSCommunicationHandler#reader_read(org.opensplice.dds.dcps.DataReader)
     * @return The XML representation of the taken sample.
     */
    public String reader_take(DataReader _r){
        return jniReaderTake(_r);
    }
    
    /**@brief Writes one userData object in the system.
     * 
     * The provided object must be specified in XML.
     * 
     * @param _w The DataWriter to write to.
     * @param _xmlUserData The XML representation of the data to write.
     * @return 0 if succeeded, any integer from 1 to 7 otherwise 
     * @see org.opensplice.dds.dcps.jni.DCPSCommunicationHandler#writer_write(org.opensplice.dds.dcps.DataWriter, java.lang.String)
     */
    public int writer_write(DataWriter _w, String _xmlUserData){
        return jniWriterWrite(_w, _xmlUserData);
    }
    
    public int create_querycondition(DataReader _r, String query_expression, String[] query_args) {
        return jniCreateQueryCondition(_r, query_expression, query_args);
    }
    
    /* Native function declarations. These functions are implemented in the
     * C language. By declaring them here the C library that contains these
     * functions can be accessed. The 'javah' utility will be used to
     * generate header files for the C language. Functions will look 
     * like: 'Java_packageName_className_functionName'
     * 
     * For example: the function 'jniCreateParticipant' will become: 
     * 'Java_splice2v3_api_communication_JNIHandler_jniCreateParticipant'
     */
    private native int jniGetParticipantFactoryInstance();
    
    private native int jniDeleteParticipantFactoryInstance();
 
    private native DomainParticipant jniCreateParticipant(  DomainParticipantFactory dfp,
                                                            long domainId, 
                                                            DomainParticipantQos qos, 
                                                            DomainParticipantListener listener);
                                                        
    private native int jniDeleteParticipant(                DomainParticipant dp);
    
    private native Publisher jniCreatePublisher(            DomainParticipant dp, 
                                                            PublisherQos qos, 
                                                            PublisherListener listener);
    
    private native int jniDeletePublisher(                  Publisher p);
    
    private native Subscriber jniCreateSubscriber(          DomainParticipant dp, 
                                                            SubscriberQos qos,
                                                            SubscriberListener listener);
    
    private native int jniDeleteSubscriber(                 Subscriber _subscriber);
    
    private native Topic jniCreateTopic(                    DomainParticipant _p, 
                                                            String _name, 
                                                            String _typeName, 
                                                            TopicQos _qos, 
                                                            TopicListener _listener);

    private native int jniDeleteTopic(                      Topic t);                                                                
    
    private native DataWriter jniCreateDataWriter(          Publisher _p, 
                                                            Topic _topic, 
                                                            DataWriterQos _qos, 
                                                            DataWriterListener _listener);
                                                                
    private native int jniDeleteDataWriter(                 DataWriter _writer);
    
    private native DataReader jniCreateDataReader(          Subscriber _s, 
                                                            TopicDescription _td, 
                                                            DataReaderQos _qos, 
                                                            DataReaderListener _listener);
    
    private native int jniDeleteDataReader(                 DataReader _reader);
    
    private native int jniDeleteParticipantEntities(        DomainParticipant _p);
    
    private native String jniReaderRead(                    DataReader _r);
    
    private native String jniReaderTake(                    DataReader _r);
    
    private native int jniWriterWrite(                      DataWriter _w,
                                                            String _xmlSample);
                                                            
    private native int jniCreateQueryCondition(             DataReader _r, 
                                                            String query_expression, 
                                                            String[] query_args);
}
