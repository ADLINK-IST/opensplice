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
using System.Collections.Generic;
using System.Diagnostics;
using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.Database;
using DDS.OpenSplice.User;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModuleI;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate bool CopyInstanceHandleDelegate(IntPtr instance, IntPtr arg);

    public class DomainParticipant : Entity, IDomainParticipant
    {
        private struct BuiltinTopicProperties
        {
            public DDS.OpenSplice.TypeSupport typeSupport;
            public string topicName;

            public BuiltinTopicProperties(DDS.OpenSplice.TypeSupport ts, string tn)
            {
                typeSupport = ts;
                topicName = tn;
            }
        };

        private static BuiltinTopicProperties[] osplBuiltinTopics = new BuiltinTopicProperties[]
        {
            new BuiltinTopicProperties(new ParticipantBuiltinTopicDataTypeSupport(),    "DCPSParticipant"    ),
            new BuiltinTopicProperties(new TopicBuiltinTopicDataTypeSupport(),          "DCPSTopic"          ),
            new BuiltinTopicProperties(new PublicationBuiltinTopicDataTypeSupport(),    "DCPSPublication"    ),
            new BuiltinTopicProperties(new SubscriptionBuiltinTopicDataTypeSupport(),   "DCPSSubscription"   ),
            new BuiltinTopicProperties(new CMParticipantBuiltinTopicDataTypeSupport(),  "CMParticipant"      ),
            new BuiltinTopicProperties(new CMPublisherBuiltinTopicDataTypeSupport(),    "CMPublisher"        ),
            new BuiltinTopicProperties(new CMSubscriberBuiltinTopicDataTypeSupport(),   "CMSubscriber"       ),
            new BuiltinTopicProperties(new CMDataWriterBuiltinTopicDataTypeSupport(),   "CMDataWriter"       ),
            new BuiltinTopicProperties(new CMDataReaderBuiltinTopicDataTypeSupport(),   "CMDataReader"       ),
            new BuiltinTopicProperties(new TypeBuiltinTopicDataTypeSupport(),           "DCPSType"           )
        };

        private const int DCPS_PARTICIPANT_INDEX = 0;
        private const int DCPS_TOPIC_INDEX = 1;

        private __ParticipantBuiltinTopicDataMarshaler ParticipantDataMarshaler;
        private __TopicBuiltinTopicDataMarshaler TopicDataMarshaler;
        private PublisherQos defaultPublisherQos = new PublisherQos();
        private SubscriberQos defaultSubscriberQos = new SubscriberQos();
        private TopicQos defaultTopicQos = new TopicQos();
        //private SchedulingQosPolicy myListenerScheduling = new SchedulingQosPolicy();
        private Subscriber builtinSubscriber = null;
        private Dictionary<string, TypeSupport> typeSupportList = new Dictionary<string, TypeSupport>();
        private List<Publisher> publisherList = new List<Publisher>();
        private List<Subscriber> subscriberList = new List<Subscriber>();
        private List<Topic> topicList = new List<Topic>();
        private List<Topic> builtinTopicList = new List<Topic>();
        private List<ContentFilteredTopic> cfTopicList = new List<ContentFilteredTopic>();
        private List<MultiTopic> mTopicList = new List<MultiTopic>();

        /**
         * Constructor is only called by DDS.DomainParticipantFactory.
         */
        internal DomainParticipant()
        {
            DDS.ReturnCode result;

            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                new OpenSplice.CustomMarshalers.TopicQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultTopicQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref defaultTopicQos);
                }
            }
            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultPublisherQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref defaultPublisherQos);
                }
            }
            using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultSubscriberQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref defaultSubscriberQos);
                }
            }
        }

        /**
         * init() takes care of all initilization.
         */
        internal ReturnCode init(DomainId domainId, DomainParticipantQos qos)
        {
            ReturnCode result;
            IntPtr uParticipant;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    Process currentProcess = Process.GetCurrentProcess();
					string name = currentProcess.MainModule.ModuleName + "<" + currentProcess.Id + ">";
                    uParticipant = User.DomainParticipant.New(null, domainId, 30, name, marshaler.UserPtr, 1);
                    if (uParticipant != IntPtr.Zero)
                    {
                        result = base.init(uParticipant);
                        MyDomainId = User.DomainParticipant.GetDomainId(uParticipant);
                    }
                    else
                    {
                        result = DDS.ReturnCode.Error;
                    }
                }
            }

            if (result == ReturnCode.Ok)
            {
                wlReq_ListenerDispatcher = new ListenerDispatcher();
                result = wlReq_ListenerDispatcher.init(rlReq_UserPeer, qos.ListenerScheduling);
            }

            if (result == ReturnCode.Ok)
            {
                result = CreateBuiltinTopics();
            }

            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            if (topicList.Count == 0 &&
                cfTopicList.Count == 0 &&
                mTopicList.Count == 0 &&
                publisherList.Count == 0 &&
                subscriberList.Count == 0)
            {
                IDomainParticipantListener dpListener = listener as IDomainParticipantListener;
                if (dpListener != null)
                {
                    this.SetListener(dpListener, (DDS.StatusKind)0);
                }
                this.DisableCallbacks();
                if (result == DDS.ReturnCode.Ok && builtinSubscriber != null)
                {
                    result = builtinSubscriber.DeleteContainedEntities();
                    if (result == DDS.ReturnCode.Ok)
                    {
                        result = builtinSubscriber.deinit();
                    }
                }
                if (result == DDS.ReturnCode.Ok)
                {
                    foreach(Topic biTopic in builtinTopicList)
                    {
                        result = biTopic.deinit();
                        if (result != DDS.ReturnCode.Ok) break;
                    }
                }
                if (wlReq_ListenerDispatcher != null)
                {
                    result = wlReq_ListenerDispatcher.deinit();
                }

                if (result == DDS.ReturnCode.Ok)
                {
                    result = base.wlReq_deinit();
                }
            }
            else
            {
                result = DDS.ReturnCode.PreconditionNotMet;
                ReportStack.Report(result, "DomainParticipant " + this + " cannot be deleted since it still contains " +
                            topicList.Count + " Topics," +
                            cfTopicList.Count + " ContentFilteredTopics," +
                            mTopicList.Count + " MultiTopics," +
                            publisherList.Count + " Publishers and " +
                            subscriberList.Count + " Subscribers.");
            }
            return result;
        }

        internal ReturnCode nlReq_LoadTypeSupport(TypeSupport ts, string typeName)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            if (typeName == null) {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "Typename is null.");
            } else {
                lock(this)
                {
                    if (rlReq_isAlive)
                    {
                        TypeSupport inMap;
                        string descriptor = ts.TypeDescriptor;
                        IntPtr uDomain = User.DomainParticipant.Domain(rlReq_UserPeer);
                        typeSupportList.TryGetValue(typeName, out inMap);
                        if (inMap == null)
                        {
                            V_RESULT uResult = User.Domain.LoadXmlDescriptor(uDomain, descriptor);
                            result = uResultToReturnCode (uResult);
                            if (result == DDS.ReturnCode.Ok) {
                                typeSupportList.Add(typeName, ts);
                                foreach (ITopicDescriptionImpl t in topicList)
                                {
                                    if (t.TypeName.Equals(ts.TypeName) || t.TypeName.Equals(ts.InternalTypeName) && t.rlReq_TypeSupport == null)
                                    {
                                        t.rlReq_TypeSupport = ts;
                                    }
                                }
                            }
                        }
                        else if (inMap == ts)
                        {
                            result = DDS.ReturnCode.Ok;
                        }
                        else
                        {
                            if (inMap.KeyList.Equals(ts.KeyList) && inMap.TypeDescriptor.Equals(ts.TypeDescriptor))
                            {
								V_RESULT uResult = User.Domain.LoadXmlDescriptor (uDomain, descriptor);
								result = uResultToReturnCode (uResult);
								if (result != DDS.ReturnCode.Ok)
                                {
                                    ReportStack.Report(result, "Given type \"" + typeName + "\" is incompatible with an already registered type using the same name.");
                                }
                            }
                            else
                            {
                                result = DDS.ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "Given type \"" + typeName + "\" is not registered.");
                            }
                        }
                    }
                }
            }
            return result;
        }

        private bool rlReq_AutoEnableCreatedEntities
        {
            get {
                DomainParticipantQos pQos = new DomainParticipantQos();
                IntPtr userQos = IntPtr.Zero;
                bool autoEnable = false;

                ReturnCode result = uResultToReturnCode(
                        User.DomainParticipant.GetQos(rlReq_UserPeer, ref userQos));

                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler(userQos, false))
                    {
                        marshaler.CopyOut(ref pQos);
                        autoEnable = pQos.EntityFactory.AutoenableCreatedEntities;
                    }
                }
                User.ParticipantQos.Free(userQos);

                return autoEnable;
            }
        }

        private ReturnCode CreateBuiltinTopic(BuiltinTopicProperties osplBuiltinTopic, TopicQos tQos)
        {
            DDS.OpenSplice.TypeSupport ts = osplBuiltinTopic.typeSupport;
            String topicName = osplBuiltinTopic.topicName;
            ReturnCode result = ts.RegisterType(this, ts.TypeName);
            if (result != ReturnCode.Ok)
            {
                ReportStack.Report(result, "Failed to register builtin topic type: " + ts.TypeName);
            } else {
                Topic biTopic = nlReq_CreateTopic(topicName, ts.TypeName, tQos, null, StatusKind.Any, builtinTopicList);
                if (biTopic == null)
                {
                    result = DDS.ReturnCode.Error;
                    ReportStack.Report(result, "Failed to create builtin topic: " + topicName);
                }
            }
            return result;
        }


        /**
         * Register and create the nine builtin topics:
         *     "DDS::ParticipantBuiltinTopicData"
         *     "DDS::TopicBuiltinTopicData"
         *     "DDS::PublicationBuiltinTopicData"
         *     "DDS::SubscriptionBuiltinTopicData"
         *     "DDS::CMParticipant"
         *     "DDS::CMPublisher"
         *     "DDS::CMSubscriber"
         *     "DDS::CMDataWriter"
         *     "DDS::CMDataReader"
         */

        internal ReturnCode CreateBuiltinTopics()
        {
            ReturnCode result;
            TopicQos tQos = new TopicQos();

            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultTopicQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref tQos);
                    tQos.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
                    tQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;

                    /* The following 3 overrides are because the kernel is using
                       the same QoS's as the 6.4 for backwards compatibility */
                    tQos.History.Kind = HistoryQosPolicyKind.KeepAllHistoryQos;
                    tQos.History.Depth = DDS.Length.Unlimited;
                }
            }

            for (int i = 0; i < osplBuiltinTopics.Length && result == DDS.ReturnCode.Ok; i++)
            {
                result = CreateBuiltinTopic(osplBuiltinTopics[i], tQos);
            }

            if (result == DDS.ReturnCode.Ok)
            {
                this.ParticipantDataMarshaler = DatabaseMarshaler.GetMarshaler(
                        this, osplBuiltinTopics[DCPS_PARTICIPANT_INDEX].typeSupport.TypeSpec) as __ParticipantBuiltinTopicDataMarshaler;
            }
            if (result == DDS.ReturnCode.Ok)
            {
               this.TopicDataMarshaler = DatabaseMarshaler.GetMarshaler(
                       this, osplBuiltinTopics[DCPS_TOPIC_INDEX].typeSupport.TypeSpec) as __TopicBuiltinTopicDataMarshaler;
            }

            return result;
        }

        internal override void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status)
        {
            ReturnCode result;

            IDomainParticipantListener dpListener = listener as IDomainParticipantListener;
            if (dpListener != null)
            {

	            if ((triggerMask & V_EVENT.ON_DATA_ON_READERS) == V_EVENT.ON_DATA_ON_READERS)
	            {
	                result = ResetDataAvailableStatus();
	                if (result == DDS.ReturnCode.Ok)
	                {
	                    dpListener.OnDataOnReaders(source as ISubscriber);
	                }
	            }
	            else
	            {
	                if ((triggerMask & V_EVENT.DATA_AVAILABLE) == V_EVENT.DATA_AVAILABLE)
	                {
	                    result = ResetDataAvailableStatus();
	                    if (result == DDS.ReturnCode.Ok)
	                    {
	                        dpListener.OnDataAvailable(source as IDataReader);
	                    }
	                }
	            }

	            if ((triggerMask & V_EVENT.SAMPLE_REJECTED) == V_EVENT.SAMPLE_REJECTED)
	            {
	                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
	                dpListener.OnSampleRejected(source as IDataReader, readerStatus.SampleRejected);
	            }

	            if ((triggerMask & V_EVENT.LIVELINESS_CHANGED) == V_EVENT.LIVELINESS_CHANGED)
	            {
	                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
	                dpListener.OnLivelinessChanged(source as IDataReader, readerStatus.LivelinessChanged);
	            }

	            if ((triggerMask & V_EVENT.SAMPLE_LOST) == V_EVENT.SAMPLE_LOST)
	            {
	                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
	                dpListener.OnSampleLost(source as IDataReader, readerStatus.SampleLost);
	            }

	            if ((triggerMask & V_EVENT.LIVELINESS_LOST) == V_EVENT.LIVELINESS_LOST)
	            {
	                DDS.OpenSplice.Common.WriterStatus writerStatus = status as DDS.OpenSplice.Common.WriterStatus;
	                dpListener.OnLivelinessLost(source as IDataWriter, writerStatus.LivelinessLost);
	            }

	            if ((triggerMask & V_EVENT.REQUESTED_DEADLINE_MISSED) == V_EVENT.REQUESTED_DEADLINE_MISSED)
	            {
	                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
	                dpListener.OnRequestedDeadlineMissed(source as IDataReader, readerStatus.DeadlineMissed);
	            }
	            if ((triggerMask & V_EVENT.OFFERED_DEADLINE_MISSED) == V_EVENT.OFFERED_DEADLINE_MISSED)
	            {
	                DDS.OpenSplice.Common.WriterStatus writerStatus = status as DDS.OpenSplice.Common.WriterStatus;
	                dpListener.OnOfferedDeadlineMissed(source as IDataWriter, writerStatus.DeadlineMissed);
	            }

	            if ((triggerMask & V_EVENT.REQUESTED_INCOMPATIBLE_QOS) == V_EVENT.REQUESTED_INCOMPATIBLE_QOS)
	            {
	                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
	                dpListener.OnRequestedIncompatibleQos(source as IDataReader, readerStatus.IncompatibleQos);
	            }

	            if ((triggerMask & V_EVENT.OFFERED_INCOMPATIBLE_QOS) == V_EVENT.OFFERED_INCOMPATIBLE_QOS)
	            {
	                DDS.OpenSplice.Common.WriterStatus writerStatus = status as DDS.OpenSplice.Common.WriterStatus;
	                dpListener.OnOfferedIncompatibleQos(source as IDataWriter, writerStatus.IncompatibleQos);
	            }

	            if ((triggerMask & V_EVENT.SUBSCRIPTION_MATCHED) == V_EVENT.SUBSCRIPTION_MATCHED)
	            {
	                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
	                dpListener.OnSubscriptionMatched(source as IDataReader, readerStatus.SubscriptionMatch);
	            }

	            if ((triggerMask & V_EVENT.PUBLICATION_MATCHED) == V_EVENT.PUBLICATION_MATCHED)
	            {
	                DDS.OpenSplice.Common.WriterStatus writerStatus = status as DDS.OpenSplice.Common.WriterStatus;
	                dpListener.OnPublicationMatched(source as IDataWriter, writerStatus.PublicationMatch);
	            }

	            if ((triggerMask & V_EVENT.INCONSISTENT_TOPIC) == V_EVENT.INCONSISTENT_TOPIC)
	            {
	                DDS.OpenSplice.Common.TopicStatus topicStatus = status as DDS.OpenSplice.Common.TopicStatus;
	                dpListener.OnInconsistentTopic(source as ITopic, topicStatus.InconsistentTopic);
	            }

	            /*if ((triggerMask & V_EVENT.ALL_DATA_DISPOSED) == V_EVENT.ALL_DATA_DISPOSED)
	            {
	                IDomainParticipantListener dpExtListener = listener as IExtDomainParticipantListener;
	                Debug.Assert(dpExtListener != null);
	                dpExtListener.OnAllDataDisposed(source as ITopic);
	            }*/
	        }
        }

        public IDomainParticipantListener Listener
        {
            get
            {
                bool isAlive;
                IDomainParticipantListener dpListener = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        dpListener = rlReq_Listener as IDomainParticipantListener;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return dpListener;
            }
        }

        public ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = wlReq_SetListener(listener, mask);
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public IPublisher CreatePublisher()
        {
            return CreatePublisher(defaultPublisherQos, null, 0);
        }

        public IPublisher CreatePublisher(IPublisherListener listener, StatusKind mask)
        {
            return CreatePublisher(defaultPublisherQos, listener, mask);
        }

        public IPublisher CreatePublisher(PublisherQos qos)
        {
            return CreatePublisher(qos, null, 0);
        }

        /// <summary>
        /// This operation creates a Publisher with the desired QosPolicy settings and if applicable,
        /// attaches the optionally specified PublisherListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a Publisher with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified PublisherListener to it. When the
        /// PublisherListener is not applicable, the NULL pointer must be supplied instead.
        /// To delete the Publisher the operation DeletePublisher or
        /// DeleteContainedEntities must be used.
        /// In case the specified QosPolicy settings are not consistent, no Publisher is
        /// created and the NULL pointer is returned.
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings for the new Publisher.
        /// In case these settings are not self consistent, no Publisher is created.</param>
        /// <param name="listener">The PublisherListener instance which will be attached to the new Publisher.
        /// It is permitted to use null as the value of the listener: this behaves as a PublisherListener
        /// whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the PublisherListener
        /// for a certain status.</param>
        /// <returns>The newly created Publisher. In case of an error, a null Publisher is returned.</returns>
        public IPublisher CreatePublisher(PublisherQos qos, IPublisherListener listener, StatusKind mask)
        {
            Publisher publisher = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok)
                    {
                        publisher = new OpenSplice.Publisher();
                        result = publisher.init(this, this.rlReq_getChildName("publisher"), qos);
                        if (result == ReturnCode.Ok)
                        {
                            publisher.wlReq_ListenerDispatcher = this.wlReq_ListenerDispatcher;
                            result = publisher.SetListener(listener, mask);
                        }
                        else
                        {
                            publisher = null;
                        }

                        if (result == ReturnCode.Ok)
                        {
                            publisherList.Add(publisher);
                            if (rlReq_AutoEnableCreatedEntities)
                            {
                                result = publisher.Enable();
                            }
                        }

                        if (result != ReturnCode.Ok && publisher != null)
                        {
                            // Ignore result because we prefer the original error.
                            DeletePublisher(publisher);
                            publisher = null;
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return publisher;
        }

        public ReturnCode DeletePublisher(IPublisher p)
        {
            ReturnCode result = ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    Publisher publisher = p as Publisher;
                    if (publisher != null)
                    {
                        if (publisherList.Remove(publisher))
                        {
                            result = publisher.deinit();
                            if (result != ReturnCode.Ok)
                            {
                                publisherList.Add(publisher);
                            }
                        }
                        else
                        {
                            /* The Publisher can be AlreadyDeleted, or it can be from another
                             * participant. Its liveliness cannot be modified without the lock
                             * of its factory, so we are safe checking it here since we hold
                             * the lock to this factory. If the publisher is from another
                             * factory, then the result may be PRECONDITION_NOT_MET while
                             * it should have been BAD_PARAMETER, but such a Use Case has
                             * an inherent race-condition anyway, and the result of such
                             * a test is by definition undefined.
                             */
                            if (publisher.rlReq_isAlive)
                            {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "Publisher " + publisher + " unknown to DomainParticipant " + this + ".");
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the DomainParticipant in this context,
                                // so for a deleted publisher use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "Publisher " + publisher + " was already deleted.");
                            }
                        }
                    }
                    else
                    {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report(result, "Publisher 'p' is null, or of unknown type");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ISubscriber CreateSubscriber()
        {
            return CreateSubscriber(defaultSubscriberQos, null, 0);
        }

        public ISubscriber CreateSubscriber(ISubscriberListener listener, StatusKind mask)
        {
            return CreateSubscriber(defaultSubscriberQos, listener, mask);
        }

        public ISubscriber CreateSubscriber(SubscriberQos qos)
        {
            return CreateSubscriber(qos, null, 0);
        }

        public ISubscriber CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask)
        {
            Subscriber subscriber = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok)
                    {
                        subscriber = new OpenSplice.Subscriber();
                        result = subscriber.init(this, this.rlReq_getChildName("subscriber"), qos);
                        if (result == ReturnCode.Ok)
                        {
                            subscriber.wlReq_ListenerDispatcher = this.wlReq_ListenerDispatcher;
                            result = subscriber.SetListener(listener, mask);
                        }
                        else
                        {
                            subscriber = null;
                        }

                        if (result == ReturnCode.Ok)
                        {
                            subscriberList.Add(subscriber);
                            if (rlReq_AutoEnableCreatedEntities &&
                                !(qos.Presentation.CoherentAccess &&
                                  qos.Presentation.AccessScope == PresentationQosPolicyAccessScopeKind.GroupPresentationQos))
                            {
                                result = subscriber.Enable();
                            }
                        }

                        if (result != ReturnCode.Ok && subscriber != null)
                        {
                            // Ignore result because we prefer the original error.
                            DeleteSubscriber(subscriber);
                            subscriber = null;
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return subscriber;
        }

        public ReturnCode DeleteSubscriber(ISubscriber s)
        {
            ReturnCode result = ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    Subscriber subscriber = s as Subscriber;
                    if (subscriber != null)
                    {
                        bool unknownSubscriber = true;

                        if (subscriber == builtinSubscriber)
                        {
                            result = subscriber.DeleteContainedEntities();
                            if (result == ReturnCode.Ok)
                            {
                                result = subscriber.wlReq_deinit();
                                if (result == ReturnCode.Ok)
                                {
                                    unknownSubscriber = false;
                                    builtinSubscriber = null;
                                }
                            }
                        }
                        else if (subscriberList.Remove(subscriber))
                        {
                            result = subscriber.deinit();
                            if (result != ReturnCode.Ok)
                            {
                                subscriberList.Add(subscriber);
                            }
                            else
                            {
                                unknownSubscriber = false;
                            }
                        }

                        if (unknownSubscriber)
                        {
                            /* The Subscriber can be AlreadyDeleted, or it can be from another
                             * participant. Its liveliness cannot be modified without the lock
                             * of its factory, so we are safe checking it here since we hold
                             * the lock to this factory. If the subscriber is from another
                             * factory, then the result may be PRECONDITION_NOT_MET while
                             * it should have been BAD_PARAMETER, but such a Use Case has
                             * an inherent race-condition anyway, and the result of such
                             * a test is by definition undefined.
                             */
                            if (subscriber.rlReq_isAlive)
                            {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "Subscriber " + subscriber + " unknown to DomainParticipant " + this + ".");
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the DomainParticipant in this context,
                                // so for a deleted subscriber use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "Subscriber " + subscriber + " was already deleted.");
                            }
                        }
                    }
                    else
                    {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report(result, "Subscriber 's' is null, or of unknown type");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ISubscriber BuiltInSubscriber
        {
            get
            {
                ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
                bool created = false;

                ReportStack.Start();
                lock(this)
                {
                    if (this.rlReq_isAlive)
                    {
                        if (builtinSubscriber == null) {
                            SubscriberQos sQos = new SubscriberQos();
                            using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                                    new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
                            {
                                result = marshaler.CopyIn(QosManager.defaultSubscriberQos);
                                if (result == ReturnCode.Ok)
                                {
                                    marshaler.CopyOut(ref sQos);
                                    sQos.Presentation.AccessScope = PresentationQosPolicyAccessScopeKind.TopicPresentationQos;
                                    sQos.Partition.Name = new string[1] { "__BUILT-IN PARTITION__" };
                                }
                            }

                            /* Create, initialize and store the Subscriber object. */
                            builtinSubscriber = new Subscriber();
                            result = builtinSubscriber.init(this, "BuiltinSubscriber", sQos);
                            if (result == DDS.ReturnCode.Ok) {
                                builtinSubscriber.wlReq_ListenerDispatcher = wlReq_ListenerDispatcher;
                                if (rlReq_AutoEnableCreatedEntities) {
                                    result = builtinSubscriber.Enable();
                                    if (result != DDS.ReturnCode.Ok) {
                                        ReportStack.Report(result, "Failed to enable builtinsubscriber.");
                                    }
                                }
                            } else {
                                ReportStack.Report(result, "Failed to initialize builtinsubscriber.");
                            }
                            if (result == DDS.ReturnCode.Ok) {
                                created = true;
                            } else {
                                builtinSubscriber = null;
                            }
                        }
                    }
                }

                if (created) {
                    DataReaderQos drQos = new DataReaderQos();

                    using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
                    {
                        result = marshaler.CopyIn(QosManager.defaultDataReaderQos);
                        if (result == ReturnCode.Ok)
                        {
                            marshaler.CopyOut(ref drQos);
                            drQos.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
                            drQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;
                        }
                    }

                    foreach(Topic biTopic in builtinTopicList)
                    {
                        builtinSubscriber.CreateDataReader(biTopic, drQos, null, StatusKind.Any);
                    }
                }
                ReportStack.Flush(this, result != ReturnCode.Ok);

                return builtinSubscriber;

            }
        }

        public ITopic CreateTopic(string topicName, string typeName)
        {
            return CreateTopic(topicName, typeName, defaultTopicQos, null, 0);
        }

        public ITopic CreateTopic(
                string topicName,
                string typeName,
                ITopicListener listener,
                StatusKind mask)
        {
            return nlReq_CreateTopic(topicName, typeName, defaultTopicQos, listener, mask, topicList);
        }

        public ITopic CreateTopic(string topicName, string typeName, TopicQos qos)
        {
            return nlReq_CreateTopic(topicName, typeName, qos, null, 0, topicList);
        }

        public ITopic CreateTopic(
                string topicName,
                string typeName,
                TopicQos qos,
                ITopicListener listener,
                StatusKind mask)
        {
            return nlReq_CreateTopic(topicName, typeName, qos, listener, mask, topicList);
        }

        internal Topic nlReq_CreateTopic(
                string topicName,
                string typeName,
                TopicQos qos,
                ITopicListener listener,
                StatusKind mask,
                List<Topic> tList)
        {
            Topic topic = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            IntPtr uTopic = IntPtr.Zero;
            TypeSupport ts = null;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    /*
                     * The creation of a Topic is somewhat different then the creation of a Subscriber
                     * or Publisher. The reason for this is the special functionality for the function
                     * find_topic(). For that, the init of Topic has a user layer object as argument,
                     * where Publisher and Subscriber don't (they get that object themselves).
                     * So, because of the find_topic() and the related Topic init, we create the user layer
                     * topic here and provide it to the init of the new Topic object.
                     */
                    if (typeName == null) {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "Typename is null for topic \"" + topicName + "\".");
                    } else {
                        result = QosManager.checkQos(qos);
                        if (result == DDS.ReturnCode.Ok) {
                            /* Find the TypeSupport needed for creation of this Topic. */
                            /* Store the created Topic object. */
                            if (!typeSupportList.TryGetValue(typeName, out ts)) {
                                result = DDS.ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "Participant doesn't contain related TypeSupport for topic \""
                                            + topicName + "\" and type \"" + typeName + "\".");
                            }
                        } else {
                            ReportStack.Report(result,
                                        "Topic QoS is not consistent for topic \"" + topicName + "\".");
                        }

                        if (result == DDS.ReturnCode.Ok) {
                            using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                                    new OpenSplice.CustomMarshalers.TopicQosMarshaler())
                            {
                                result = marshaler.CopyIn(qos);
                                if (result == DDS.ReturnCode.Ok)
                                {
                                    /* Create and initialize user layer topic. */
                                    uTopic = User.Topic.New(rlReq_UserPeer,
                                                topicName,
                                                ts.InternalTypeName,
                                                ts.KeyList,
                                                marshaler.UserPtr);

                                    if (uTopic == IntPtr.Zero) {
                                        result = DDS.ReturnCode.Error;
                                        ReportStack.Report(result, "Failed to create internals for topic \"" + topicName + "\".");
                                    }
                                }
                            }
                        }

                        /* Create, initialize and store the Topic object. */
                        if (result == DDS.ReturnCode.Ok) {
                            topic = new Topic();
                            result = topic.init(uTopic, this, topicName, typeName, ts);
                            if (result == ReturnCode.Ok)
                            {
                                topic.wlReq_ListenerDispatcher = this.wlReq_ListenerDispatcher;
                                result = topic.SetListener(listener, mask);
                            }
                            else
                            {
                                topic = null;
                                ReportStack.Report(result, "Failed to initialize topic \"" + topicName + "\".");
                            }
                        }

                        if (result == DDS.ReturnCode.Ok) {
                            tList.Add(topic);
                            if (rlReq_AutoEnableCreatedEntities) {
                                result = topic.Enable();
                            }
                        }

                        /* Cleanup */
                        if (result != ReturnCode.Ok && topic != null)
                        {
                            // Ignore result because we prefer the original error.
                            DeleteTopic(topic);
                            topic = null;
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return topic;
        }

        public ReturnCode DeleteTopic(ITopic t)
        {
            ReturnCode result = ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    Topic topic = t as Topic;

                    if (topic != null)
                    {
                        if (topicList.Remove(topic))
                        {
                            result = topic.deinit();
                            if (result != ReturnCode.Ok)
                            {
                                topicList.Add(topic);
                            }
                        }
                        else
                        {
                            /* The Topic can be AlreadyDeleted, or it can be from another
                             * participant. Its liveliness cannot be modified without the lock
                             * of its factory, so we are safe checking it here since we hold
                             * the lock to this factory. If the topic is from another
                             * factory, then the result may be PRECONDITION_NOT_MET while
                             * it should have been BAD_PARAMETER, but such a Use Case has
                             * an inherent race-condition anyway, and the result of such
                             * a test is by definition undefined.
                             */
                            if (topic.rlReq_isAlive)
                            {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "Topic " + topic + " unknown to DomainParticipant " + this + ".");
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the DomainParticipant in this context,
                                // so for a deleted topic use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "Topic " + topic + " was already deleted.");
                            }
                        }
                    }
                    else
                    {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report(result, "Topic 't' is null, or of unknown type");
                    }
                }
            }

            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        private ReturnCode checkTopicKeys(string topicName, IntPtr uTopic, TypeSupport ts)
        {
            ReturnCode result = ReturnCode.Ok;

            if (ts != null)
            {
                string typeKeyList = ts.KeyList;
                string topicKeyList = BaseMarshaler.ReadString(User.Topic.KeyExpr(uTopic));

                if (typeKeyList == null || topicKeyList == null)
                {
                    if (typeKeyList != topicKeyList)
                    {
                        typeKeyList = (typeKeyList == null) ? "null" : typeKeyList;
                        topicKeyList = (topicKeyList == null) ? "null" : topicKeyList;
                        ReportStack.Warning("TypeSupport " + ts.TypeName + " key " + typeKeyList +
                                            " doesn't match Topic " + topicName + " key " + topicKeyList + ".");
                    }
                }
                else
                {
                    char[] delim = { ',', ' ', '\t' };
                    string[] typeKeyArr = typeKeyList.Split(delim);
                    string[] topicKeyArr = topicKeyList.Split(delim);

                    bool consistent = typeKeyArr.Length == topicKeyArr.Length;
                    if (consistent)
                    {
                        for (int i = 0; consistent && i < typeKeyArr.Length; i++)
                        {
                            consistent = typeKeyArr[i].Equals(topicKeyArr[i]);
                        }
                    }
                    if (!consistent)
                    {
                        ReportStack.Warning("TypeSupport " + ts.TypeName + " key " + typeKeyList +
                                            " doesn't match Topic " + topicName + " key " + topicKeyList + ".");
                    }
                }
            }

            return result;
        }

        public ITopic FindTopic(string topicName, Duration timeout)
        {
            Topic topic = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();

            if (topicName == null || topicName.IndexOfAny(new char[] {'*', '?'}) != -1)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "No topic name or topic name contains invalid characters.");
            }
            else if (QosManager.countErrors(timeout) > 0)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "Timeout parameter invalid.");
            }
            else
            {
                result = DDS.ReturnCode.Ok;
            }

            IntPtr matches = User.DomainParticipant.FindTopic(rlReq_UserPeer, topicName, timeout.OsDuration);
            Debug.Assert(c.iterLength(matches) <= 1);
            IntPtr uTopic = c.iterTakeFirst(matches);
            c.iterFree(matches);
            if (uTopic == IntPtr.Zero) {
                result = DDS.ReturnCode.PreconditionNotMet;
                string tn = (topicName != null) ? topicName : "null";
                ReportStack.Report(result, "Failed to resolve Topic '" + tn + "'.");
            }

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (result == DDS.ReturnCode.Ok)
                    {
                        TypeSupport typeSupport = null;

                        string typeName = BaseMarshaler.ReadString(
                                        User.Topic.TypeName(uTopic));
                        typeSupportList.TryGetValue(typeName, out typeSupport);
                        if (typeSupport == null)
                        {
                            // Handle Builtin topics first.
                            foreach (TypeSupport ts in typeSupportList.Values)
                            {
                                if (ts.InternalTypeName.Equals(typeName))
                                {
                                    typeSupport = ts;
                                    break;
                                }
                            }
                        }

                        result = checkTopicKeys(topicName, uTopic, typeSupport);

                        if (result == DDS.ReturnCode.Ok)
                        {
                            /*
                             * typeMetaHolder == NULL is allowed when creating a topic by means of a find.
                             *
                             * The Topic will start off with no typeMetaHolder.
                             * When the related TypeSupport is registered, the typeMetaHolder of it will be
                             * stored in this participant.
                             * When, after that, a DataReader is created, then participant::find_type_support_factory()
                             * will be called and the result will be added to the related Topic, which was
                             * supplied with the create_reader() call.
                             * If the type wasn't registered, then the DataReader creation will fail.
                             */
                            /* Create, initialize and store the Topic object. */
                            topic = new Topic();
                            result = topic.init(uTopic, this, topicName, typeName, typeSupport);
                            if (result == DDS.ReturnCode.Ok)
                            {
                                topicList.Add(topic);
                            }
                            else
                            {
                                topic = null;
                                ReportStack.Report(result, "Failed to initialize topic \"" + topicName + "\".");
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return topic;
        }

        public ITopicDescription LookupTopicDescription (string name)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ITopicDescription td = null;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (name != null) {
                        td = topicList.Find (
                            delegate (Topic a_topic) {
                            return name.Equals (a_topic.Name);
                        }
                        );

                        if (td == null) {
                            td = builtinTopicList.Find (
                                delegate (Topic a_biTopic) {
                                return a_biTopic.Name == name;
                            }
                            );
                        }

                        if (td == null) {
                            td = cfTopicList.Find (
                                delegate (ContentFilteredTopic a_cftopic) {
                                return a_cftopic.Name == name;
                            }
                            );
                        }

                        if (td == null) {
                            td = mTopicList.Find (
                                delegate (MultiTopic a_mtopic) {
                                return a_mtopic.Name == name;
                            }
                            );
                        }
                        result = DDS.ReturnCode.Ok;
                    } else {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report (result, "name '<NULL>' is invalid.");
                    }
                }
            }
            ReportStack.Flush(this, result != DDS.ReturnCode.Ok);


            return td;
        }

        public IContentFilteredTopic CreateContentFilteredTopic(
                string name,
                ITopic relatedTopic,
                string filterExpression,
                params string[] expressionParameters)
        {
            ContentFilteredTopic contentFilteredTopic = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            Topic topicObj = null;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if ((name == null) ||
                        (filterExpression == null)) {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "Name or filterExpression parameters contain bad values.");
                    } else {
                        result = DDS.ReturnCode.Ok;
                    }

                    if (result == DDS.ReturnCode.Ok) {
                        topicObj = relatedTopic as Topic;
                        if (topicObj == null) {
                            result = DDS.ReturnCode.BadParameter;
                            ReportStack.Report(result, "RelatedTopic parameter contains bad value.");
                        }
                    }

                    if (result == DDS.ReturnCode.Ok) {
                        /* Create, initialize and store the ContentFilteredTopic object. */
                        contentFilteredTopic = new ContentFilteredTopic(topicObj.TypeName, name, this, filterExpression, expressionParameters);
                        if (contentFilteredTopic != null) {
                            result = contentFilteredTopic.init(topicObj);// this, name, topicObj, filterExpression, filterParameters);
                            if (result == DDS.ReturnCode.Ok) {
                                /* Store the created ContentFilteredTopic object. */
                                cfTopicList.Add(contentFilteredTopic);
                            } else {
                                /* Destroy created object after init failure. */
                                contentFilteredTopic = null;
                                result = DDS.ReturnCode.Error;
                                ReportStack.Report(result, "Failed to initialize ContentFilteredTopic \"" + name + "\".");
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return contentFilteredTopic;
        }

        public ReturnCode DeleteContentFilteredTopic (IContentFilteredTopic t)
        {
            ContentFilteredTopic cftopic = t as ContentFilteredTopic;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start ();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (t != null) {
                        if (cftopic != null) {
                            if (cfTopicList.Remove (cftopic)) {
                                result = cftopic.deinit ();
                                if (result == DDS.ReturnCode.AlreadyDeleted) {
                                    // ALREADY_DELETED may only apply to the DomainParticipant in this context,
                                    // so for a deleted contentfilteredtopic use BAD_PARAMETER instead.
                                    result = DDS.ReturnCode.BadParameter;
                                } else if (result != ReturnCode.Ok) {
                                    cfTopicList.Add (cftopic);
                                }
                            } else {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report (result, "ContentFilteredTopic " + cftopic + " unknown to DomainParticipant " + this + ".");
                            }
                        }
                    } else {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report (result, "contentfilteredtopic is invalid, not of type " +
                            "DDS::OpenSplice::ContentFilteredTopic.");
                    }
                } else {
                    result = ReturnCode.BadParameter;
                    ReportStack.Report (result, "contentfilteredtopic '<NULL>' is invalid.");
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }


        public IMultiTopic CreateMultiTopic(
                string name,
                string typeName,
                string subscriptionExpression,
                params string[] expressionParameters)
        {
            MultiTopic multiTopic = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            TypeSupport ts;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if ((name == null) ||
                        (typeName == null) ||
                        (subscriptionExpression == null)) {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "Name, typeName or filterExpression parameters contain bad values.");
                    } else {
                        result = DDS.ReturnCode.Ok;
                    }

                    if (result == DDS.ReturnCode.Ok) {
                        /* Find the TypeSupport needed for creation of this Topic. */
                        /* Store the created Topic object. */
                        if (!typeSupportList.TryGetValue(typeName, out ts)) {
                            result = DDS.ReturnCode.PreconditionNotMet;
                            ReportStack.Report(result, "Participant doesn't contain related TypeSupport for topic \""
                                        + name + "\" and type \"" + typeName + "\".");
                        }

                        if (result == DDS.ReturnCode.Ok)
                        {
                            /* Create, initialize and store the ContentFilteredTopic object. */
                            multiTopic = new MultiTopic(typeName, name, this, ts, subscriptionExpression, expressionParameters);
                            result = multiTopic.init();
                            if (result == DDS.ReturnCode.Ok) {
                                /* Store the created MultiTopic object. */
                                mTopicList.Add(multiTopic);
                            } else {
                                /* Destroy created object after init failure. */
                                multiTopic = null;
                                result = DDS.ReturnCode.Error;
                                ReportStack.Report(result,
                                          "Failed to initialize MultiTopic \"" + name + "\".");
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return multiTopic;
        }

        public ReturnCode DeleteMultiTopic(IMultiTopic t)
        {
            MultiTopic mtopic = t as MultiTopic;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (mtopic != null)
                    {
                        if (mTopicList.Remove(mtopic))
                        {
                            result = mtopic.deinit();
                            if (result == DDS.ReturnCode.AlreadyDeleted)
                            {
                                // ALREADY_DELETED may only apply to the DomainParticipant in this context,
                                // so for a deleted multitopic use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                            }
                            else if (result != ReturnCode.Ok)
                            {
                                mTopicList.Add(mtopic);
                            }
                        }
                        else
                        {
                            result = ReturnCode.PreconditionNotMet;
                            ReportStack.Report(result, "MultiTopic " + mtopic + " unknown to DomainParticipant " + this + ".");
                        }
                    }
                    else
                    {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report(result, "MultiTopic 't' is null, or of unknown type");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode DeleteContainedEntities()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            // TODO: Delete builtin subscriber.

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = DDS.ReturnCode.Ok;
                    foreach (Publisher p in publisherList)
                    {
                        result = p.DeleteContainedEntities();
                        if (result == DDS.ReturnCode.Ok)
                        {
                            result = p.deinit();
                        }
                        if (result != DDS.ReturnCode.Ok)
                        {
                            break;
                        }
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        publisherList.Clear();
                        foreach (Subscriber s in subscriberList)
                        {
                            result = s.DeleteContainedEntities();
                            if (result == DDS.ReturnCode.Ok)
                            {
                                result = s.deinit();
                            }
                            if (result != DDS.ReturnCode.Ok)
                            {
                                break;
                            }
                        }
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        subscriberList.Clear();
                        foreach (MultiTopic mt in mTopicList)
                        {
                            result = mt.deinit();
                            if (result != DDS.ReturnCode.Ok) break;
                        }
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        mTopicList.Clear();
                        foreach (ContentFilteredTopic cft in cfTopicList)
                        {
                            result = cft.deinit();
                            if (result != DDS.ReturnCode.Ok) break;
                        }
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        cfTopicList.Clear();
                        foreach (Topic t in topicList)
                        {
                            result = t.deinit();
                            if (result != DDS.ReturnCode.Ok) break;
                        }
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        topicList.Clear();
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetQos(DomainParticipantQos qos)
        {
            ListenerDispatcher dispatcher;

            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = QosManager.checkQos(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
                    {
                        result = marshaler.CopyIn(qos);
                        if (result == ReturnCode.Ok)
                        {
                            result = uResultToReturnCode(
                                User.DomainParticipant.SetQos(rlReq_UserPeer, marshaler.UserPtr));
                            if (result == ReturnCode.Ok)
                            {
                                dispatcher = wlReq_ListenerDispatcher;
                                result = dispatcher.SetScheduling(
                                    qos.ListenerScheduling);
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetProperty(ref Property property)
        {
            IntPtr Value = IntPtr.Zero;

            ReturnCode result = DDS.ReturnCode.Unsupported;
            result = checkProperty(ref property);
            if (result == DDS.ReturnCode.Ok) {
                IntPtr uDomain = User.DomainParticipant.Domain(rlReq_UserPeer);
                result = uResultToReturnCode(User.Entity.GetProperty(uDomain, property.Name, ref Value));
                if (Value != IntPtr.Zero) {
                    property.Value = Marshal.PtrToStringAnsi(Value);
                }
            }
            return result;
        }
    
        public ReturnCode SetProperty(Property property)
        {
            ReturnCode result = DDS.ReturnCode.Unsupported;
            result = checkProperty(ref property);
            if (result == DDS.ReturnCode.Ok) {
                IntPtr uDomain = User.DomainParticipant.Domain(rlReq_UserPeer);
                result = uResultToReturnCode(User.Entity.SetProperty(uDomain, property.Name, property.Value));
            }
            return result;
        }

        public ReturnCode GetQos(ref DomainParticipantQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            IntPtr userQos = IntPtr.Zero;
            ListenerDispatcher dispatcher;

            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.DomainParticipant.GetQos(rlReq_UserPeer, ref userQos));
                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler(userQos, true))
                    {
                        marshaler.CopyOut(ref qos);

                        /* Listener scheduling qospolicy is not part of User
                           Layer, so obtain it separately from participant. */
                        dispatcher = wlReq_ListenerDispatcher;
                        result = dispatcher.GetScheduling(
                            ref qos.ListenerScheduling);
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode IgnoreParticipant(InstanceHandle handle)
        {
//            return Gapi.DomainParticipant.ignore_participant(
//                    GapiPeer,
//                    handle);
              return ReturnCode.Unsupported;
        }

        public ReturnCode IgnoreTopic(InstanceHandle handle)
        {
//            return Gapi.DomainParticipant.ignore_topic(
//                    GapiPeer,
//                    handle);
            return ReturnCode.Unsupported;
        }

        public ReturnCode IgnorePublication(InstanceHandle handle)
        {
//            return Gapi.DomainParticipant.ignore_publication(
//                    GapiPeer,
//                    handle);
            return ReturnCode.Unsupported;
        }

        public ReturnCode IgnoreSubscription(InstanceHandle handle)
        {
//            return Gapi.DomainParticipant.ignore_subscription(
//                    GapiPeer,
//                    handle);
            return ReturnCode.Unsupported;
        }


        public DomainId DomainId
        {
            get
            {
                ReportStack.Start();
                DomainId id = User.DomainParticipant.GetDomainId(rlReq_UserPeer);
                ReportStack.Flush(this, id == DDS.DomainId.Invalid);

                return id;
            }
        }

        public ReturnCode AssertLiveliness()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.DomainParticipant.AssertLiveliness(rlReq_UserPeer));
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetDefaultPublisherQos(PublisherQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok) {
                        using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
                        {
                            result = marshaler.CopyIn(qos);
                            if (result == ReturnCode.Ok)
                            {
                                marshaler.CopyOut(ref defaultPublisherQos);
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetDefaultPublisherQos(ref PublisherQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
                    {
                        result = marshaler.CopyIn(defaultPublisherQos);
                        if (result == ReturnCode.Ok)
                        {
                            marshaler.CopyOut(ref qos);
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetDefaultSubscriberQos(SubscriberQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok) {
                        using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
                        {
                            result = marshaler.CopyIn(qos);
                            if (result == ReturnCode.Ok)
                            {
                                marshaler.CopyOut(ref defaultSubscriberQos);
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
                    {
                        result = marshaler.CopyIn(defaultSubscriberQos);
                        if (result == ReturnCode.Ok)
                        {
                            marshaler.CopyOut(ref qos);
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetDefaultTopicQos(TopicQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok) {
                        using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.TopicQosMarshaler())
                        {
                            result = marshaler.CopyIn(qos);
                            if (result == ReturnCode.Ok)
                            {
                                marshaler.CopyOut(ref defaultTopicQos);
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetDefaultTopicQos(ref TopicQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.TopicQosMarshaler())
                    {
                        result = marshaler.CopyIn(defaultTopicQos);
                        if (result == ReturnCode.Ok)
                        {
                            marshaler.CopyOut(ref qos);
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        internal class InstanceHandleUserData
        {
            internal InstanceHandleUserData(int Index, InstanceHandle[] Seq)
            {
                this.Index = Index;
                this.Seq = Seq;
            }

            internal int Index;
            internal InstanceHandle[] Seq;
        }

        bool
        CopyInstanceHandle(IntPtr instance, IntPtr arg)
        {
            bool result = true;
            InstanceHandle ghandle;
            uint length;
            GCHandle tmpGCHandleData = GCHandle.FromIntPtr(arg);
            InstanceHandleUserData a = tmpGCHandleData.Target as InstanceHandleUserData;

            if (a.Index == 0) {
                length = Kernel.DataReaderInstance.GetNotEmptyInstanceCount(instance);
                if (a.Seq == null || length != a.Seq.Length) {
                    a.Seq = new InstanceHandle[length]; /* potentially reallocate */
                }
            }

            ghandle = Kernel.InstanceHandle.New(instance);
            if (a.Index < a.Seq.Length) {
                a.Seq[a.Index++] = ghandle;
            } else {
                /* error index out of bounds */
            }

            return result;
        }

        private ReturnCode nlReq_getDiscoveredEntities(string topicName, ref InstanceHandle[] instanceHandles)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    DataReader entityReader = BuiltInSubscriber.LookupDataReader(topicName) as DataReader;
                    if (entityReader != null)
                    {
                        result = entityReader.nlReq_getInstanceHandles(ref instanceHandles);
                    }
                    else
                    {
                        result = DDS.ReturnCode.Error;
                        ReportStack.Report(result, "Unable to locate BuiltinDataReader for topic " + topicName);
                    }
                }
            }

            return result;
        }

        public ReturnCode GetDiscoveredParticipants (ref InstanceHandle[] participantHandles)
        {
            ReportStack.Start();
            ReturnCode result = nlReq_getDiscoveredEntities("DCPSParticipant", ref participantHandles);
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetDiscoveredParticipantData (ref ParticipantBuiltinTopicData data, InstanceHandle handle)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    ParticipantBuiltinTopicDataDataReader participantReader;

                    participantReader = BuiltInSubscriber.LookupDataReader("DCPSParticipant") as ParticipantBuiltinTopicDataDataReader;
                    if (participantReader != null)
                    {
                        ParticipantBuiltinTopicData[] dataSeq = null;
                        SampleInfo[] infoSeq = null;

                        result = participantReader.ReadInstance(ref dataSeq, ref infoSeq, 1, handle,
                                                                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            Debug.Assert(dataSeq.Length == 1);
                            data = dataSeq[0];
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.Error;
                        ReportStack.Report(result, "Unable to locate BuiltinDataReader for topic 'DCPSParticipant'");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetDiscoveredTopics (ref InstanceHandle[] topicHandles)
        {
            ReportStack.Start();
            ReturnCode result = nlReq_getDiscoveredEntities("DCPSTopic", ref topicHandles);
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetDiscoveredTopicData (ref TopicBuiltinTopicData data, InstanceHandle handle)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    TopicBuiltinTopicDataDataReader topicReader;

                    topicReader = BuiltInSubscriber.LookupDataReader("DCPSTopic") as TopicBuiltinTopicDataDataReader;
                    if (topicReader != null)
                    {
                        TopicBuiltinTopicData[] dataSeq = null;
                        SampleInfo[] infoSeq = null;

                        result = topicReader.ReadInstance(ref dataSeq, ref infoSeq, 1, handle,
                                                          SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            Debug.Assert(dataSeq.Length == 1);
                            data = dataSeq[0];
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.Error;
                        ReportStack.Report(result, "Unable to locate BuiltinDataReader for topic 'DCPSTopic'");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public bool ContainsEntity(InstanceHandle handle)
        {
            bool contained = false;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = DDS.ReturnCode.Ok;
                    contained = publisherList.Exists(
                        delegate (Publisher a_publisher)
                        {
                            bool found = (a_publisher.InstanceHandle == handle);
                            if (!found) found = a_publisher.ContainsEntity(handle);
                            return found;
                        }
                    );
                    if (!contained) {
                        contained = subscriberList.Exists(
                            delegate (Subscriber a_subscriber)
                            {
                                bool found = (a_subscriber.InstanceHandle == handle);
                                if (!found) found = a_subscriber.ContainsEntity(handle);
                                return found;
                            }
                        );
                    }
                    if (!contained) {
                        contained = topicList.Exists(
                            delegate (Topic a_topic)
                            {
                                return a_topic.InstanceHandle == handle;
                            }
                        );
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return contained;
        }

        public ReturnCode GetCurrentTime(out Time currentTime)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = DDS.ReturnCode.Ok;
                    currentTime = new Time();
                    currentTime.OsTimeW = u.timeWGet();
                } else {
                    currentTime = Time.Invalid;
                }
            }

            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }
        string rlReq_getChildName(string prefix)
        {
            string name = Marshal.PtrToStringAnsi(User.Entity.Name (rlReq_UserPeer));
            return(prefix + " <"+name.Replace ("<" + Process.GetCurrentProcess().Id + ">", "").TrimEnd()+">");
        }
    }
}
