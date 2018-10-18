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
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModule;
using DDS.OpenSplice.kernelModuleI;

namespace DDS.OpenSplice
{
    public class DataWriter : Entity, IDataWriter
    {
        private Publisher publisher;
        private Topic topic;

        internal ReturnCode init(Publisher publisher, DataWriterQos dwQos, Topic aTopic, string dwName)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            MyDomainId = publisher.MyDomainId;

            using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
            {
                result = marshaler.CopyIn(dwQos);
                if (result == ReturnCode.Ok)
                {
                    IntPtr uWriter = User.Writer.New(publisher.rlReq_UserPeer, dwName, aTopic.rlReq_UserPeer, marshaler.UserPtr);
                    if (uWriter != IntPtr.Zero)
                    {
                        result = base.init(uWriter);
                    }
                    else
                    {
                        ReportStack.Report(result, "Could not create DataWriter.");
                        result = DDS.ReturnCode.OutOfResources;
                    }
                } else {
                    ReportStack.Report(result, "Could not copy DataWriterQos.");
                }

            }
            if (result == ReturnCode.Ok)
            {
                this.publisher = publisher;
                this.topic = aTopic;
                (aTopic as ITopicDescriptionImpl).wlReq_IncrNrUsers();
            }

            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            IDataWriterListener dwListener = listener as IDataWriterListener;
            if (dwListener != null)
            {
                this.SetListener(dwListener,(DDS.StatusKind)0);
            }
            this.DisableCallbacks();
            result = base.wlReq_deinit();
            if (result == DDS.ReturnCode.Ok)
            {
                if (this.topic != null) {
                    result = (this.topic as ITopicDescriptionImpl).DecrNrUsers();
                    if (result == DDS.ReturnCode.Ok)
                    {
                        this.topic = null;
                    }
                }
                this.publisher = null;
            }
            return result;
        }

        internal override void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status)
        {
            IDataWriterListener dwListener = listener as IDataWriterListener;
            if(dwListener != null)
            {
                DDS.OpenSplice.Common.WriterStatus writerStatus = status as DDS.OpenSplice.Common.WriterStatus;

	            if ((triggerMask & V_EVENT.LIVELINESS_LOST) == V_EVENT.LIVELINESS_LOST)
	            {
	                dwListener.OnLivelinessLost(source as IDataWriter, writerStatus.LivelinessLost);
	            }

	            if ((triggerMask & V_EVENT.OFFERED_DEADLINE_MISSED) == V_EVENT.OFFERED_DEADLINE_MISSED)
	            {
	                dwListener.OnOfferedDeadlineMissed(source as IDataWriter, writerStatus.DeadlineMissed);
	            }

	            if ((triggerMask & V_EVENT.OFFERED_INCOMPATIBLE_QOS) == V_EVENT.OFFERED_INCOMPATIBLE_QOS)
	            {
	                dwListener.OnOfferedIncompatibleQos(source as IDataWriter, writerStatus.IncompatibleQos);
	            }

	            if ((triggerMask & V_EVENT.PUBLICATION_MATCHED) == V_EVENT.PUBLICATION_MATCHED)
	            {
	                dwListener.OnPublicationMatched(source as IDataWriter, writerStatus.PublicationMatch);
	            }
            }
        }

        internal static V_RESULT CopyMatchedSubscription(IntPtr info, IntPtr arg)
        {
            v_subscriptionInfo subInfo = (v_subscriptionInfo) Marshal.PtrToStructure(info, typeof(v_subscriptionInfo));
            GCHandle argGCHandle = GCHandle.FromIntPtr(arg);
            List<InstanceHandle> handleList = argGCHandle.Target as List<InstanceHandle>;
            InstanceHandle handle = User.InstanceHandle.FromGID(subInfo.key);
            handleList.Add(handle);
            argGCHandle.Target = handleList;
            return V_RESULT.OK;
        }

        internal static V_RESULT CopyMatchedSubscriptionData(IntPtr info, IntPtr arg)
        {
            __SubscriptionBuiltinTopicData nativeImage =
                    (__SubscriptionBuiltinTopicData) Marshal.PtrToStructure(info, typeof(__SubscriptionBuiltinTopicData));
            GCHandle argGCHandle = GCHandle.FromIntPtr(arg);
            SubscriptionBuiltinTopicData to = argGCHandle.Target as SubscriptionBuiltinTopicData;
            __SubscriptionBuiltinTopicDataMarshaler.CopyOut(ref nativeImage, ref to);
            argGCHandle.Target = to;
            return V_RESULT.OK;
        }

        public IDataWriterListener Listener
        {
            get
            {
                bool isAlive;
                IDataWriterListener dwListener = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        dwListener = rlReq_Listener as IDataWriterListener;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return dwListener;
            }
        }

        public ReturnCode SetListener(IDataWriterListener listener, StatusKind mask)
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

        public ReturnCode SetQos(DataWriterQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = QosManager.checkQos(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.DataWriterQosMarshaler())
                    {
                        result = marshaler.CopyIn(qos);
                        if (result == ReturnCode.Ok)
                        {
                            result = uResultToReturnCode(
                                    User.Writer.SetQos(rlReq_UserPeer, marshaler.UserPtr));
                        }
                        else
                        {
                            ReportStack.Report(result, "Could not apply DataWriterQos.");
                        }
                    }
                }
            }

            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode GetQos(ref DataWriterQos qos)
        {
            IntPtr userQos = IntPtr.Zero;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                    User.Writer.GetQos(rlReq_UserPeer, ref userQos));
                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.DataWriterQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.DataWriterQosMarshaler(userQos, true))
                    {
                        marshaler.CopyOut(ref qos);
                    }
                }
                else
                {
                    ReportStack.Report(result, "Could not copy DataWriterQos.");
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ITopic Topic
        {
            get
            {
                bool isAlive;
                Topic topicObj = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        topicObj = topic;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return topicObj;
            }
        }

        public IPublisher Publisher
        {
            get
            {
                bool isAlive;
                Publisher publisherObj = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        publisherObj = publisher;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return publisherObj;
            }
        }

        public ReturnCode WaitForAcknowledgments(Duration maxWait)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (QosManager.countErrors(maxWait) > 0)
            {
                result = DDS.ReturnCode.BadParameter;
            }
            else
            {
                if (this.rlReq_isAlive)
                {
                    result = uResultToReturnCode(
                            User.Writer.WaitForAcknowledgments(rlReq_UserPeer, maxWait.OsDuration));
                }
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok) && (result != ReturnCode.Timeout));
            return result;
        }

        public ReturnCode GetLivelinessLostStatus(
                ref LivelinessLostStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
            if (status == null) status = new LivelinessLostStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.GetLivelinessLostStatus(
                                rlReq_UserPeer, 1, LivelinessLostStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as LivelinessLostStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetOfferedDeadlineMissedStatus(
                ref OfferedDeadlineMissedStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new OfferedDeadlineMissedStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.GetDeadlineMissedStatus(
                                rlReq_UserPeer, 1, OfferedDeadlineMissedStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as OfferedDeadlineMissedStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetOfferedIncompatibleQosStatus(
                ref OfferedIncompatibleQosStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new OfferedIncompatibleQosStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.GetIncompatibleQosStatus(
                                rlReq_UserPeer, 1, OfferedIncompatibleQosStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as OfferedIncompatibleQosStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetPublicationMatchedStatus(
                ref PublicationMatchedStatus status)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new PublicationMatchedStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.GetPublicationMatchStatus(
                                rlReq_UserPeer, 1, PublicationMatchedStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as PublicationMatchedStatus;
                statusGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode AssertLiveliness()
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.Writer.AssertLiveliness(rlReq_UserPeer));
                ReportStack.Flush(this, result != ReturnCode.Ok);
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            return result;
        }

        public ReturnCode GetMatchedSubscriptions(ref InstanceHandle[] subscriptionHandles)
        {
            ReturnCode result;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
            List<InstanceHandle> handleList = new List<InstanceHandle>();
            GCHandle listGCHandle = GCHandle.Alloc(handleList, GCHandleType.Normal);
            result = uResultToReturnCode(
                    User.Writer.GetMatchedSubscriptions(
                            rlReq_UserPeer,
                            CopyMatchedSubscription,
                            GCHandle.ToIntPtr(listGCHandle)));
            handleList = listGCHandle.Target as List<InstanceHandle>;
            subscriptionHandles = handleList.ToArray();
            listGCHandle.Free();
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetMatchedSubscriptionData(
                ref SubscriptionBuiltinTopicData subscriptionData,
                InstanceHandle subscriptionHandle)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (subscriptionHandle != InstanceHandle.Nil)
                {
                    GCHandle dataGCHandle = GCHandle.Alloc(subscriptionData, GCHandleType.Normal);
                    result = uResultToReturnCode(
                            User.Writer.GetMatchedSubscriptionData(
                                    rlReq_UserPeer,
                                    subscriptionHandle,
                                    CopyMatchedSubscriptionData,
                                    GCHandle.ToIntPtr(dataGCHandle)));
                    subscriptionData = dataGCHandle.Target as SubscriptionBuiltinTopicData;
                    dataGCHandle.Free();
                }
                else
                {
                    result = ReturnCode.BadParameter;
                    ReportStack.Report(result, "subscriptionHandle = DDS.InstanceHandle.Nil.");
                }
            }
            else
            {
                result = DDS.ReturnCode.AlreadyDeleted;
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }
    }
}
