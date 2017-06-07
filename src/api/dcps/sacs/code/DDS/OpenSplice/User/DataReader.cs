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
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice.User
{
    internal class DataReader
    {
        /*
         *     u_dataReader
         *     u_dataReaderNew(
         *         const u_subscriber s,
         *         const os_char *name,
         *         const q_expr OQLexpr,
         *         const c_value params[],
         *         const u_readerQos qos,
         *         u_bool enable);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderNew", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr New(
            IntPtr s,
            string name,
            IntPtr OQLexpr,
            IntPtr _params,
            IntPtr qos,
            byte enable);

        /*
         *     u_dataReader
         *     u_dataReaderNew(
         *         const u_subscriber s,
         *         const os_char *name,
         *         const os_char *expr,
         *         const c_value params[],
         *         const u_readerQos qos,
         *         u_bool enable);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderNew", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr NewBySQL(
            IntPtr s,
            string name,
            string expr,
            IntPtr _params,
            IntPtr qos,
            byte enable);

        /*
         *     u_result
         *     u_dataReaderGetInstanceHandles (
         *         const u_dataReader _this,
         *         u_result (*action)(u_instanceHandle *buf, os_uint32 length, void *arg),
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderGetInstanceHandles", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT GetInstanceHandles(
            IntPtr _this,
            discoveryActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_dataReaderRead(
         *         const u_dataReader _this,
         *         u_sampleMask mask,
         *         u_dataReaderAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderRead", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT Read(
            IntPtr _this,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderTake(
         *         const u_dataReader _this,
         *         u_sampleMask mask,
         *         u_dataReaderAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderTake", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT Take(
            IntPtr _this,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderReadInstance(
         *         const u_dataReader _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_dataReaderAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderReadInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT ReadInstance(
            IntPtr _this,
            long handle,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderTakeInstance(
         *         const u_dataReader _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_dataReaderAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderTakeInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT TakeInstance(
            IntPtr _this,
            long handle,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderReadNextInstance(
         *         const u_dataReader _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_dataReaderAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderReadNextInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT ReadNextInstance(
            IntPtr _this,
            long handle,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderTakeNextInstance(
         *         const u_dataReader _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_dataReaderAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderTakeNextInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT TakeNextInstance(
            IntPtr _this,
            long handle,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderWaitForHistoricalData(
         *         const u_dataReader _this,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderWaitForHistoricalData", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT WaitForHistoricalData(
            IntPtr _this,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderWaitForHistoricalDataWithCondition(
         *         const u_dataReader _this,
         *         const os_char* filter,
         *         const os_char* params[],
         *         os_uint32 paramsLength,
         *         os_timeW minSourceTime,
         *         os_timeW maxSourceTime,
         *         os_int32 max_samples,
         *         os_int32 max_instances,
         *         os_int32 max_samples_per_instance,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderWaitForHistoricalDataWithCondition", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT WaitForHistoricalDataWithCondition(
            IntPtr _this,
            string filter,
            IntPtr _params,
            uint paramsLength,
            os_timeW minSourceTime,
            os_timeW maxSourceTime,
            int max_samples,
            int max_instances,
            int max_samples_per_instance,
            long timeout);

        /*
         *     u_result
         *     u_dataReaderLookupInstance(
         *         const u_dataReader _this,
         *         void *keyTemplate,
         *         u_copyIn copyIn,
         *         u_instanceHandle *handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderLookupInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT LookupInstance(
            IntPtr _this,
            IntPtr keyTemplate,
            SampleCopyInDelegate copyIn,
            ref long handle);

        /*
         *     u_result
         *     u_dataReaderCopyKeysFromInstanceHandle(
         *         const u_dataReader _this,
         *         u_instanceHandle handle,
         *         u_copyOut action,
         *         void *copyArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderCopyKeysFromInstanceHandle", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT CopyKeysFromInstanceHandle(
            IntPtr _this,
            long handle,
            SampleCopyOutDelegate action,
            IntPtr copyArg);

        /*
         *     u_result
         *     u_dataReaderGetQos (
         *         const u_dataReader _this,
         *         u_readerQos *qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderGetQos", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT GetQos(
            IntPtr _this,
            ref IntPtr qos);

        /*
         *     u_result
         *     u_dataReaderSetQos (
         *         const u_dataReader _this,
         *         const u_readerQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_dataReaderSetQos", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT SetQos(
            IntPtr _this,
            IntPtr qos);
    }
}
