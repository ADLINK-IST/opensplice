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
using System.Collections.Generic;
using System.Diagnostics;
using DDS;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.Database;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModuleI;

namespace DDS.OpenSplice
{
    internal class Topic : Entity, ITopic, ITopicDescriptionImpl
    {
//        private readonly TopicListenerHelper listenerHelper;
        private DomainParticipant domainParticipant;
        private TypeSupport typeSupport;
        private string topicName;
        private string typeName;
        private string topicExpression;
        private uint nrUsers = 0;

        internal Topic()
        {
        }

        internal ReturnCode init (
                IntPtr uTopic,
                DomainParticipant participant,
                string topicName,
                string typeName,
                TypeSupport ts)
        {
            MyDomainId = participant.MyDomainId;

            ReturnCode result = base.init (uTopic);

            if (result == DDS.ReturnCode.Ok) {
                this.domainParticipant = participant;
                this.topicName = topicName;
                this.typeName = typeName;
                this.typeSupport = ts;
                this.topicExpression = "select * from " + topicName;
            } else {
                ReportStack.Report (result, "Could not create Topic" + topicName + ".");
            }


            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            if (nrUsers > 0)
            {
                result = DDS.ReturnCode.PreconditionNotMet;
                ReportStack.Report(result, "Topic \"" + topicName + "\" still in use by " + nrUsers + " Writers/Readers.");
            }
            else
            {
                ITopicListener tpListener = listener as ITopicListener;
                if (tpListener != null)
                {
                    this.SetListener(tpListener,(DDS.StatusKind)0);
                }
                this.DisableCallbacks();
                result = base.wlReq_deinit();
                if (result == DDS.ReturnCode.Ok)
                {
                    this.domainParticipant = null;
                    this.topicName = null;
	                this.typeName = null;
	                this.typeSupport = null;
	                this.topicExpression = null;
                }
            }
            return result;
        }

        string ITopicDescriptionImpl.rlReq_TopicExpression
        {
            get
            {
                return topicExpression;
            }
        }

        string[] ITopicDescriptionImpl.rlReq_TopicExpressionParameters
        {
            get
            {
                return null;
            }
        }

        void ITopicDescriptionImpl.wlReq_IncrNrUsers()
        {
            nrUsers++;
        }

        ReturnCode ITopicDescriptionImpl.DecrNrUsers()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    nrUsers--;
                    result = DDS.ReturnCode.Ok;
                }
            }
            return result;
        }

        TypeSupport ITopicDescriptionImpl.rlReq_TypeSupport
        {
            get
            {
                return typeSupport;
            }
            set
            {
                typeSupport = value;
            }
        }

        internal override void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status)
        {
            ITopicListener tpListener = listener as ITopicListener;
            if (tpListener != null)
            {
                DDS.OpenSplice.Common.TopicStatus topicStatus = status as DDS.OpenSplice.Common.TopicStatus;

	            if ((triggerMask & V_EVENT.INCONSISTENT_TOPIC) == V_EVENT.INCONSISTENT_TOPIC)
	            {
	                tpListener.OnInconsistentTopic(source as ITopic, topicStatus.InconsistentTopic);
	            }

	            /*if ((triggerMask & V_EVENT.ALL_DATA_DISPOSED) == V_EVENT.ALL_DATA_DISPOSED)
	            {
	                IDomainParticipantListener dpExtListener = listener as IExtDomainParticipantListener;
	                Debug.Assert(dpExtListener != null);
	                dpExtListener.OnAllDataDisposed(source as ITopic);
	            }*/
	        }
        }

        internal ReturnCode ValidateFilter(string filterExpression, string[] filterParameters)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;
            int length = (filterParameters != null) ? filterParameters.Length : 0;
            IntPtr qExpr = IntPtr.Zero;
            IntPtr qParams = IntPtr.Zero;

            ReportStack.Start();
            if (length < 100)
            {
                qExpr = DDS.OpenSplice.Kernel.Parser.parse(filterExpression);
                if (qExpr != IntPtr.Zero && length > 0)
                {
                    SequenceStringToCValueArrMarshaler.CopyIn(filterParameters, ref qParams);
                }
                else
                {
                    ReportStack.Report(result, "ContentFilteredTopic cannot be created. Invalid expression: " + filterExpression);
                }
            }
            else
            {
                ReportStack.Report(result, "ContentFilteredTopic cannot be created. Too many parameters " + length + " (max = 99)");
            }

            if (qExpr != IntPtr.Zero)
            {
                if (User.Topic.ContentFilterValidate2(rlReq_UserPeer, qExpr, qParams) != 0)
                {
                    result = DDS.ReturnCode.Ok;
                }
                else
                {
                    ReportStack.Report(result, "ContentFilteredTopic cannot be created. Filter invalid: " + filterExpression);
                }
                if (qParams != IntPtr.Zero)
                {
                    SequenceStringToCValueArrMarshaler.CleanupIn(ref qParams, length);
                }
                q.dispose(qExpr);
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ITopicListener Listener
        {
            get
            {
                bool isAlive;
                ITopicListener tpListener = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        tpListener = rlReq_Listener as ITopicListener;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return tpListener;
            }
        }

        public ReturnCode SetListener(ITopicListener listener, StatusKind mask)
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

        public ReturnCode GetInconsistentTopicStatus(ref InconsistentTopicStatus status)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                if (status == null) status = new InconsistentTopicStatus();
                GCHandle statusGCHandle = GCHandle.Alloc(status, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Topic.GetInconsistentTopicStatus(
                                rlReq_UserPeer, 1, InconsistentTopicStatusMarshaler.CopyOut, GCHandle.ToIntPtr(statusGCHandle)));
                status = statusGCHandle.Target as InconsistentTopicStatus;
                statusGCHandle.Free();
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public ReturnCode GetQos(ref TopicQos qos)
        {
            IntPtr userQos = IntPtr.Zero;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = uResultToReturnCode(
                        User.Topic.GetQos(rlReq_UserPeer, ref userQos));
                if (result == ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                            new OpenSplice.CustomMarshalers.TopicQosMarshaler(userQos, true))
                    {
                        marshaler.CopyOut(ref qos);
                    }
                }
                else
                {
                    ReportStack.Report(result, "Could not copy TopicQos.");
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetQos(TopicQos qos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
            ReportStack.Start();
            if (this.rlReq_isAlive)
            {
                result = QosManager.checkQos(qos);
                if (result == DDS.ReturnCode.Ok)
                {
                    using (OpenSplice.CustomMarshalers.TopicQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.TopicQosMarshaler())
                    {
                        result = marshaler.CopyIn(qos);
                        if (result == ReturnCode.Ok)
                        {
                            result = uResultToReturnCode(
                                    User.Topic.SetQos(rlReq_UserPeer, marshaler.UserPtr));
                            if (result != ReturnCode.Ok)
                            {
                                ReportStack.Report(result, "Could not apply TopicQos.");
                            }
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        /// <summary>
        /// This property returns the registered name of the data type associated with the TopicDescription
        /// (inherited from TopicDescription)
        /// </summary>
        public string TypeName
        {
            get
            {
                bool isAlive;
                string name = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        name = typeName;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return name;
            }
        }

        /// <summary>
        /// This property returns the name used to create the TopicDescription.
        /// (inherited from TopicDescription)
        /// </summary>
        public string Name
        {
            get
            {
                bool isAlive;
                string name = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        name = topicName;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return name;
            }
        }

        /// <summary>
        /// This property returns the DomainParticipant associated with the TopicDescription or
        /// a null DomainParticipant.
        /// (inherited from TopicDescription)
        /// </summary>
        public IDomainParticipant Participant
        {
            get
            {
                bool isAlive;
                DomainParticipant participant = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        participant = domainParticipant;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return participant;
            }
        }
    }
}
