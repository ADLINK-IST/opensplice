package Common;

/**
 * A common use case for DLRL is to map the DLRL objects onto already existing
 * DCPS topics. In most of these situations the DCPS topics that already exist
 * will have very specific system wide QoS settings in place.
 *
 * When the DLRL application enters the playing field and creates it's topics
 * it will always first do a lookup of the topic and re-use the already set
 * topic qos. If no topic can be found, then it will create the topic itself.
 *
 * This class helps to simulate how the DLRL easily copies over the existing
 * topic QoS settings when it joins a system where the topics are already
 * defined. The DLRL will thus effectively apply all QoS settings needed on the
 * topic, removing that burden from the application developer.
 */
public class PreconfigureTopics
{
    private static DDS.DomainParticipantFactory dpFactory = DDS.DomainParticipantFactory.get_instance();

    public static void simulateExternalTopicCreation(
        ) throws DDS.DCPSError
    {
        DDS.DomainParticipantQosHolder dpQos;
        DDS.DomainParticipant participant;

        /* Create a participant to use for the topic creation */
        dpQos = new DDS.DomainParticipantQosHolder();
        dpFactory.get_default_participant_qos(dpQos);
        participant = dpFactory.create_participant(DDS.DOMAIN_ID_DEFAULT.value, dpQos.value, null, 0);
        if (participant == null)
        {
            throw new DDS.DCPSError("Could not create DCPS DomainParticipant: Aborting...\n");
        }
        /* Now we can create the topics and by doing that inject them into the
         * system. We will only do this for the topics that were explicitly
         * mapped, as the 'WhiteList' object will be mapped by default by DLRL
         * and therefore created by the DLRL application.
         */
        try
        {
            createTopicWithQos(
                participant,
                new Chat.ChatMessageTypeSupport(),
                "Chat_ChatMessage",
                DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS,
                DDS.DurabilityQosPolicyKind.VOLATILE_DURABILITY_QOS);

            createTopicWithQos(
                participant,
                new Chat.NameServiceTypeSupport(),
                "Chat_NameService",
                DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS,
                DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS);
        }
        catch (DDS.DCPSError e)
        {

            throw e;
        }
        /* Do not forget to clean up resources! */
        DlrlUtility.deleteParticipant(participant);
    }

    /**
     * This operation registers the specified TypeSupport in the specified
     * DomainParticipant and creates the corresponding topic in that participant
     * using the specified topic name and the specified Reliability and
     * Durability QosPolicy settings.
     *
     * @param participant the participant in which the specified topic will need
     * to be created.
     * @param ts the TypeSupport that needs to be registered in this participant
     * for the specified topic.
     * @param topicName the name of the topic that needs to be created in this
     * participant.
     * @param rKind the value for the Reliability QosPolicy setting of the
     * specified topic.
     * @param dKind the value for the Durability QosPolicy setting of the
     * specified topic.
     * @throws DDS.DCPSError when something during initialization goes wrong.
     */
    private static final void createTopicWithQos(
        DDS.DomainParticipant participant,
        DDS.TypeSupport ts,
        String topicName,
        DDS.ReliabilityQosPolicyKind rKind,
        DDS.DurabilityQosPolicyKind dKind) throws DDS.DCPSError
    {
        DDS.TopicQosHolder tQos = new DDS.TopicQosHolder();
        DDS.Topic aTopic;
        String typeName;
        int result;

        /* Invoke the method that obtains the type_name. */
        typeName = ts.get_type_name();

        /* Register the specified TypeSupport in the specified participant. */
        result = ts.register_type(participant, typeName);
        if(result != DDS.RETCODE_OK.value)
        {
        	throw new DDS.DCPSError("Unable to register the TypeSupport for type '"+
                typeName + "'.");
        }

        /* Obtain the default QoS settings for this domain and tailor the result
         * to the specified Qos policy values.
         */
        participant.get_default_topic_qos(tQos);
        tQos.value.reliability.kind = rKind;
        tQos.value.durability.kind = dKind;

        /* Create a topic with the tailored Qos. */
        aTopic = participant.create_topic(
            topicName,
            typeName,
            tQos.value,
            null,
            0);
        if(aTopic == null)
        {
        	throw new DDS.DCPSError("Unable to create a topic with name = '" +
                topicName + "' and type = '" + typeName + "'.");
        }
	}
}
