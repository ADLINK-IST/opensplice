package org.opensplice.dds.dcps;


import java.util.*;

import org.opensplice.dds.dcps.jni.*;
import org.opensplice.dds.dcps.policy.*;

/**@class DomainParticipant
 * @brief The DCPS domainparticipant object.
 * 
 * The DomainParticipant object plays several roles:
 * - It acts as a container for all other Entity objects.
 * 
 * - It acts as a factory for The Publisher, Subscriber, Topic and
 *   Multitopic Entity objects.
 * 
 * - It represents the participation of the application on a communication 
 *   plan that isolates applications tunning on the same set of physical
 *   computers from each other. A domain establishes a "virtual network" linking
 *   all applications that share the same domanId and isolating them from
 *   applications running on different domains. In this way, several independent 
 *   distributed applications cab coexist in the same physical network without
 *   interfering, or even being aware of each other
 * 
 * - It provides administration services in the domain, offering operations that
 *   allow the application to 'ignore' locally any information about a given
 *   participant (ignore_participant), publication (ignore_publication), 
 *   subscription (ignore_subscription) or topic (ignore_topic)
 * 
 * @see org.opensplice.dds.dcps.Publisher  
 * @see org.opensplice.dds.dcps.Subscriber
 * @see org.opensplice.dds.dcps.Topic
 * @see org.opensplice.dds.dcps.Entity
 */
public class DomainParticipant extends Entity {
    private long domainId;      /*!<The domainId of the Domain this object participates in.*/
    private HashSet publishers; /*!<The set of Publisher objects attached to this DomainParticipant.*/
    private HashSet subscribers;/*!<The set of Subscriber objects attached to this DomainParticipant.*/
    private HashSet topics;     /*!<The set of Topic objects attached to this DomainParticipant.*/
    
    /** @brief Creates a DomainParticipant.
     * 
     * @param _domainId The domain the DomainParticipant participates in.
     * @param _qos  The QoS policy that determines the behaviour of the DomainParticipant
     * @param _listener A listener that will be notified if the status of the DomainParticipant
     *                  changes.
     */
    protected DomainParticipant(long _domainId, 
                                DomainParticipantQos _qos, 
                                DomainParticipantListener _listener){
        domainId    = _domainId;
        qos         = _qos;
        listener    = _listener;
    }
    
    /**@brief This operation creates a Publisher with the desired QoS policies and attaches it
     * to the specified PublisherListener. 
     * 
     * If the specified QoS policies are not
     * compatible, the operation will fail and no Publisher will be created. The created
     * Publisher will belong to the DomainParticipant that is its factory.
     *   
     * @param _qos The desired QoS policies.
     * @param _listener The PublisherListener where the Publisher will be attached to.
     * 
     * @return The newly created Publisher. In case of failure null will be returned.
     * 
     * @see org.opensplice.dds.dcps.Publisher
     * @see org.opensplice.dds.dcps.PublisherQos
     * @see org.opensplice.dds.dcps.PublisherListener
     */
    public Publisher create_publisher(PublisherQos _qos, PublisherListener _listener){   
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();     
        Publisher pub = ch.create_publisher(this, _qos, _listener);
        
        if(pub != null){
            publishers.add(pub);
        }
        return pub;
    }
    
    /**@brief This operation deletes an existing Publisher. 
     * 
     * A Publisher cannot be deleted if
     * it has any attached DataWriter objects. It delete_publisher is called on a 
     * Publisher with existing DataWriter objects it will return 
     * ReturnCode.PRECONDITION_NOT_MET. This operation must be called on the same
     * DomainParticipant object used to created the Publisher. If it is called on a 
     * different DomainParticipant, the operation will have no effect and return
     * ReturnCode.PRECONDITION_NOT_MET. 
     * 
     * @param _publisher The publisher to is to be deleted.
     * 
     * @return ReturnCode.OK if succeeded. ReturnCode.PRECONDITION_NO_MET otherwise.
     * 
     * @see org.opensplice.dds.dcps.ReturnCode
     * @see org.opensplice.dds.dcps.Publisher
     */
    public ReturnCode delete_publisher(Publisher _publisher){
        ReturnCode rc;
        
        /* No Publisher supplied. */
        if(_publisher == null){
            rc = ReturnCode.BAD_PARAMETER;
        }
        /* Publisher not in this DomainParticipant. */
        else if(!(publishers.contains(_publisher))){
            rc = ReturnCode.BAD_PARAMETER;
        }
        /* DataWriters have not all been deleted. */
        else if(_publisher.get_datawriters().size() != 0){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        /* All preconditions met; delete Publisher. */
        else{
            DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();                
            rc = ReturnCode.from_user_result(ch.delete_publisher(_publisher));
            
            if(rc.equals(ReturnCode.OK)){
                publishers.remove(_publisher);
            }
        } 
        return rc;
    }
    
    /**@brief This operation creates a Subscriber with the desired QoS policies attaches to it the
     * specified SubscriberListener. 
     * 
     * If the specified QoS policies are not compatible, the
     * operation will fail and no Subscriber will be created. The created Subscriber belongs
     * to this DomainParticipant that is its factory.
     * 
     * @param _qos The desired QoS policies.
     * @param _listener The SubscriberListener that will be attached to the Subscriber.
     * 
     * @return The newly created Subscriber. In case of failure null will be returned.
     * 
     * @see org.opensplice.dds.dcps.Subscriber
     * @see org.opensplice.dds.dcps.policy.SubscriberQos
     * @see org.opensplice.dds.dcps.SubscriberListener
     */
    public Subscriber create_subscriber(SubscriberQos _qos, SubscriberListener _listener){
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
        Subscriber sub = ch.create_subscriber(this, _qos, _listener);

        if(sub != null){
            subscribers.add(sub);
        }
        return sub;
    }
    
    /**@brief This operation deletes an existing Subscriber.
     * 
     * A Subscriber cannot be deleted if it has any attahed DataReader object. If the
     * delete_subscriber operation is called on a Subscriber with existing DataReader
     * objects, it will return ReturnCode.PRECONDITION_NOT_MET. The delete_subscriber
     * operation must be called on the same DomainParticipant object used to create the
     * Subscriber. If delete_subscriber is called on a different DomainParticipant the
     * operation will have no effect and will return ReturnCode.PRECONDITION_NOT_MET.
     * 
     * @param _subscriber
     * @return ReturnCode.OK if succeeded, ReturnCode.PRECONDITION_NOT_MET otherwise.
     */
    public ReturnCode delete_subscriber(Subscriber _subscriber){
        ReturnCode rc;
        
        /* No Subscriber supplied. */
        if(_subscriber == null){
            rc = ReturnCode.BAD_PARAMETER;
        }
        /* Subscriber not in this DomainParticipant. */
        else if(!(subscribers.contains(_subscriber))){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        /* DataReaders of the Subscriber have not been deleted. */
        else if(_subscriber.get_datareaders().size() != 0){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        /* All preconditions met; delete Subscriber. */
        else{
            DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
            rc = ReturnCode.from_user_result(ch.delete_subscriber(_subscriber));

            if(rc.equals(ReturnCode.OK)){
                subscribers.remove(_subscriber);
            }
        }
        
        return rc;
    }
    
    /**@brief This operation creates a Topic with the desired QoS policies and attaches to it the
     * specified TopicListener.
     * 
     * If the specified QoS policies are not compatible, the operation
     * will fail and no Topic will be created. The Topic belongs to the DomainParticipant
     * that is its factory. The Topic is bound to a type described by the _typeName argument.
     * Prior to creating a Topic the type must have been registered with the Service. This
     * is done by the register_type operation on a derived class of the DataType interface.
     * The application is not allowed to create two Topic objects with the same name attached
     * to the same DomainParticipant. If the application attempts this, create_topic will fail.
     * 
     * @param _name The name of the Topic.
     * @param _typeName The name of the type of the Topic
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the Topic.
     * 
     * @return The newly created Topic. In case of failure null will be returned.
     * 
     * @see org.opensplice.dds.dcps.Topic
     * @see org.opensplice.dds.dcps.TopicQos
     * @see org.opensplice.dds.dcps.TopicListener
     */
    public Topic create_topic(String _name, String _typeName, TopicQos _qos, TopicListener _listener){
        Iterator i = topics.iterator();
        Topic t;
        
        /* Check first if a Topic with the specified name already exists. */
        while(i.hasNext()){
            t = (Topic)i.next();
            if(t.get_name().equals(_name)){
                return null;
            }
        }
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();                
        t = ch.create_topic(this, _name, _typeName, _qos, _listener);
        
        if(t != null){
            topics.add(t);
        }
        return t;
    }
    
    /**@brief This operation deletes a Topic.
     * 
     * The deletion of a Topic is not allowed of there are existing DataReader, DataWriter,
     * ContentFilteredTopic or MultiTopic objects that are using the Topic. If the
     * delete_topic operation is called with any of these existing objects attached to it, it
     * will return ReturnCode.PRECONDITION_NOT_MET. delete_topic must be called on the same
     * DomainParticipant used to create the Topic. If delete_topic is called on a different
     * DomainParticipant it will return ReturnCode.PRECONDITION_NOT_MET.
     * 
     * @param _topic The Topic to delete.
     * 
     * @return ReturnCode.OK if succeeded, ReturnCode.PRECONDITION_NOT_MET otherwise.
     * 
     * @see org.opensplice.dds.dcps.Topic
     * @see org.opensplice.dds.dcps.ReturnCode
     */
    public ReturnCode delete_topic(Topic _topic){
        ReturnCode rc;
        
        if(_topic == null){
            rc = ReturnCode.BAD_PARAMETER;
        }
        else if(!(topics.contains(_topic))){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        else{
            DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();
            rc = ReturnCode.from_user_result(ch.delete_topic(_topic));
            
            if(rc.equals(ReturnCode.OK)){
                topics.remove(_topic);
            }
        }
        return rc;
    }
    
    /**@brief This operation gives access to an existing (or ready to exist) enabled Topic, based
     * on its name.
     * 
     * If a Topic of the same name already exists, it gives access to it,
     * otherwise it waits (blocks the caller) until another mechanism creates it (or the 
     * specified timeout occurs). 
     *   
     * @param name The name of the Topic to lookup.
     * @param timeout The maximum amount of time that may be used to lookup the Topic.
     * 
     * @return The newly created Topic. If the operation fails or the timeout occurs null
     * will be returned.
     */
    public Topic lookup_topic(String name, long timeout){
        Iterator i = topics.iterator();
        
        while(i.hasNext()){
            Topic t = (Topic)i.next();
            if(t.get_name().equals(name)){
                return t;    
            }
        }
        return null;
    }
    
    /**@brief Returns the domainId of the Domain the DomainParticipant participates in 
     * 
     * @return The domainId of the Domain the DomainParticipant participates in.
     */
    public long get_domain_id(){
        return domainId;
    }
    
    /**@brief This operation deletes all the entities that were created by means of the "create"
     * operations on the DomainParticipant. 
     * 
     * That is, it deletes all contained Publisher,
     * Subscriber, Topic, ContentFilteredTopic and MultiTopic objects. Prior to deleting
     * each contained entity, this operation will recursively call the corresponding 
     * delete_contained_entities operation on each entity (if applicable). This pattern
     * is applied recusively. In this manner the operation delete_contained_entities on
     * the DomainParticipant will end up deleteing all the entities recursively contained
     * in the DomainParticipant, that is also the DataReader, DataWriter objects as well
     * as QueryCondition and ReadCondition objects belonging to the contained DataReader
     * objects. Once delete_contained_entities returns successfully, the application may
     * delete the DomainParticipant knowing it has no contained entities.
     * 
     * @return ReturnCode.OK if succeeded. If the operation fails any other ReturnCode
     * will be returned.
     */
    public ReturnCode delete_contained_entities(){
        ReturnCode rc;
        
        DCPSCommunicationHandler ch = DomainParticipantFactory.get_instance().getCommunicationHandler();        
        rc = ReturnCode.from_user_result(ch.delete_participant_entities(this));
        
        if(rc.equals(ReturnCode.OK)){
            publishers.clear();
            subscribers.clear();
            topics.clear();
        }
        return rc;
    }
    
    
    /**@brief Looks up all Publisher entities of the DomainParticipant.
     * 
     * @return The set of Publisher entities.
     */
    HashSet get_publishers(){
        return publishers;
    }
    
    /**@brief Looks up all Subscriber entities of the DomainParticipant.
     * 
     * @return The set of Subscriber entities.
     */
    HashSet get_subscribers(){
        return subscribers;
    }
    
    /**@brief Looks up all Topic entities of the DomainParticipant.
     * 
     * @return The set of Topic entities.
     */
    HashSet get_topics(){
        return topics;
    }
}

