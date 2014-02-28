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
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
import org.opensplice.cm.CMFactory;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Topic;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.TopicQoS;

/**
 * Implementation of the Topic interface.
 * 
 * @date May 18, 2005 
 */
public class TopicImpl extends EntityImpl implements Topic{
    private String typeName;
    private MetaType dataType;
    protected String keyList;
    
    /** 
     * Constructs a new Topic from the supplied arguments. This function
     * is for internal use only and should not be used by API users.
     * 
     * @param _index The index of the handle of the kernel entity that is
     *               associated with this entity.
     * @param _serial The serial of the handle of the kernel entity that is
     *                associated with this entity.
     * @param _pointer The address of the user layer entity that is associated
     *                 with this entity.
     * @param _keyList Comma separated list of keys of the description.
     * @param _name The name of the kernel entity that is associated with this
     *              entity.
     * @param _typeName The name of the type of the topic.
     */
    public TopicImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name, String _keyList, String _typeName) {
        super(communicator, _index, _serial, _pointer, _name);
        typeName = _typeName;
        keyList = _keyList;
        dataType = null;
    }
    
    /**
     * Constructs a new Topic from the supplied arguments. The Topic is owned 
     * by the caller of this constructor. The topic must have been registered
     * in the Splice database prior to calling this function.
     * @param participant The participant to create the Topic in.
     * @param name The name of the Topic.
     * @param typeName The type name of the Topic.
     * @param keyList The keyList of the Topic.
     * @param qos The quality of service to apply to the Topic.
     *
     * @throws CMException Thrown when the topic could not be created.
     */
    public TopicImpl(ParticipantImpl participant, String name, String typeName, String keyList, TopicQoS qos) throws CMException{
        super(participant.getCommunicator(), 0, 0, "", "");
        owner = true;
        TopicImpl t;
        try {
            t = (TopicImpl)getCommunicator().topicNew(participant, name, typeName, keyList, qos);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(t == null){
            throw new CMException("Topic could not be created.");
        }
        this.index = t.index;
        this.serial = t.serial;
        this.name = t.name;
        this.pointer = t.pointer;
        this.keyList = t.keyList;
        this.typeName = t.typeName;
        this.enabled = t.enabled;
        this.dataType = null;
        t.freed = true;
    }
   
    /**
     * Provides access to the key list.
     *
     * @return Comma separated list of keys.
     */
    public String getKeyList(){
        return keyList;
    }
 
    /**
     * Provides access to the type name of the topic.
     * 
     * @return The type name of the topic.
     */
    public String getTypeName(){
        return typeName;
    }
    
    /**
     * Resolves the data type of the topic.
     * 
     * @return The dataType of the topic.
     * @throws CMException
     * @throws DataTypeUnsupportedException
     */
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException{
        if(freed){
            throw new CMException("Topic already freed.");
        }
        if(dataType == null){
            try {
                dataType = getCommunicator().topicGetDataType(this);
            } catch (CommunicationException e) {
                throw new CMException("Topic not available (anymore) or data type could not be parsed.");
            }
        }
        return dataType;
    }
    
    public void setQoS(QoS qos) throws CMException{
        if(freed){
            throw new CMException("Supplied entity is not available (anymore).");
        } else if(qos instanceof TopicQoS){
            try {
                getCommunicator().entitySetQoS(this, qos);
            } catch (CommunicationException ce) {
                throw new CMException(ce.getMessage());
            }
        } else {
            throw new CMException("Supplied entity requires a TopicQoS.");
        }
    }
}
