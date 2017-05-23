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
    static internal class Publisher
    {
        /*
         *     u_publisher
         *     u_publisherNew(
         *         const u_participant participant,
         *         const os_char *name,
         *         const u_publisherQos qos,
         *         u_bool enable);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr participant,
            string name,
            IntPtr qos,
            byte enable);

        /*
         *     u_result
         *     u_publisherGetQos (
         *         const u_publisher _this,
         *         u_publisherQos *qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherGetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetQos(
            IntPtr _this,
            ref IntPtr qos);

        /*
         *     u_result
         *     u_publisherSetQos (
         *         const u_publisher _this,
         *         const u_publisherQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherSetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetQos(
            IntPtr _this,
            IntPtr qos);

        /*
         *     u_result
         *     u_publisherSuspend(
         *         const u_publisher _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherSuspend", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Suspend(
            IntPtr _this);

        /*
         *     u_result
         *     u_publisherResume(
         *         const u_publisher _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherResume", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Resume(
            IntPtr _this);

        /*
         *     u_result
         *     u_publisherCoherentBegin(
         *         const u_publisher _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherCoherentBegin", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT CoherentBegin(
            IntPtr _this);

        /*
         *     u_result
         *     u_publisherCoherentEnd(
         *         const u_publisher _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherCoherentEnd", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT CoherentEnd(
            IntPtr _this);
    }
}
