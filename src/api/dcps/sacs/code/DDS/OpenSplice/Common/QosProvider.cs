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
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice.Common
{
//    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//    public delegate bool copyInDelegate(IntPtr basePtr, IntPtr from, IntPtr to);
//
//    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//    public delegate void copyOutDelegate(IntPtr from, IntPtr to);

    /* qp_qosAttr is an opaque type to this layer. It's a descriptor to a language
       binding named QoS copyOut routine. */
    [StructLayout(LayoutKind.Sequential)]
    public class qp_qosInputAttr
    {
        public SampleCopyOutDelegate    copyOut;
    };

    /* qp_qosProviderAttr is used to provide the qp_qosProvider with the means to
       to determine default values for every QoS policy. */
    [StructLayout(LayoutKind.Sequential)]
    public class qp_qosProviderInputAttr
    {
        public qp_qosInputAttr          participantQos;
        public qp_qosInputAttr          topicQos;
        public qp_qosInputAttr          subscriberQos;
        public qp_qosInputAttr          dataReaderQos;
        public qp_qosInputAttr          publisherQos;
        public qp_qosInputAttr          dataWriterQos;
    };
    
    static internal class QosProvider
    {
        // from qp_qosProvider.h

        // OS_API qp_qosProvider
        // qp_qosProviderNew (
        //         const c_char *uri,
        //         const c_char *profile,
        //         const qp_qosProviderInputAttr attr);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderNew")]
        public static extern IntPtr New(string uri, string profile, IntPtr args);
    
        // OS_API void
        // qp_qosProviderFree(
        //         qp_qosProvider _this);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderFree")]
        public static extern void Free(IntPtr _this);

        // OS_API qp_result
        // qp_qosProviderGetParticipantQos(
        //         qp_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderGetParticipantQos")]
        public static extern ReturnCode GetParticipantQos(IntPtr _this, string id, IntPtr qos);

        // OS_API qp_result
        // qp_qosProviderGetTopicQos(
        //         qp_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderGetTopicQos")]
        public static extern ReturnCode GetTopicQos(IntPtr _this, string id, IntPtr qos);

        // OS_API qp_result
        // qp_qosProviderGetPublisherQos(
        //         qp_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderGetPublisherQos")]
        public static extern ReturnCode GetPublisherQos(IntPtr _this, string id, IntPtr qos);

        // OS_API qp_result
        // qp_qosProviderGetDataWriterQos(
        //         qp_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderGetDataWriterQos")]
        public static extern ReturnCode GetDataWriterQos(IntPtr _this, string id, IntPtr qos);

        // OS_API qp_result
        // qp_qosProviderGetSubscriberQos(
        //         qp_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderGetSubscriberQos")]
        public static extern ReturnCode GetSubscriberQos(IntPtr _this, string id, IntPtr qos);

        // OS_API qp_result
        // qp_qosProviderGetDataReaderQos(
        //         qp_qosProvider _this,
        //         const c_char *id,
        //         c_voidp qos);
        [DllImport("ddsqosprovider", EntryPoint = "qp_qosProviderGetDataReaderQos")]
        public static extern ReturnCode GetDataReaderQos(IntPtr _this, string id, IntPtr qos);
    }
}

