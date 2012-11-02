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
 * LOGICAL_NAME:    multitopic.h
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the headers for all operations required to simulate 
 * the MultiTopic behavior.
 * 
 ***/

#include <string>

#include "ccpp_dds_dcps.h"
#include "ccpp_Chat.h"
#include "orb_abstraction.h"


namespace DDS {
    
class DataReaderListenerImpl : public virtual DDS::DataReaderListener {

    /* Caching variables */
    Long                                previous;
    std::string                         userName;

public:
    /* Type-specific DDS entities */
    Chat::ChatMessageDataReader_var     chatMessageDR;
    Chat::NameServiceDataReader_var     nameServiceDR;
    Chat::NamedMessageDataWriter_var    namedMessageDW;

    /* Query related stuff */
    DDS::QueryCondition_var             nameFinder;
    DDS::StringSeq                      nameFinderParams;
    
    
    /* Constructor */
    DataReaderListenerImpl();
    
    /* Callback method implementation. */    
    virtual void on_requested_deadline_missed (
        DDS::DataReader_ptr reader,
        const DDS::RequestedDeadlineMissedStatus & status
    ) THROW_ORB_EXCEPTIONS;

    virtual void on_requested_incompatible_qos (
        DDS::DataReader_ptr reader,
        const DDS::RequestedIncompatibleQosStatus & status
    ) THROW_ORB_EXCEPTIONS;

    virtual void on_sample_rejected (
        DDS::DataReader_ptr reader,
        const DDS::SampleRejectedStatus & status
    ) THROW_ORB_EXCEPTIONS;

    virtual void on_liveliness_changed (
        DDS::DataReader_ptr reader,
        const DDS::LivelinessChangedStatus & status
    ) THROW_ORB_EXCEPTIONS;

    virtual void on_data_available (
        DDS::DataReader_ptr reader
    ) THROW_ORB_EXCEPTIONS;

    virtual void on_subscription_matched (
        DDS::DataReader_ptr reader,
        const DDS::SubscriptionMatchedStatus & status
    ) THROW_ORB_EXCEPTIONS;
                   
    virtual void on_sample_lost (
        DDS::DataReader_ptr reader,
        const DDS::SampleLostStatus & status
    ) THROW_ORB_EXCEPTIONS;
};

class ExtDomainParticipantImpl;

typedef ExtDomainParticipantImpl *ExtDomainParticipant_ptr;

class ExtDomainParticipant_var {
    ExtDomainParticipant_ptr ptr_;
public:
    ExtDomainParticipant_var() : ptr_(NULL){};
    ~ExtDomainParticipant_var();
    ExtDomainParticipant_var & operator=(const DDS::ExtDomainParticipant_ptr ep);
    DDS::ExtDomainParticipant_ptr operator->() const;
    operator const DDS::DomainParticipant_ptr() const;
    DDS::DomainParticipant_ptr in() const;
};


class ExtDomainParticipantImpl
    : public virtual DDS::DomainParticipant,
      public LOCAL_REFCOUNTED_OBJECT              
{
    /***
     * Attributes
     ***/
     
    // Encapsulated DomainParticipant.
    DDS::DomainParticipant_var          realParticipant;
    
    /*Implementation for DataReaderListener */
    DDS::DataReaderListenerImpl         *msgListener;

    /* Generic DDS entities */
    DDS::Topic_var                      chatMessageTopic;
    DDS::Topic_var                      nameServiceTopic;
    DDS::ContentFilteredTopic_var       filteredMessageTopic;
    DDS::Topic_var                      namedMessageTopic;
    DDS::Subscriber_var                 multiSub;
    DDS::Publisher_var                  multiPub;
    
    /***
     * Operations
     ***/
public:

    // Simulating a narrow operation.
    static ExtDomainParticipant_ptr _narrow (
        DDS::DomainParticipant_ptr obj
    );
    
    // Simulating an in() parameter where a DomainParticipant is expected.
    DDS::DomainParticipant_ptr in();

    // Constructor
    ExtDomainParticipantImpl(DomainParticipant_ptr participant);

    virtual DDS::Topic_ptr create_simulated_multitopic (
        const char * name,
        const char * type_name,
        const char * subscription_expression,
        const DDS::StringSeq & expression_parameters
    );

    virtual DDS::ReturnCode_t delete_simulated_multitopic (
        DDS::TopicDescription_ptr a_topic
    );
    
    virtual DDS::ReturnCode_t enable (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::StatusCondition_ptr get_statuscondition (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::StatusMask get_status_changes (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::InstanceHandle_t get_instance_handle (
    ) THROW_ORB_EXCEPTIONS;
    
    virtual DDS::Publisher_ptr create_publisher (
        const DDS::PublisherQos & qos,
        DDS::PublisherListener_ptr a_listener,
        DDS::StatusMask mask
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t delete_publisher (
        DDS::Publisher_ptr p
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::Subscriber_ptr create_subscriber (
        const DDS::SubscriberQos & qos,
        DDS::SubscriberListener_ptr a_listener,
        DDS::StatusMask mask
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t delete_subscriber (
        DDS::Subscriber_ptr s
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::Subscriber_ptr get_builtin_subscriber (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::Topic_ptr create_topic (
        const char * topic_name,
        const char * type_name,
        const DDS::TopicQos & qos,
        DDS::TopicListener_ptr a_listener,
        DDS::StatusMask mask
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t delete_topic (
        DDS::Topic_ptr a_topic
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::Topic_ptr find_topic (
        const char * topic_name,
        const DDS::Duration_t & timeout
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::TopicDescription_ptr lookup_topicdescription (
        const char * name
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ContentFilteredTopic_ptr create_contentfilteredtopic (
        const char * name,
        DDS::Topic_ptr related_topic,
        const char * filter_expression,
        const DDS::StringSeq & filter_parameters
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t delete_contentfilteredtopic (
        DDS::ContentFilteredTopic_ptr a_contentfilteredtopic
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::MultiTopic_ptr create_multitopic (
        const char * name,
        const char * type_name,
        const char * subscription_expression,
        const DDS::StringSeq & expression_parameters
    ) THROW_ORB_EXCEPTIONS;
            
    virtual DDS::ReturnCode_t delete_multitopic (
        DDS::MultiTopic_ptr a_multitopic
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t delete_contained_entities (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t set_qos (
        const DDS::DomainParticipantQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_qos (
        DDS::DomainParticipantQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t set_listener (
        DDS::DomainParticipantListener_ptr a_listener,
        DDS::StatusMask mask
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::DomainParticipantListener_ptr get_listener (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t ignore_participant (
        DDS::InstanceHandle_t handle
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t ignore_topic (
        DDS::InstanceHandle_t handle
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t ignore_publication (
        DDS::InstanceHandle_t handle
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t ignore_subscription (
        DDS::InstanceHandle_t handle
    ) THROW_ORB_EXCEPTIONS;

    virtual char * get_domain_id (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t assert_liveliness (
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t set_default_publisher_qos (
        const DDS::PublisherQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_default_publisher_qos (
        DDS::PublisherQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t set_default_subscriber_qos (
        const DDS::SubscriberQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_default_subscriber_qos (
        DDS::SubscriberQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t set_default_topic_qos (
        const DDS::TopicQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_default_topic_qos (
        DDS::TopicQos & qos
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_discovered_participants (
        DDS::InstanceHandleSeq & participant_handles
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_discovered_participant_data (
        DDS::ParticipantBuiltinTopicData & participant_data,
        DDS::InstanceHandle_t participant_handle
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_discovered_topics (
        DDS::InstanceHandleSeq & topic_handles
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_discovered_topic_data (
        DDS::TopicBuiltinTopicData & topic_data,
        DDS::InstanceHandle_t topic_handle
    ) THROW_ORB_EXCEPTIONS;

    virtual Boolean contains_entity (
        DDS::InstanceHandle_t a_handle
    ) THROW_ORB_EXCEPTIONS;

    virtual DDS::ReturnCode_t get_current_time (
        DDS::Time_t & current_time
    ) THROW_ORB_EXCEPTIONS;
};

}
