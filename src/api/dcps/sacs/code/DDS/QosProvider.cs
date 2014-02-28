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
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Common;
using DDS;

namespace DDS
{
    /// <summary>
    /// Create a QosProvider fetching QoS configuration from the specified
    /// URI. For instance, the following code:
    ///
    /// <pre><code>
    /// QosProvider xml_file_provider("file://somewhere/on/disk/qos-config.xml");
    /// QosProvider json_file_provider("file://somewhere/on/disk/json-config.json");
    /// QosProvider json_http_provider("http:///somewhere.org/here/json-config.json");
    /// </code></pre>
    ///
    /// The URI determines the how the Qos configuration is fetched and the
    /// format in which it is represented. This specification requires compliant
    /// implementations to support at least one file based configuration using
    /// the XML syntax defined as part of the DDS for CCM specification (formal/12.02.01).
    ///
    /// constructor (
    ///     in string uri,
    ///     in string profile);
    /// </summary>
    public class QosProvider : SacsSuperClass, IQosProvider
    {
        private qp_qosProviderInputAttr attr = new qp_qosProviderInputAttr();
        
        private void loadQosInputAttr(
                OpenSplice.CustomMarshalers.DatabaseMarshaler qosMarshaler,
                Type dataType,
                ref OpenSplice.Common.qp_qosInputAttr attr)
        {
            DatabaseMarshaler.Add(null, dataType, qosMarshaler);
            qosMarshaler.InitEmbeddedMarshalers(null);

            attr.copyOut = qosMarshaler.CopyOutDelegate;
        }
        
        public QosProvider(
            string uri,
            string profile)
        {
            attr = new qp_qosProviderInputAttr ();
            attr.participantQos = new qp_qosInputAttr();
            attr.topicQos = new qp_qosInputAttr();
            attr.publisherQos = new qp_qosInputAttr();
            attr.dataWriterQos = new qp_qosInputAttr();
            attr.subscriberQos = new qp_qosInputAttr();
            attr.dataReaderQos = new qp_qosInputAttr();

            /* let's assign defaults and see what happens */

            loadQosInputAttr(
                    new NamedDomainParticipantQosMarshaler(),
                    typeof(NamedDomainParticipantQos),
                    ref attr.participantQos);
            loadQosInputAttr(
                    new NamedTopicQosMarshaler(),
                    typeof(NamedTopicQos),
                    ref attr.topicQos);
            loadQosInputAttr(
                    new NamedPublisherQosMarshaler(),
                    typeof(NamedPublisherQos),
                    ref attr.publisherQos);
            loadQosInputAttr(
                    new NamedSubscriberQosMarshaler(),
                    typeof(NamedSubscriberQos),
                    ref attr.subscriberQos);
            loadQosInputAttr(
                    new NamedDataWriterQosMarshaler(),
                    typeof(NamedDataWriterQos),
                    ref attr.dataWriterQos);
            loadQosInputAttr(
                    new NamedDataReaderQosMarshaler(),
                    typeof(NamedDataReaderQos),
                    ref attr.dataReaderQos);

            IntPtr attrPtr = Marshal.AllocHGlobal(Marshal.SizeOf(attr));
            Marshal.StructureToPtr(attr, attrPtr, false);
            IntPtr ptr = OpenSplice.Common.QosProvider.New(uri, profile, attrPtr);
            Marshal.FreeHGlobal(attrPtr);
            
            if (ptr != IntPtr.Zero)
            {
                SetPeer(ptr, true);
            }
            else
            {
                // qp_qosProviderNew already logged that the qosprovider has not been created 
                // successfully. Now create a deliberate null pointer exception to let the current 
                // constructor fail.
                throw new System.NullReferenceException("qp_qosProviderNew returned a NULL pointer.");
            }
        }

        ~QosProvider()
        {
            OpenSplice.Common.QosProvider.Free(GapiPeer);
        }

        public ReturnCode
        GetParticipantQos (
            ref DomainParticipantQos participantQos,
            string id)
        {
            NamedDomainParticipantQos pQos = new NamedDomainParticipantQos();
            GCHandle qosHandle = GCHandle.Alloc(pQos, GCHandleType.Normal);
            ReturnCode result = OpenSplice.Common.QosProvider.GetParticipantQos(GapiPeer, id, GCHandle.ToIntPtr(qosHandle));
            participantQos = pQos.DomainparticipantQos;
            qosHandle.Free();
            return result;
        }

        public ReturnCode
        GetTopicQos (
            ref TopicQos topicQos,
            string id)
        {
            NamedTopicQos tQos = new NamedTopicQos();
            GCHandle qosHandle = GCHandle.Alloc(tQos, GCHandleType.Normal);
            ReturnCode result = OpenSplice.Common.QosProvider.GetTopicQos(GapiPeer, id, GCHandle.ToIntPtr(qosHandle));
            topicQos = tQos.TopicQos;
            qosHandle.Free();
            return result;
        }

        public ReturnCode
        GetSubscriberQos (
            ref SubscriberQos subscriberQos,
            string id)
        {
            NamedSubscriberQos sQos = new NamedSubscriberQos();
            GCHandle qosHandle = GCHandle.Alloc(sQos, GCHandleType.Normal);
            ReturnCode result = OpenSplice.Common.QosProvider.GetSubscriberQos(GapiPeer, id, GCHandle.ToIntPtr(qosHandle));
            subscriberQos = sQos.SubscriberQos;
            qosHandle.Free();
            return result;
        }

        public ReturnCode
        GetDataReaderQos (
            ref DataReaderQos datareaderQos,
            string id)
        {
            NamedDataReaderQos drQos = new NamedDataReaderQos();
            GCHandle qosHandle = GCHandle.Alloc(drQos, GCHandleType.Normal);
            ReturnCode result = OpenSplice.Common.QosProvider.GetDataReaderQos(GapiPeer, id, GCHandle.ToIntPtr(qosHandle));
            datareaderQos = drQos.DatareaderQos;
            qosHandle.Free();
            return result;
        }

        public ReturnCode
        GetPublisherQos (
            ref PublisherQos publisherQos,
            string id)
        {
            NamedPublisherQos pQos = new NamedPublisherQos();
            GCHandle qosHandle = GCHandle.Alloc(pQos, GCHandleType.Normal);
            ReturnCode result = OpenSplice.Common.QosProvider.GetPublisherQos(GapiPeer, id, GCHandle.ToIntPtr(qosHandle));
            publisherQos = pQos.PublisherQos;
            qosHandle.Free();
            return result;
        }

        public ReturnCode
        GetDataWriterQos (
            ref DataWriterQos datawriterQos,
            string id)
        {
            NamedDataWriterQos dwQos = new NamedDataWriterQos();
            GCHandle qosHandle = GCHandle.Alloc(dwQos, GCHandleType.Normal);
            ReturnCode result = OpenSplice.Common.QosProvider.GetDataWriterQos(GapiPeer, id, GCHandle.ToIntPtr(qosHandle));
            datawriterQos = dwQos.DatawriterQos;
            qosHandle.Free();
            return result;
        }

        // Future expansion will allow the user to share QoSs over DDS
        //
        // ReturnCode_t
        // subscribe ();
        //
        // ReturnCode_t
        // publish ();
    }
}
