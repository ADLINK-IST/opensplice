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

package org.opensplice.dds.dcps;

public class QosProviderBase extends SajSuperClass {

    public class QosProviderBaseImpl extends DDS._QosProviderInterfaceLocalBase {
        /* QosProviderInterfaceOperations  */
        public int get_participant_qos (DDS.DomainParticipantQosHolder participantQos, String id) { return 0; }
        public int get_topic_qos (DDS.TopicQosHolder topicQos, String id) { return 0; }
        public int get_subscriber_qos (DDS.SubscriberQosHolder subscriberQos, String id) { return 0; }
        public int get_datareader_qos (DDS.DataReaderQosHolder datareaderQos, String id) { return 0; }
        public int get_publisher_qos (DDS.PublisherQosHolder publisherQos, String id) { return 0; }
        public int get_datawriter_qos (DDS.DataWriterQosHolder datawriterQos, String id) { return 0; }
    }

    private DDS._QosProviderInterfaceLocalBase base;

    public QosProviderBase() {
        base = new QosProviderBaseImpl();
    }

    public String[] _ids() {
        return base._ids();
    }
}
