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

namespace DDS.OpenSplice.User
{
    static internal class Topic
    {
        /*
         *     u_result
         *     u_topicInit(
         *         u_topic _this,
         *         const os_char *name);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicInit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Init(
            IntPtr _this,
            string name);

        /*
         *     u_topic
         *     u_topicNew(
         *         const u_participant p,
         *         const os_char *name,
         *         const os_char *typeName,
         *         const os_char *keyList,
         *         u_topicQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr p,
            string name,
            string typeName,
            string keyList,
            IntPtr qos);

        /*
         *     u_result
         *     u_topicGetQos (
         *         const u_topic _this,
         *         u_topicQos *qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicGetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetQos(
            IntPtr _this,
            ref IntPtr qos);

        /*
         *     u_result
         *     u_topicSetQos (
         *         const u_topic _this,
         *         const u_topicQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicSetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetQos(
            IntPtr _this,
            IntPtr qos);

        /*
         *     os_char *
         *     u_topicName(
         *         const u_topic _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicName", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Name(
            IntPtr _this);

        /*
         *     os_char *
         *     u_topicTypeName(
         *         const u_topic _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicTypeName", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr TypeName(
            IntPtr _this);

        /*
         *     os_char *
         *     u_topicKeyExpr(
         *         const u_topic _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicKeyExpr", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr KeyExpr(
            IntPtr _this);

        /*
         *     u_result
         *     u_topicGetInconsistentTopicStatus (
         *         const u_topic _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicGetInconsistentTopicStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetInconsistentTopicStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_topicGetAllDataDisposedStatus (
         *         const u_topic _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicGetAllDataDisposedStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetAllDataDisposedStatus(
            IntPtr _this,
            byte reset,
            Delegate action,
            IntPtr arg);

        /*
         *     u_result
         *     u_topicDisposeAllData (
         *         const u_topic _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicDisposeAllData", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT DisposeAllData(
            IntPtr _this);

        /*
         *     u_bool
         *     u_topicContentFilterValidate (
         *         const u_topic _this,
         *         const q_expr expr,
         *         const c_value params[]);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicContentFilterValidate", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT ContentFilterValidate(
            IntPtr _this,
            IntPtr expr,
            IntPtr _params);

        /*
         *     u_bool
         *     u_topicContentFilterValidate (
         *         const u_topic _this,
         *         const q_expr expr,
         *         const c_value params[]);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicContentFilterValidate2", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte ContentFilterValidate2(
            IntPtr _this,
            IntPtr expr,
            IntPtr _params);

        /*
         *     os_char *
         *     u_topicMetaDescriptor(
         *         const u_topic _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicMetaDescriptor", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr MetaDescriptor(
            IntPtr _this);

    }
}
