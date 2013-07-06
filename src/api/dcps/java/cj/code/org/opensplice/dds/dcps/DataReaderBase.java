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

public class DataReaderBase extends EntityImpl {

    public class DataReaderBaseImpl extends DDS._DataReaderLocalBase {
        /* datareader operations  */
        public DDS.ReadCondition create_readcondition(int sample_states, int view_states, int instance_states) { return null; }
        public DDS.QueryCondition create_querycondition(int sample_states, int view_states, int instance_states, java.lang.String query_expression, java.lang.String[] query_parameters) { return null; }
        public int delete_readcondition(DDS.ReadCondition a_condition) { return 0; }
        public int delete_contained_entities() { return 0; }
        public int set_qos(DDS.DataReaderQos qos) { return 0; }
        public int get_qos(DDS.DataReaderQosHolder qos) { return 0; }
        public int set_listener(DDS.DataReaderListener a_listener, int mask) { return 0; }
        public DDS.DataReaderListener get_listener() { return null; }
        public DDS.TopicDescription get_topicdescription() { return null; }
        public DDS.Subscriber get_subscriber() { return null; }
        public int get_sample_rejected_status(DDS.SampleRejectedStatusHolder status) { return 0; }
        public int get_liveliness_changed_status(DDS.LivelinessChangedStatusHolder status) { return 0; }
        public int get_requested_deadline_missed_status(DDS.RequestedDeadlineMissedStatusHolder status) { return 0; }
        public int get_requested_incompatible_qos_status(DDS.RequestedIncompatibleQosStatusHolder status) { return 0; }
        public int get_subscription_matched_status(DDS.SubscriptionMatchedStatusHolder status) { return 0; }
        public int get_sample_lost_status(DDS.SampleLostStatusHolder status) { return 0; }
        public int wait_for_historical_data(DDS.Duration_t max_wait) { return 0; }
        public int wait_for_historical_data_w_condition(java.lang.String filter_expression, java.lang.String[] filter_parameters, DDS.Time_t min_source_timestamp, DDS.Time_t max_source_timestamp, DDS.ResourceLimitsQosPolicy resource_limits, DDS.Duration_t max_wait) { return 0; }
        public int get_matched_publications(DDS.InstanceHandleSeqHolder publication_handles) { return 0; }
        public int get_matched_publication_data(DDS.PublicationBuiltinTopicDataHolder publication_data, long publication_handle) { return 0; }
        public DDS.DataReaderView create_view(DDS.DataReaderViewQos qos) { return null; }
        public int delete_view(DDS.DataReaderView a_view) { return 0; }
        public int set_default_datareaderview_qos(DDS.DataReaderViewQos qos) { return 0; }
        public int get_default_datareaderview_qos(DDS.DataReaderViewQosHolder qos) { return 0; }

        /* entity operations */
        public int enable() { return 0; }
        public DDS.StatusCondition get_statuscondition() { return null; }
        public int get_status_changes() { return 0; }
        public long get_instance_handle() { return 0; }

        /* DDS.PropertyInterfaceOperations */
        public int set_property(DDS.Property a_property){ return 0; }
        public int get_property(DDS.PropertyHolder a_property){ return 0; }
    }

    private DDS._DataReaderLocalBase base;

    public DataReaderBase() {
        base = new DataReaderBaseImpl();
    }

    public String[] _ids() {
        return base._ids();
    }

}
