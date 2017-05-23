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
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Diagnostics;
using DDS;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModuleI;

namespace DDS.OpenSplice
{
    internal class Publisher : Entity, IPublisher
    {
        private DomainParticipant participant;
        private DataWriterQos defaultDataWriterQos = new DataWriterQos();
        private List<DataWriter> writerList = new List<DataWriter>();

        internal Publisher()
        {
            DDS.ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler =
                new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultDataWriterQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref defaultDataWriterQos);
                }
            }
        }

        internal ReturnCode init(DomainParticipant participant, string name, PublisherQos qos)
        {
            ReturnCode result;

            ReportStack.Start();
            MyDomainId = participant.MyDomainId;
            using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    IntPtr uPublisher = User.Publisher.New(participant.rlReq_UserPeer, name, marshaler.UserPtr, 0);
                    if (uPublisher != IntPtr.Zero)
                    {
                        result = base.init(uPublisher);
                    }
                    else
                    {
                        result = DDS.ReturnCode.Error;
                        ReportStack.Report(result, "Could not create Publisher.");
                    }
                }
            }

            if (result == ReturnCode.Ok)
            {
                this.participant = participant;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            if (writerList.Count == 0)
            {
                IPublisherListener pubListener = listener as IPublisherListener;
                if (pubListener != null)
	            {
	                this.SetListener(pubListener,(DDS.StatusKind)0);
	            }
	            this.DisableCallbacks();
                result = base.wlReq_deinit();
                if (result == DDS.ReturnCode.Ok)
                {
                    this.participant = null;
                }
            }
            else
            {
                result = DDS.ReturnCode.PreconditionNotMet;
                ReportStack.Report(result, "Publisher " + this + " cannot be deleted since it still contains " +
                            writerList.Count + " DataWriters.");
            }
            return result;
        }

        private bool rlReq_AutoEnableCreatedEntities
        {
            get {
                PublisherQos pQos = new PublisherQos();
                IntPtr userQos = IntPtr.Zero;
                bool autoEnable = false;

                ReturnCode result = uResultToReturnCode(
                        User.Publisher.GetQos(rlReq_UserPeer, ref userQos));

                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.PublisherQosMarshaler(userQos, false))
                    {
                        marshaler.CopyOut(ref pQos);
                        autoEnable = pQos.EntityFactory.AutoenableCreatedEntities;
                    }
                }
                User.PublisherQos.Free(userQos);

                return autoEnable;
            }
        }

        internal bool ContainsEntity(InstanceHandle handle)
        {
            bool contained = false;

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    contained = writerList.Exists(
                        delegate (DataWriter a_datawriter)
                        {
                            return a_datawriter.InstanceHandle == handle;
                        }
                    );
                }
            }

            return contained;
        }

        internal override void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status)
        {
            IPublisherListener pubListener = listener as IPublisherListener;
            if(pubListener != null)
            {
                DDS.OpenSplice.Common.WriterStatus writerStatus = status as DDS.OpenSplice.Common.WriterStatus;

	            if ((triggerMask & V_EVENT.LIVELINESS_LOST) == V_EVENT.LIVELINESS_LOST)
	            {
	                pubListener.OnLivelinessLost(source as IDataWriter, writerStatus.LivelinessLost);
	            }

	            if ((triggerMask & V_EVENT.OFFERED_DEADLINE_MISSED) == V_EVENT.OFFERED_DEADLINE_MISSED)
	            {
	                pubListener.OnOfferedDeadlineMissed(source as IDataWriter, writerStatus.DeadlineMissed);
	            }

	            if ((triggerMask & V_EVENT.OFFERED_INCOMPATIBLE_QOS) == V_EVENT.OFFERED_INCOMPATIBLE_QOS)
	            {
	                pubListener.OnOfferedIncompatibleQos(source as IDataWriter, writerStatus.IncompatibleQos);
	            }

	            if ((triggerMask & V_EVENT.PUBLICATION_MATCHED) == V_EVENT.PUBLICATION_MATCHED)
	            {
	                pubListener.OnPublicationMatched(source as IDataWriter, writerStatus.PublicationMatch);
	            }
            }
        }

        public IPublisherListener Listener
        {
            get
            {
                bool isAlive;
                IPublisherListener pubListener = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        pubListener = rlReq_Listener as IPublisherListener;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return pubListener;
            }
        }

        public ReturnCode SetListener(IPublisherListener listener, StatusKind mask)
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

        public IDataWriter CreateDataWriter(ITopic topic)
        {
            return CreateDataWriter(topic, defaultDataWriterQos, null, 0);
        }

        public IDataWriter CreateDataWriter(
                ITopic topic,
                IDataWriterListener listener,
                StatusKind mask)
        {
            return CreateDataWriter(topic, defaultDataWriterQos, listener, mask);
        }

        public IDataWriter CreateDataWriter(ITopic topic, DataWriterQos qos)
        {
            return CreateDataWriter(topic, qos, null, 0);
        }

        public IDataWriter CreateDataWriter(
                ITopic topic,
                DataWriterQos qos,
                IDataWriterListener listener,
                StatusKind mask)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            DataWriter dataWriter = null;
            Topic topicObj = topic as Topic;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (topicObj == null)
                    {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "topic is invalid (null), or not of type " +
                                                    "DDS::OpenSplice::Topic.");
                    }
                    else
                    {
                        result = QosManager.checkQos(qos);
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        lock(topicObj)
                        {
                            if (topicObj.rlReq_isAlive)
                            {
                                TypeSupport ts = (topicObj as ITopicDescriptionImpl).rlReq_TypeSupport;
                                DatabaseMarshaler marshaler = DatabaseMarshaler.GetMarshaler(participant, ts.TypeSpec);
                                dataWriter = ts.CreateDataWriter(marshaler);
                                result = dataWriter.init(this, qos, topicObj, "writer <" + topicObj.Name + ">");
                                if (result == ReturnCode.Ok)
                                {
                                    dataWriter.wlReq_ListenerDispatcher = this.wlReq_ListenerDispatcher;
                                    result = dataWriter.SetListener(listener, mask);
                                }
                                else
                                {
                                    dataWriter = null;
                                }

                                if (result == DDS.ReturnCode.Ok)
                                {
                                    writerList.Add(dataWriter);
                                    if (rlReq_AutoEnableCreatedEntities)
                                    {
                                        result = dataWriter.Enable();
                                    }
                                }
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the Publisher in this context,
                                // so for a deleted topic use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                            }
                        }
                    }

                    if (result != ReturnCode.Ok && dataWriter != null)
                    {
                        // Ignore result because we prefer the original error.
                        DeleteDataWriter(dataWriter);
                        dataWriter = null;
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return dataWriter;
        }

        public ReturnCode DeleteDataWriter (IDataWriter dataWriter)
        {
            DataWriter dwObj = dataWriter as DataWriter;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (dwObj != null) {
                        if (writerList.Remove(dwObj))
                        {
                            result = dwObj.deinit();
                            if (result != ReturnCode.Ok)
                            {
                                writerList.Add(dwObj);
                            }
                        }
                        else
                        {
                            /* The DataWriter can be AlreadyDeleted, or it can be from another
                             * Publisher. Its liveliness cannot be modified without the lock
                             * of its factory, so we are safe checking it here since we hold
                             * the lock to this factory. If the DataWriter is from another
                             * publisher, then the result may be PRECONDITION_NOT_MET while
                             * it should have been BAD_PARAMETER, but such a Use Case has
                             * an inherent race-condition anyway, and the result of such
                             * a test is by definition undefined.
                             */
                            if (dwObj.rlReq_isAlive)
                            {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "DataWriter " + dwObj + " unknown to Publisher " + this + ".");
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the Publisher in this context,
                                // so for a deleted datawriter use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "DataWriter " + dwObj + " was already deleted.");
                            }
                        }
                    } else {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report (result, "datawriter is invalid (null), or not of type " +
                                   "DDS::OpenSplice::DataWriter.");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public IDataWriter LookupDataWriter(string topicName)
        {
            bool isAlive;
            DataWriter dw = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    dw = writerList.Find(
                        delegate (DataWriter a_datawriter)
                        {
                            return a_datawriter.Topic.Name.Equals(topicName);
                        }
                    );
                }
            }
            ReportStack.Flush(this, !isAlive);

            return dw;
        }

        public ReturnCode DeleteContainedEntities()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = DDS.ReturnCode.Ok;
                    foreach (DataWriter dw in writerList)
                    {
                        result = dw.deinit();
                        if (result != DDS.ReturnCode.Ok) break;
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        writerList.Clear();
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetQos(PublisherQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start ();
            if (this.rlReq_isAlive)
            {
                result = QosManager.checkQos(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.PublisherQosMarshaler())
                    {
                        result = marshaler.CopyIn(qos);
                        if (result == ReturnCode.Ok)
                        {
                            result = uResultToReturnCode(
                                    User.Publisher.SetQos(rlReq_UserPeer, marshaler.UserPtr));
                            if (result != ReturnCode.Ok)
                            {
                                ReportStack.Report(result, "Could not apply PublisherQos.");
                            }
                        }
                        else
                        {
                            ReportStack.Report(result, "Could not copy PublisherQos.");
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetQos(ref PublisherQos qos)
        {
            IntPtr userQos = IntPtr.Zero;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.Publisher.GetQos(rlReq_UserPeer, ref userQos));
                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.PublisherQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.PublisherQosMarshaler(userQos, true))
                    {
                        marshaler.CopyOut(ref qos);
                    }
                }
                else
                {
                    ReportStack.Report(result, "Could not copy PublisherQos.");
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }


        public ReturnCode SuspendPublications()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = SacsSuperClass.uResultToReturnCode(
                        User.Publisher.Suspend(rlReq_UserPeer));
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode ResumePublications()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = SacsSuperClass.uResultToReturnCode(
                        User.Publisher.Resume(rlReq_UserPeer));
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode BeginCoherentChanges()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = SacsSuperClass.uResultToReturnCode(
                        User.Publisher.CoherentBegin(rlReq_UserPeer));
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode EndCoherentChanges()
        {
            ReportStack.Start();
            ReturnCode result = SacsSuperClass.uResultToReturnCode(
                    User.Publisher.CoherentEnd(rlReq_UserPeer));
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode WaitForAcknowledgments(Duration maxWait)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (QosManager.countErrors(maxWait) == 0)
            {
                if (this.rlReq_isAlive)
                {
                    /*result = uResultToReturnCode(
                            User.Publisher.WaitForAcknowledgments(rlReq_UserPeer, maxWait.DatabaseTime));*/
                    result = DDS.ReturnCode.Unsupported;
                }
                else
                {
                    result = DDS.ReturnCode.AlreadyDeleted;
                }
            }

            return result;
        }

        public IDomainParticipant GetParticipant()
        {
            bool isAlive;
            DomainParticipant participantObj = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    participantObj = participant;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return participantObj;
        }

        public ReturnCode SetDefaultDataWriterQos(DataWriterQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok) {
                        using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
                        {
                            result = marshaler.CopyIn(qos);
                            if (result == ReturnCode.Ok)
                            {
                                marshaler.CopyOut(ref defaultDataWriterQos);
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetDefaultDataWriterQos(ref DataWriterQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
                    {
                        result = marshaler.CopyIn(defaultDataWriterQos);
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

        public ReturnCode CopyFromTopicQos(ref DataWriterQos dataWriterQos, TopicQos topicQos)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            ReportStack.Start ();
            if (topicQos != null)
            {
                if (dataWriterQos == null)
                {
                    GetDefaultDataWriterQos(ref dataWriterQos);
                }
                dataWriterQos.Durability = topicQos.Durability;
                dataWriterQos.Deadline = topicQos.Deadline;
                dataWriterQos.LatencyBudget = topicQos.LatencyBudget;
                dataWriterQos.Liveliness = topicQos.Liveliness;
                dataWriterQos.Reliability = topicQos.Reliability;
                dataWriterQos.DestinationOrder = topicQos.DestinationOrder;
                dataWriterQos.History = topicQos.History;
                dataWriterQos.ResourceLimits = topicQos.ResourceLimits;
                dataWriterQos.TransportPriority = topicQos.TransportPriority;
                dataWriterQos.Lifespan = topicQos.Lifespan;
                dataWriterQos.Ownership = topicQos.Ownership;
                result = DDS.ReturnCode.Ok;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }
    }
}
