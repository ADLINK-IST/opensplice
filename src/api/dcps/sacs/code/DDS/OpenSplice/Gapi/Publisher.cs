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
    static internal class Publisher
    {
        /*     DataWriter
         *     create_datawriter(
         *         in Topic a_topic,
         *         in DataWriterQos qos,
         *         in DataWriterListener a_listener);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_create_datawriter")]
        public static extern IntPtr create_datawriter(
            IntPtr _this,
            IntPtr a_topic,
            IntPtr qos,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     ReturnCode_t
         *     delete_datawriter(
         *         in DataWriter a_datawriter);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_delete_datawriter")]
        public static extern ReturnCode delete_datawriter(
            IntPtr _this,
            IntPtr a_datawriter);

        /*     DataWriter
         *     lookup_datawriter(
         *         in string topic_name);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_lookup_datawriter")]
        public static extern IntPtr lookup_datawriter(
            IntPtr _this,
            string topic_name);

        /*     ReturnCode_t
         *     delete_contained_entities();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_delete_contained_entities")]
        public static extern ReturnCode delete_contained_entities(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_qos(
         *         in PublisherQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout PublisherQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_listener(
         *         in PublisherListener a_listener,
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_set_listener")]
        public static extern ReturnCode set_listener(
            IntPtr _this,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     PublisherListener
         *     get_listener();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_get_listener")]
        public static extern IntPtr get_listener(
            IntPtr _this);

        /*     ReturnCode_t
         *     suspend_publications();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_suspend_publications")]
        public static extern ReturnCode suspend_publications(
            IntPtr _this);

        /*     ReturnCode_t
         *     resume_publications();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_resume_publications")]
        public static extern ReturnCode resume_publications(
            IntPtr _this);

        /*     ReturnCode_t
         *     begin_coherent_changes();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_begin_coherent_changes")]
        public static extern ReturnCode begin_coherent_changes(
            IntPtr _this);

        /*     ReturnCode_t
         *     end_coherent_changes();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_end_coherent_changes")]
        public static extern ReturnCode end_coherent_changes(
            IntPtr _this);

        /* ReturnCode_t
         *   wait_for_acknowledgments(
         *      in Duration_t max_wait);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_wait_for_acknowledgments")]
        public static extern ReturnCode wait_for_acknowledgments(
            IntPtr _this,
            ref Duration max_wait
            );

        /*     DomainParticipant
         *     get_participant();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_get_participant")]
        public static extern IntPtr get_participant(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_default_datawriter_qos(
         *         in DataWriterQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_set_default_datawriter_qos")]
        public static extern ReturnCode set_default_datawriter_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_datawriter_qos(
         *         inout DataWriterQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_get_default_datawriter_qos")]
        public static extern ReturnCode get_default_datawriter_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     copy_from_topic_qos(
         *         inout DataWriterQos a_datawriter_qos,
         *         in TopicQos a_topic_qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publisher_copy_from_topic_qos")]
        public static extern ReturnCode copy_from_topic_qos(
            IntPtr _this,
            IntPtr a_datawriter_qos,
            IntPtr a_topic_qos);
    }
}
