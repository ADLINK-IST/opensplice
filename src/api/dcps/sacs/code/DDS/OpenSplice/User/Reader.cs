/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate uint readerActionFn(IntPtr o, IntPtr arg);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate V_RESULT publicationInfoActionFn(IntPtr info, IntPtr arg);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate V_RESULT discoveryActionFn(IntPtr buf, uint length, IntPtr arg);

    static internal class Reader
    {
        /*
         *     u_result
         *     u_readerGetDeadlineMissedStatus(
         *         const u_reader _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetDeadlineMissedStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetDeadlineMissedStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetIncompatibleQosStatus(
         *         const u_reader _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetIncompatibleQosStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetIncompatibleQosStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetSampleRejectedStatus(
         *         const u_reader _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetSampleRejectedStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetSampleRejectedStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetLivelinessChangedStatus(
         *         const u_reader _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetLivelinessChangedStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetLivelinessChangedStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetSampleLostStatus(
         *         const u_reader _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetSampleLostStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetSampleLostStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetSubscriptionMatchStatus(
         *         const u_reader _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetSubscriptionMatchStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetSubscriptionMatchStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetMatchedPublications (
         *         const u_reader _this,
         *         u_publicationInfo_action action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetMatchedPublications", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetMatchedPublications(
            IntPtr _this,
            publicationInfoActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerGetMatchedPublicationData (
         *         const u_reader _this,
         *         u_instanceHandle publication_handle,
         *         u_publicationInfo_action action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerGetMatchedPublicationData", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetMatchedPublicationData(
            IntPtr _this,
            long publication_handle,
            publicationInfoActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_readerRead(
         *         const u_reader r,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerRead", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Read(
            IntPtr r,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_readerTake(
         *         const u_reader r,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerTake", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Take(
            IntPtr r,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_readerReadInstance(
         *         const u_reader r,
         *         u_instanceHandle h,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerReadInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT ReadInstance(
            IntPtr r,
            long h,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_readerTakeInstance(
         *         const u_reader r,
         *         u_instanceHandle h,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerTakeInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT TakeInstance(
            IntPtr r,
            long h,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_readerReadNextInstance(
         *         const u_reader r,
         *         u_instanceHandle h,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerReadNextInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT ReadNextInstance(
            IntPtr r,
            long h,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_readerTakeNextInstance(
         *         const u_reader r,
         *         u_instanceHandle h,
         *         u_sampleMask mask,
         *         u_readerAction action,
         *         void *actionArg,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerTakeNextInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT TakeNextInstance(
            IntPtr r,
            long h,
            uint mask,
            readerActionFn action,
            IntPtr actionArg,
            long timeout);

        /*
         *     u_result
         *     u_readerProtectCopyOutEnter(
         *         u_reader r);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerProtectCopyOutEnter", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT ProtectCopyOutEnter(
            IntPtr r);

        /*
         *     u_result
         *     u_readerProtectCopyOutExit(
         *         u_reader r);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerProtectCopyOutExit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT ProtectCopyOutExit(
            IntPtr r);

    }
}
