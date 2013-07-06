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

/************************************************************************
 * LOGICAL_NAME:    ExtDomainParticipant.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             Octoboer 2000.
 ************************************************************************
 *
 * This file contains the implementation for an extended DomainParticipant
 * class, that adds a new operations named 'simulate_multitopic', which
 * simulates the behavior of a multitopic by combining a ContentFilteredTopic
 * with a QueryCondition and a DataReaderListener.
 *
 ***/

using DDS;
using DDS.OpenSplice;
using Chat;

namespace Chatroom {

public class ExtDomainParticipant : IDomainParticipant {

    /***
     * Attributes
     ***/

    // Encapsulated DomainParticipant.
    private IDomainParticipant               realParticipant;

    /*Implementation for DataReaderListener */
    private DataReaderListenerImpl          msgListener;

    /* Generic DDS entities */
    private ITopic                           chatMessageTopic;
    private ITopic                           nameServiceTopic;
    private IContentFilteredTopic            filteredMessageTopic;
    private ITopic                           namedMessageTopic;
    private ISubscriber                      multiSub;
    private IPublisher                       multiPub;


    /***
     * Constructor
     ***/
    public ExtDomainParticipant(IDomainParticipant aParticipant) {
        this.realParticipant = aParticipant;
    }

    public InstanceHandle InstanceHandle { get { return realParticipant.InstanceHandle; } }
    public StatusKind StatusChanges { get { return realParticipant.StatusChanges;  } }
    public IStatusCondition StatusCondition { get { return realParticipant.StatusCondition; } }

    /***
     * Operations
     ***/
    public ITopic CreateSimulatedMultitopic (
        string name,
        string type_name,
        string subscription_expression,
        string[] expression_parameters)
    {

        /* Type-specific DDS entities */
        ChatMessageDataReader   chatMessageDR;
        NameServiceDataReader   nameServiceDR;
        NamedMessageDataWriter  namedMessageDW;

        /* Query related stuff */
        IQueryCondition          nameFinder;
        string[]                nameFinderParams;

        /* QosPolicy holders */
        TopicQos          namedMessageQos = new TopicQos();
        SubscriberQos     subQos          = new SubscriberQos();
        PublisherQos      pubQos          = new PublisherQos();

        /* Others */
        IDataReader              parentReader;
        IDataWriter              parentWriter;
        string                  partitionName   = "ChatRoom";
        string                  nameFinderExpr;
        ReturnCode                     status;

        /* Lookup both components that constitute the multi-topic. */
        chatMessageTopic = realParticipant.FindTopic(
            "Chat_ChatMessage", Duration.Infinite);
        ErrorHandler.checkHandle(
            chatMessageTopic,
            "DDS.DomainParticipant.FindTopic (Chat_ChatMessage)");

        nameServiceTopic = realParticipant.FindTopic(
            "Chat_NameService", Duration.Infinite);
        ErrorHandler.checkHandle(
            nameServiceTopic,
            "DDS.DomainParticipant.FindTopic (Chat_NameService)");

        /* Create a ContentFilteredTopic to filter out
           our own ChatMessages. */
        filteredMessageTopic = realParticipant.CreateContentFilteredTopic(
            "Chat_FilteredMessage",
            chatMessageTopic,
            "userID <> %0",
            expression_parameters);
        ErrorHandler.checkHandle(
            filteredMessageTopic,
            "DDS.DomainParticipant.CreateContentFilteredTopic");

        /* Adapt the default SubscriberQos to read from the
           "ChatRoom" Partition. */
        status = realParticipant.GetDefaultSubscriberQos (ref subQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.GetDefaultSubscriberQos");
        subQos.Partition.Name = new string[1];
        subQos.Partition.Name[0] = partitionName;

        /* Create a private Subscriber for the multitopic simulator. */
        multiSub = realParticipant.CreateSubscriber(subQos);
        ErrorHandler.checkHandle(
            multiSub,
            "DDS.DomainParticipant.CreateSubscriber (for multitopic)");

        /* Create a DataReader for the FilteredMessage Topic
           (using the appropriate QoS). */
        DataReaderQos drQos = new DataReaderQos();
        TopicQos topicQos = new TopicQos();
        multiSub.GetDefaultDataReaderQos(ref drQos);
        filteredMessageTopic.RelatedTopic.GetQos(ref topicQos);
        multiSub.CopyFromTopicQos(ref drQos, topicQos);

        parentReader = multiSub.CreateDataReader(filteredMessageTopic, drQos);
        ErrorHandler.checkHandle(
            parentReader, "DDS.Subscriber.create_datareader (ChatMessage)");

        /* Narrow the abstract parent into its typed representative. */
        chatMessageDR = parentReader as ChatMessageDataReader;

        /* Allocate the DataReaderListener Implementation. */
        msgListener = new DataReaderListenerImpl();

        /* Attach the DataReaderListener to the DataReader,
           only enabling the data_available event. */
        status = chatMessageDR.SetListener(msgListener, StatusKind.DataAvailable);
        ErrorHandler.checkStatus(status, "DDS.DataReader_set_listener");

        /* Create a DataReader for the nameService Topic
           (using the appropriate QoS). */
        DataReaderQos nsDrQos = new DataReaderQos();
        TopicQos nsQos = new TopicQos();
        status = multiSub.GetDefaultDataReaderQos(ref nsDrQos);
        ErrorHandler.checkStatus(status, "DDS.Subscriber.GetDefaultDataReaderQos");
        nameServiceTopic.GetQos(ref nsQos);
        multiSub.CopyFromTopicQos(ref nsDrQos, nsQos);

        parentReader = multiSub.CreateDataReader(nameServiceTopic, nsDrQos);
        ErrorHandler.checkHandle(parentReader, "DDS.Subscriber.CreateDatareader (NameService)");

        /* Narrow the abstract parent into its typed representative. */
        nameServiceDR = parentReader as NameServiceDataReader;

        /* Define the SQL expression (using a parameterized value). */
        nameFinderExpr = "userID = %0";

        /* Allocate and assign the query parameters. */
        nameFinderParams = new string[1];
        nameFinderParams[0] = expression_parameters[0];

        /* Create a QueryCondition to only read corresponding
           nameService information by key-value. */
        nameFinder = nameServiceDR.CreateQueryCondition(
            SampleStateKind.Any,
            ViewStateKind.Any,
            InstanceStateKind.Any,
            nameFinderExpr,
            nameFinderParams);
        ErrorHandler.checkHandle(
            nameFinder, "DDS.DataReader.create_querycondition (nameFinder)");

        /* Create the Topic that simulates the multi-topic
           (use Qos from chatMessage).*/
        status = chatMessageTopic.GetQos(ref namedMessageQos);
        ErrorHandler.checkStatus(status, "DDS.Topic.GetQos");

        /* Create the NamedMessage Topic whose samples simulate
           the MultiTopic */
        namedMessageTopic = realParticipant.CreateTopic(
            "Chat_NamedMessage",
            type_name,
            namedMessageQos);
        ErrorHandler.checkHandle(
            namedMessageTopic,
            "DDS.DomainParticipant.CreateTopic (NamedMessage)");

        /* Adapt the default PublisherQos to write into the
           "ChatRoom" Partition. */
        status = realParticipant.GetDefaultPublisherQos(ref pubQos);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.get_default_publisher_qos");
        pubQos.Partition.Name = new string[1];
        pubQos.Partition.Name[0] = partitionName;

        /* Create a private Publisher for the multitopic simulator. */
        multiPub = realParticipant.CreatePublisher(pubQos);
        ErrorHandler.checkHandle(
            multiPub,
            "DDS.DomainParticipant.create_publisher (for multitopic)");

        DataWriterQos nmDwQos = new DataWriterQos();
        TopicQos nmQos = new TopicQos();
        multiPub.GetDefaultDataWriterQos(ref nmDwQos);
        namedMessageTopic.GetQos(ref nmQos);
        multiPub.CopyFromTopicQos(ref nmDwQos, nmQos);

        /* Create a DataWriter for the multitopic. */
        parentWriter = multiPub.CreateDataWriter(namedMessageTopic, nmDwQos);
        ErrorHandler.checkHandle(
            parentWriter, "DDS.Publisher.CreateDatawriter (NamedMessage)");

        /* Narrow the abstract parent into its typed representative. */
        namedMessageDW = parentWriter as NamedMessageDataWriter;

        /* Store the relevant Entities in our Listener. */
        msgListener.ChatMessageDR = chatMessageDR;
        msgListener.NameServiceDR = nameServiceDR;
        msgListener.NamedMessageDW = namedMessageDW;
        msgListener.NameFinder = nameFinder;
        msgListener.NameFinderParams = nameFinderParams;

        /* Return the simulated Multitopic. */
        return namedMessageTopic;

    }

    public ReturnCode DeleteSimulatedMultitopic(ITopicDescription smt)
    {
        ReturnCode status;

        /* Remove the DataWriter */
        status = multiPub.DeleteDataWriter(msgListener.NamedMessageDW);
        ErrorHandler.checkStatus(status, "DDS.Publisher.DeleteDatawriter");

        /* Remove the Publisher. */
        status = realParticipant.DeletePublisher(multiPub);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.DeletePublisher");

        /* Remove the QueryCondition. */
        status = msgListener.NameServiceDR.DeleteReadCondition(
            msgListener.NameFinder);
        ErrorHandler.checkStatus(
            status, "DDS.DataReader.DeleteReadcondition");

        /* Remove the DataReaders. */
        status = multiSub.DeleteDataReader(msgListener.NameServiceDR);
        ErrorHandler.checkStatus(status, "DDS.Subscriber.DeleteDatareader");
        status = multiSub.DeleteDataReader(msgListener.ChatMessageDR);
        ErrorHandler.checkStatus(status, "DDS.Subscriber.DeleteDatareader");

        /* Remove the Subscriber. */
        status = realParticipant.DeleteSubscriber(multiSub);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.DeleteSubscriber");

        /* Remove the ContentFilteredTopic. */
        status = realParticipant.DeleteContentFilteredTopic(
            filteredMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.DeleteContentfilteredtopic");

        /* Remove all other topics. */
        status = realParticipant.DeleteTopic(namedMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.DeleteTopic (namedMessageTopic)");
        status = realParticipant.DeleteTopic(nameServiceTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.DeleteTopic (nameServiceTopic)");
        status = realParticipant.DeleteTopic(chatMessageTopic);
        ErrorHandler.checkStatus(
            status, "DDS.DomainParticipant.DeleteTopic (chatMessageTopic)");

        return status;
    }

    public IPublisher CreatePublisher()
    {
        return realParticipant.CreatePublisher();
    }

    public IPublisher CreatePublisher(PublisherQos qos)
    {
        return realParticipant.CreatePublisher(qos);
    }

    public IPublisher CreatePublisher(IPublisherListener listener, StatusKind mask)
    {
        return realParticipant.CreatePublisher(listener, mask);
    }

    public IPublisher CreatePublisher(PublisherQos qos, IPublisherListener listener, StatusKind mask)
    {
        return realParticipant.CreatePublisher(qos, listener, mask);
    }

    public ISubscriber CreateSubscriber()
    {
        return realParticipant.CreateSubscriber();
    }

    public ISubscriber CreateSubscriber(SubscriberQos qos)
    {
        return realParticipant.CreateSubscriber(qos);
    }

    public ISubscriber CreateSubscriber(ISubscriberListener listener, StatusKind mask)
    {
        return realParticipant.CreateSubscriber(listener, mask);
    }

    public ISubscriber CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask)
    {
        return realParticipant.CreateSubscriber(qos, listener, mask);
    }

    public ReturnCode DeletePublisher(IPublisher p) {
        return realParticipant.DeletePublisher(p);
    }

    public ReturnCode DeleteSubscriber(ISubscriber s) {
        return realParticipant.DeleteSubscriber(s);
    }

    public ISubscriber BuiltInSubscriber
    {
        get { return realParticipant.BuiltInSubscriber; }
    }

    public ITopic CreateTopic(string topicName, string typeName)
    {
        return realParticipant.CreateTopic(topicName, typeName);
    }

    public ITopic CreateTopic(string topicName, string typeName, TopicQos qos)
    {
        return realParticipant.CreateTopic(topicName, typeName, qos);
    }

    public ITopic CreateTopic(string topicName, string typeName, ITopicListener listener, StatusKind mask)
    {
        return realParticipant.CreateTopic(topicName, typeName, listener, mask);
    }

    public ITopic CreateTopic(string topicName, string typeName, TopicQos qos, ITopicListener listener, StatusKind mask)
    {
        return realParticipant.CreateTopic(topicName, typeName, qos, listener, mask);
    }

    public ReturnCode DeleteTopic(ITopic a_topic) {
        return realParticipant.DeleteTopic(a_topic);
    }

    public ITopic FindTopic(string topic_name, Duration timeout) {
        return realParticipant.FindTopic(topic_name, timeout);
    }

    public ITopicDescription LookupTopicDescription(string name) {
        return realParticipant.LookupTopicDescription(name);
    }

    public IContentFilteredTopic CreateContentFilteredTopic(
            string name,
            ITopic related_topic,
            string filter_expression,
            string[] filter_parameters) {
        return realParticipant.CreateContentFilteredTopic(
                name,
                related_topic,
                filter_expression,
                filter_parameters);
    }

    public ReturnCode DeleteContentFilteredTopic(
            IContentFilteredTopic a_contentfilteredtopic) {
        return realParticipant.DeleteContentFilteredTopic(
                a_contentfilteredtopic);
    }

    public IMultiTopic CreateMultiTopic(
            string name,
            string type_name,
            string subscription_expression,
            string[] expression_parameters) {
        return realParticipant.CreateMultiTopic(
                name,
                type_name,
                subscription_expression,
                expression_parameters);
    }

    public ReturnCode DeleteMultiTopic(IMultiTopic a_multitopic) {
        return realParticipant.DeleteMultiTopic(a_multitopic);
    }

    public ReturnCode DeleteContainedEntities() {
        return realParticipant.DeleteContainedEntities();
    }

    public ReturnCode SetQos(DomainParticipantQos qos) {
        return realParticipant.SetQos(qos);
    }

    public ReturnCode GetQos(ref DomainParticipantQos qos) {
        return realParticipant.GetQos(ref qos);
    }

    public ReturnCode SetListener(IDomainParticipantListener a_listener, StatusKind mask) {
        return realParticipant.SetListener(a_listener, mask);
    }

    public DomainParticipantListener GetListener() {
        throw new System.NotImplementedException();
        // return realParticipant. .get_listener();
    }

    public ReturnCode IgnoreParticipant(InstanceHandle handle) {
        return realParticipant.IgnoreParticipant(handle);
    }

    public ReturnCode IgnoreTopic(InstanceHandle handle) {
        return realParticipant.IgnoreTopic(handle);
    }

    public ReturnCode IgnorePublication(InstanceHandle handle) {
        return realParticipant.IgnorePublication(handle);
    }

    public ReturnCode IgnoreSubscription(InstanceHandle handle) {
        return realParticipant.IgnoreSubscription(handle);
    }

    public DDS.DomainId DomainId {
        get { return realParticipant.DomainId;}
    }

    public ReturnCode AssertLiveliness()
    {
        return realParticipant.AssertLiveliness();
    }

    public ReturnCode SetDefaultPublisherQos(PublisherQos qos) {
        return realParticipant.SetDefaultPublisherQos(qos);
    }

    public ReturnCode GetDefaultPublisherQos(ref PublisherQos qos) {
        return realParticipant.GetDefaultPublisherQos(ref qos);
    }

    public ReturnCode SetDefaultSubscriberQos(SubscriberQos qos) {
        return realParticipant.SetDefaultSubscriberQos (qos);
    }

    public ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos) {
        return realParticipant.GetDefaultSubscriberQos(ref qos);
    }

    public ReturnCode SetDefaultTopicQos(TopicQos qos) {
        return realParticipant.SetDefaultTopicQos (qos);
    }

    public ReturnCode GetDefaultTopicQos(ref TopicQos qos) {
        return realParticipant.GetDefaultTopicQos(ref qos);
    }

    public ReturnCode GetDiscoveredParticipants(ref InstanceHandle[] handles) {
        throw new System.NotImplementedException();
        // return realParticipant.get_discovered_participants(handles);
    }

    public DDS.ReturnCode GetDiscoveredParticipantData(
            ref ParticipantBuiltinTopicData participant_data, InstanceHandle particpant_handle)
    {
        throw new System.NotImplementedException();
        //return realParticipant.get_discovered_participant_data(
          //      participant_data, participant_handle);
    }

    public DDS.ReturnCode GetDiscoveredTopics(ref DDS.InstanceHandle[] handles)
    {
        throw new System.NotImplementedException();
        // return realParticipant.get_discovered_topics(handles);
    }

    public ReturnCode GetDiscoveredTopicData(
            ref DDS.TopicBuiltinTopicData topic_data, DDS.InstanceHandle topic_handle) 
    {
        throw new System.NotImplementedException();
        //return realParticipant.get_discovered_topic_data(
        //        topic_data, topic_handle);
    }

    public bool ContainsEntity(InstanceHandle a_handle) {
        return realParticipant.ContainsEntity(a_handle);
    }

    public ReturnCode GetCurrentTime(out Time current_time) {
        return realParticipant.GetCurrentTime(out current_time);
    }

    public ReturnCode Enable()
    {
        return realParticipant.Enable();
    }

    public ITypeSupport LookupTypeSupport(string registeredTypeName)
    {
        throw new System.NotImplementedException();
    }

    public ITypeSupport GetTypeSupport(string registeredName)
    {
        throw new System.NotImplementedException();
    }

}


}
