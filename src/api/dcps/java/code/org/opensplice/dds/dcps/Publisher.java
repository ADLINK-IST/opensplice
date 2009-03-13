package org.opensplice.dds.dcps;

import java.util.HashSet;
import java.util.Iterator;

import org.opensplice.dds.dcps.jni.*;
import org.opensplice.dds.dcps.policy.*;


/**@class Publisher
 * @brief A Publisher is the object responsible for the actual dissemination of publications.
 * 
 * The Publisher acts on behalf of one or several DataWriter objects tha belong to it.
 */
public class Publisher extends Entity{
    private DomainParticipant participant;  /*!<The DomainParticipant of the Publisher.*/
    private HashSet writers;                /*!<The set of DataWriter that belong to the Publisher.*/
    
    /**@brief Creates a new Publisher.
     * 
     * @param _participant The DomainParticipant that will hold the Publisher.
     * @param _qos The desired QoS policies of the Publisher. 
     * @param _listener The listener that will be attached to the Publisher.
     */
    private Publisher(  DomainParticipant _participant,
                        PublisherQos _qos,
                        PublisherListener _listener){
        participant = _participant;                        
        qos         = _qos;
        listener    = _listener;
        writers     = new HashSet();
    }
    
    /**@brief Gives access to all DataWriter entities, that are associated with this Publisher.
     * 
     * @return The set of DataWriter entities.
     */
    HashSet get_datawriters(){
        return writers;
    }
    
    /**@brief This operation creates a DataWriter.
     * 
     * The returned DataWriter will be attached and belongs to this Publisher.
     * 
     * @param _topic The Topic that the DataWriter must write.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataWriter.
     * 
     * @return The newly created DataWriter. In case of failure this operation will
     * return null.
     */
    public DataWriter create_datawriter(Topic _topic, DataWriterQos _qos, DataWriterListener _listener){
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
        DataWriter dw =  ch.create_datawriter(this, _topic, _qos, _listener);
        
        if(dw != null){
            writers.add(dw);
        }
        return dw;
    }
    
    /**@brief This operation deletes a DataWriter that belongs to the Publisher. 
     * 
     * If the DataWriter does not belong to the Publisher, the operation will return 
     * ReturnCode.PRECONDITION_NOT_MET. The delete_datawriter operation must be called on the
     * same Publisher used to create the DataWriter. If delete_datawriter is called on another
     * Publisher, ReturnCode.PRECONDITION_NOT_MET will be returned.
     * 
     * @param _writer The DataWriter to delete.
     * @return ReturnCode.OK if succeeded, any other ReturnCode if failed.
     */
    public ReturnCode delete_datawriter(DataWriter _writer){
        ReturnCode rc;
        /* Check if DataWriter != null */
        if(_writer == null){
            rc = ReturnCode.BAD_PARAMETER;
        }
        /* Check if DataWriter belongs to this Publisher. */
        else if(!(writers.contains(_writer))){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        /* All preconditions met; delete it. */
        else{
            DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
            rc = ReturnCode.from_user_result(ch.delete_datawriter(_writer));
            
            if(rc == ReturnCode.OK){
                writers.remove(_writer);
            }
        }
        return rc;
    }
    
    /**@brief This operation reltrieves a previously created DataWriter belonging to the
     * Publisher that is attache to a Topic with a matching topicName. 
     * 
     * If no such DataWriter exists, the operation will return null.
     * 
     * @param topicName The topicName of the Topic that is associated with the 
     * DataWriter to look up.
     *  
     * @return The associated DataWriter if found or null otherwise. 
     */
    public DataWriter lookup_datawriter(String topicName){
        Topic t;
        DataWriter dw;
        Iterator i = writers.iterator();
        
        while(i.hasNext()){
            dw = (DataWriter)i.next();
            t = dw.get_topic();
            
            if(t.get_name().equals(topicName)){
                return dw;     
            }
        }
        return null;
    }
}

