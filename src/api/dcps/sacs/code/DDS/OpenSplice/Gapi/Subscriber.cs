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
    static internal class Subscriber
    {
        /*     DataReader
         *     create_datareader(
         *         in TopicDescription a_topic,
         *         in DataReaderQos qos,
         *         in DataReaderListener a_listener);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_create_datareader")]
        public static extern IntPtr create_datareader(
            IntPtr _this,
            IntPtr a_topic,
            IntPtr qos,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     ReturnCode_t
         *     delete_datareader(
         *         in DataReader a_datareader);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_delete_datareader")]
        public static extern ReturnCode delete_datareader(
            IntPtr _this,
            IntPtr a_datareader);

        /*     ReturnCode_t
         *     delete_contained_entities();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_delete_contained_entities")]
        public static extern ReturnCode delete_contained_entities(
            IntPtr _this);

        /*     DataReader
         *     lookup_datareader(
         *         in string topic_name);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_lookup_datareader")]
        public static extern IntPtr lookup_datareader(
            IntPtr _this,
            string topic_name);

        /*     ReturnCode_t
         *     get_datareaders(
         *         inout DataReaderSeq readers,
         *         in SampleStateMask sample_states,
         *         in ViewStateMask view_states,
         *         in InstanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_get_datareaders")]
        public static extern ReturnCode get_datareaders(
            IntPtr _this,
            IntPtr readers,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /*     ReturnCode_t
         *     notify_datareaders();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_notify_datareaders")]
        public static extern ReturnCode notify_datareaders(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_qos(
         *         in SubscriberQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout SubscriberQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_listener(
         *         in SubscriberListener a_listener,
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_set_listener")]
        public static extern ReturnCode set_listener(
            IntPtr _this,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     SubscriberListener
         *     get_listener();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_get_listener")]
        public static extern IntPtr get_listener(
            IntPtr _this);

        /*     ReturnCode_t
         *     begin_access();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_begin_access")]
        public static extern ReturnCode begin_access(
            IntPtr _this);

        /*     ReturnCode_t
         *     end_access();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_end_access")]
        public static extern ReturnCode end_access(
            IntPtr _this);

        /*     DomainParticipant
         *     get_participant();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_get_participant")]
        public static extern IntPtr get_participant(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_default_datareader_qos(
         *         in DataReaderQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_set_default_datareader_qos")]
        public static extern ReturnCode set_default_datareader_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_datareader_qos(
         *         inout DataReaderQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_get_default_datareader_qos")]
        public static extern ReturnCode get_default_datareader_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     copy_from_topic_qos(
         *         inout DataReaderQos a_datareader_qos,
         *         in TopicQos a_topic_qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriber_copy_from_topic_qos")]
        public static extern ReturnCode copy_from_topic_qos(
            IntPtr _this,
            IntPtr a_datareader_qos,
            IntPtr a_topic_qos);
    }
}
