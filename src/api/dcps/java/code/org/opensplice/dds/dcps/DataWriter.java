package org.opensplice.dds.dcps;

import org.opensplice.dds.dcps.jni.DCPSCommunicationHandler;
import org.opensplice.dds.dcps.policy.*;


/**@class DataWriter
 * @brief The DataWriter object allows the application to set the value of the data
 * to be published under a given Topic.
 *  
 * - A DataWriter is attached to exactly one Publisher which acts as a factory for it.
 * - A DataWriter is bound to exactly one Topic and therefore to exactly one data type.
 * - The Topic must exist prior to creating the DataWriter. 
 */
public class DataWriter extends Entity {
    private Publisher publisher;    //!<The associated Publisher.
    private Topic topic;            //!<The associated Topic.
    
    /**@brief Creates a new DataWriter.
     * 
     * @param _publisher The associated Publisher
     * @param _topic The associated Topic
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataWriter.
     */
    protected DataWriter(   Publisher _publisher,
                            Topic _topic,
                            DataWriterQos _qos,
                            DataWriterListener _listener){
        publisher = _publisher;
        topic = _topic;
        qos = _qos;
        listener = _listener;
    }
    
    /**@brief Returns the associated Topic.
     * 
     * @return Returns the associated Topic. This is the same Topic that is used to 
     * create the DataWriter.
     */
    public Topic get_topic(){
        return topic;
    }
    
    /**@brief Returns the associated Publisher.
     * 
     * @return Returns the associated Publisher to which the DataWriter belongs.
     */
    public Publisher get_publisher(){
        return publisher;
    }
    
    /**@brief Writes the supplied XML userdata.
     * 
     * @param userData The XML data to write.
     * @return ReturnCode.OK if succeeded, any other ReturnCode otherwise.
     */
    public ReturnCode write(String userData){
        DCPSCommunicationHandler ch = 
                    DomainParticipantFactory.get_instance().getCommunicationHandler();
        return ReturnCode.from_int(ch.writer_write(this, userData));
    }
}
