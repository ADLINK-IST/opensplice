#include "ccpp_dds_dlrl.h"
#include "dlrl_tutorial_if.h"

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
namespace Common
{

    class DLRLTUT_API PreconfigureTopics
    {
        public:
            static void
            simulateExternalTopicCreation(
                );


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
        private:
            static void
            createTopicWithQos(
                DDS::DomainParticipant_ptr participant,
                DDS::TypeSupport_ptr ts,
                const char* topicName,
                DDS::ReliabilityQosPolicyKind rKind,
                DDS::DurabilityQosPolicyKind dKind);
    };
};
