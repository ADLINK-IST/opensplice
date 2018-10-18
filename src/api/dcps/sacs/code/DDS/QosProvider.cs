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
using DDS.OpenSplice;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Common;
using DDS;

namespace DDS
{
    /// <summary>
    /// Create a QosProvider fetching QoS configuration from the specified URI.
    /// </summary>
    /// <remarks>
    /// For instance, the following code:
    /// <code>
    /// DDS.QosProvider xml_file_provider("file://somewhere/on/disk/qos-config.xml", null);
    /// DDS.QosProvider json_file_provider("file://somewhere/on/disk/json-config.json", null);
    /// DDS.QosProvider json_http_provider("http:///somewhere.org/here/json-config.json", null);
    /// </code>
    /// The URI determines the how the Qos configuration is fetched and the
    /// format in which it is represented. This specification requires compliant
    /// implementations to support at least one file based configuration using
    /// the XML syntax defined as part of the DDS for CCM specification (formal/12.02.01).
    ///
    /// Then you can extract QoS Policies from the QosProvider
    /// <code>
    /// /* Create QoS Policies. */
    /// DDS.DomainParticipantQos parQos = new DDS.DomainParticipantQos();
    /// DDS.TopicQos             tpcQos = new DDS.TopicQos();
    /// DDS.SubscriberQos        subQos = new DDS.SubscriberQos();
    /// DDS.DataReaderQos        rdrQos = new DDS.DataReaderQos();
    /// DDS.PublisherQos         pubQos = new DDS.PublisherQos();
    /// DDS.DataWriterQos        wrtQos = new DDS.DataWriterQos();
    ///
    /// /* Set the QoS Policies according to the QosProvider XML content. */
    /// DDS.QosProvider qosProvider("file://somewhere/on/disk/qos-config.xml", string.Empty);
    /// /* Expect the functions to return ReturnCode.Ok. */
    /// qosProvider.GetParticipantQos(ref parQos, null);
    /// qosProvider.GetTopicQos      (ref tpcQos, null);
    /// qosProvider.GetSubscriberQos (ref subQos, null);
    /// qosProvider.GetDataReaderQos (ref rdrQos, null);
    /// qosProvider.GetPublisherQos  (ref pubQos, null);
    /// qosProvider.GetDataWriterQos (ref wrtQos, null);
    /// </code>
    ///
    /// @see @ref DCPS_QoS_Provider "QoS Provider extensive information."
    /// </remarks>
    public class QosProvider : IQosProvider
    {
        private IntPtr cmnQpPtr;
        private cmn_qosProviderInputAttr attr = new cmn_qosProviderInputAttr();

        private void loadQosInputAttr(
                OpenSplice.CustomMarshalers.DatabaseMarshaler qosMarshaler,
                Type dataType,
                ref OpenSplice.Common.cmn_qosInputAttr attr)
        {
            DatabaseMarshaler.Add(null, dataType, qosMarshaler);
            qosMarshaler.InitEmbeddedMarshalers(null);

            attr.copyOut = qosMarshaler.CopyOutDelegate;
        }

        private ReturnCode qpResultToReturnCode (QP_RESULT qpResult)
        {
            ReturnCode result;

            switch (qpResult) {
                case QP_RESULT.OK:
                    result = DDS.ReturnCode.Ok;
                    break;
                case QP_RESULT.NO_DATA:
                    result = DDS.ReturnCode.NoData;
                    break;
                case QP_RESULT.OUT_OF_MEMORY:
                    result = DDS.ReturnCode.OutOfResources;
                    break;
                case QP_RESULT.ILL_PARAM:
                    result = DDS.ReturnCode.BadParameter;
                    break;
                default:
                    result = DDS.ReturnCode.Error;
                    break;
            }

            return result;
        }


        /// <summary>
        /// Constructs a new QosProvider based on the provided uri and profile.
        /// </summary>
        /// <remarks>
        /// A QosProvider instance that is instantiated with all profiles and/or QoS’s loaded
        /// from the location specified by the provided uri.
        ///
        /// Initialization of the QosProvider will fail under the following conditions:<br>
        /// - No uri is provided.
        /// - The resource pointed to by uri cannot be found.
        /// - The content of the resource pointed to by uri is malformed (e.g., malformed XML).
        /// When initialization fails (for example, due to a parse error or when the resource
        /// identified by uri cannot be found), then System.NullReferenceException will be thrown.
        ///
        /// Look @ref DCPS_QoS_Provider "here" for more information.
        /// </remarks>
        /// <param name="uri">A Uniform Resource Identifier (URI) that points to the location
        ///                   where the QoS profile needs to be loaded from. Currently only URI’s with a
        ///                   ‘file’ scheme that point to an XML file are supported. If profiles and/or QoS
        ///                   settings are not uniquely identifiable by name within the resource pointed to by
        ///                   uri, a random one of them will be stored.</param>
        /// <param name="profile">The name of the QoS profile within the xml file that serves as the default QoS
        ///                       profile for the get qos operations.</param>
        /// <returns>New initialized QosProvider</returns>
        ///
        public QosProvider(
            string uri,
            string profile)
        {
            attr = new cmn_qosProviderInputAttr ();
            attr.participantQos = new cmn_qosInputAttr();
            attr.topicQos = new cmn_qosInputAttr();
            attr.publisherQos = new cmn_qosInputAttr();
            attr.dataWriterQos = new cmn_qosInputAttr();
            attr.subscriberQos = new cmn_qosInputAttr();
            attr.dataReaderQos = new cmn_qosInputAttr();

            /* let's assign defaults and see what happens */

            loadQosInputAttr(
                    new __NamedDomainParticipantQosMarshaler(),
                    typeof(NamedDomainParticipantQos),
                    ref attr.participantQos);
            loadQosInputAttr(
                    new __NamedTopicQosMarshaler(),
                    typeof(NamedTopicQos),
                    ref attr.topicQos);
            loadQosInputAttr(
                    new __NamedPublisherQosMarshaler(),
                    typeof(NamedPublisherQos),
                    ref attr.publisherQos);
            loadQosInputAttr(
                    new __NamedSubscriberQosMarshaler(),
                    typeof(NamedSubscriberQos),
                    ref attr.subscriberQos);
            loadQosInputAttr(
                    new __NamedDataWriterQosMarshaler(),
                    typeof(NamedDataWriterQos),
                    ref attr.dataWriterQos);
            loadQosInputAttr(
                    new __NamedDataReaderQosMarshaler(),
                    typeof(NamedDataReaderQos),
                    ref attr.dataReaderQos);

            IntPtr attrPtr = os.malloc(new IntPtr(Marshal.SizeOf(attr)));
            Marshal.StructureToPtr(attr, attrPtr, false);
            cmnQpPtr = OpenSplice.Common.QosProvider.New(uri, profile, attrPtr);
            os.free(attrPtr);

            if (cmnQpPtr == IntPtr.Zero)
            {
                // cmn_qosProviderNew already logged that the qosprovider has not been created
                // successfully. Now create a deliberate null pointer exception to let the current
                // constructor fail.
                throw new System.NullReferenceException("cmn_qosProviderNew returned a NULL pointer.");
            }
        }

        /// @cond
        ~QosProvider()
        {
            OpenSplice.Common.QosProvider.Free(cmnQpPtr);
        }
        /// @endcond

        /// <summary>
        /// Resolves the DomainParticipantQos identified by the id from the uri this
        /// QosProvider is associated with.
        /// </summary>
        /// <param name="participantQos">
        ///        Reference to a DomainParticipantQos that will be set with the
        ///        DomainParticipantQos from the given URI (and profile) using the id.</param>
        /// <param name="id">
        ///        The fully-qualified name that identifies a QoS within the uri
        ///        associated with the QosProvider or a name that identifies a QoS within the
        ///        uri associated with the QosProvider instance relative to its default QoS
        ///        profile. Id’s starting with ‘::’ are interpreted as fully-qualified names and all
        ///        others are interpreted as names relative to the default QoS profile of the
        ///        QosProvider instance.<br>When id is null, it is interpreted as a non-named QoS
        ///        within the default QoS profile associated with the QosProvider.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The DomainParticipantQos was found and set in participantQos.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode PreconditionNotMetError - No DomainParticipantQos that matches the provided id can be
        ///                                 found within the uri associated with the QosProvider.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public ReturnCode
        GetParticipantQos (
            ref DomainParticipantQos participantQos,
            string id)
        {
            ReportStack.Start ();
            NamedDomainParticipantQos pQos = new NamedDomainParticipantQos ();
            GCHandle qosHandle = GCHandle.Alloc (pQos, GCHandleType.Normal);
            ReturnCode result = qpResultToReturnCode (
                    OpenSplice.Common.QosProvider.GetParticipantQos (cmnQpPtr, id, GCHandle.ToIntPtr (qosHandle)));
            if (result == ReturnCode.Ok) {
                participantQos = pQos.DomainparticipantQos;
            } else {
                ReportStack.Report (result, "Could not copy DomainParticipantQos.");
            }
            qosHandle.Free();
            ReportStack.Flush(null, result != ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// Resolves the TopicQos identified by the id from the uri this
        /// QosProvider is associated with.
        /// </summary>
        /// <param name="topicQos">
        ///        Reference to a TopicQos that will be set with the
        ///        TopicQos from the given URI (and profile) using the id.</param>
        /// <param name="id">
        ///        The fully-qualified name that identifies a QoS within the uri
        ///        associated with the QosProvider or a name that identifies a QoS within the
        ///        uri associated with the QosProvider instance relative to its default QoS
        ///        profile. Id’s starting with ‘::’ are interpreted as fully-qualified names and all
        ///        others are interpreted as names relative to the default QoS profile of the
        ///        QosProvider instance.<br>When id is null, it is interpreted as a non-named QoS
        ///        within the default QoS profile associated with the QosProvider.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The TopicQos was found and set in topicQos.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode PreconditionNotMetError - No TopicQos that matches the provided id can be
        ///                                 found within the uri associated with the QosProvider.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public ReturnCode
        GetTopicQos (
            ref TopicQos topicQos,
            string id)
        {
            ReportStack.Start ();
            NamedTopicQos tQos = new NamedTopicQos ();
            GCHandle qosHandle = GCHandle.Alloc (tQos, GCHandleType.Normal);
            ReturnCode result = qpResultToReturnCode (
                    OpenSplice.Common.QosProvider.GetTopicQos (cmnQpPtr, id, GCHandle.ToIntPtr (qosHandle)));
            if (result == ReturnCode.Ok) {
                topicQos = tQos.TopicQos;
            } else {
                ReportStack.Report (result, "Could not copy TopicQos.");
            }
            qosHandle.Free();
            ReportStack.Flush(null, result != ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// Resolves the SubscriberQos identified by the id from the uri this
        /// QosProvider is associated with.
        /// </summary>
        /// <param name="subscriberQos">
        ///        Reference to a SubscriberQos that will be set with the
        ///        SubscriberQos from the given URI (and profile) using the id.</param>
        /// <param name="id">
        ///        The fully-qualified name that identifies a QoS within the uri
        ///        associated with the QosProvider or a name that identifies a QoS within the
        ///        uri associated with the QosProvider instance relative to its default QoS
        ///        profile. Id’s starting with ‘::’ are interpreted as fully-qualified names and all
        ///        others are interpreted as names relative to the default QoS profile of the
        ///        QosProvider instance.<br>When id is null, it is interpreted as a non-named QoS
        ///        within the default QoS profile associated with the QosProvider.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The SubscriberQos was found and set in subscriberQos.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode PreconditionNotMetError - No SubscriberQos that matches the provided id can be
        ///                                 found within the uri associated with the QosProvider.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public ReturnCode
        GetSubscriberQos (
            ref SubscriberQos subscriberQos,
            string id)
        {
            ReportStack.Start ();
            NamedSubscriberQos sQos = new NamedSubscriberQos ();
            GCHandle qosHandle = GCHandle.Alloc (sQos, GCHandleType.Normal);
            ReturnCode result = qpResultToReturnCode (
                    OpenSplice.Common.QosProvider.GetSubscriberQos (cmnQpPtr, id, GCHandle.ToIntPtr (qosHandle)));
            if (result == ReturnCode.Ok) {
                subscriberQos = sQos.SubscriberQos;
            } else {
                ReportStack.Report (result, "Could not copy subscriberQos.");
            }
            qosHandle.Free();
            ReportStack.Flush(null, result != ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// Resolves the DataReaderQos identified by the id from the uri this
        /// QosProvider is associated with.
        /// </summary>
        /// <param name="datareaderQos">
        ///        Reference to a DataReaderQos that will be set with the
        ///        DataReaderQos from the given URI (and profile) using the id.</param>
        /// <param name="id">
        ///        The fully-qualified name that identifies a QoS within the uri
        ///        associated with the QosProvider or a name that identifies a QoS within the
        ///        uri associated with the QosProvider instance relative to its default QoS
        ///        profile. Id’s starting with ‘::’ are interpreted as fully-qualified names and all
        ///        others are interpreted as names relative to the default QoS profile of the
        ///        QosProvider instance.<br>When id is null, it is interpreted as a non-named QoS
        ///        within the default QoS profile associated with the QosProvider.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The DataReaderQos was found and set in datareaderQos.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode PreconditionNotMetError - No DataReaderQos that matches the provided id can be
        ///                                 found within the uri associated with the QosProvider.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public ReturnCode
        GetDataReaderQos (
            ref DataReaderQos datareaderQos,
            string id)
        {
            ReportStack.Start ();
            NamedDataReaderQos drQos = new NamedDataReaderQos ();
            GCHandle qosHandle = GCHandle.Alloc (drQos, GCHandleType.Normal);
            ReturnCode result = qpResultToReturnCode (
                    OpenSplice.Common.QosProvider.GetDataReaderQos (cmnQpPtr, id, GCHandle.ToIntPtr (qosHandle)));
            if (result == ReturnCode.Ok) {
                datareaderQos = drQos.DatareaderQos;
            } else {
                ReportStack.Report (result, "Could not copy datareaderQos.");
            }

            qosHandle.Free();
            ReportStack.Flush(null, result != ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// Resolves the PublisherQos identified by the id from the uri this
        /// QosProvider is associated with.
        /// </summary>
        /// <param name="publisherQos">
        ///        Reference to a PublisherQos that will be set with the
        ///        PublisherQos from the given URI (and profile) using the id.</param>
        /// <param name="id">
        ///        The fully-qualified name that identifies a QoS within the uri
        ///        associated with the QosProvider or a name that identifies a QoS within the
        ///        uri associated with the QosProvider instance relative to its default QoS
        ///        profile. Id’s starting with ‘::’ are interpreted as fully-qualified names and all
        ///        others are interpreted as names relative to the default QoS profile of the
        ///        QosProvider instance.<br>When id is null, it is interpreted as a non-named QoS
        ///        within the default QoS profile associated with the QosProvider.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The PublisherQos was found and set in publisherQos.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode PreconditionNotMetError - No PublisherQos that matches the provided id can be
        ///                                 found within the uri associated with the QosProvider.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public ReturnCode
        GetPublisherQos (
            ref PublisherQos publisherQos,
            string id)
        {
            ReportStack.Start ();
            NamedPublisherQos pQos = new NamedPublisherQos ();
            GCHandle qosHandle = GCHandle.Alloc (pQos, GCHandleType.Normal);
            ReturnCode result = qpResultToReturnCode (
                    OpenSplice.Common.QosProvider.GetPublisherQos (cmnQpPtr, id, GCHandle.ToIntPtr (qosHandle)));
            if (result == ReturnCode.Ok) {
                publisherQos = pQos.PublisherQos;
            } else {
                ReportStack.Report (result, "Could not copy publisherQos.");
            }

            qosHandle.Free();
            ReportStack.Flush(null, result != ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// Resolves the DataWriterQos identified by the id from the uri this
        /// QosProvider is associated with.
        /// </summary>
        /// <param name="datawriterQos">
        ///        Reference to a DataWriterQos that will be set with the
        ///        DataWriterQos from the given URI (and profile) using the id.</param>
        /// <param name="id">
        ///        The fully-qualified name that identifies a QoS within the uri
        ///        associated with the QosProvider or a name that identifies a QoS within the
        ///        uri associated with the QosProvider instance relative to its default QoS
        ///        profile. Id’s starting with ‘::’ are interpreted as fully-qualified names and all
        ///        others are interpreted as names relative to the default QoS profile of the
        ///        QosProvider instance.<br>When id is null, it is interpreted as a non-named QoS
        ///        within the default QoS profile associated with the QosProvider.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The DataWriterQos was found and set in datawriterQos.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode PreconditionNotMetError - No DataWriterQos that matches the provided id can be
        ///                                 found within the uri associated with the QosProvider.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public ReturnCode
        GetDataWriterQos (
            ref DataWriterQos datawriterQos,
            string id)
        {
            ReportStack.Start ();
            NamedDataWriterQos dwQos = new NamedDataWriterQos ();
            GCHandle qosHandle = GCHandle.Alloc (dwQos, GCHandleType.Normal);
            ReturnCode result = qpResultToReturnCode (
                    OpenSplice.Common.QosProvider.GetDataWriterQos (cmnQpPtr, id, GCHandle.ToIntPtr (qosHandle)));
            if (result == ReturnCode.Ok) {
                datawriterQos = dwQos.DatawriterQos;
            } else {
                ReportStack.Report (result, "Could not copy datawriterQos.");
            }

            qosHandle.Free();
            ReportStack.Flush(null, result != ReturnCode.Ok);
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
