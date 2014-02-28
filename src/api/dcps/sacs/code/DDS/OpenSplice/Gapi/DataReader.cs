// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

using System;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.Gapi
{
    static internal class DataReader
    {
        /*     ReadCondition
         *     create_readcondition(
         *         in SampleStateMask sample_states,
         *         in ViewStateMask view_states,
         *         in InstanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_create_readcondition")]
        public static extern IntPtr create_readcondition(
            IntPtr _this,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /*     QueryCondition
         *     create_querycondition(
         *         in SampleStateMask sample_states,
         *         in ViewStateMask view_states,
         *         in InstanceStateMask instance_states,
         *         in string query_expression,
         *         in StringSeq query_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_create_querycondition")]
        public static extern IntPtr create_querycondition(
            IntPtr _this,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states,
            string query_expression,
            IntPtr query_parameters);

        /*     ReturnCode_t
         *     delete_readcondition(
         *         in ReadCondition a_condition);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_delete_readcondition")]
        public static extern ReturnCode delete_readcondition(
            IntPtr _this,
            IntPtr a_condition);

        /*     ReturnCode_t
         *     delete_contained_entities();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_delete_contained_entities")]
        public static extern ReturnCode delete_contained_entities(
            IntPtr _this);

        /*     DataReaderView
          *     create_view (
          *     in DataReaderViewQos * qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_create_view")]
        public static extern IntPtr create_view(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     delete_view(
         *        in DataReaderView a_view);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_delete_view")]
        public static extern ReturnCode delete_view(
            IntPtr _this,
            IntPtr a_view);

        /*     ReturnCode_t
         *     set_default_datareaderview_qos(
         *         in DataReaderViewQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_set_default_datareaderview_qos")]
        public static extern ReturnCode set_default_datareaderview_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_datareaderview_qos(
         *         inout DataReaderViewQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_default_datareaderview_qos")]
        public static extern ReturnCode get_default_datareaderview_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_qos(
         *         in DataReaderQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout DataReaderQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_listener(
         *         in DataReaderListener a_listener,
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_set_listener")]
        public static extern ReturnCode set_listener(
            IntPtr _this,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     DataReaderListener
         *     get_listener();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_listener")]
        public static extern IntPtr get_listener(
            IntPtr _this);

        /*     TopicDescription
         *     get_topicdescription();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_topicdescription")]
        public static extern IntPtr get_topicdescription(
            IntPtr _this);

        /*     Subscriber
         *     get_subscriber();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_subscriber")]
        public static extern IntPtr get_subscriber(
            IntPtr _this);

        /*     SampleRejectedStatus
         *     get_sample_rejected_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_sample_rejected_status")]
        public static extern ReturnCode get_sample_rejected_status(
            IntPtr _this,
            SampleRejectedStatus status
            );

        /*     LivelinessChangedStatus
         *     get_liveliness_changed_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_liveliness_changed_status")]
        public static extern ReturnCode get_liveliness_changed_status(
            IntPtr _this,
            LivelinessChangedStatus status
            );

        /*     RequestedDeadlineMissedStatus
         *     get_requested_deadline_missed_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_requested_deadline_missed_status")]
        public static extern ReturnCode get_requested_deadline_missed_status(
            IntPtr _this,
            RequestedDeadlineMissedStatus status
            );

        /*     RequestedIncompatibleQosStatus
         *     get_requested_incompatible_qos_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_requested_incompatible_qos_status")]
        public static extern ReturnCode get_requested_incompatible_qos_status(
            IntPtr _this,
            IntPtr status
            );

        /*     SubscriptionMatchedStatus
         *     get_subscription_match_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_subscription_matched_status")]
        public static extern ReturnCode get_subscription_matched_status(
            IntPtr _this,
            SubscriptionMatchedStatus status
            );

        /*     SampleLostStatus
         *     get_sample_lost_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_sample_lost_status")]
        public static extern ReturnCode get_sample_lost_status(
            IntPtr _this,
            SampleLostStatus status
            );

        /*     ReturnCode_t
         *     wait_for_historical_data(
         *         in Duration_t max_wait);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_wait_for_historical_data")]
        public static extern ReturnCode wait_for_historical_data(
            IntPtr _this,
            ref Duration max_wait);

        /*     ReturnCode_t
         *     wait_for_historical_data_w_condition(
         *         in String filter_expression,
         *         in StringSeq filter_parameters,
         *         in Time_t min_source_timestamp,
         *         in Time_t max_source_timestamp,
         *         in ResourceLimitsQosPolicy resource_limits,
         *         in Duration_t max_wait);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_wait_for_historical_data_w_condition")]
        public static extern ReturnCode wait_for_historical_data_w_condition(
            IntPtr _this,
            string filter_expression,
            IntPtr filter_parameters,
            ref Time min_source_timestamp,
            ref Time max_source_timestamp,
            IntPtr resource_limits,
            ref Duration max_wait);

        /*     ReturnCode_t get_matched_publications(
         *     inout InstanceHandleSeq publication_handles);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_matched_publications")]
        public static extern ReturnCode get_matched_publications(
            IntPtr _this,
            IntPtr publication_handles);

        /*     ReturnCode_t
         *     get_matched_publication_data(
         *         inout PublicationBuiltinTopicData publication_data,
         *         in InstanceHandle_t publication_handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReader_get_matched_publication_data")]
        public static extern ReturnCode get_matched_publication_data(
            IntPtr _this,
            IntPtr publication_data,
            InstanceHandle publication_handle);
    }
}
