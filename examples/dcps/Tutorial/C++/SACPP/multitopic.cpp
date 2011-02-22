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
 * LOGICAL_NAME:    multitopic.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             june 2007.
 ************************************************************************
 *
 * This file contains the headers for all operations required to simulate
 * the MultiTopic behavior.
 *
 ***/

#include "multitopic.h"
#include "CheckStatus.h"
#include <sstream>

DDS::DataReaderListenerImpl::DataReaderListenerImpl() : previous(0x80000000) {
    nameFinderParams.length(1);
}

void DDS::DataReaderListenerImpl::on_requested_deadline_missed (
    DDS::DataReader_ptr,
    const DDS::RequestedDeadlineMissedStatus &
) THROW_ORB_EXCEPTIONS { }

void DDS::DataReaderListenerImpl::on_requested_incompatible_qos (
    DDS::DataReader_ptr,
    const DDS::RequestedIncompatibleQosStatus &
) THROW_ORB_EXCEPTIONS { }

void DDS::DataReaderListenerImpl::on_sample_rejected (
    DDS::DataReader_ptr,
    const DDS::SampleRejectedStatus &
) THROW_ORB_EXCEPTIONS { }

void DDS::DataReaderListenerImpl::on_liveliness_changed (
    DDS::DataReader_ptr,
    const DDS::LivelinessChangedStatus &
) THROW_ORB_EXCEPTIONS { }

void DDS::DataReaderListenerImpl::on_subscription_matched (
    DDS::DataReader_ptr,
    const DDS::SubscriptionMatchedStatus &
) THROW_ORB_EXCEPTIONS { }

void DDS::DataReaderListenerImpl::on_sample_lost (
    DDS::DataReader_ptr,
    const DDS::SampleLostStatus &
) THROW_ORB_EXCEPTIONS { }

void DDS::DataReaderListenerImpl::on_data_available (
    DDS::DataReader_ptr
) THROW_ORB_EXCEPTIONS {
    Chat::ChatMessageSeq                msgSeq;
    Chat::NameServiceSeq                nameSeq;
    DDS::SampleInfoSeq                  infoSeq1;
    DDS::SampleInfoSeq                  infoSeq2;
    DDS::ReturnCode_t                   status;
    previous =                          0x80000000;

    /* Take all messages. */
    status = chatMessageDR->take(
        msgSeq,
        infoSeq1,
        DDS::LENGTH_UNLIMITED,
        DDS::ANY_SAMPLE_STATE,
        DDS::ANY_VIEW_STATE,
        DDS::ANY_INSTANCE_STATE);
    checkStatus(status, "Chat::ChatMessageDataReader::take");

    /* For each message, extract the key-field and find the corresponding name. */
    for (ULong i = 0; i < msgSeq.length(); i++)
    {
        if (infoSeq1[i].valid_data)
        {
            Chat::NamedMessage joinedSample;

            /* Find the corresponding named message. */
            if (msgSeq[i].userID != previous)
            {
                ostringstream numberStr;
                previous = msgSeq[i].userID;
                numberStr << previous;
                nameFinderParams[0UL] = numberStr.str().c_str();
                status = nameFinder->set_query_parameters(nameFinderParams);
                checkStatus(status, "DDS::QueryCondition::set_query_parameters");
                status = nameServiceDR->read_w_condition(
                    nameSeq,
                    infoSeq2,
                    DDS::LENGTH_UNLIMITED,
                    nameFinder.in());
                checkStatus(status, "Chat::NameServiceDataReader::read_w_condition");

                /* Extract Name (there should only be one result). */
                if (status == DDS::RETCODE_NO_DATA)
                {
                    ostringstream msg;
                    msg << "Name not found!! id = " << previous;
                    userName = msg.str();
                }
                else
                {
                    userName = nameSeq[0].name;
                }

                /* Release the name sample again. */
                status = nameServiceDR->return_loan(nameSeq, infoSeq2);
                checkStatus(status, "Chat::NameServiceDataReader::return_loan");
            }
            /* Write merged Topic with userName instead of userID. */
            joinedSample.userName = userName.c_str();
            joinedSample.userID = msgSeq[i].userID;
            joinedSample.index = msgSeq[i].index;
            joinedSample.content = msgSeq[i].content;
            status = namedMessageDW->write(joinedSample, DDS::HANDLE_NIL);
            checkStatus(status, "Chat::NamedMessageDataWriter::write");
        }
    }
    status = chatMessageDR->return_loan(msgSeq, infoSeq1);
    checkStatus(status, "Chat::ChatMessageDataReader::return_loan");
}


DDS::ExtDomainParticipant_ptr DDS::ExtDomainParticipantImpl::_narrow(DDS::DomainParticipant_ptr obj) {
    return new DDS::ExtDomainParticipantImpl(obj);
}

DDS::DomainParticipant_ptr DDS::ExtDomainParticipantImpl::in() {
    return realParticipant.in();
}


DDS::ExtDomainParticipantImpl::ExtDomainParticipantImpl(DDS::DomainParticipant_ptr participant) {
    realParticipant = DDS::DomainParticipant::_duplicate(participant);
}



DDS::Topic_ptr
DDS::ExtDomainParticipantImpl::create_simulated_multitopic (
    const char *,
    const char * type_name,
    const char *,
    const DDS::StringSeq & expression_parameters)
{
    /* Type-specific DDS entities */
    Chat::ChatMessageDataReader_ptr     chatMessageDR;
    Chat::NameServiceDataReader_ptr     nameServiceDR;
    Chat::NamedMessageDataWriter_ptr    namedMessageDW;

    /* Query related stuff */
    DDS::QueryCondition_ptr             nameFinder;

    /* QosPolicy holders */
    DDS::TopicQos                       namedMessageQos;
    DDS::SubscriberQos                  sub_qos;
    DDS::PublisherQos                   pub_qos;

    /* Others */
    DDS::DataReader_ptr                 parentReader;
    DDS::DataWriter_ptr                 parentWriter;
    char                                *nameFinderExpr;
    const char                          *partitionName = "ChatRoom";
    DDS::ReturnCode_t                   status;

    /* Lookup both components that constitute the multi-topic. */
    chatMessageTopic = realParticipant->find_topic("Chat_ChatMessage", DDS::DURATION_INFINITE);
    checkHandle(chatMessageTopic.in(), "DDS::DomainParticipant::find_topic (Chat_ChatMessage)");

    nameServiceTopic = realParticipant->find_topic("Chat_NameService", DDS::DURATION_INFINITE);
    checkHandle(nameServiceTopic.in(), "DDS::DomainParticipant::find_topic (Chat_NameService)");

    /* Create a ContentFilteredTopic to filter out our own ChatMessages. */
    filteredMessageTopic = realParticipant->create_contentfilteredtopic(
        "Chat_FilteredMessage",
        chatMessageTopic.in(),
        "userID <> %0",
        expression_parameters);
    checkHandle(filteredMessageTopic.in(), "DDS::DomainParticipant::create_contentfilteredtopic");


    /* Adapt the default SubscriberQos to read from the "ChatRoom" Partition. */
    status = realParticipant->get_default_subscriber_qos (sub_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_subscriber_qos");
    sub_qos.partition.name.length(1);
    sub_qos.partition.name[0] = partitionName;

    /* Create a private Subscriber for the multitopic simulator. */
    multiSub = realParticipant->create_subscriber(sub_qos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(multiSub.in(), "DDS::DomainParticipant::create_subscriber (for multitopic)");

    /* Create a DataReader for the FilteredMessage Topic (using the appropriate QoS). */
    parentReader = multiSub->create_datareader(
        filteredMessageTopic.in(),
        DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS::STATUS_MASK_NONE);
    checkHandle(parentReader, "DDS::Subscriber::create_datareader (ChatMessage)");

    /* Narrow the abstract parent into its typed representative. */
    chatMessageDR = Chat::ChatMessageDataReader::_narrow(parentReader);
    checkHandle(chatMessageDR, "Chat::ChatMessageDataReader::_narrow");

    /* Allocate the DataReaderListener Implementation. */
    msgListener = new DDS::DataReaderListenerImpl();
    checkHandle(msgListener, "new DDS::DataReaderListenerImpl");

    /* Attach the DataReaderListener to the DataReader, only enabling the data_available event. */
    status = chatMessageDR->set_listener(msgListener, DDS::DATA_AVAILABLE_STATUS);
    checkStatus(status, "DDS::DataReader_set_listener");

    /* Create a DataReader for the nameService Topic (using the appropriate QoS). */
    parentReader = multiSub->create_datareader(
        nameServiceTopic.in(),
        DATAREADER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS::STATUS_MASK_NONE);
    checkHandle(parentReader, "DDS::Subscriber::create_datareader (NameService)");

    /* Narrow the abstract parent into its typed representative. */
    nameServiceDR = Chat::NameServiceDataReader::_narrow(parentReader);
    checkHandle(nameServiceDR, "Chat::NameServiceDataReader::_narrow");

    /* Define the SQL expression (using a parameterized value). */
    nameFinderExpr = "userID = %0";

    /* Create a QueryCondition to only read corresponding nameService information by key-value. */
    nameFinder = nameServiceDR->create_querycondition(
        DDS::ANY_SAMPLE_STATE,
        DDS::ANY_VIEW_STATE,
        DDS::ANY_INSTANCE_STATE,
        nameFinderExpr,
        expression_parameters);
    checkHandle(nameFinder, "DDS::DataReader::create_querycondition (nameFinder)");

    /* Create the Topic that simulates the multi-topic (use Qos from chatMessage).*/
    status = chatMessageTopic->get_qos(namedMessageQos);
    checkStatus(status, "DDS::Topic::get_qos");

    /* Create the NamedMessage Topic whose samples simulate the MultiTopic */
    namedMessageTopic = realParticipant->create_topic(
        "Chat_NamedMessage",
        type_name,
        namedMessageQos,
        NULL,
        DDS::STATUS_MASK_NONE);
    checkHandle(namedMessageTopic.in(), "DDS::DomainParticipant::create_topic (NamedMessage)");

    /* Adapt the default PublisherQos to write into the "ChatRoom" Partition. */
    status = realParticipant->get_default_publisher_qos(pub_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_publisher_qos");
    pub_qos.partition.name.length(1);
    pub_qos.partition.name[0] = partitionName;

    /* Create a private Publisher for the multitopic simulator. */
    multiPub = realParticipant->create_publisher(pub_qos, NULL, DDS::STATUS_MASK_NONE);
    checkHandle(multiPub.in(), "DDS::DomainParticipant::create_publisher (for multitopic)");

    /* Create a DataWriter for the multitopic. */
    parentWriter = multiPub->create_datawriter(
        namedMessageTopic.in(),
        DATAWRITER_QOS_USE_TOPIC_QOS,
        NULL,
        DDS::STATUS_MASK_NONE);
    checkHandle(parentWriter, "DDS::Publisher::create_datawriter (NamedMessage)");

    /* Narrow the abstract parent into its typed representative. */
    namedMessageDW = Chat::NamedMessageDataWriter::_narrow(parentWriter);
    checkHandle(namedMessageDW, "Chat::NamedMessageDataWriter::_narrow");

    /* Store the relevant Entities in our Listener. */
    msgListener->chatMessageDR = chatMessageDR;
    msgListener->nameServiceDR = nameServiceDR;
    msgListener->namedMessageDW = namedMessageDW;
    msgListener->nameFinder = nameFinder;

    /* Return the simulated Multitopic. */
    return DDS::Topic::_duplicate( namedMessageTopic.in() );
}

DDS::ReturnCode_t
DDS::ExtDomainParticipantImpl::delete_simulated_multitopic(
    DDS::TopicDescription_ptr
)
{
    DDS::ReturnCode_t status;

    /* Remove the DataWriter */
    status = multiPub->delete_datawriter(msgListener->namedMessageDW.in());
    checkStatus(status, "DDS::Publisher::delete_datawriter");

    /* Remove the Publisher. */
    status = realParticipant->delete_publisher(multiPub.in());
    checkStatus(status, "DDS::DomainParticipant::delete_publisher");

    /* Remove the QueryCondition. */
    status = msgListener->nameServiceDR->delete_readcondition(
        msgListener->nameFinder.in());
    checkStatus(status, "DDS::DataReader::delete_readcondition");

    /* Remove the DataReaders. */
    status = multiSub->delete_datareader(msgListener->nameServiceDR.in());
    checkStatus(status, "DDS::Subscriber::delete_datareader");
    status = multiSub->delete_datareader(msgListener->chatMessageDR.in());
    checkStatus(status, "DDS::Subscriber::delete_datareader");

    /* Remove the DataReaderListener. */
    DDS::release(msgListener);

    /* Remove the Subscriber. */
    status = realParticipant->delete_subscriber(multiSub.in());
    checkStatus(status, "DDS::DomainParticipant::delete_subscriber");

    /* Remove the ContentFilteredTopic. */
    status = realParticipant->delete_contentfilteredtopic(filteredMessageTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_contentfilteredtopic");

    /* Remove all other topics. */
    status = realParticipant->delete_topic(namedMessageTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_topic (namedMessageTopic)");
    status = realParticipant->delete_topic(nameServiceTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_topic (nameServiceTopic)");
    status = realParticipant->delete_topic(chatMessageTopic.in());
    checkStatus(status, "DDS::DomainParticipant::delete_topic (chatMessageTopic)");

    return status;
}



DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::enable (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->enable();
}

DDS::StatusCondition_ptr DDS::ExtDomainParticipantImpl::get_statuscondition (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_statuscondition();
}

DDS::StatusMask DDS::ExtDomainParticipantImpl::get_status_changes (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_status_changes();
}

DDS::InstanceHandle_t DDS::ExtDomainParticipantImpl::get_instance_handle (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_instance_handle();
}

DDS::Publisher_ptr DDS::ExtDomainParticipantImpl::create_publisher (
    const DDS::PublisherQos & qos,
    DDS::PublisherListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS {
    return realParticipant->create_publisher(qos, a_listener, mask);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::delete_publisher (
    DDS::Publisher_ptr p
) THROW_ORB_EXCEPTIONS {
    return realParticipant->delete_publisher(p);
}

DDS::Subscriber_ptr DDS::ExtDomainParticipantImpl::create_subscriber (
    const DDS::SubscriberQos & qos,
    DDS::SubscriberListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS {
    return realParticipant->create_subscriber(qos, a_listener, mask);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::delete_subscriber (
    DDS::Subscriber_ptr s
) THROW_ORB_EXCEPTIONS {
    return realParticipant->delete_subscriber(s);
}

DDS::Subscriber_ptr DDS::ExtDomainParticipantImpl::get_builtin_subscriber (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_builtin_subscriber();
}

DDS::Topic_ptr DDS::ExtDomainParticipantImpl::create_topic (
    const char * topic_name,
    const char * type_name,
    const DDS::TopicQos & qos,
    DDS::TopicListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS {
    return realParticipant->create_topic(topic_name, type_name, qos, a_listener, mask);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::delete_topic (
    DDS::Topic_ptr a_topic
) THROW_ORB_EXCEPTIONS {
    return realParticipant->delete_topic(a_topic);
}

DDS::Topic_ptr DDS::ExtDomainParticipantImpl::find_topic (
    const char * topic_name,
    const DDS::Duration_t & timeout
) THROW_ORB_EXCEPTIONS {
    return realParticipant->find_topic(topic_name, timeout);
}

DDS::TopicDescription_ptr DDS::ExtDomainParticipantImpl::lookup_topicdescription (
    const char * name
) THROW_ORB_EXCEPTIONS {
    return realParticipant->lookup_topicdescription(name);
}

DDS::ContentFilteredTopic_ptr DDS::ExtDomainParticipantImpl::create_contentfilteredtopic (
    const char * name,
    DDS::Topic_ptr related_topic,
    const char * filter_expression,
    const DDS::StringSeq & filter_parameters
) THROW_ORB_EXCEPTIONS {
    return realParticipant->create_contentfilteredtopic(
        name,
        related_topic,
        filter_expression,
        filter_parameters);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::delete_contentfilteredtopic (
    DDS::ContentFilteredTopic_ptr a_contentfilteredtopic
) THROW_ORB_EXCEPTIONS {
    return realParticipant->delete_contentfilteredtopic(a_contentfilteredtopic);
}

DDS::MultiTopic_ptr DDS::ExtDomainParticipantImpl::create_multitopic (
    const char * name,
    const char * type_name,
    const char * subscription_expression,
    const DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS {
    return realParticipant->create_multitopic(
        name,
        type_name,
        subscription_expression,
        expression_parameters);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::delete_multitopic (
    DDS::MultiTopic_ptr a_multitopic
) THROW_ORB_EXCEPTIONS {
    return realParticipant->delete_multitopic(a_multitopic);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::delete_contained_entities (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->delete_contained_entities();
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::set_qos (
    const DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->set_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_qos (
    DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::set_listener (
    DDS::DomainParticipantListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS {
    return realParticipant->set_listener(a_listener, mask);
}

DDS::DomainParticipantListener_ptr DDS::ExtDomainParticipantImpl::get_listener (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_listener();
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::ignore_participant (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->ignore_participant(handle);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::ignore_topic (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->ignore_topic(handle);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::ignore_publication (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->ignore_publication(handle);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::ignore_subscription (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->ignore_subscription(handle);
}

char * DDS::ExtDomainParticipantImpl::get_domain_id (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_domain_id();
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::assert_liveliness (
) THROW_ORB_EXCEPTIONS {
    return realParticipant->assert_liveliness();
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::set_default_publisher_qos (
    const DDS::PublisherQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->set_default_publisher_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_default_publisher_qos (
    DDS::PublisherQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_default_publisher_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::set_default_subscriber_qos (
    const DDS::SubscriberQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->set_default_subscriber_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_default_subscriber_qos (
    DDS::SubscriberQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_default_subscriber_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::set_default_topic_qos (
    const DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->set_default_topic_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_default_topic_qos (
    DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_default_topic_qos(qos);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_discovered_participants (
    DDS::InstanceHandleSeq & participant_handles
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_discovered_participants(participant_handles);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_discovered_participant_data (
    DDS::ParticipantBuiltinTopicData & participant_data,
    DDS::InstanceHandle_t participant_handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_discovered_participant_data(participant_data, participant_handle);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_discovered_topics (
    DDS::InstanceHandleSeq & topic_handles
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_discovered_topics(topic_handles);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_discovered_topic_data (
    DDS::TopicBuiltinTopicData & topic_data,
    DDS::InstanceHandle_t topic_handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_discovered_topic_data(topic_data, topic_handle);
}

DDS::Boolean DDS::ExtDomainParticipantImpl::contains_entity (
    DDS::InstanceHandle_t a_handle
) THROW_ORB_EXCEPTIONS {
    return realParticipant->contains_entity(a_handle);
}

DDS::ReturnCode_t DDS::ExtDomainParticipantImpl::get_current_time (
    DDS::Time_t & current_time
) THROW_ORB_EXCEPTIONS {
    return realParticipant->get_current_time(current_time);
}

DDS::ExtDomainParticipant_var::~ExtDomainParticipant_var() {
    DDS::release(ptr_);
}

DDS::ExtDomainParticipant_var & DDS::ExtDomainParticipant_var::operator=(
    const DDS::ExtDomainParticipant_ptr ep
) {
    ptr_ = ep;
    return *this;
}

DDS::ExtDomainParticipant_ptr DDS::ExtDomainParticipant_var::operator->() const {
    return ptr_;
}

DDS::ExtDomainParticipant_var::operator const DDS::DomainParticipant_ptr() const {
    return ptr_->in();
}

DDS::DomainParticipant_ptr DDS::ExtDomainParticipant_var::in() const {
    return ptr_->in();
}
