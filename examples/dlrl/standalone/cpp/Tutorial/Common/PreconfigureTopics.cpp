#include "PreconfigureTopics.h"
#include "DlrlUtility.h"
#include "ccpp_ExtChat.h"

using namespace Common;

static DDS::DomainParticipantFactory_ptr dpFactory = DDS::DomainParticipantFactory::get_instance();

void
PreconfigureTopics::createTopicWithQos(
    DDS::DomainParticipant_ptr participant,
    DDS::TypeSupport_ptr ts,
    const char* topicName,
    DDS::ReliabilityQosPolicyKind rKind,
    DDS::DurabilityQosPolicyKind dKind)
{
    DDS::TopicQos tQos;
    DDS::Topic_var aTopic;
    char* typeName;
    int result;

    /* Invoke the method that obtains the type_name. */
    typeName = ts->get_type_name();

    /* Register the specified TypeSupport in the specified participant. */
    result = ts->register_type(participant, typeName);
    if(result != DDS::RETCODE_OK)
    {
        throw DDS::DCPSError("Unable to register the TypeSupport for requested type.");
    }

    /* Obtain the default QoS settings for this domain and tailor the result
     * to the specified Qos policy values.
     */
    participant->get_default_topic_qos(tQos);
    tQos.reliability.kind = rKind;
    tQos.durability.kind = dKind;

    /* Create a topic with the tailored Qos. */
    aTopic = participant->create_topic(
        topicName,
        typeName,
        tQos,
        NULL,
        0);
    if(!aTopic)
    {
        throw DDS::DCPSError("Unable to create a topic with name for request type.");
    }
}

void
PreconfigureTopics::simulateExternalTopicCreation(
    )
{
    DDS::DomainParticipantQos dpQos;
    DDS::DomainParticipant_var participant;

    /* Create a participant to use for the topic creation */
    dpFactory->get_default_participant_qos(dpQos);
    participant = dpFactory->create_participant(DDS::DOMAIN_ID_DEFAULT, dpQos, NULL, 0);
    if (!participant)
    {
        throw DDS::DCPSError("Could not create DCPS DomainParticipant: Aborting...");
    }
    /* Now we can create the topics and by doing that inject them into the
     * system. We will only do this for the topics that were explicitly
     * mapped, as the 'WhiteList' object will be mapped by default by DLRL
     * and therefore created by the DLRL application.
     */
    try
    {
        Chat::ChatMessageTypeSupport_var chatmessageTS;
        Chat::NameServiceTypeSupport_var nameServiceTS;

        chatmessageTS = new Chat::ChatMessageTypeSupport();
        createTopicWithQos(
            participant.in(),
            chatmessageTS,
            "Chat_ChatMessage",
            DDS::RELIABLE_RELIABILITY_QOS,
            DDS::VOLATILE_DURABILITY_QOS);

        nameServiceTS = new Chat::NameServiceTypeSupport();
        createTopicWithQos(
            participant.in(),
            nameServiceTS,
            "Chat_NameService",
            DDS::RELIABLE_RELIABILITY_QOS,
            DDS::TRANSIENT_DURABILITY_QOS);
    }
    catch (DDS::DCPSError e)
    {
        /* Do not forget to clean up resources! */
        DlrlUtility::deleteParticipant(participant);
        throw e;
    }
    /* Do not forget to clean up resources! */
    DlrlUtility::deleteParticipant(participant);
}

