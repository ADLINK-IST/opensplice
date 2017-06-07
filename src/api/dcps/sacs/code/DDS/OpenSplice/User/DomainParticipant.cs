/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 */

using System;
using System.Runtime.InteropServices;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.Database;

namespace DDS.OpenSplice.User
{
    internal static class DomainParticipant
    {
        /*
         *     u_result
         *     u_participantDeinit (
         *         const u_object _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantDeinit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Deinit(
            IntPtr _this);

        /*
         *     u_participant
         *     u_participantNew(
         *         const os_char *uri,
         *         const u_domainId_t id,
         *         os_uint32 timeout,
         *         const os_char *name,
         *         const u_participantQos qos,
         *         u_bool enable);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            string uri,
            int id,
            uint timeout,
            string name,
            IntPtr qos,
            byte enable);

        /*
         *     u_result
         *     u_participantInit (
         *         const u_participant _this,
         *         const u_domain domain);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantInit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Init(
            IntPtr _this,
            IntPtr domain);

        /*
         *     u_result
         *     u_participantGetQos (
         *         const u_participant _this,
         *         u_participantQos *qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantGetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetQos(
            IntPtr _this,
            ref IntPtr qos);

        /*
         *     u_result
         *     u_participantSetQos (
         *         const u_participant _this,
         *         u_participantQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantSetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetQos(
            IntPtr _this,
            IntPtr qos);

        /*
         *     u_result
         *     u_participantDetach(
         *         const u_participant _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantDetach", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Detach(
            IntPtr _this);

        /*
         *     u_domain
         *     u_participantDomain(
         *         const u_participant _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantDomain", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Domain(
            IntPtr _this);

        /*     u_domainId_t
         *     u_participantGetDomainId(
         *         const u_participant _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantGetDomainId", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetDomainId(
            IntPtr _this);

        /*
         *     u_cfElement
         *     u_participantGetConfiguration(
         *         const u_participant _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantGetConfiguration", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetConfiguration(
            IntPtr _this);

        /*
         *     c_iter
         *     u_participantFindTopic(
         *         const u_participant _this,
         *         const os_char *name,
         *         const os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantFindTopic", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr FindTopic(
            IntPtr _this,
            string name,
            long timeout);

        /*
         *     u_result
         *     u_participantAssertLiveliness(
         *         const u_participant _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantAssertLiveliness", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT AssertLiveliness(
            IntPtr _this);

        /*
         *     u_result
         *     u_participantDeleteHistoricalData(
         *         const u_participant _this,
         *         const os_char *partitionExpr,
         *         const os_char *topicExpr);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantDeleteHistoricalData", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT DeleteHistoricalData(
            IntPtr _this,
            string partitionExpr,
            string topicExpr);

        /*
         *     u_result
         *     u_participantFederationSpecificPartitionName (
         *         u_participant _this,
         *         c_char *buf,
         *         os_size_t bufsize);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantFederationSpecificPartitionName", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT FederationSpecificPartitionName(
            IntPtr _this,
            string buf,
            IntPtr bufsize);
    }
}
