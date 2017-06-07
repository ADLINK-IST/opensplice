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
    static internal class Subscriber
    {
        /*
         *     u_subscriber
         *     u_subscriberNew(
         *         const u_participant _scope,
         *         const os_char *name,
         *         const u_subscriberQos qos,
         *         u_bool enable);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr _scope,
            string name,
            IntPtr qos,
            byte enable);

        /*
         *     u_result
         *     u_subscriberGetQos (
         *         const u_subscriber _this,
         *         u_subscriberQos *qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberGetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetQos(
            IntPtr _this,
            ref IntPtr qos);

        /*
         *     u_result
         *     u_subscriberSetQos (
         *         const u_subscriber _this,
         *         const u_subscriberQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberSetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetQos(
            IntPtr _this,
            IntPtr qos);

		/*
         *     u_result
         *     u_subscriberGetDataReaders (
         *          const u_subscriber _this,
         *          u_sampleMask mask,
         *          c_iter *readers);
		 */
		[DllImport("ddskernel", EntryPoint = "u_subscriberGetDataReaders", CallingConvention = CallingConvention.Cdecl)]
		public static extern V_RESULT GetDataReaders(
			IntPtr _this,
			uint mask,
			ref IntPtr readers);

        /*
         *     u_result
         *     u_subscriberBeginAccess (
         *         const u_subscriber _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberBeginAccess", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT BeginAccess(
            IntPtr _this);

        /*
         *     u_result
         *     u_subscriberEndAccess (
         *         const u_subscriber _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberEndAccess", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT EndAccess(
            IntPtr _this);

        /*
         *     u_dataReader
         *     u_subscriberCreateDataReader (
         *         const u_subscriber _this,
         *         const os_char *name,
         *         const os_char *expression,
         *         const c_value params[],
         *         const u_readerQos qos,
         *         u_bool enable);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberCreateDataReader", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateDataReader(
            IntPtr _this,
            string name,
            string expression,
            IntPtr _params,
            IntPtr qos,
            byte enable);
    }
}
