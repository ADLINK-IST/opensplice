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
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate byte queryActionFn(IntPtr o, IntPtr arg);

    static internal class Query
    {
        /*
         *     u_query
         *     u_queryNew(
         *         const u_reader reader,
         *         const os_char *name,
         *         const os_char *predicate,
         *         const os_char *params[],
         *         const os_uint32 nrOfParams,
         *         const u_sampleMask sampleMask);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryNew", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr New(
            IntPtr reader,
            string name,
            string predicate,
            IntPtr _params,
            uint nrOfParams,
            uint sampleMask);

        /*
         *     u_result
         *     u_queryRead(
         *         const u_query _this,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryRead", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT Read(
            IntPtr _this,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_queryTake(
         *         const u_query _this,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryTake", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT Take(
            IntPtr _this,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_bool
         *     u_queryTest(
         *         const u_query _this,
         *         u_queryAction action,
         *         void *args);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryTest", CallingConvention = CallingConvention.Cdecl)]
        internal static extern byte Test(
            IntPtr _this,
            queryActionFn action,
            IntPtr args);

        /*
         *     u_result
         *     u_querySet(
         *         const u_query _this,
         *         const os_char *params[],
         *         const os_uint32 nrOfParams);
         */
        [DllImport("ddskernel", EntryPoint = "u_querySet", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT Set(
            IntPtr _this,
            IntPtr _params,
            uint nrOfParams);

        /*
         *     u_result
         *     u_queryReadInstance(
         *         const u_query _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryReadInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT ReadInstance(
            IntPtr _this,
            long handle,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_queryTakeInstance(
         *         const u_query _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryTakeInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT TakeInstance(
            IntPtr _this,
            long handle,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_queryReadNextInstance(
         *         const u_query _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryReadNextInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT ReadNextInstance(
            IntPtr _this,
            long handle,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_queryTakeNextInstance(
         *         const u_query _this,
         *         u_instanceHandle handle,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_queryTakeNextInstance", CallingConvention = CallingConvention.Cdecl)]
        internal static extern V_RESULT TakeNextInstance(
            IntPtr _this,
            long handle,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);
    }
}
