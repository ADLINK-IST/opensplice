package org.opensplice.dds.dcps;

import org.opensplice.dds.dcps.policy.*;

/**@class Topic
 * @brief Topic is the most basic description of the data to be published and subscribed.
 * 
 * A Topic is identified by its name, which must be unique in the whole Domain.
 * In addition (by virtue of extending TopicDescription) it fully specifies the
 * type of the data that can be communicated when publishing or subscribing to the
 * Topic. Topic is the only TopicDescription that can be used for publications and therefore
 * associated with a DataWriter.
 */
public class Topic extends TopicDescription {
    
    /**@brief Creates a new Topic.
     * 
     * @param _name The name of the Topic.
     * @param _typeName The name of the type of the Topic.
     * @param _participant The DomainParticipant that creates the Topic.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the Topic.
     */  
    protected Topic(String _name, 
                    String _typeName, 
                    DomainParticipant _participant, 
                    TopicQos _qos,
                    TopicListener _listener){
        super(_name, _typeName, _participant);
        qos = _qos;
        listener = _listener;
    }
}
