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
    /*
     * interface DomainParticipantFactory {
     */
    static internal class DomainParticipantFactory
    {
        /*
         * From Specification
         *
         *     DomainParticipantFactory get_instance (void)
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_get_instance")]
        public static extern IntPtr get_instance();

        /*     DomainParticipant
         *     create_participant(
         *         in DomainId_t domainId,
         *         in DomainParticipantQos qos,
         *         in DomainParticipantListener a_listener,
         *         in gapi_listenerThreadAction thread_start_action,
         *         in gapi_listenerThreadAction thread_stop_action,
         *         in void *thread_action_arg);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_create_participant")]
        public static extern IntPtr create_participant(
            IntPtr _this,
            int domainId,
            IntPtr qos,
            IntPtr a_listener,
            StatusKind mask,
            IntPtr thread_start_action,
            IntPtr thread_stop_action,
            IntPtr thread_action_arg,
            string name);

        /*     ReturnCode_t
         *     delete_participant(
         *         in DomainParticipant a_participant);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_delete_participant")]
        public static extern ReturnCode delete_participant(
            IntPtr _this,
            IntPtr a_participant);

        /*     DomainParticipant
         *     lookup_participant(
         *         in DomainId_t domainId);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_lookup_participant")]
        public static extern IntPtr lookup_participant(
            IntPtr _this,
            int domainId);

        /*     ReturnCode_t
         *     set_qos(
         *         in DomainParticipantFactoryQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout DomainParticipantFactoryQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     set_default_participant_qos(
         *         in DomainParticipantQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_set_default_participant_qos")]
        public static extern ReturnCode set_default_participant_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_default_participant_qos(
         *         inout DomainParticipantQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_domainParticipantFactory_get_default_participant_qos")]
        public static extern ReturnCode get_default_participant_qos(
            IntPtr _this,
            IntPtr qos);

    }
}
