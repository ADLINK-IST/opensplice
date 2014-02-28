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
    static internal class Topic
    {
        /*     // Access the status
         *     ReturnCode_t
         *     get_inconsistent_topic_status( inout InconsistentTopicStatus);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topic_get_inconsistent_topic_status")]
        public static extern ReturnCode get_inconsistent_topic_status(
            IntPtr _this,
            InconsistentTopicStatus status
            );

        /*     ReturnCode_t
         *     set_listener(
         *         in TopicListener a_listener,
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topic_set_listener")]
        public static extern ReturnCode set_listener(
            IntPtr _this,
            IntPtr a_listener,
            StatusKind mask
            );

        /*     TopicListener
         *     get_listener();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topic_get_listener")]
        public static extern IntPtr get_listener(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_qos(
         *         in TopicQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topic_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        // TODO: Why is this inout? should be out only?
        /*     ReturnCode_t
         *     get_qos(
         *         inout TopicQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topic_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        // TODO: so #define function names don't actuall exist in the dll
        // this means we need to map them somehow, for now we will just
        // call the static method where the "real" function exists.
        //
        // EntryPointNotFoundException: Unable to find an entry point named 
        // '<name of function>' in DLL 'ddskernel'.
        //

        // gapi_topicDescription_get_type_name
        /*     string
        *     get_type_name();
        */
        //[DllImport("ddskernel", EntryPoint = "gapi_topic_get_type_name")]
        //public static extern IntPtr get_type_name(
        //    IntPtr _this);

        // gapi_topicDescription_get_name
        /*     string
         *     get_name();
         */
        //[DllImport("ddskernel", EntryPoint = "gapi_topic_get_name")]
        //public static extern IntPtr get_name(
        //    IntPtr _this);

        // gapi_topicDescription_get_participant
        /*     DomainParticipant
         *     get_participant();
         */

        //[DllImport("ddskernel", EntryPoint = "gapi_topic_get_participant")]
        //public static extern IntPtr get_participant(
        //    IntPtr _this);
    }
}
