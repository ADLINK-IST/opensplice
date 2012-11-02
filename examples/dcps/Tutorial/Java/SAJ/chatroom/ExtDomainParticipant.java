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

/************************************************************************
 * LOGICAL_NAME:    ExtDomainParticipant.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for an extended DomainParticipant
 * class, that adds a new operations named 'simulate_multitopic', which
 * simulates the behavior of a multitopic by combining a ContentFilteredTopic
 * with a QueryCondition and a DataReaderListener. 
 * 
 ***/

package chatroom;

import DDS.*;
import Chat.*;

public class ExtDomainParticipant implements DomainParticipant {
    
    /***
     * Attributes
     ***/
     
    // Encapsulated DomainParticipant.
    private DomainParticipant               realParticipant;
    
    /*Implementation for DataReaderListener */
    private DataReaderListenerImpl          msgListener;

    /* Generic DDS entities */
    private Topic                           chatMessageTopic;
    private Topic                           nameServiceTopic;
    private ContentFilteredTopic            filteredMessageTopic;
    private Topic                           namedMessageTopic;
    private Subscriber                      multiSub;
    private Publisher                       multiPub;
    

    /***
     * Constructor
     ***/
    ExtDomainParticipant(DomainParticipant aParticipant) {
        this.realParticipant = aParticipant;
    }

    
    /***
     * Operations
     ***/
    public Topic create_simulated_multitopic (
        String name, 
        String type_name, 
        String subscription_expression,
        String[] expression_parameters) 
    {

        /* Type-specific DDS entities */
        ChatMessageDataReader   chatMessageDR;
        NameServiceDataReader   nameServiceDR;
        NamedMessageDataWriter  namedMessageDW;

        /* Query related stuff */
        QueryCondition          nameFinder;
        String[]                nameFinderParams;

        /* QosPolicy holders */
        TopicQosHolder          namedMessageQos = new TopicQosHolder();
        SubscriberQosHolder     subQos          = new SubscriberQosHolder();    
        PublisherQosHolder      pubQos          = new PublisherQosHolder();

        /* Others */
        DataReader              parentReader;
        DataWriter              parentWriter;
        String                  partitionName   = new String("ChatRoom");
        String                  nameFinderExpr;
        int                     status;

        /* Lookup both components that constitute the multi-topic. */
        chatMessageTopic = realParticipant.find_topic(
            "Chat_ChatMessage", DURATION_INFINITE.value);
        ErrorHandler.checkHandle(
            chatMessageTopic, 
            "DDS.DomainParticipant.find_topic (Chat_ChatMessage)");

        nameServiceTopic = realParticipant.find_topic(
            "Chat_NameService", DURATION_INFINITE.value);
        ErrorHandler.checkHandle(
            nameServiceTopic, 
            "DDS.DomainParticipant.find_topic (Chat_NameService)");

        /* Create a ContentFilteredTopic to filter out 
           our own ChatMessages. */
        filteredMessageTopic = realParticipant.create_contentfilteredtopic(
            "Chat_FilteredMessage", 
            chatMessageTopic,
            "userID <> %0",
            expression_parameters);
        ErrorHandler.checkHandle(
            filteredMessageTopic, 
            "DDS.DomainParticipant.create_contentfilteredtopic");
            

        /* Adapt the default SubscriberQos to read from the 
           "ChatRoom" Partition. */
        status = realParticipant.get_default_subscriber_qos (subQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_subscriber_qos");
        subQos.value.partition.name = new String[1];
        subQos.value.partition.name[0] = partitionName;

        /* Create a private Subscriber for the multitopic simulator. */
        multiSub = realParticipant.create_subscriber(
            subQos.value, null, STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            multiSub, 
            "DDS.DomainParticipant.create_subscriber (for multitopic)");
        
        /* Create a DataReader for the FilteredMessage Topic 
           (using the appropriate QoS). */
        parentReader = multiSub.create_datareader( 
            filteredMessageTopic, 
            DATAREADER_QOS_USE_TOPIC_QOS.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentReader, "DDS.Subscriber.create_datareader (ChatMessage)");

        /* Narrow the abstract parent into its typed representative. */
        chatMessageDR = ChatMessageDataReaderHelper.narrow(parentReader);
        ErrorHandler.checkHandle(
            chatMessageDR, "Chat.ChatMessageDataReaderHelper.narrow");
        
        /* Allocate the DataReaderListener Implementation. */
        msgListener = new DataReaderListenerImpl();
        ErrorHandler.checkHandle(msgListener, "new DataReaderListenerImpl");
        
        /* Attach the DataReaderListener to the DataReader, 
           only enabling the data_available event. */
        status = chatMessageDR.set_listener(
            msgListener, DDS.DATA_AVAILABLE_STATUS.value);
        ErrorHandler.checkStatus(status, "DDS.DataReader_set_listener");

        /* Create a DataReader for the nameService Topic 
           (using the appropriate QoS). */
        parentReader = multiSub.create_datareader( 
            nameServiceTopic, 
            DATAREADER_QOS_USE_TOPIC_QOS.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentReader, "DDS.Subscriber.create_datareader (NameService)");
        
        /* Narrow the abstract parent into its typed representative. */
        nameServiceDR = NameServiceDataReaderHelper.narrow(parentReader);
        ErrorHandler.checkHandle(
            nameServiceDR, "Chat.NameServiceDataReaderHelper.narrow");
        
        /* Define the SQL expression (using a parameterized value). */
        nameFinderExpr = new String("userID = %0");
        
        /* Allocate and assign the query parameters. */
        nameFinderParams = new String[1];
        nameFinderParams[0] = expression_parameters[0];

        /* Create a QueryCondition to only read corresponding 
           nameService information by key-value. */
        nameFinder = nameServiceDR.create_querycondition( 
            ANY_SAMPLE_STATE.value, 
            ANY_VIEW_STATE.value, 
            ANY_INSTANCE_STATE.value,
            nameFinderExpr, 
            nameFinderParams);
        ErrorHandler.checkHandle(
            nameFinder, "DDS.DataReader.create_querycondition (nameFinder)");
        
        /* Create the Topic that simulates the multi-topic 
           (use Qos from chatMessage).*/
        status = chatMessageTopic.get_qos(namedMessageQos);
        ErrorHandler.checkStatus(status, "DDS.Topic.get_qos");
        
        /* Create the NamedMessage Topic whose samples simulate 
           the MultiTopic */
        namedMessageTopic = realParticipant.create_topic( 
            "Chat_NamedMessage", 
            type_name, 
            namedMessageQos.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            namedMessageTopic, 
            "DDS.DomainParticipant.create_topic (NamedMessage)");
        
        /* Adapt the default PublisherQos to write into the 
           "ChatRoom" Partition. */
        status = realParticipant.get_default_publisher_qos(pubQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_publisher_qos");
        pubQos.value.partition.name = new String[1];
        pubQos.value.partition.name[0] = partitionName;

        /* Create a private Publisher for the multitopic simulator. */
        multiPub = realParticipant.create_publisher(
            pubQos.value, null, STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            multiPub, 
            "DDS.DomainParticipant.create_publisher (for multitopic)");
        
        /* Create a DataWriter for the multitopic. */
        parentWriter = multiPub.create_datawriter( 
            namedMessageTopic, 
            DATAWRITER_QOS_USE_TOPIC_QOS.value, 
            null,
            STATUS_MASK_NONE.value);
        ErrorHandler.checkHandle(
            parentWriter, "DDS.Publisher.create_datawriter (NamedMessage)");
        
        /* Narrow the abstract parent into its typed representative. */
        namedMessageDW = NamedMessageDataWriterHelper.narrow(parentWriter);
        ErrorHandler.checkHandle(
            namedMessageDW, "Chat.NamedMessageDataWriterHelper.narrow");
        
        /* Store the relevant Entities in our Listener. */
        msgListener.chatMessageDR = chatMessageDR;
        msgListener.nameServiceDR = nameServiceDR;
        msgListener.namedMessageDW = namedMessageDW;
        msgListener.nameFinder = nameFinder;
        msgListener.nameFinderParams = nameFinderParams;

        /* Return the simulated Multitopic. */
        return namedMessageTopic;

    }
    
    public int delete_simulated_multitopic(
        TopicDescription smt) 
    {
        int status;

        /* Remove the DataWriter */
        status = multiPub.delete_datawriter(msgListener.namedMessageDW);
        ErrorHandler.checkStatus(status, "DDS.Publisher.delete_datawriter");

        /* Remove the Publisher. */
        status = realParticipant.delete_publisher(multiPub);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_publisher");

        /* Remove the QueryCondition. */
        status = msgListener.nameServiceDR.delete_readcondition(
            msgListener.nameFinder);
        ErrorHandler.checkStatus(
            status, "DDS.DataReader.delete_readcondition");

        /* Remove the DataReaders. */
        status = multiSub.delete_datareader(msgListener.nameServiceDR);
        ErrorHandler.checkStatus(status, "DDS.Subscriber.delete_datareader");
        status = multiSub.delete_datareader(msgListener.chatMessageDR);
        ErrorHandler.checkStatus(status, "DDS.Subscriber.delete_datareader");

        /* Remove the Subscriber. */
        status = realParticipant.delete_subscriber(multiSub);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_subscriber");

        /* Remove the ContentFilteredTopic. */
        status = realParticipant.delete_contentfilteredtopic(
            filteredMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_contentfilteredtopic");

        /* Remove all other topics. */
        status = realParticipant.delete_topic(namedMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (namedMessageTopic)");
        status = realParticipant.delete_topic(nameServiceTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (nameServiceTopic)");
        status = realParticipant.delete_topic(chatMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.delete_topic (chatMessageTopic)");

        return status;
    };
    
    public Publisher create_publisher(
            PublisherQos qos, PublisherListener a_listener, int mask) {
        return realParticipant.create_publisher(qos, a_listener, mask);
    }

    public int delete_publisher(Publisher p) {
        return realParticipant.delete_publisher(p);
    }

    public Subscriber create_subscriber(
            SubscriberQos qos, SubscriberListener a_listener, int mask) {
        return realParticipant.create_subscriber(qos, a_listener, mask);
    }

    public int delete_subscriber(Subscriber s) {
        return realParticipant.delete_subscriber(s);
    }

    public Subscriber get_builtin_subscriber() {
        return realParticipant.get_builtin_subscriber();
    }

    public Topic create_topic(
            String topic_name, 
            String type_name, 
            TopicQos qos,
            TopicListener a_listener, 
            int mask) {
        return realParticipant.create_topic(
            topic_name, type_name, qos, a_listener, mask);
    }

    public int delete_topic(Topic a_topic) {
        return realParticipant.delete_topic(a_topic);
    }

    public Topic find_topic(String topic_name, Duration_t timeout) {
        return realParticipant.find_topic(topic_name, timeout);
    }

    public TopicDescription lookup_topicdescription(String name) {
        return realParticipant.lookup_topicdescription(name);
    }

    public ContentFilteredTopic create_contentfilteredtopic(
            String name,
            Topic related_topic, 
            String filter_expression, 
            String[] filter_parameters) {
        return realParticipant.create_contentfilteredtopic(
                name, 
                related_topic, 
                filter_expression, 
                filter_parameters);
    }

    public int delete_contentfilteredtopic(
            ContentFilteredTopic a_contentfilteredtopic) {
        return realParticipant.delete_contentfilteredtopic(
                a_contentfilteredtopic);
    }

    public MultiTopic create_multitopic(
            String name, 
            String type_name, 
            String subscription_expression,
            String[] expression_parameters) {
        return realParticipant.create_multitopic(
                name, 
                type_name, 
                subscription_expression, 
                expression_parameters);
    }

    public int delete_multitopic(MultiTopic a_multitopic) {
        return realParticipant.delete_multitopic(a_multitopic);
    }

    public int delete_contained_entities() {
        return realParticipant.delete_contained_entities();
    }

    public int set_qos(DomainParticipantQos qos) {
        return realParticipant.set_qos(qos);
    }

    public int get_qos(DomainParticipantQosHolder qos) {
        return realParticipant.get_qos(qos);
    }

    public int set_listener(DomainParticipantListener a_listener, int mask) {
        return realParticipant.set_listener(a_listener, mask);
    }

    public DomainParticipantListener get_listener() {
        return realParticipant.get_listener();
    }

    public int ignore_participant(long handle) {
        return realParticipant.ignore_participant(handle);
    }

    public int ignore_topic(long handle) {
        return realParticipant.ignore_topic(handle);
    }

    public int ignore_publication(long handle) {
        return realParticipant.ignore_publication(handle);
    }

    public int ignore_subscription(long handle) {
        return realParticipant.ignore_subscription(handle);
    }

    public String get_domain_id() {
        return realParticipant.get_domain_id();
    }

    public int assert_liveliness() {
        return realParticipant.assert_liveliness();
    }

    public int set_default_publisher_qos(PublisherQos qos) {
        return realParticipant.set_default_publisher_qos(qos);
    }

    public int get_default_publisher_qos(PublisherQosHolder qos) {
        return realParticipant.get_default_publisher_qos(qos);
    }

    public int set_default_subscriber_qos(SubscriberQos qos) {
        return realParticipant.set_default_subscriber_qos(qos);
    }

    public int get_default_subscriber_qos(SubscriberQosHolder qos) {
        return realParticipant.get_default_subscriber_qos(qos);
    }

    public int set_default_topic_qos(TopicQos qos) {
        return realParticipant.set_default_topic_qos(qos);
    }

    public int get_default_topic_qos(TopicQosHolder qos) {
        return realParticipant.get_default_topic_qos(qos);
    }

    public int get_discovered_participants(InstanceHandleSeqHolder handles) {
        return realParticipant.get_discovered_participants(handles);
    }

    public int get_discovered_participant_data(
            ParticipantBuiltinTopicDataHolder participant_data,
            long participant_handle) {
        return realParticipant.get_discovered_participant_data(
                participant_data, participant_handle);
    }

    public int get_discovered_topics(InstanceHandleSeqHolder handles) {
        return realParticipant.get_discovered_topics(handles);
    }

    public int get_discovered_topic_data(
            TopicBuiltinTopicDataHolder topic_data,
            long topic_handle) {
        return realParticipant.get_discovered_topic_data(
                topic_data, topic_handle);
    }

    public boolean contains_entity(long a_handle) {
        return realParticipant.contains_entity(a_handle);
    }

    public int get_current_time(Time_tHolder current_time) {
        return realParticipant.get_current_time(current_time);
    }

    public int enable() {
        return realParticipant.enable();
    }

    public StatusCondition get_statuscondition() {
        return realParticipant.get_statuscondition();
    }

    public int get_status_changes() {
        return realParticipant.get_status_changes();
    }

    public long get_instance_handle() {
        return realParticipant.get_instance_handle();
    }
    
}
