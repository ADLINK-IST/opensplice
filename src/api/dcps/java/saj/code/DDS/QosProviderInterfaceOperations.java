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

package DDS;

public interface QosProviderInterfaceOperations
{
    int get_participant_qos (DDS.DomainParticipantQosHolder participantQos, String id);
    int get_topic_qos (DDS.TopicQosHolder topicQos, String id);
    int get_subscriber_qos (DDS.SubscriberQosHolder subscriberQos, String id);
    int get_datareader_qos (DDS.DataReaderQosHolder datareaderQos, String id);
    int get_publisher_qos (DDS.PublisherQosHolder publisherQos, String id);
    int get_datawriter_qos (DDS.DataWriterQosHolder datawriterQos, String id);
} // interface QosProviderInterfaceOperations
