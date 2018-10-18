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
using DDS;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    internal class ContentFilteredTopic : SacsSuperClass, IContentFilteredTopic, ITopicDescriptionImpl
    {
        private DomainParticipant domainParticipant;
        private TypeSupport typeSupport;
        private string typeName;
        private string topicName;
        private string filterExpression;
        private string[] filterParameters;
        private string topicExpression;
        private uint nrUsers = 0;
        private Topic relatedTopic;

        internal ContentFilteredTopic(
                string typeName,
                string topicName,
                DomainParticipant domainParticipant,
                string filterExpression,
                string[] filterParameters)
        {
            this.typeName = typeName;
            this.topicName = topicName;
            this.domainParticipant = domainParticipant;
            this.filterExpression = filterExpression;
            this.filterParameters = filterParameters;
        }

        internal ReturnCode init(Topic relatedTopic)
        {
            ReturnCode result;

            MyDomainId = relatedTopic.MyDomainId;

            this.relatedTopic = relatedTopic;
            lock(relatedTopic)
            {
                if (relatedTopic.rlReq_isAlive)
                {
                    result = relatedTopic.ValidateFilter(filterExpression, filterParameters);
                    if (result == DDS.ReturnCode.Ok)
                    {
                        ITopicDescriptionImpl relatedTopicDescr = relatedTopic as ITopicDescriptionImpl;
                        relatedTopicDescr.wlReq_IncrNrUsers();
                        this.topicExpression = relatedTopicDescr.rlReq_TopicExpression + " where " + this.filterExpression;
                        this.typeSupport = relatedTopicDescr.rlReq_TypeSupport;
                        result = base.init(IntPtr.Zero, false);
                    }
                }
                else
                {
                    result = DDS.ReturnCode.BadParameter;
                    ReportStack.Report(result, "ContentFilteredTopic \"" + topicName + "\" is referring to an already deleted topic.");
                }
            }
            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result;

            if (nrUsers > 0)
            {
                result = DDS.ReturnCode.PreconditionNotMet;
                ReportStack.Report(result, "ContentFilteredTopic \"" + topicName + "\" still in use by " + nrUsers + " Writers/Readers.");
            }
            else
            {
                result = base.wlReq_deinit();
                if (result == DDS.ReturnCode.Ok)
                {
                    result = (relatedTopic as ITopicDescriptionImpl).DecrNrUsers();
                }
            }
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

        public string GetFilterExpression()
        {
            bool isAlive;
            string fexpr = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    fexpr = filterExpression;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return fexpr;
        }

        public ReturnCode GetExpressionParameters(ref string[] expressionParameters)
        {
            expressionParameters = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    expressionParameters = this.filterParameters;
                    result = DDS.ReturnCode.Ok;
                }
            }
            ReportStack.Flush(this, result != DDS.ReturnCode.Ok);

            return result;
        }

        public ReturnCode SetExpressionParameters(params string[] expressionParameters)
        {
            ReturnCode result = DDS.ReturnCode.Unsupported;
            ReportStack.Start();

//            using (SequenceStringToArrMarshaler marshaler = new SequenceStringToArrMarshaler())
//            {
//                result = WriteLock();
//                if (result == DDS.ReturnCode.Ok)
//                {
//                    result = marshaler.CopyIn(expressionParameters);
//                    if (result == ReturnCode.Ok)
//                    {
//                        result = uResultToReturnCode(
//                                User.ContentFilteredTopic.Set(rlReq_UserPeer, marshaler.UserPtr, (uint) expressionParameters.Length));
//                        if (result == ReturnCode.Ok)
//                        {
//                            marshaler.CopyOut(ref this.filterParameters); // Make deep copy.
//                        }
//                    }
//                    WriteUnlock();
//                }
//            }
            ReportStack.Report(result, "Operation not yet supported.");
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        public ITopic RelatedTopic
        {
            get
            {
                bool isAlive;
                Topic topic = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        topic = relatedTopic;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return topic;
            }
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
                return filterParameters;
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
    }
}
