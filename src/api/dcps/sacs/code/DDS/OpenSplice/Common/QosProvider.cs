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
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice.Common
{
//    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//    public delegate bool copyInDelegate(IntPtr basePtr, IntPtr from, IntPtr to);
//
//    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//    public delegate void copyOutDelegate(IntPtr from, IntPtr to);

    /* cmn_qosAttr is an opaque type to this layer. It's a descriptor to a language
       binding named QoS copyOut routine. */
    public enum QP_RESULT
    {
        OK,
        NO_DATA,
        OUT_OF_MEMORY,
        PARSE_ERROR,
        ILL_PARAM,
        UNKNOWN_ELEMENT,
        UNEXPECTED_ELEMENT,
        UNKNOWN_ARGUMENT,
        ILLEGAL_VALUE,
        NOT_IMPLEMENTED
    }


    [StructLayout(LayoutKind.Sequential)]
    public class cmn_qosInputAttr
    {
        public SampleCopyOutDelegate    copyOut;
    };

    /* cmn_qosProviderAttr is used to provide the cmn_qosProvider with the means to
       to determine default values for every QoS policy. */
    [StructLayout(LayoutKind.Sequential)]
    public class cmn_qosProviderInputAttr
    {
        public cmn_qosInputAttr          participantQos;
        public cmn_qosInputAttr          topicQos;
        public cmn_qosInputAttr          subscriberQos;
        public cmn_qosInputAttr          dataReaderQos;
        public cmn_qosInputAttr          publisherQos;
        public cmn_qosInputAttr          dataWriterQos;
    };

    static internal class QosProvider
    {
        // from cmn_qosProvider.h

        // OS_API cmn_qosProvider
        // cmn_qosProviderNew (
        //         const c_char *uri,
        //         const c_char *profile,
        //         const cmn_qosProviderInputAttr attr);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(string uri, string profile, IntPtr args);

        // OS_API void
        // cmn_qosProviderFree(
        //         cmn_qosProvider _this);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(IntPtr _this);

        // OS_API cmn_qpResult
        // cmn_qosProviderGetParticipantQos(
        //         cmn_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderGetParticipantQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern QP_RESULT GetParticipantQos(IntPtr _this, string id, IntPtr qos);

        // OS_API cmn_qpResult
        // cmn_qosProviderGetTopicQos(
        //         cmn_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderGetTopicQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern QP_RESULT GetTopicQos(IntPtr _this, string id, IntPtr qos);

        // OS_API cmn_qpResult
        // cmn_qosProviderGetPublisherQos(
        //         cmn_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderGetPublisherQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern QP_RESULT GetPublisherQos(IntPtr _this, string id, IntPtr qos);

        // OS_API cmn_qpResult
        // cmn_qosProviderGetDataWriterQos(
        //         cmn_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderGetDataWriterQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern QP_RESULT GetDataWriterQos(IntPtr _this, string id, IntPtr qos);

        // OS_API cmn_qpResult
        // cmn_qosProviderGetSubscriberQos(
        //         cmn_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderGetSubscriberQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern QP_RESULT GetSubscriberQos(IntPtr _this, string id, IntPtr qos);

        // OS_API cmn_qpResult
        // cmn_qosProviderGetDataReaderQos(
        //         cmn_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddskernel", EntryPoint = "cmn_qosProviderGetDataReaderQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern QP_RESULT GetDataReaderQos(IntPtr _this, string id, IntPtr qos);
    }
}

