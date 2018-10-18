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
    internal delegate V_RESULT subscriptionInfoActionFn(IntPtr info, IntPtr arg);

    static internal class Writer
    {
        /*
         *     u_writer
         *     u_writerNew(
         *         const u_publisher publisher,
         *         const os_char *name,
         *         const u_topic topic,
         *         const u_writerQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr publisher,
            string name,
            IntPtr topic,
            IntPtr qos);

        /*
         *     u_result
         *     u_writerWaitForAcknowledgments(
         *         const u_writer _this,
         *         os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerWaitForAcknowledgments", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT WaitForAcknowledgments(
            IntPtr _this,
            long timeout);

        /*
         *     u_result
         *     u_writerGetQos (
         *         const u_writer _this,
         *         u_writerQos *qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetQos(
            IntPtr _this,
            ref IntPtr qos);

        /*
         *     u_result
         *     u_writerSetQos (
         *         const u_writer _this,
         *         const u_writerQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerSetQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetQos(
            IntPtr _this,
            IntPtr qos);

        /*
         *     u_result
         *     u_writerWrite(
         *         const u_writer _this,
         *         u_writerCopy copy,
         *         void *data,
         *         os_timeW timestamp,
         *         u_instanceHandle handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerWrite", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Write(
            IntPtr _this,
            SampleCopyInDelegate copy,
            IntPtr data,
            os_timeW timestamp,
            long handle);

        /*
         *     u_result
         *     u_writerDispose(
         *         const u_writer _this,
         *         u_writerCopy copy,
         *         void *data,
         *         os_timeW timestamp,
         *         u_instanceHandle handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerDispose", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Dispose(
            IntPtr _this,
            SampleCopyInDelegate copy,
            IntPtr data,
            os_timeW timestamp,
            long handle);

        /*
         *   u_result
         *   u_writerWriteDispose(
         *       const u_writer _this,
         *       u_writerCopy copy,
         *       void *data,
         *       os_timeW timestamp,
         *       u_instanceHandle handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerWriteDispose", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT WriteDispose(
            IntPtr _this,
            SampleCopyInDelegate copy,
            IntPtr data,
            os_timeW timestamp,
            long handle);

        /*
         *     u_result
         *     u_writerAssertLiveliness(
         *         const u_writer _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerAssertLiveliness", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT AssertLiveliness(
            IntPtr _this);

        /*
         *     u_result
         *     u_writerRegisterInstance(
         *         const u_writer _this,
         *         u_writerCopy copy,
         *         void *data,
         *         os_timeW timestamp,
         *         u_instanceHandle *handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerRegisterInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT RegisterInstance(
            IntPtr _this,
            SampleCopyInDelegate copy,
            IntPtr data,
            os_timeW timestamp,
            ref long handle);

        /*
         *     u_result
         *     u_writerUnregisterInstance(
         *         const u_writer _this,
         *         u_writerCopy copy,
         *         void *data,
         *         os_timeW timestamp,
         *         u_instanceHandle handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerUnregisterInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT UnregisterInstance(
            IntPtr _this,
            SampleCopyInDelegate copy,
            IntPtr data,
            os_timeW timestamp,
            long handle);

        /*
         *     u_result
         *     u_writerLookupInstance(
         *         const u_writer _this,
         *         u_writerCopy copy,
         *         void* keyTemplate,
         *         u_instanceHandle *handle);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerLookupInstance", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT LookupInstance(
            IntPtr _this,
            SampleCopyInDelegate copy,
            IntPtr keyTemplate,
            ref long handle);

        /*
         *     u_result
         *     u_writerCopyKeysFromInstanceHandle (
         *         const u_writer _this,
         *         u_instanceHandle handle,
         *         u_writerCopyKeyAction action,
         *         void *copyArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerCopyKeysFromInstanceHandle", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT CopyKeysFromInstanceHandle(
            IntPtr _this,
            long handle,
            SampleCopyOutDelegate action,
            IntPtr copyArg);

        /*
         *     u_result
         *     u_writerGetLivelinessLostStatus (
         *         const u_writer _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetLivelinessLostStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetLivelinessLostStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_writerGetDeadlineMissedStatus (
         *         const u_writer _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetDeadlineMissedStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetDeadlineMissedStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_writerGetIncompatibleQosStatus (
         *         const u_writer _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetIncompatibleQosStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetIncompatibleQosStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_writerGetPublicationMatchStatus (
         *         const u_writer _this,
         *         u_bool reset,
         *         u_statusAction action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetPublicationMatchStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetPublicationMatchStatus(
            IntPtr _this,
            byte reset,
            statusActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_writerGetMatchedSubscriptions (
         *         const u_writer _this,
         *         u_subscriptionInfo_action action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetMatchedSubscriptions", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetMatchedSubscriptions(
            IntPtr _this,
            subscriptionInfoActionFn action,
            IntPtr arg);

        /*
         *     u_result
         *     u_writerGetMatchedSubscriptionData (
         *         const u_writer _this,
         *         u_instanceHandle subscription_handle,
         *         u_subscriptionInfo_action action,
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerGetMatchedSubscriptionData", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetMatchedSubscriptionData(
            IntPtr _this,
            long subscription_handle,
            subscriptionInfoActionFn action,
            IntPtr arg);
    }
}
