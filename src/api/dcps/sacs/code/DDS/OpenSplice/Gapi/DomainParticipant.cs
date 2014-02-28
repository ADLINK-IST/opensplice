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
    internal static class DomainParticipant
    {
        /*     Publisher
         *     create_publisher(
         *         in PublisherQos qos,
         *         in PublisherListener a_listener);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_create_publisher")]
        public static extern IntPtr create_publisher(
            IntPtr _this,
            IntPtr qos,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     ReturnCode_t
         *     delete_publisher(
         *         in Publisher p);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_publisher")]
        public static extern ReturnCode delete_publisher(
            IntPtr _this,
            IntPtr p);

        /*     Subscriber
         *     create_subscriber(
         *         in SubscriberQos qos,
         *         in SubscriberListener a_listener);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_create_subscriber")]
        public static extern IntPtr create_subscriber(
            IntPtr _this,
            IntPtr qos,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     ReturnCode_t
         *     delete_subscriber(
         *         in Subscriber s);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_subscriber")]
        public static extern ReturnCode delete_subscriber(
            IntPtr _this,
            IntPtr s);

        /*     Subscriber
         *     get_builtin_subscriber();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_builtin_subscriber")]
        public static extern IntPtr get_builtin_subscriber(
            IntPtr _this);

        /*     Topic
         *     create_topic(
         *         in string topic_name,
         *         in string type_name,
         *         in TopicQos qos,
         *         in TopicListener a_listener);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_create_topic")]
        public static extern IntPtr create_topic(
            IntPtr _this,
            string topic_name,
            string type_name,
            IntPtr qos,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     ReturnCode_t
         *     delete_topic(
         *         in Topic a_topic);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_topic")]
        public static extern ReturnCode delete_topic(
            IntPtr _this,
            IntPtr a_topic);

        /*     Topic
         *     find_topic(
         *         in string topic_name,
         *         in Duration_t timeout);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_find_topic")]
        public static extern IntPtr find_topic(
            IntPtr _this,
            string topic_name,
            ref Duration timeout);

        /*     TopicDescription
         *     lookup_topicdescription(
         *         in string name);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_lookup_topicdescription")]
        public static extern IntPtr lookup_topicdescription(
            IntPtr _this,
            string name);

        /*     ContentFilteredTopic
         *     create_contentfilteredtopic(
         *         in string name,
         *         in Topic related_topic,
         *         in string filter_expression,
         *         in StringSeq filter_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_create_contentfilteredtopic")]
        public static extern IntPtr create_contentfilteredtopic(
            IntPtr _this,
            string name,
            IntPtr related_topic,
            string filter_expression,
            IntPtr filter_parameters);

        /*     ReturnCode_t
         *     delete_contentfilteredtopic(
         *         in ContentFilteredTopic a_contentfilteredtopic);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_contentfilteredtopic")]
        public static extern ReturnCode delete_contentfilteredtopic(
            IntPtr _this,
            IntPtr a_contentfilteredtopic);

        /*     MultiTopic
         *     create_multitopic(
         *         in string name,
         *         in string type_name,
         *         in string subscription_expression,
         *         in StringSeq expression_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_create_multitopic")]
        public static extern IntPtr create_multitopic(
            IntPtr _this,
            string name,
            string type_name,
            string subscription_expression,
            IntPtr expression_parameters);

        /*     ReturnCode_t
         *     delete_multitopic(
         *         in MultiTopic a_multitopic);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_multitopic")]
        public static extern ReturnCode delete_multitopic(
            IntPtr _this,
            IntPtr a_multitopic);


        //typedef void (*gapi_deleteEntityAction)(void *entity_data, void *arg);


        /*     ReturnCode_t
         *     delete_contained_entities();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_contained_entities")]
        public static extern ReturnCode delete_contained_entities(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_qos(
         *         in DomainParticipantQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout DomainParticipantQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_listener(
         *         in DomainParticipantListener a_listener,
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_set_listener")]
        public static extern ReturnCode set_listener(
            IntPtr _this,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     DomainParticipantListener
         *     get_listener();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_listener")]
        public static extern IntPtr get_listener(
            IntPtr _this);

        /*     ReturnCode_t
         *     ignore_participant(
         *         in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_ignore_participant")]
        public static extern ReturnCode ignore_participant(
            IntPtr _this,
            InstanceHandle handle);

        /*     ReturnCode_t
         *     ignore_topic(
         *         in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_ignore_topic")]
        public static extern ReturnCode ignore_topic(
            IntPtr _this,
            InstanceHandle handle);

        /*     ReturnCode_t
         *     ignore_publication(
         *         in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_ignore_publication")]
        public static extern ReturnCode ignore_publication(
            IntPtr _this,
            InstanceHandle handle);

        /*     ReturnCode_t
         *     ignore_subscription(
         *         in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_ignore_subscription")]
        public static extern ReturnCode ignore_subscription(
            IntPtr _this,
            InstanceHandle handle);

        /*     DomainId_t
         *     get_domain_id();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_domain_id")]
        public static extern int get_domain_id(
            IntPtr _this);

        /*     ReturnCode_t
         *     assert_liveliness();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_assert_liveliness")]
        public static extern ReturnCode assert_liveliness(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_default_publisher_qos(
         *         in PublisherQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_set_default_publisher_qos")]
        public static extern ReturnCode set_default_publisher_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_publisher_qos(
         *         inout PublisherQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_default_publisher_qos")]
        public static extern ReturnCode get_default_publisher_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_default_subscriber_qos(
         *         in SubscriberQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_set_default_subscriber_qos")]
        public static extern ReturnCode set_default_subscriber_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_subscriber_qos(
         *         inout SubscriberQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_default_subscriber_qos")]
        public static extern ReturnCode get_default_subscriber_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_default_topic_qos(
         *         in TopicQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_set_default_topic_qos")]
        public static extern ReturnCode set_default_topic_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_topic_qos(
         *         inout TopicQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_default_topic_qos")]
        public static extern ReturnCode get_default_topic_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_discovered_participants (
         *         inout InstanceHandleSeq participant_handles);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_discovered_participants")]
        public static extern ReturnCode get_discovered_participants(
            IntPtr _this,
            Delegate action,
            IntPtr arg);

        /*     ReturnCode_t
         *     get_discovered_participant_data (
         *         in InstanceHandle_t handle,
         *         inout ParticipantBuiltinTopicData *participant_data,
         *         in gapi_readerAction action);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_discovered_participant_data")]
        public static extern ReturnCode get_discovered_participant_data(
            IntPtr _this,
            IntPtr participant_data,
            long handle,
            Delegate action);

        /*     ReturnCode_t
         *     get_discovered_topics (
         *         inout InstanceHandleSeq topic_handles);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_discovered_topics")]
        public static extern ReturnCode get_discovered_topics(
           IntPtr _this,
           Delegate action,
           IntPtr arg);

        /*     ReturnCode_t
         *     get_discovered_topic_data (
         *         in InstanceHandle_t handle,
         *         inout TopicBuiltinTopicData *topic_data);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_discovered_topic_data")]
        public static extern ReturnCode get_discovered_topic_data(
            IntPtr _this,
            IntPtr topic_data,
            long handle,
            Delegate action);

        /*     Boolean
         *     contains_entity (
         *         in InstanceHandle_t a_hande);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_contains_entity")]
        public static extern byte contains_entity(
            IntPtr _this,
            InstanceHandle a_handle);

        /*     ReturnCode_t
         *     get_current_time (
         *         inout Time_t current_time);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_current_time")]
        public static extern ReturnCode get_current_time(
            IntPtr _this,
            out Time current_time);

        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_delete_historical_data")]
        public static extern ReturnCode delete_historical_data(
            IntPtr _this,
            string partition_expression,
            string topic_expression);

        /*     gapi_metaDescription
         *     get_type_metadescription (
         *         in string type_name);
         */
        //typedef void *gapi_metaDescription;

        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_type_metadescription")]
        public static extern IntPtr get_type_metadescription(
            IntPtr _this,
            string type_name);

        /*     gapi_typeSupport
         *     get_typesupport (
         *         in string registered_name);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_get_typesupport")]
        public static extern IntPtr get_typesupport(
            IntPtr _this,
            string type_name);

        /*     gapi_typeSupport
         *     find_typesupport (
         *         in string registered_type_name);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipant_lookup_typesupport")]
        public static extern IntPtr lookup_typesupport(
            IntPtr _this,
            string type_name);
    }
}
