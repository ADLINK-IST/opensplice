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
using DDS.OpenSplice.kernelModule;
using DDS.OpenSplice.kernelModuleI;

namespace DDS.OpenSplice
{
    public class DataReader : Entity, IDataReader
    {
        public const SampleStateKind SAMPLE_STATE_FLAGS = (SampleStateKind.NotRead | SampleStateKind.Read);
        public const ViewStateKind VIEW_STATE_FLAGS = (ViewStateKind.NotNew | ViewStateKind.New);
        public const InstanceStateKind INSTANCE_STATE_FLAGS = (InstanceStateKind.Alive | InstanceStateKind.NotAlive);

        private Subscriber subscriber;
        private ITopicDescriptionImpl topic;
        private List<ReadCondition> conditionList = new List<ReadCondition>();

        public DataReader()
        {
        }

        internal virtual ReturnCode init(Subscriber subscriber, DataReaderQos drQos, ITopicDescriptionImpl aTopic, string drName)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            MyDomainId = subscriber.MyDomainId;

            using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler qosMarshaler =
                    new OpenSplice.CustomMarshalers.DataReaderQosMarshaler())
            {
                result = qosMarshaler.CopyIn(drQos);
                if (result == ReturnCode.Ok)
                {
                    using (SequenceStringToCValueArrMarshaler paramsMarshaler = new SequenceStringToCValueArrMarshaler())
                    {
                        string[] _params = aTopic.rlReq_TopicExpressionParameters;
                        result = paramsMarshaler.CopyIn(_params);
                        if (result == ReturnCode.Ok)
                        {
                            IntPtr uReader = User.DataReader.NewBySQL(
                                    subscriber.rlReq_UserPeer,
                                    drName,
                                    aTopic.rlReq_TopicExpression,
                                    paramsMarshaler.UserPtr,
                                    _params==null ? 0 : Convert.ToUInt32(_params.Length),
                                    qosMarshaler.UserPtr);
                            if (uReader != IntPtr.Zero)
                            {
                                result = base.init(uReader);
                            }
                            else
                            {
                                ReportStack.Report(result, "Could not allocate memory.");
                                result = DDS.ReturnCode.OutOfResources;
                            }
                        } else {
                            ReportStack.Report(result, "Could not create DataReader.");
                        }
                    }
                } else {
                    ReportStack.Report(result, "Could not copy DataReaderQos.");
                }
            }
            if (result == ReturnCode.Ok)
            {
                this.subscriber = subscriber;
                this.topic = aTopic;
                aTopic.wlReq_IncrNrUsers();
            }

            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            if (conditionList.Count == 0)
            {
                IDataReaderListener drListener = listener as IDataReaderListener;
                if (drListener != null)
                {
                    this.SetListener(drListener,(DDS.StatusKind)0);
                }
                this.DisableCallbacks();

                result = base.wlReq_deinit();
                if (result == DDS.ReturnCode.Ok)
                {
                    if (this.topic != null) {
                        result = this.topic.DecrNrUsers();
                        if (result == DDS.ReturnCode.Ok)
                        {
                            this.topic = null;
                        }
                        this.subscriber = null;
                    }
                }
            }
            else
            {
                result = DDS.ReturnCode.PreconditionNotMet;
                ReportStack.Report(result, "DataReader " + this + " cannot be deleted since it still contains " +
                            conditionList.Count + " Read/QueryConditions.");
            }
            return result;
        }



        internal List<ReadCondition> rlReq_ConditionList
        {
            get
            {
                return conditionList;
            }
        }

        public static bool SampleStateMaskIsValid(SampleStateKind mask)
        {
            return (((mask == SampleStateKind.Any) || ((mask & ~SAMPLE_STATE_FLAGS) == 0)));
        }

        public static bool ViewStateMaskIsValid(ViewStateKind mask)
        {
            return (((mask == ViewStateKind.Any) || ((mask & ~VIEW_STATE_FLAGS) == 0)));
        }

        public static bool InstanceStateMaskIsValid(InstanceStateKind mask)
        {
            return (((mask == InstanceStateKind.Any) || ((mask & ~INSTANCE_STATE_FLAGS) == 0)));
        }

        public static uint StateMask(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            return ((uint) (sampleStates & SAMPLE_STATE_FLAGS) |
                    ((uint)(viewStates & VIEW_STATE_FLAGS) << 2 ) |
                    ((uint)(instanceStates & INSTANCE_STATE_FLAGS) << 4));
        }

        internal override void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status)
        {
            ReturnCode result;

            IDataReaderListener drListener = listener as IDataReaderListener;
            if(drListener != null)
            {
                DDS.OpenSplice.Common.ReaderStatus readerStatus = status as DDS.OpenSplice.Common.ReaderStatus;
                if ((triggerMask & V_EVENT.DATA_AVAILABLE) == V_EVENT.DATA_AVAILABLE)
                {
                    result = ResetDataAvailableStatus();
                    if (result == DDS.ReturnCode.Ok)
                    {
                        drListener.OnDataAvailable(source as IDataReader);
                    }
                }

                if ((triggerMask & V_EVENT.SAMPLE_REJECTED) == V_EVENT.SAMPLE_REJECTED)
                {
                    drListener.OnSampleRejected(source as IDataReader, readerStatus.SampleRejected);
                }

                if ((triggerMask & V_EVENT.LIVELINESS_CHANGED) == V_EVENT.LIVELINESS_CHANGED)
                {
                    drListener.OnLivelinessChanged(source as IDataReader, readerStatus.LivelinessChanged);
                }

                if ((triggerMask & V_EVENT.SAMPLE_LOST) == V_EVENT.SAMPLE_LOST)
                {
                    drListener.OnSampleLost(source as IDataReader, readerStatus.SampleLost);
                }

                if ((triggerMask & V_EVENT.REQUESTED_DEADLINE_MISSED) == V_EVENT.REQUESTED_DEADLINE_MISSED)
                {
                    drListener.OnRequestedDeadlineMissed(source as IDataReader, readerStatus.DeadlineMissed);
                }

                if ((triggerMask & V_EVENT.REQUESTED_INCOMPATIBLE_QOS) == V_EVENT.REQUESTED_INCOMPATIBLE_QOS)
                {
                    drListener.OnRequestedIncompatibleQos(source as IDataReader, readerStatus.IncompatibleQos);
                }

                if ((triggerMask & V_EVENT.SUBSCRIPTION_MATCHED) == V_EVENT.SUBSCRIPTION_MATCHED)
                {
                    drListener.OnSubscriptionMatched(source as IDataReader, readerStatus.SubscriptionMatch);
                }
            }
        }

        internal static V_RESULT CopyMatchedPublication(IntPtr info, IntPtr arg)
        {
            v_publicationInfo pubInfo = (v_publicationInfo) Marshal.PtrToStructure(info, typeof(v_publicationInfo));
            GCHandle argGCHandle = GCHandle.FromIntPtr(arg);
            List<InstanceHandle> handleList = argGCHandle.Target as List<InstanceHandle>;
            InstanceHandle handle = User.InstanceHandle.FromGID(pubInfo.key);
            handleList.Add(handle);
            argGCHandle.Target = handleList;
            return V_RESULT.OK;
        }

        internal static V_RESULT CopyMatchedPublicationData(IntPtr info, IntPtr arg)
        {
            __PublicationBuiltinTopicData nativeImage =
                    (__PublicationBuiltinTopicData) Marshal.PtrToStructure(info, typeof(__PublicationBuiltinTopicData));
            GCHandle argGCHandle = GCHandle.FromIntPtr(arg);
            PublicationBuiltinTopicData to = argGCHandle.Target as PublicationBuiltinTopicData;
            __PublicationBuiltinTopicDataMarshaler.CopyOut(ref nativeImage, ref to);
            argGCHandle.Target = to;
            return V_RESULT.OK;
        }

        internal static V_RESULT CopyDiscoveredInstanceHandles(IntPtr buf, uint length, IntPtr arg)
        {
            GCHandle argGCHandle = GCHandle.FromIntPtr(arg);
            InstanceHandle[] instanceHandles = argGCHandle.Target as InstanceHandle[];
            long[] handleBuf = new long[length];
            Marshal.Copy(buf, handleBuf, 0, (int) length);

            instanceHandles = new InstanceHandle[length];
            for (uint i = 0; i < length; i++)
            {
                instanceHandles[i] = handleBuf[i];
            }
            argGCHandle.Target = instanceHandles;
            return V_RESULT.OK;
        }

        internal ReturnCode nlReq_getInstanceHandles(ref InstanceHandle[] instanceHandles)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    GCHandle listGCHandle = GCHandle.Alloc(instanceHandles, GCHandleType.Normal);
                    result = uResultToReturnCode(
                            User.DataReader.GetInstanceHandles(rlReq_UserPeer, CopyDiscoveredInstanceHandles, GCHandle.ToIntPtr(listGCHandle)));
                    instanceHandles = listGCHandle.Target as InstanceHandle[];
                    listGCHandle.Free();
                }
            }
            return result;
        }

        public IDataReaderListener Listener
        {
            get
            {
                bool isAlive;
                IDataReaderListener drListener = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        drListener = rlReq_Listener as IDataReaderListener;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return drListener;
            }
        }

        public ReturnCode SetListener(IDataReaderListener listener, StatusKind mask)
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


        public IReadCondition CreateReadCondition()
        {
            return CreateReadCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public IReadCondition CreateReadCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReadCondition readCondition = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            IntPtr uQuery;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (SampleStateMaskIsValid(sampleStates) &&
                        ViewStateMaskIsValid(viewStates) &&
                        InstanceStateMaskIsValid(instanceStates))
                    {
                        uint stateMask = StateMask(sampleStates, viewStates, instanceStates);
                        uQuery = User.Query.New(rlReq_UserPeer, null, "1=1", IntPtr.Zero, 0, stateMask);
                        if (uQuery != IntPtr.Zero)
                        {
                            readCondition = new ReadCondition(this, sampleStates, viewStates, instanceStates);
                            result = readCondition.init(uQuery);
                            if (result == ReturnCode.Ok)
                            {
                                conditionList.Add(readCondition);
                            }
                            else
                            {
                                ReportStack.Report(result, "Could not create ReadCondition.");
                                readCondition = null;
                            }
                        }
                        else
                        {
                            result = DDS.ReturnCode.Error;
                            ReportStack.Report(result, "Unable to create ReadCondition in kernel.");
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "One mor more of your SampleState/ViewState/InstanceState masks are invalid.");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return readCondition;
        }

        public IQueryCondition CreateQueryCondition(
                string queryExpression,
                params string[] queryParameters)
        {
            return CreateQueryCondition(
                    SampleStateKind.Any,
                    ViewStateKind.Any,
                    InstanceStateKind.Any,
                    queryExpression, queryParameters);
        }

        public IQueryCondition CreateQueryCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates,
                string queryExpression,
                params string[] queryParameters)
        {
            QueryCondition queryCondition = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            IntPtr uQuery;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (SequenceStringToArrMarshaler marshaler = new SequenceStringToArrMarshaler())
                    {
                        result = marshaler.CopyIn(queryParameters);
                        if (result == ReturnCode.Ok)
                        {
                            if (SampleStateMaskIsValid(sampleStates) &&
                                ViewStateMaskIsValid(viewStates) &&
                                InstanceStateMaskIsValid(instanceStates))
                            {
                                uint stateMask = StateMask(sampleStates, viewStates, instanceStates);
                                uQuery = User.Query.New(
                                        rlReq_UserPeer,
                                        null,
                                        queryExpression,
                                        marshaler.UserPtr,
                                        queryParameters != null ? (uint) queryParameters.Length : 0,
                                        stateMask);
                                if (uQuery != IntPtr.Zero)
                                {
                                    queryCondition = new QueryCondition(
                                            this,
                                            sampleStates,
                                            viewStates,
                                            instanceStates,
                                            queryExpression,
                                            queryParameters);
                                    result = queryCondition.init(uQuery);
                                    if (result == ReturnCode.Ok)
                                    {
                                        conditionList.Add(queryCondition);
                                    }
                                    else
                                    {
                                        ReportStack.Report(result, "Could not create QueryCondition.");
                                        queryCondition = null;
                                    }
                                }
                                else
                                {
                                    result = DDS.ReturnCode.Error;
                                    ReportStack.Report(result, "Unable to create QueryCondition in kernel.");
                                }
                            }
                            else
                            {
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "One mor more of your SampleState/ViewState/InstanceState masks are invalid.");
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return queryCondition;
        }

        public ReturnCode DeleteReadCondition(IReadCondition condition)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    ReadCondition condObj = condition as ReadCondition;
                    if (condObj != null)
                    {
                        if (conditionList.Remove(condObj))
                        {
                            result = condObj.deinit();
                            if (result != DDS.ReturnCode.Ok)
                            {
                                conditionList.Add(condObj);
                            }
                        }
                        else
                        {
                            /* The ReadCondition can be AlreadyDeleted, or it can be from another
                             * DataReader. Its liveliness cannot be modified without the lock
                             * of its factory, so we are safe checking it here since we hold
                             * the lock to this factory. If the ReadCondition is from another
                             * DataReader, then the result may be PRECONDITION_NOT_MET while
                             * it should have been BAD_PARAMETER, but such a Use Case has
                             * an inherent race-condition anyway, and the result of such
                             * a test is by definition undefined.
                             */
                            if (condObj.rlReq_isAlive)
                            {
                                result = ReturnCode.PreconditionNotMet;
                                ReportStack.Report(result, "ReadCondition " + condObj + " unknown to DataReader " + this + ".");
                            }
                            else
                            {
                                // ALREADY_DELETED may only apply to the DataReader in this context,
                                // so for a deleted readcondition use BAD_PARAMETER instead.
                                result = DDS.ReturnCode.BadParameter;
                                ReportStack.Report(result, "ReadCondition " + condObj + " was already deleted.");
                            }
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "ReadCondition " + condObj + " is of unknown type.");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
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
                    foreach (ReadCondition rc in conditionList)
                    {
                        result = rc.deinit();
                        if (result != DDS.ReturnCode.Ok) break;
                    }

                    if (result == DDS.ReturnCode.Ok)
                    {
                        conditionList.Clear();
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode SetQos(DataReaderQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();

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
                            result = uResultToReturnCode(
                                    User.DataReader.SetQos(rlReq_UserPeer, marshaler.UserPtr));
                            if (result != ReturnCode.Ok)
                            {
                                ReportStack.Report(result, "Could not apply DataReaderQos.");
                            }
                        }
                        else
                        {
                            ReportStack.Report(result, "Could not copy DataReaderQos.");
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetQos(ref DataReaderQos qos)
        {

            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            IntPtr userQos = IntPtr.Zero;

            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.DataReader.GetQos(rlReq_UserPeer, ref userQos));
                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.DataReaderQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataReaderQosMarshaler(userQos, true))
                    {
                        marshaler.CopyOut(ref qos);
                    }
                }
                else
                {
                    ReportStack.Report(result, "Could not copy DataReaderQos.");
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ITopicDescription GetTopicDescription()
        {
            bool isAlive;
            ITopicDescription td = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    td = topic;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return td;
        }

        public ISubscriber Subscriber
        {
            get
            {
                bool isAlive;
                Subscriber subscriberObj = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        subscriberObj = subscriber;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return subscriberObj;
            }
        }

        public ReturnCode GetSampleRejectedStatus(
                ref SampleRejectedStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new SampleRejectedStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetSampleRejectedStatus(
                                rlReq_UserPeer, 1, SampleRejectedStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as SampleRejectedStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetLivelinessChangedStatus(
                ref LivelinessChangedStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new LivelinessChangedStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetLivelinessChangedStatus(
                                rlReq_UserPeer, 1, LivelinessChangedStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as LivelinessChangedStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetRequestedDeadlineMissedStatus(
                ref RequestedDeadlineMissedStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new RequestedDeadlineMissedStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetDeadlineMissedStatus(
                                rlReq_UserPeer, 1, RequestedDeadlineMissedStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as RequestedDeadlineMissedStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetRequestedIncompatibleQosStatus(
                ref RequestedIncompatibleQosStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new RequestedIncompatibleQosStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetIncompatibleQosStatus(
                                rlReq_UserPeer, 1, RequestedIncompatibleQosStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as RequestedIncompatibleQosStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetSubscriptionMatchedStatus(
                ref SubscriptionMatchedStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new SubscriptionMatchedStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetSubscriptionMatchStatus(
                                rlReq_UserPeer, 1, SubscriptionMatchedStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as SubscriptionMatchedStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetSampleLostStatus(ref SampleLostStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new SampleLostStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetSampleLostStatus(
                                rlReq_UserPeer, 1, SampleLostStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as SampleLostStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode WaitForHistoricalData(Duration maxWait)
        {
            ReturnCode result;

            ReportStack.Start();
            if (QosManager.countErrors(maxWait) > 0)
            {
                result = DDS.ReturnCode.BadParameter;
            }
            else if (!this.IsEnabled())
            {
                result = DDS.ReturnCode.NotEnabled;
            }
            else
            {
                if (this.rlReq_isAlive)
                {
                    result = uResultToReturnCode(
                            User.DataReader.WaitForHistoricalData(rlReq_UserPeer, maxWait.OsDuration));
                }
                else
                {
                    result = DDS.ReturnCode.AlreadyDeleted;
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode WaitForHistoricalDataWithCondition(
                string filterExpression,
                string[] filterParameters,
                Time minSourceTimeStamp,
                Time maxSourceTimeStamp,
                ResourceLimitsQosPolicy resourceLimits,
                Duration maxWait)
        {
            ReturnCode result;

            ReportStack.Start();
            if (QosManager.countErrors(minSourceTimeStamp) > 0 ||
                QosManager.countErrors(maxSourceTimeStamp) > 0 ||
                QosManager.countErrors(resourceLimits) > 0 ||
                QosManager.countErrors(maxWait) > 0)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "Invalid function parameter(s) passed.");
            }
            else
            {
                if (this.rlReq_isAlive)
                {
                    using (SequenceStringToArrMarshaler marshaler = new SequenceStringToArrMarshaler())
                    {
                        result = marshaler.CopyIn(filterParameters);
                        if (result == ReturnCode.Ok)
                        {
                            uint length = filterParameters == null ? 0 : (uint)filterParameters.Length;
                            result = uResultToReturnCode(
                                    User.DataReader.WaitForHistoricalDataWithCondition(
                                            rlReq_UserPeer,
                                            filterExpression,
                                            marshaler.UserPtr,
                                            length,
                                            minSourceTimeStamp.OsTimeW,
                                            maxSourceTimeStamp.OsTimeW,
                                            resourceLimits.MaxSamples,
                                            resourceLimits.MaxInstances,
                                            resourceLimits.MaxSamplesPerInstance,
                                            maxWait.OsDuration));
                        }
                    }
                }
                else
                {
                    result = DDS.ReturnCode.AlreadyDeleted;
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetMatchedPublications(ref InstanceHandle[] publicationHandles)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                List<InstanceHandle> handleList = new List<InstanceHandle>();
                GCHandle listGCHandle = GCHandle.Alloc(handleList, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Reader.GetMatchedPublications(
                                rlReq_UserPeer,
                                CopyMatchedPublication,
                                GCHandle.ToIntPtr(listGCHandle)));
                handleList = listGCHandle.Target as List<InstanceHandle>;
                publicationHandles = handleList.ToArray();
                listGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetMatchedPublicationData(
                ref PublicationBuiltinTopicData publicationData,
                InstanceHandle publicationHandle)
        {
            ReturnCode result;

            ReportStack.Start();
            if (publicationHandle != InstanceHandle.Nil)
            {
                if (this.rlReq_isAlive)
                {
                    GCHandle dataGCHandle = GCHandle.Alloc(publicationData, GCHandleType.Normal);
                    result = uResultToReturnCode(
                            User.Reader.GetMatchedPublicationData(
                                    rlReq_UserPeer,
                                    publicationHandle,
                                    CopyMatchedPublicationData,
                                    GCHandle.ToIntPtr(dataGCHandle)));
                    publicationData = dataGCHandle.Target as PublicationBuiltinTopicData;
                    dataGCHandle.Free();
                }
                else
                {
                    result = DDS.ReturnCode.AlreadyDeleted;
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "publication_handle 'HANDLE_NIL' is invalid.");
            }

            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }
    }
}
