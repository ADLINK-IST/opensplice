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


package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.Subscriber} interface. 
 */ 
public class SubscriberImpl extends EntityImpl implements DDS.Subscriber { 

    /* see DDS.SubscriberOperations for javadoc */ 
    public DDS.DataReader create_datareader (DDS.TopicDescription a_topic, DDS.DataReaderQos qos, DDS.DataReaderListener a_listener, int mask) {
        return jniCreateDatareader(a_topic, qos, a_listener,mask);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int delete_datareader (DDS.DataReader a_datareader) {
        return jniDeleteDatareader(a_datareader);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public DDS.DataReader lookup_datareader (String topic_name) {
        return jniLookupDatareader(topic_name);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int get_datareaders (DDS.DataReaderSeqHolder readers, int sample_states, int view_states, int instance_states) {
        return jniGetDatareaders(readers, sample_states, view_states, instance_states);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int notify_datareaders () {
        return jniNotifyDatareaders();
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int set_qos (DDS.SubscriberQos qos) {
        return jniSetQos(qos);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int get_qos (DDS.SubscriberQosHolder qos) {
        return jniGetQos(qos);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int set_listener (DDS.SubscriberListener a_listener, int mask) {
        return jniSetListener(a_listener, mask);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public DDS.SubscriberListener get_listener () {
        return jniGetListener();
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int begin_access () {
        return jniBeginAccess();
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int end_access () {
        return jniEndAccess();
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public DDS.DomainParticipant get_participant () {
        return jniGetParticipant();
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int set_default_datareader_qos (DDS.DataReaderQos qos) {
        return jniSetDefaultDatareaderQos(qos);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int get_default_datareader_qos (DDS.DataReaderQosHolder qos) {
        return jniGetDefaultDatareaderQos(qos);
    }

    /* see DDS.SubscriberOperations for javadoc */ 
    public int copy_from_topic_qos (DDS.DataReaderQosHolder a_datareader_qos, DDS.TopicQos a_topic_qos) {
        return jniCopyFromTopicQos(a_datareader_qos, a_topic_qos);
    }

    private native DDS.DataReader jniCreateDatareader(DDS.TopicDescription a_topic, DDS.DataReaderQos qos, DDS.DataReaderListener a_listener, int mask);
    private native int jniDeleteDatareader(DDS.DataReader a_datareader);
    private native int jniDeleteContainedEntities();
    private native DDS.DataReader jniLookupDatareader(String topic_name);
    private native int jniGetDatareaders(DDS.DataReaderSeqHolder readers, int sample_states, int view_states, int instance_states);
    private native int jniNotifyDatareaders();
    private native int jniSetQos(DDS.SubscriberQos qos);
    private native int jniGetQos(DDS.SubscriberQosHolder qos);
    private native int jniSetListener(DDS.SubscriberListener a_listener, int mask);
    private native DDS.SubscriberListener jniGetListener();
    private native int jniBeginAccess();
    private native int jniEndAccess();
    private native DDS.DomainParticipant jniGetParticipant();
    private native int jniSetDefaultDatareaderQos(DDS.DataReaderQos qos);
    private native int jniGetDefaultDatareaderQos(DDS.DataReaderQosHolder qos);
    private native int jniCopyFromTopicQos(DDS.DataReaderQosHolder a_datareader_qos, DDS.TopicQos a_topic_qos);
}
