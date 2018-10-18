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

    internal class Subscriber : Entity, ISubscriber
    {
        private DomainParticipant participant;
        private DataReaderQos defaultDataReaderQos = new DataReaderQos();
        private List<DataReader> readerList = new List<DataReader>();

        internal Subscriber()
        {
            DDS.ReturnCode result;

            using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler =
                new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultDataReaderQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref defaultDataReaderQos);
                }
            }
        }

        internal ReturnCode init(DomainParticipant participant, string name, SubscriberQos qos)
        {
            ReturnCode result;

            MyDomainId = participant.MyDomainId;

            using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    IntPtr uSubscriber = User.Subscriber.New(participant.rlReq_UserPeer, name, marshaler.UserPtr);
                    if (uSubscriber != IntPtr.Zero)
                    {
                        result = base.init(uSubscriber);
                    }
                    else
                    {
                        result = DDS.ReturnCode.Error;
                    }
                }
            }

            if (result == ReturnCode.Ok)
            {
                this.participant = participant;
            }

            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            if (readerList.Count == 0)
            {
                ISubscriberListener subListener = listener as ISubscriberListener;
                if (subListener != null)
                {
                    this.SetListener(subListener,(DDS.StatusKind)0);
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
                ReportStack.Report(result, "Subscriber " + this + " cannot be deleted since it still contains " +
                            readerList.Count + " DataReaders.");
            }
            return result;
        }

        private bool rlReq_AutoEnableCreatedEntities
        {
            get {
                SubscriberQos sQos = new SubscriberQos();
                IntPtr userQos = IntPtr.Zero;
                bool autoEnable = false;

                ReturnCode result = uResultToReturnCode(
                        User.Subscriber.GetQos(rlReq_UserPeer, ref userQos));

                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.SubscriberQosMarshaler(userQos, false))
                    {
                        marshaler.CopyOut(ref sQos);
                        autoEnable = sQos.EntityFactory.AutoenableCreatedEntities;
                    }
                }
                User.SubscriberQos.Free(userQos);

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
                    contained = readerList.Exists(
                        delegate (DataReader a_datareader)
                        {
                            return a_datareader.InstanceHandle == handle;
                        }
                    );
                }
            }

            return contained;
        }

        internal override void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status)
        {
            ReturnCode result;
            ISubscriberListener subListener = listener as ISubscriberListener;
            if (subListener != null)
            {
                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;

                if ((triggerMask & V_EVENT.ON_DATA_ON_READERS) == V_EVENT.ON_DATA_ON_READERS)
                {
                    result = ResetDataAvailableStatus();
                    if (result == DDS.ReturnCode.Ok)
                    {
                        subListener.OnDataOnReaders(source as ISubscriber);
                    }
                }
                else
                {
                    if ((triggerMask & V_EVENT.DATA_AVAILABLE) == V_EVENT.DATA_AVAILABLE)
                    {
                        result = ResetDataAvailableStatus();
                        if (result == DDS.ReturnCode.Ok)
                        {
                            subListener.OnDataAvailable(source as IDataReader);
                        }
                    }
                }

                if ((triggerMask & V_EVENT.SAMPLE_REJECTED) == V_EVENT.SAMPLE_REJECTED)
                {
                    subListener.OnSampleRejected(source as IDataReader, readerStatus.SampleRejected);
                }

                if ((triggerMask & V_EVENT.LIVELINESS_CHANGED) == V_EVENT.LIVELINESS_CHANGED)
                {
                    subListener.OnLivelinessChanged(source as IDataReader, readerStatus.LivelinessChanged);
                }

                if ((triggerMask & V_EVENT.SAMPLE_LOST) == V_EVENT.SAMPLE_LOST)
                {
                    subListener.OnSampleLost(source as IDataReader, readerStatus.SampleLost);
                }

                if ((triggerMask & V_EVENT.REQUESTED_DEADLINE_MISSED) == V_EVENT.REQUESTED_DEADLINE_MISSED)
                {
                    subListener.OnRequestedDeadlineMissed(source as IDataReader, readerStatus.DeadlineMissed);
                }

                if ((triggerMask & V_EVENT.REQUESTED_INCOMPATIBLE_QOS) == V_EVENT.REQUESTED_INCOMPATIBLE_QOS)
                {
                    subListener.OnRequestedIncompatibleQos(source as IDataReader, readerStatus.IncompatibleQos);
                }

                if ((triggerMask & V_EVENT.SUBSCRIPTION_MATCHED) == V_EVENT.SUBSCRIPTION_MATCHED)
                {
                    subListener.OnSubscriptionMatched(source as IDataReader, readerStatus.SubscriptionMatch);
                }
            }
        }

        public ISubscriberListener Listener
        {
            get
            {
                bool isAlive;
                ISubscriberListener subListener = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        subListener = rlReq_Listener as ISubscriberListener;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return subListener;
            }
        }

        public ReturnCode SetListener(ISubscriberListener listener, StatusKind mask)
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

        public IDataReader CreateDataReader(ITopicDescription topic)
        {
            return CreateDataReader(topic, defaultDataReaderQos, null, 0);
        }

        public IDataReader CreateDataReader(
                ITopicDescription topic,
                IDataReaderListener listener,
                StatusKind mask)
        {
            return CreateDataReader(topic, defaultDataReaderQos, listener, mask);
        }

        public IDataReader CreateDataReader(ITopicDescription topic, DataReaderQos qos)
        {
            return CreateDataReader(topic, qos, null, 0);
        }

        public IDataReader CreateDataReader(
                ITopicDescription topic,
                DataReaderQos qos,
                IDataReaderListener listener,
                StatusKind mask)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            DataReader dataReader = null;
            ITopicDescriptionImpl topicObj = topic as ITopicDescriptionImpl;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (topicObj == null)
                    {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "TopicDescription is invalid (null), or not of type " +
                                                    "DDS::OpenSplice::TopicDescription.");
                    }
                    else
                    {
                        result = QosManager.checkQos(qos);
                    }

                    if (result == DDS.ReturnCode.Ok) {
                        lock(topicObj)
                        {
                            if ((topicObj as SacsSuperClass).rlReq_isAlive)
                            {
                                TypeSupport ts = topicObj.rlReq_TypeSupport;
                                if (ts != null)
                                {
                                    DatabaseMarshaler marshaler = DatabaseMarshaler.GetMarshaler(participant, ts.TypeSpec);
                                    dataReader = ts.CreateDataReader(marshaler);
                                    result = dataReader.init(this, qos, topicObj, "reader <" + topicObj.Name+">" );
                                    if (result == ReturnCode.Ok)
                                    {
                                        dataReader.wlReq_ListenerDispatcher = this.wlReq_ListenerDispatcher;
                                        result = dataReader.SetListener(listener, mask);
                                    }
                                    else
                                    {
                                        dataReader = null;
                                    }

                                    if (result == DDS.ReturnCode.Ok)
                                    {
                                        readerList.Add(dataReader);
                                        if (rlReq_AutoEnableCreatedEntities && this.IsEnabled())
                                        {
                                            result = dataReader.Enable();
                                        }
                                    }
                                }
                                else
                                {
                                    ReportStack.Report(DDS.ReturnCode.PreconditionNotMet, "Were not able to locate TypeSupport belonging to topic " + topic.Name + ".");
                                }
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the Subscriber in this context,
                                // so for a deleted TopicDescription use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                            }
                        }
                        if (result != ReturnCode.Ok && dataReader != null)
                        {
                            // Ignore result because we prefer the original error.
                            DeleteDataReader(dataReader);
                            dataReader = null;
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return dataReader;
        }

        public ReturnCode DeleteDataReader(IDataReader dataReader)
        {
            DataReader drObj = dataReader as DataReader;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (drObj != null)
                    {
                        if (readerList.Remove(drObj))
                        {
                            result = drObj.deinit();
                            if (result != ReturnCode.Ok)
                            {
                                readerList.Add(drObj);
                            }
                        }
                        else
                        {
                            /* The DataReader can be AlreadyDeleted, or it can be from another
                             * Subscriber. Its liveliness cannot be modified without the lock
                             * of its factory, so we are safe checking it here since we hold
                             * the lock to this factory. If the DataReader is from another
                             * Subscriber, then the result may be PRECONDITION_NOT_MET while
                             * it should have been BAD_PARAMETER, but such a Use Case has
                             * an inherent race-condition anyway, and the result of such
                             * a test is by definition undefined.
                             */
                            if (drObj.rlReq_isAlive)
                            {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "DataReader " + drObj + " unknown to Subscriber " + this + ".");
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the Subscriber in this context,
                                // so for a deleted datareader use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "DataReader " + drObj + " was already deleted.");
                            }
                        }
                    } else {
                        result = ReturnCode.BadParameter;
                        ReportStack.Report (result, "datareader is invalid (null), or not of type " +
                                   "DDS::OpenSplice::DataReader.");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public IDataReader LookupDataReader(string topicName)
        {
            bool isAlive;
            DataReader dr = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    dr = readerList.Find(
                        delegate (DataReader a_datareader)
                        {
                            return a_datareader.GetTopicDescription().Name.Equals(topicName);
                        }
                    );
                }
            }
            ReportStack.Flush(this, !isAlive);
            return dr;
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
                    foreach (DataReader dr in readerList)
                    {
                        result = dr.DeleteContainedEntities();
                        if (result == DDS.ReturnCode.Ok)
                        {
                            result = dr.deinit();
                        }
                        if (result != DDS.ReturnCode.Ok)
                        {
                            break;
                        }
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        readerList.Clear();
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetDataReaders(ref IDataReader[] readers)
        {
            return GetDataReaders(ref readers, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode GetDataReaders(
                ref IDataReader[] readers,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            if (this.rlReq_isAlive) {
                IntPtr iterp = IntPtr.Zero;
                uint mask = DataReader.StateMask(sampleStates, viewStates, instanceStates);

                result = uResultToReturnCode(User.Subscriber.GetDataReaders(this.rlReq_UserPeer, mask, ref iterp));
                if (result == ReturnCode.Ok) {
                    int length = Database.c.iterLength (iterp);
                    if (readers == null || readers.Length != length) {
                        readers = new IDataReader[length];
                    }
                    for (int i = 0; i < length; i++) {
                        IntPtr ureader = Database.c.iterTakeFirst (iterp);

                        readers [i] = SacsSuperClass.fromUserData (ureader) as IDataReader;
                    }
                    Database.c.iterFree(iterp);
                }
            }

            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode NotifyDataReaders()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    foreach (DataReader dr in readerList)
                    {
                        if (dr != null)
                        {
                            if ((dr.StatusChanges & StatusKind.DataAvailable) != 0)
                            {
                                DDS.IDataReaderListener readerListener = dr.Listener;
                                if (readerListener != null) {
                                    readerListener.OnDataAvailable(dr);
                                }
                            }
                        }
                        else
                        {
                            result = DDS.ReturnCode.Error;
                            break;
                        }
                    }
                }
            }
            return result;
        }

        public ReturnCode SetQos(SubscriberQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = QosManager.checkQos(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.SubscriberQosMarshaler())
                    {
                        result = marshaler.CopyIn(qos);
                        if (result == ReturnCode.Ok)
                        {
                            result = uResultToReturnCode(
                                    User.Subscriber.SetQos(rlReq_UserPeer, marshaler.UserPtr));
                            if (result != ReturnCode.Ok)
                            {
                                ReportStack.Report(result, "Could not apply SubscriberQos.");
                            }
                        }
                        else
                        {
                            ReportStack.Report(result, "Could not copy SubscriberQos.");
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetQos(ref SubscriberQos qos)
        {
            IntPtr userQos = IntPtr.Zero;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.Subscriber.GetQos(rlReq_UserPeer, ref userQos));
                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.SubscriberQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.SubscriberQosMarshaler(userQos, true))
                    {
                        marshaler.CopyOut(ref qos);
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode BeginAccess()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(User.Subscriber.BeginAccess(rlReq_UserPeer));
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode EndAccess()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(User.Subscriber.EndAccess(rlReq_UserPeer));
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public IDomainParticipant Participant
        {
            get
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
        }

        public ReturnCode SetDefaultDataReaderQos(DataReaderQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    result = QosManager.checkQos(qos);
                    if (result == DDS.ReturnCode.Ok)
                    {
                        using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
                        {
                            result = marshaler.CopyIn(qos);
                            if (result == ReturnCode.Ok)
                            {
                                marshaler.CopyOut(ref defaultDataReaderQos);
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetDefaultDataReaderQos(ref DataReaderQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
                    {
                        result = marshaler.CopyIn(defaultDataReaderQos);
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

        public ReturnCode CopyFromTopicQos(ref DataReaderQos dataReaderQos, TopicQos topicQos)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (topicQos != null)
            {
                if (dataReaderQos == null)
                {
                    GetDefaultDataReaderQos(ref dataReaderQos);
                }
                dataReaderQos.Durability = topicQos.Durability;
                dataReaderQos.Deadline = topicQos.Deadline;
                dataReaderQos.LatencyBudget = topicQos.LatencyBudget;
                dataReaderQos.Liveliness = topicQos.Liveliness;
                dataReaderQos.Reliability = topicQos.Reliability;
                dataReaderQos.DestinationOrder = topicQos.DestinationOrder;
                dataReaderQos.History = topicQos.History;
                dataReaderQos.ResourceLimits = topicQos.ResourceLimits;
                dataReaderQos.Ownership = topicQos.Ownership;
                result = DDS.ReturnCode.Ok;
            }
            return result;
        }
    }
}
