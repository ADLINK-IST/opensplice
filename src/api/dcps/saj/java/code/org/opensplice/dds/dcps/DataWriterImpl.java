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
 * Implementation of the {@link DDS.DataWriter} interface. 
 */ 
public class DataWriterImpl extends EntityImpl implements DDS.DataWriter { 

    /* see DDS.DataWriterOperations for javadoc */ 
    public int set_qos (DDS.DataWriterQos qos) {
        return jniSetQos(qos);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_qos (DDS.DataWriterQosHolder qos) {
        return jniGetQos(qos);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int set_listener (DDS.DataWriterListener a_listener, int mask) {
        return jniSetListener(a_listener, mask);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public DDS.DataWriterListener get_listener () {
        return jniGetListener();
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public DDS.Topic get_topic () {
        return jniGetTopic();
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public DDS.Publisher get_publisher () {
        return jniGetPublisher();
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int wait_for_acknowledgments(DDS.Duration_t max_wait){
	 return jniWaitForAcknowledgments(max_wait);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_liveliness_lost_status (DDS.LivelinessLostStatusHolder status) {
        return jniGetLivelinessLostStatus(status);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_offered_deadline_missed_status (DDS.OfferedDeadlineMissedStatusHolder status) {
        return jniGetOfferedDeadlineMissedStatus(status);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_offered_incompatible_qos_status (DDS.OfferedIncompatibleQosStatusHolder status) {
        return jniGetOfferedIncompatibleQosStatus(status);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_publication_matched_status (DDS.PublicationMatchedStatusHolder status) {
        return jniGetPublicationMatchedStatus(status);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int assert_liveliness () {
        return jniAssertLiveliness();
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_matched_subscriptions (DDS.InstanceHandleSeqHolder subscription_handles) {
        return jniGetMatchedSubscriptions(subscription_handles);
    }

    /* see DDS.DataWriterOperations for javadoc */ 
    public int get_matched_subscription_data (DDS.SubscriptionBuiltinTopicDataHolder subscription_data, long subscription_handle) {
        return jniGetMatchedSubscriptionData(subscription_data, subscription_handle);
    }

    private native int jniSetQos(DDS.DataWriterQos qos);
    private native int jniGetQos(DDS.DataWriterQosHolder qos);
    private native int jniSetListener(DDS.DataWriterListener a_listener, int mask);
    private native DDS.DataWriterListener jniGetListener();
    private native DDS.Topic jniGetTopic();
    private native DDS.Publisher jniGetPublisher();
    private native int jniWaitForAcknowledgments(DDS.Duration_t max_wait);
    private native int jniGetLivelinessLostStatus(DDS.LivelinessLostStatusHolder status);
    private native int jniGetOfferedDeadlineMissedStatus(DDS.OfferedDeadlineMissedStatusHolder status);
    private native int jniGetOfferedIncompatibleQosStatus(DDS.OfferedIncompatibleQosStatusHolder status);
    private native int jniGetPublicationMatchedStatus(DDS.PublicationMatchedStatusHolder status);
    private native int jniAssertLiveliness();
    private native int jniGetMatchedSubscriptions(DDS.InstanceHandleSeqHolder subscription_handles);
    private native int jniGetMatchedSubscriptionData(DDS.SubscriptionBuiltinTopicDataHolder subscription_data, long subscription_handle);
}
