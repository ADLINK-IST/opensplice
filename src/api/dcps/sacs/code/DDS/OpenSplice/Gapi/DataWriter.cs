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
    static internal class DataWriter
    {
        /*     ReturnCode_t
         *     set_qos(
         *         in DataWriterQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout DataWriterQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_listener(
         *         in DataWriterListener a_listener,
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_set_listener")]
        public static extern ReturnCode set_listener(
            IntPtr _this,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     DataWriterListener
         *     get_listener();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_listener")]
        public static extern IntPtr get_listener(
            IntPtr _this);

        /*     Topic
         *     get_topic();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_topic")]
        public static extern IntPtr get_topic(
            IntPtr _this);

        /*     Publisher
         *     get_publisher();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_publisher")]
        public static extern IntPtr get_publisher(
            IntPtr _this);

        /* ReturnCode_t
         *   wait_for_acknowledgments(
         *      in Duration_t max_wait);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_wait_for_acknowledgments")]
        public static extern ReturnCode wait_for_acknowledgments(
            IntPtr _this,
            ref Duration max_wait
            );

        /*     // Access the status
         *     LivelinessLostStatus
         *     get_liveliness_lost_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_liveliness_lost_status")]
        public static extern ReturnCode get_liveliness_lost_status(
            IntPtr _this,
            LivelinessLostStatus status
            );

        /*     OfferedDeadlineMissedStatus
         *     get_offered_deadline_missed_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_offered_deadline_missed_status")]
        public static extern ReturnCode get_offered_deadline_missed_status(
            IntPtr _this,
            OfferedDeadlineMissedStatus status
            );

        /*     OfferedIncompatibleQosStatus
         *     get_offered_incompatible_qos_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_offered_incompatible_qos_status")]
        public static extern ReturnCode get_offered_incompatible_qos_status(
            IntPtr _this,
            IntPtr status
            );

        /*     PublicationMatchedStatus
         *     get_publication_matched_status();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_publication_matched_status")]
        public static extern ReturnCode get_publication_matched_status(
            IntPtr _this,
            PublicationMatchedStatus status
            );

        /*     ReturnCode_t
         *     assert_liveliness();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_assert_liveliness")]
        public static extern ReturnCode assert_liveliness(
            IntPtr _this);

        /*     ReturnCode_t
         *     get_matched_subscriptions(
         *         inout InstanceHandleSeq subscription_handles);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_matched_subscriptions")]
        public static extern ReturnCode get_matched_subscriptions(
            IntPtr _this,
            IntPtr subscription_handles);

        /*     ReturnCode_t
         *     get_matched_subscription_data(
         *         inout SubscriptionBuiltinTopicData subscription_data,
         *         in InstanceHandle_t subscription_handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataWriter_get_matched_subscription_data")]
        public static extern ReturnCode get_matched_subscription_data(
            IntPtr _this,
            IntPtr subscription_data,
            InstanceHandle subscription_handle);
    }
}
