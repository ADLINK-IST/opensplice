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
 * Implementation of the {@link DDS.Publisher} interface. 
 */ 
public class PublisherImpl extends EntityImpl implements DDS.Publisher { 

    /* see DDS.PublisherOperations for javadoc */ 
    public DDS.DataWriter create_datawriter (DDS.Topic a_topic, DDS.DataWriterQos qos, DDS.DataWriterListener a_listener, int mask) {
        return jniCreateDatawriter(a_topic, qos, a_listener, mask);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int delete_datawriter (DDS.DataWriter a_datawriter) {
        return jniDeleteDatawriter(a_datawriter);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public DDS.DataWriter lookup_datawriter (String topic_name) {
        return jniLookupDatawriter(topic_name);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int set_qos (DDS.PublisherQos qos) {
        return jniSetQos(qos);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int get_qos (DDS.PublisherQosHolder qos) {
        return jniGetQos(qos);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int set_listener (DDS.PublisherListener a_listener, int mask) {
        return jniSetListener(a_listener, mask);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public DDS.PublisherListener get_listener () {
        return jniGetListener();
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int suspend_publications () {
        return jniSuspendPublications();
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int resume_publications () {
        return jniResumePublications();
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int begin_coherent_changes () {
        return jniBeginCoherentChanges();
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int end_coherent_changes () {
        return jniEndCoherentChanges();
    }

    public int wait_for_acknowledgments(DDS.Duration_t max_wait){
	 return jniWaitForAcknowledgments(max_wait);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public DDS.DomainParticipant get_participant () {
        return jniGetParticipant();
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int set_default_datawriter_qos (DDS.DataWriterQos qos) {
        return jniSetDefaultDatawriterQos(qos);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int get_default_datawriter_qos (DDS.DataWriterQosHolder qos) {
        return jniGetDefaultDatawriterQos(qos);
    }

    /* see DDS.PublisherOperations for javadoc */ 
    public int copy_from_topic_qos (DDS.DataWriterQosHolder a_datawriter_qos, DDS.TopicQos a_topic_qos) {
        return jniCopyFromTopicQos(a_datawriter_qos, a_topic_qos);
    }

    private native DDS.DataWriter jniCreateDatawriter(DDS.Topic a_topic, DDS.DataWriterQos qos, DDS.DataWriterListener a_listener,int mask);
    private native int jniDeleteDatawriter(DDS.DataWriter a_datawriter);
    private native DDS.DataWriter jniLookupDatawriter(String topic_name);
    private native int jniDeleteContainedEntities();
    private native int jniSetQos(DDS.PublisherQos qos);
    private native int jniGetQos(DDS.PublisherQosHolder qos);
    private native int jniSetListener(DDS.PublisherListener a_listener, int mask);
    private native DDS.PublisherListener jniGetListener();
    private native int jniSuspendPublications();
    private native int jniResumePublications();
    private native int jniBeginCoherentChanges();
    private native int jniEndCoherentChanges();
    private native int jniWaitForAcknowledgments(DDS.Duration_t max_wait);
    private native DDS.DomainParticipant jniGetParticipant();
    private native int jniSetDefaultDatawriterQos(DDS.DataWriterQos qos);
    private native int jniGetDefaultDatawriterQos(DDS.DataWriterQosHolder qos);
    private native int jniCopyFromTopicQos(DDS.DataWriterQosHolder a_datawriter_qos, DDS.TopicQos a_topic_qos);
}
