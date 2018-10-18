/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.dds.dcps;

public abstract class DataReaderBase extends EntityImpl {

    private static final long serialVersionUID = 5928312686023862501L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class DataReaderBaseImpl extends DDS._DataReaderLocalBase {
        private static final long serialVersionUID = -4861800415329657203L;

        /* datareader operations */
        @Override
        public DDS.ReadCondition create_readcondition(int sample_states,
                int view_states, int instance_states) {
            return null;
        }

        @Override
        public DDS.QueryCondition create_querycondition(int sample_states,
                int view_states, int instance_states,
                java.lang.String query_expression,
                java.lang.String[] query_parameters) {
            return null;
        }

        @Override
        public int delete_readcondition(DDS.ReadCondition a_condition) {
            return 0;
        }

        @Override
        public int delete_contained_entities() {
            return 0;
        }

        @Override
        public int set_qos(DDS.DataReaderQos qos) {
            return 0;
        }

        @Override
        public int get_qos(DDS.DataReaderQosHolder qos) {
            return 0;
        }

        @Override
        public int set_listener(DDS.DataReaderListener a_listener, int mask) {
            return 0;
        }

        @Override
        public DDS.DataReaderListener get_listener() {
            return null;
        }

        @Override
        public DDS.TopicDescription get_topicdescription() {
            return null;
        }

        @Override
        public DDS.Subscriber get_subscriber() {
            return null;
        }

        @Override
        public int get_sample_rejected_status(
                DDS.SampleRejectedStatusHolder status) {
            return 0;
        }

        @Override
        public int get_liveliness_changed_status(
                DDS.LivelinessChangedStatusHolder status) {
            return 0;
        }

        @Override
        public int get_requested_deadline_missed_status(
                DDS.RequestedDeadlineMissedStatusHolder status) {
            return 0;
        }

        @Override
        public int get_requested_incompatible_qos_status(
                DDS.RequestedIncompatibleQosStatusHolder status) {
            return 0;
        }

        @Override
        public int get_subscription_matched_status(
                DDS.SubscriptionMatchedStatusHolder status) {
            return 0;
        }

        @Override
        public int get_sample_lost_status(DDS.SampleLostStatusHolder status) {
            return 0;
        }

        @Override
        public int wait_for_historical_data(DDS.Duration_t max_wait) {
            return 0;
        }

        @Override
        public int wait_for_historical_data_w_condition(
                java.lang.String filter_expression,
                java.lang.String[] filter_parameters,
                DDS.Time_t min_source_timestamp,
                DDS.Time_t max_source_timestamp,
                DDS.ResourceLimitsQosPolicy resource_limits,
                DDS.Duration_t max_wait) {
            return 0;
        }

        @Override
        public int get_matched_publications(
                DDS.InstanceHandleSeqHolder publication_handles) {
            return 0;
        }

        @Override
        public int get_matched_publication_data(
                DDS.PublicationBuiltinTopicDataHolder publication_data,
                long publication_handle) {
            return 0;
        }

        @Override
        public DDS.DataReaderView create_view(DDS.DataReaderViewQos qos) {
            return null;
        }

        @Override
        public int delete_view(DDS.DataReaderView a_view) {
            return 0;
        }

        @Override
        public int set_default_datareaderview_qos(DDS.DataReaderViewQos qos) {
            return 0;
        }

        @Override
        public int get_default_datareaderview_qos(
                DDS.DataReaderViewQosHolder qos) {
            return 0;
        }

        /* entity operations */
        @Override
        public int enable() {
            return 0;
        }

        @Override
        public DDS.StatusCondition get_statuscondition() {
            return null;
        }

        @Override
        public int get_status_changes() {
            return 0;
        }

        @Override
        public long get_instance_handle() {
            return 0;
        }

        /* DDS.PropertyInterfaceOperations */
        @Override
        public int set_property(DDS.Property a_property) {
            return 0;
        }

        @Override
        public int get_property(DDS.PropertyHolder a_property) {
            return 0;
        }
    }

    private DDS._DataReaderLocalBase base;

    public DataReaderBase() {
        base = new DataReaderBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
