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

import DDS.DataReaderView;
import DDS.DataReaderViewQos;
import DDS.DataReaderViewQosHolder;

/** 
 * Implementation of the {@link DDS.DataReader} interface. 
 */ 
public class DataReaderImpl extends EntityImpl implements DDS.DataReader { 

    /* see DDS.DataReaderOperations for javadoc */ 
    public DDS.ReadCondition create_readcondition (int sample_states, int view_states, int instance_states) {
        return jniCreateReadcondition(sample_states, view_states, instance_states);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public DDS.QueryCondition create_querycondition (int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters) {
        return jniCreateQuerycondition(sample_states, view_states, instance_states, query_expression, query_parameters);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int delete_readcondition (DDS.ReadCondition a_condition) {
        return jniDeleteReadcondition(a_condition);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int set_qos (DDS.DataReaderQos qos) {
        return jniSetQos(qos);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_qos (DDS.DataReaderQosHolder qos) {
        return jniGetQos(qos);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int set_listener (DDS.DataReaderListener a_listener, int mask) {
        return jniSetListener(a_listener, mask);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public DDS.DataReaderListener get_listener () {
        return jniGetListener();
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public DDS.TopicDescription get_topicdescription () {
        return jniGetTopicdescription();
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public DDS.Subscriber get_subscriber () {
        return jniGetSubscriber();
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_sample_rejected_status (DDS.SampleRejectedStatusHolder status) {
        return jniGetSampleRejectedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_liveliness_changed_status (DDS.LivelinessChangedStatusHolder status) {
        return jniGetLivelinessChangedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_requested_deadline_missed_status (DDS.RequestedDeadlineMissedStatusHolder status) {
        return jniGetRequestedDeadlineMissedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_requested_incompatible_qos_status (DDS.RequestedIncompatibleQosStatusHolder status) {
        return jniGetRequestedIncompatibleQosStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_subscription_matched_status (DDS.SubscriptionMatchedStatusHolder status) {
        return jniGetSubscriptionMatchedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_sample_lost_status (DDS.SampleLostStatusHolder status) {
        return jniGetSampleLostStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int wait_for_historical_data (DDS.Duration_t max_wait) {
        return jniWaitForHistoricalData(max_wait);
    }
    
    public int wait_for_historical_data_w_condition(String filter_expression, String[] expression_parameters, DDS.Time_t min_source_timestamp, DDS.Time_t max_source_timestamp, DDS.ResourceLimitsQosPolicy resource_limits, DDS.Duration_t max_wait){
        return jniWaitForHistoricalDataWCondition(filter_expression, expression_parameters, min_source_timestamp, max_source_timestamp, resource_limits, max_wait);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_matched_publications (DDS.InstanceHandleSeqHolder publication_handles) {
        return jniGetMatchedPublications(publication_handles);
    }

    /* see DDS.DataReaderOperations for javadoc */ 
    public int get_matched_publication_data (DDS.PublicationBuiltinTopicDataHolder publication_data, long publication_handle) {
        return jniGetMatchedPublicationData(publication_data, publication_handle);
    }

    public DDS.DataReaderView create_view(DataReaderViewQos qos){
        return jniCreateView(qos);
    }
    public int delete_view(DataReaderView a_view){
        return jniDeleteView(a_view);
    }
    
    public int get_default_datareaderview_qos(DataReaderViewQosHolder qos){
        return jniGetDefaultDataReaderViewQos(qos);
    }
    
    public int set_default_datareaderview_qos(DataReaderViewQos qos){
        return jniSetDefaultDataReaderViewQos(qos);
    }

    private native DDS.ReadCondition jniCreateReadcondition(int sample_states, int view_states, int instance_states);
    private native DDS.QueryCondition jniCreateQuerycondition(int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters);
    private native int jniDeleteReadcondition(DDS.ReadCondition a_condition);
    private native int jniDeleteContainedEntities();
    private native int jniSetQos(DDS.DataReaderQos qos);
    private native int jniGetQos(DDS.DataReaderQosHolder qos);
    private native int jniSetListener(DDS.DataReaderListener a_listener, int mask);
    private native DDS.DataReaderListener jniGetListener();
    private native DDS.TopicDescription jniGetTopicdescription();
    private native DDS.Subscriber jniGetSubscriber();
    private native int jniGetSampleRejectedStatus(DDS.SampleRejectedStatusHolder status);
    private native int jniGetLivelinessChangedStatus(DDS.LivelinessChangedStatusHolder status);
    private native int jniGetRequestedDeadlineMissedStatus(DDS.RequestedDeadlineMissedStatusHolder status);
    private native int jniGetRequestedIncompatibleQosStatus(DDS.RequestedIncompatibleQosStatusHolder status);
    private native int jniGetSubscriptionMatchedStatus(DDS.SubscriptionMatchedStatusHolder status);
    private native int jniGetSampleLostStatus(DDS.SampleLostStatusHolder status);
    private native int jniWaitForHistoricalData(DDS.Duration_t max_wait);
    private native int jniWaitForHistoricalDataWCondition(String filter_expression, String[] expression_parameters, DDS.Time_t min_source_timestamp, DDS.Time_t max_source_timestamp, DDS.ResourceLimitsQosPolicy resource_limits, DDS.Duration_t max_wait);
    private native int jniGetMatchedPublications(DDS.InstanceHandleSeqHolder publication_handles);
    private native int jniGetMatchedPublicationData(DDS.PublicationBuiltinTopicDataHolder publication_data, long publication_handle);
    private native DDS.DataReaderView jniCreateView(DDS.DataReaderViewQos qos);
    private native int jniDeleteView(DataReaderView a_view);
    private native int jniGetDefaultDataReaderViewQos(DataReaderViewQosHolder qos);
    private native int jniSetDefaultDataReaderViewQos(DataReaderViewQos qos);
}
