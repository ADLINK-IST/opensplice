package org.opensplice.dds.dcps.jni;

import org.opensplice.dds.dcps.*;
import org.opensplice.dds.dcps.policy.*;


/**@interface DCPSCommunicationHandler
 * @brief Abstract communication interface. The DCPS API uses this interface to communicate with
 * other Splice2v3 components. 
 * 
 * Each communication type must implement this interface. By
 * using this interface it is possible to plug in multiple communication protocols without
 * changing the DCPS API. At this time only JNI communication is supported by the 
 * DCPSJNIHandler class. 
 * 
 * This interface should only be used by the DCPS API and is not to be used as an API for
 * user applications or other Splice components. This is because no integretity or input
 * validation is available in this interface, nor in its implementing classes.
 * 
 * Some of the functions return an integer. This integer maps to the integers in 
 * the splice2v3.dcps.api.ReturnCode class.
 */
public abstract interface DCPSCommunicationHandler{
    
    /**@brief This operation initializes the DomainParticipantFactory.
     * 
     * @return 0 if succeeded an integer from 1 to 7 otherwise.
     */
    public int init_participant_factory();
    
    /**@brief This operation deinitializes the DomainParticipantFactory.
     * 
     * @return 0 if succeeded an integer from 1 to 7 otherwise.
     */
    public int delete_participant_factory();
    
    /**@brief This operation creates a DomainParticipant.
     * 
     * @param _dfp The DomainParticipantFactory singleton instance.
     * @param _domainId The domainId to participate in.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DomainParticipant.
     * 
     * @return The newly created DomainParticipant. In case of failure, null is returned.
     */
    public DomainParticipant create_participant(    DomainParticipantFactory _dfp, 
                                                    long _domainId, 
                                                    DomainParticipantQos _qos, 
                                                    DomainParticipantListener _listener);
                                                    
    /**@brief Deletes the DomainParticipant.
     * 
     * @param _dp The DomainParticipant to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */                                                        
    public int delete_participant(                  DomainParticipant _dp);
    
    /**@brief Creates a Publisher.
     * 
     * @param _p The DomainParticipant that holds the Publisher.
     * @param _qos The desired QoS policies.
     * @param _listener The PublisherListener that will be attached to the Publisher.
     * 
     * @return The newly created Publisher. If the operation fails it returns null.
     */       
    public Publisher create_publisher(              DomainParticipant _p,
                                                    PublisherQos _qos, 
                                                    PublisherListener _listener);
    
    /**@brief Deletes a Publisher.
     * 
     * @param _pub The Publisher to delete.
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */                                                    
    public int delete_publisher(                    Publisher _pub);
    
    /**@brief Creates a Subscriber.
     * 
     * @param _p The DomainParticipant that holds the Subscriber.
     * @param _qos The QoS policies.
     * @param _listener The listener that will be attached to the Subscriber. 
     * 
     * @return The newly created Subscriber. If the operation fails, null is returned.
     */
    public Subscriber create_subscriber(            DomainParticipant _p, 
                                                    SubscriberQos _qos,
                                                    SubscriberListener _listener);

    /**@brief Deletes a Subscriber.
     * 
     * @param _subscriber The Subscriber to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */
    public int delete_subscriber(                   Subscriber _subscriber);                                                    
    
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
    public Topic create_topic(                      DomainParticipant _p, 
                                                    String _name, 
                                                    String _typeName, 
                                                    TopicQos _qos, 
                                                    TopicListener _listener);

    /**@brief Deletes a Topic.
     * 
     * @param _t The Topic to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */                                                    
    public int delete_topic(                        Topic _t);
        
    /**@brief Creates a DataWriter.
     * 
     * @param _p The Publisher that holds the DataWriter.
     * @param _topic The Topic where the DataWriter writes instances of.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataWriter.
     * 
     * @return The newly created DataWriter. If the operation fails, null is returned.
     */
    public DataWriter create_datawriter(            Publisher _p,
                                                    Topic _topic, 
                                                    DataWriterQos _qos, 
                                                    DataWriterListener _listener);
                                                    
    /**@brief Deletes a DataWriter.
     * 
     * @param _writer The DataWriter to delete.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */                                                    
    public int delete_datawriter(                   DataWriter _writer);
    
    /**@brief Creates a DataReader.
     * 
     * @param _s The Subscriber that holds the DataReader.
     * @param _td The associated TopicDescription.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataReader.
     * 
     * @return The newly created DataWriter. If the operation fails, null is returned.
     */
    public DataReader create_datareader(            Subscriber _s,
                                                    TopicDescription _td,
                                                    DataReaderQos _qos,
                                                    DataReaderListener _listener);
    
    /**@brief Deletes a DataReader.
     * 
     * @param _reader The DataReader to delete.
     * 
     * @return 0 if succeeeded, an integer from 1 to 7 otherwise.
     */
    public int delete_datareader(                   DataReader _reader);
    
    /**@brief Recursively deletes all contained entities of a DomainParticipant.
     * 
     * @param _p The DomainParticipant where to delete all the contained entities from.
     * 
     * @return 0 if succeeded, an integer from 1 to 7 otherwise.
     */
    public int delete_participant_entities(         DomainParticipant _p);
    
    
    /**@brief Reads one sample from a DataReader.
     * 
     * @param _r The DataReader to read from.
     * @return The XML representation of the read data, or null if no data has been found.
     */
    public String reader_read(                      DataReader _r);
    
    /**@brief Takes one sample from a DataReader.
     * 
     * Reads and removes the data afterwards.
     * 
     * @param _r The DataReader to take from.
     * @return The XML representation of the read data, or null if no data has been found.
     */
    public String reader_take(                      DataReader _r);
    
    /**@brief Writes one instance of userdata into the system.
     * 
     * @param _w The DataWriter to write in.
     * @param _xmlUserData The XML data to write. 
     * @return
     */
    public int writer_write(                        DataWriter _w,
                                                    String _xmlUserData);
                                                    
    public int create_querycondition(               DataReader _r,
                                                    String query_expression,
                                                    String[] query_args);
}
