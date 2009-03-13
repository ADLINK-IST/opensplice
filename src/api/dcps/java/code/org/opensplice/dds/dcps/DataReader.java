/**@package org.opensplice.dds.dcps
 * @brief Provides the functionality required for an application to 
 * publish and subscribe to the values of data objects.
 * 
 * It allows:
 * - Publishing applications to identify the data objects they intend to
 *   publish, and then provide values for these objects.
 * - Subscribing applications to identiy which data objects they are
 *   interested in, and then access their data values.
 * - Applications to:
 *      -# Define topics
 *      -# Attach type information to the topics
 *      -# Create publisher and subscriber entities
 *      -# Attach QoS policies to all these entities
 *      -# Make all these entities operate.
 */
package org.opensplice.dds.dcps;

import org.opensplice.dds.dcps.jni.*;
import org.opensplice.dds.dcps.policy.*;


/**@class DataReader
 * @brief DCPS DataReader.
 * 
 * A DataReader allows an application to:
 * -# Declare the data it wishes to recieve(i.e. make a subscription).
 * -# Access the data received by the attached Subscriber.
 */
public class DataReader extends Entity {
    private Subscriber subscriber;              /*!<The Subscriber the DataReader belongs to.*/
    private TopicDescription topicDescription;  /*!<The associated TopicDescription.*/
    
    /**@brief Creates a new DataReader.
     * 
     * @param _subscriber The Subscriber the DataReader belongs to.
     * @param _topicDescription The TopicDescription associated with the DataReader.
     * @param _qos The QoS policies.
     * @param _listener The listener that will be attached to the DataReader.
     */
    protected DataReader(   Subscriber _subscriber,
                            TopicDescription _topicDescription,
                            DataReaderQos _qos,
                            DataReaderListener _listener){
        subscriber          = _subscriber;
        topicDescription    = _topicDescription;
        qos                 = _qos;
        listener            = _listener;
    }
    
    /**@brief Returns the Subscriber the DataReader belongs to.
     * 
     * @return The Subscriber the DataReader belongs to.
     */
    public Subscriber get_subscriber(){
        return subscriber;
    }
    
    /**@brief Returns the TopicDescription associated with the DataReader.
     * 
     * @return The TopicDescription associated with the DataReader. This is the
     * same TopicDescription used to create the DataReader.
     */
    public TopicDescription get_topic_description(){
        return topicDescription;
    }
    
    /**@brief Reads a sample from the reader in XML format.
     * 
     * @return The read sample, or null if no sample was available.
     */
    public String readXML(){
        DCPSCommunicationHandler ch = 
            DomainParticipantFactory.get_instance().getCommunicationHandler();
        return ch.reader_read(this);
    }
    
    /**@brief Takes a sample from the reader in XML format.
     * 
     * The sample is read and removed from the reader database.
     * 
     * @return The taken sample, or null if no sample was available.
     */
    public String takeXML(){
        DCPSCommunicationHandler ch = 
            DomainParticipantFactory.get_instance().getCommunicationHandler();
        return ch.reader_take(this);
    }
    
    /**Creates a query condition for the DataReader.
     * 
     * Currently only one QueryCondition can be attached to one DataReader and 
     * once it is created it cannot e removed. Each read and take action
     * will result in a read/take with the attached QueryCondition.
     *  
     * @param sample_states Array of sample states.
     * @param lifecycle_states Array of lifcycle states.
     * @param query_expression The query expression in OQL format.
     * @param query_parameters The query parameters of the query.
     * @return The newly attached QueryCondition.
     */
    public QueryCondition create_querycondition(String[] sample_states, 
                                                String[] lifecycle_states,
                                                String query_expression,
                                                String[] query_parameters){
        QueryCondition condition = null;
        
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
        int result = ch.create_querycondition(this, query_expression, query_parameters);
        
        if(result == 0){
            condition = new QueryCondition(query_expression, query_parameters);
        }
        return condition;
    }
}
