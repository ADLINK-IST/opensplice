
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

#include "Chat_DCPS.hpp"

namespace examples { namespace dcps { namespace Tutorial { namespace isocpp {

#define TERMINATION_MESSAGE -1

/**
 * This class serves as a container holding initialised entities used for publishing.
 */
class PubEntities
{
public:
    /**
     * This constructor initialises the entities used for publishing.
     */
    PubEntities() :
        chatMessageWriter(dds::core::null), nameServiceWriter(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant
            = dds::domain::DomainParticipant(org::opensplice::domain::default_id());

        /** A dds::pub::Publisher is created on the domain participant. */
        dds::pub::qos::PublisherQos pubQos
            = participant.default_publisher_qos()
                << dds::core::policy::Partition("ChatRoom");
        dds::pub::Publisher publisher(participant, pubQos);

        /**
         * A dds::topic::qos::TopicQos is created with Reliability set to Reliable to
         * guarantee delivery.
         */
        dds::topic::qos::TopicQos reliableTopicQos
            = participant.default_topic_qos() << dds::core::policy::Reliability::Reliable();

        /** Set the Reliable TopicQos as the new default */
        participant.default_topic_qos(reliableTopicQos);

        /**
         * A dds::topic::Topic is created for the Chat_ChatMessage sample type on the
         * domain participant.
         */
        dds::topic::Topic<Chat::ChatMessage> chatMessageTopic
            = dds::topic::Topic<Chat::ChatMessage>(participant, "Chat_ChatMessage", reliableTopicQos);

        /** A dds::pub::DataWriter is created for the Chat_ChatMessage Topic with the TopicQos */
        chatMessageWriter
            = dds::pub::DataWriter<Chat::ChatMessage>(publisher, chatMessageTopic, chatMessageTopic.qos());

        /**
         * A dds::topic::qos::TopicQos is created with Durability set to Transient to
         * ensure that if a subscriber joins after the sample is written then DDS
         * will still retain the sample for it.
         */
        dds::topic::qos::TopicQos transientTopicQos
            = participant.default_topic_qos() << dds::core::policy::Durability::Transient();

        /**
         * A dds::topic::Topic is created for the Chat_NameService sample type on the
         * domain participant.
         */
        dds::topic::Topic<Chat::NameService> nameServiceTopic
            = dds::topic::Topic<Chat::NameService>(participant, "Chat_NameService", transientTopicQos);

        /** A dds::pub::DataWriter is created for the Chat_NameService topic with a modififed Qos. */
        dds::pub::qos::DataWriterQos dwQos = nameServiceTopic.qos();
        dwQos << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances();
        nameServiceWriter = dds::pub::DataWriter<Chat::NameService>(publisher, nameServiceTopic, dwQos);
    }

public:
    dds::pub::DataWriter<Chat::ChatMessage> chatMessageWriter;
    dds::pub::DataWriter<Chat::NameService> nameServiceWriter;
};

/**
 * This class serves as a container holding initialised entities used for subscribing.
 */
class SubEntities
{
public:
    /**
     * This constructor initialises the entities for subscribing.
     */
    SubEntities() :
        chatMessageReader(dds::core::null), nameServiceReader(dds::core::null)
    {
        /** A dds::domain::DomainParticipant is created for the default domain. */
        dds::domain::DomainParticipant participant
            = dds::domain::DomainParticipant(org::opensplice::domain::default_id());

        /** A dds::sub::Subscriber is created on the domain participant. */
        dds::sub::qos::SubscriberQos subQos
            = participant.default_subscriber_qos()
                << dds::core::policy::Partition("ChatRoom");
        dds::sub::Subscriber subscriber(participant, subQos);

        /**
         * A dds::topic::qos::TopicQos is created with Reliability set to Reliable to
         * guarantee delivery.
         */
        dds::topic::qos::TopicQos reliableTopicQos
            = participant.default_topic_qos() << dds::core::policy::Reliability::Reliable();

        /** Set the Reliable TopicQos as the new default */
        participant.default_topic_qos(reliableTopicQos);

        /**
         * A dds::topic::Topic is created for the Chat_ChatMessage sample type on the
         * domain participant.
         */
        dds::topic::Topic<Chat::ChatMessage> chatMessageTopic
            = dds::topic::Topic<Chat::ChatMessage>(participant, "Chat_ChatMessage", reliableTopicQos);

        /**
         * A dds::sub::qos::DataReaderQos is created with the history set to KeepAll so
         * that all messages are kept
         */
        dds::sub::qos::DataReaderQos drQos = chatMessageTopic.qos();
        drQos << dds::core::policy::History::KeepAll();

        /** A dds::sub::DataReader is created for the Chat_ChatMessage topic with a modified Qos. */
        chatMessageReader = dds::sub::DataReader<Chat::ChatMessage>(subscriber, chatMessageTopic, drQos);

        /** A dds::topic::qos::TopicQos is created with Durability set to Transient */
        dds::topic::qos::TopicQos transientTopicQos
            = participant.default_topic_qos() << dds::core::policy::Durability::Transient();

        /**
         * A dds::topic::Topic is created for the Chat_NameService sample type on the
         * domain participant.
         */
        dds::topic::Topic<Chat::NameService> nameServiceTopic
            = dds::topic::Topic<Chat::NameService>(participant, "Chat_NameService", transientTopicQos);

        /** A dds::sub::DataReader is created for the Chat_ChatMessage topic with the TopicQos. */
        nameServiceReader
            = dds::sub::DataReader<Chat::NameService>(subscriber, nameServiceTopic, nameServiceTopic.qos());
    }

public:
    dds::sub::DataReader<Chat::ChatMessage> chatMessageReader;
    dds::sub::DataReader<Chat::NameService> nameServiceReader;
};

}
}
}
}
