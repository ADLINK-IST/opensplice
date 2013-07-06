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
package org.opensplice.dds.dlrl;

public final class DCPSUtil{

	private static final DDS.Topic createTopic (DDS.DomainParticipant participant, String topic_name, String type_name){
		DDS.TopicQosHolder qosHolder = new DDS.TopicQosHolder();
        participant.get_default_topic_qos(qosHolder);
        DDS.Duration_t timeout =  new DDS.Duration_t(0, 0);
        /* first do a find, maye the topic already exist. We want to find first so we dont have any nasty error messages
         * when we are creating a topic that already exists with the wrong QoS
         */
        DDS.Topic aTopic  = participant.find_topic (topic_name, timeout);
        if(aTopic == null){
            aTopic = participant.create_topic(topic_name, type_name, qosHolder.value, null, 0);
            if(aTopic == null){
                aTopic  = participant.find_topic (topic_name, timeout);
            }
        }
        return aTopic;
	}

    //TODO ID: 185
	private static final DDS.DataReader createDataReader(DDS.DomainParticipant participant, DDS.Subscriber subscriber,
                                                    DDS.Topic topic, String topicName) throws DDS.DCPSError{
		DDS.DataReader reader = null;
        DDS.TopicDescription topicDes = participant.lookup_topicdescription (topicName);
        if(topicDes != null){
            DDS.TopicQosHolder topicQosHolder =	new DDS.TopicQosHolder();
            DDS.DataReaderQosHolder readerQosHolder = new DDS.DataReaderQosHolder();
            subscriber.get_default_datareader_qos(readerQosHolder);
            topic.get_qos(topicQosHolder);
            int retCode = subscriber.copy_from_topic_qos(readerQosHolder, topicQosHolder.value);
            if(retCode != DDS.RETCODE_OK.value){
                throw new DDS.DCPSError("Unable to create DataReader for topic "+
                                        topicName+
                                        ". Copy from topic QoS failed! ReturnCode: "+
                                        retCode);
            }
            reader = subscriber.create_datareader(topicDes, readerQosHolder.value, null, 0);
        }
		return reader;
	}

    //TODO ID: 185
	private static final DDS.DataWriter createDataWriter(DDS.Publisher publisher, DDS.Topic topic) throws DDS.DCPSError{
		DDS.DataWriter writer = null;
        DDS.TopicQosHolder topicQosHolder =	new DDS.TopicQosHolder();
        DDS.DataWriterQosHolder writerQosHolder =	new DDS.DataWriterQosHolder();
        publisher.get_default_datawriter_qos(writerQosHolder);
        topic.get_qos(topicQosHolder);
		int retCode = publisher.copy_from_topic_qos(writerQosHolder, topicQosHolder.value);
        if(retCode != DDS.RETCODE_OK.value){
            throw new DDS.DCPSError("Unable to create DataWriter. Copy from topic QoS failed! ReturnCode: "+retCode);
        }
        writer = publisher.create_datawriter(topic, writerQosHolder.value, null, 0);

		return writer;
	}

    private static final int deleteTopic(DDS.DomainParticipant participant, DDS.Topic topic){
        return participant.delete_topic(topic);
    }

    private static final int deleteDataReader(DDS.Subscriber subscriber, DDS.DataReader reader){
        return subscriber.delete_datareader(reader);

    }

    private static final int deleteDataWriter(DDS.Publisher publisher, DDS.DataWriter writer){
        return publisher.delete_datawriter(writer);
    }

}