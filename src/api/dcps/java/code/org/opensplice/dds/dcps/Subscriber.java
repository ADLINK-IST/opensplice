package org.opensplice.dds.dcps;


import java.util.*;

import org.opensplice.dds.dcps.jni.*;
import org.opensplice.dds.dcps.policy.*;

/**@brief A Subscriber is the object responsible for the actual reception of the data resulting from
 * its subscriptions. 
 * 
 * A Subscriber acts on behalf of one or several DataReader objects that are related to it.
 */
public class Subscriber extends Entity{
    private DomainParticipant participant;  /*!<The associated DomainParticipant.*/
    private HashSet readers;                /*!<The set of associated DataReader objects.*/
    
    /**@brief Creates a new Subscriber.
     * 
     * @param _participant The associated DomainParticipant.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the Subscriber.
     */
    protected Subscriber(   DomainParticipant _participant,
                            SubscriberQos _qos,
                            SubscriberListener _listener){
        participant     = _participant;
        qos             = _qos;
        listener        = _listener;
        readers         = new HashSet();
    }
    
    /**@brief This operation creates a DataReader.
     * 
     * The returned DataReader will be attached and belong to the Subscriber.
     * 
     * @param _td The associated TopicDescription that the DataReader will read.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DataReader.
     * 
     * @return The newly created DataReader.In case of failure null will be returned.
     */
    public DataReader create_datareader(    TopicDescription _td, 
                                            DataReaderQos _qos, 
                                            DataReaderListener _listener){
        DataReader dr;
        
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
        dr = ch.create_datareader(this, _td, _qos, _listener);
        
        if(dr != null){
            readers.add(dr);     
        }
        return dr;
    }
    
    /**@brief This operation deletes a DataReader that belongs to the Subscriber.
     *  
     * If the DataReader does not belong to the Subscriber the operation will return
     * ReturnCode.PRECONDITION_NOT_MET. The deletion of the DataReader is not allowed if
     * there are any ReadCondition or QueryCondition objects that are attached to the
     * DataReader. If the delete_datareader operation is called with any of these objects
     * attached the operation will return ReturnCode.PRECONDITION_NOT_MET. The 
     * delete_datareader operation must be called on the same Subscriber used to create the
     * DataReader. If delete_datareader is called on a different Subscriber, the operation
     * will return ReturnCode.PRECONDITION_NOT_MET.
     * 
     * @param _reader The DataReader to delete.
     * @return ReturnCode.OK if succeeded, any other ReturnCode otherwise.
     */
    public ReturnCode delete_datareader(DataReader _reader){
        ReturnCode rc;
        /* No DataReader supplied. */
        if(_reader == null){
            rc = ReturnCode.BAD_PARAMETER;    
        }
        /* DataReader is not part of this Subscriber. */
        else if(!(readers.contains(_reader))){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        /* All preconditions met: delete it. */
        else{
            DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
            rc = ReturnCode.from_user_result(ch.delete_datareader(_reader));
            
            if(rc.equals(ReturnCode.OK)){
                readers.remove(_reader);
            }
        }
        return rc;
    }
    
    /**@brief This operation retrieves a previously created DataReader belonging to the
     * Subscriber that is attached to the Topic with a matching _topicName. 
     * 
     * If no such DataReader exists, null will be returned. If muliple DataReader objects
     * attached to the Subscriber satisfy this condition, then the operation will return
     * one of them. It is not specified which one.
     * 
     * @param _topicName The name of the Topic associated with the DataReader to lookup.
     * @return A DataReader that satisfies the condition or null if no DataReader satisfies
     * the condition.
     */
    public DataReader lookup_datareader(String _topicName){
        TopicDescription t;
        DataReader dr;
        Iterator i = readers.iterator();
        
        while(i.hasNext()){
            dr = (DataReader)i.next();
            t = dr.get_topic_description();
            
            if(t.get_name().equals(_topicName)){
                return dr;
            }
        }
        return null;
    }
    
    /**@brief This operation allows the application to access the DataReader objects.
     * 
     * NOT consistent with DCPS specification.
     * 
     * @return All DataReaders of the Subscriber. 
     */
    public HashSet get_datareaders(){
        return readers;
    }
    
    /**@brief Gives access to the DomainParticipant the Subscriber belongs to.
     * 
     * @return The DomainParticipant the Subscriber belongs to.
     */
    public DomainParticipant get_participant(){
        return participant;
    }
}
