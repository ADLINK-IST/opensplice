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
using DDS;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{    
    internal class MultiTopic : SacsSuperClass, ITopicDescriptionImpl, IMultiTopic
    {
        private DomainParticipant domainParticipant;
        private TypeSupport typeSupport;
        private string typeName;
        private string topicName;
        private string subscriptionExpression;
        private string[] subscriptionParameters;
        private uint nrUsers = 0;
        private ITopicDescriptionImpl[] relatedTopics = new ITopicDescriptionImpl[0];
        
        internal MultiTopic(
                string typeName,
                string topicName,
                DomainParticipant domainParticipant,
                TypeSupport typeSupport,
                string subscriptionExpression, 
                string[] subscriptionParameters)
        {
            this.typeName = typeName;
            this.topicName = topicName;
            this.domainParticipant = domainParticipant;
            this.typeSupport = typeSupport;
            this.subscriptionExpression = subscriptionExpression;
            this.subscriptionParameters = subscriptionParameters;
            MyDomainId = domainParticipant.MyDomainId;
        }
        
        internal ReturnCode init()
        {
            ReturnCode result;
            
            result = DDS.ReturnCode.Unsupported;
            // Try to obtain the list of related topics.
            //relatedTopics = getRelatedTopics();
            for (int i = 0; i < relatedTopics.Length && result == DDS.ReturnCode.Ok; i++)
            {
                ITopicDescriptionImpl t = relatedTopics[i];
                if (t != null)
                {
                    lock(t)
                    {
                        if ((t as SacsSuperClass).rlReq_isAlive)
                        {
                            t.wlReq_IncrNrUsers();
                        }
                        else
                        {
                            result = DDS.ReturnCode.PreconditionNotMet;
                            ReportStack.Report(result, "MultiTopic \"" + topicName + "\" is referring to topics that have already been deleted.");
                            break;
                        }
                    }
                }
                if (result != ReturnCode.Ok)
                {
                    for (int j = 0; j < i; j++)
                    {
                        relatedTopics[j].DecrNrUsers();
                    }
                }
                else
                {
                    result = DDS.ReturnCode.Error;
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
                ReportStack.Report(result, "MultiTopic \"" + topicName + "\" still in use by " + nrUsers + " Writers/Readers.");
            }
            else
            {
                result = base.wlReq_deinit();
                for (int i = 0; i < relatedTopics.Length && result == DDS.ReturnCode.Ok; i++)
                {
                    result = relatedTopics[i].DecrNrUsers();
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
        
        public string SubscriptionExpression
        {
            get
            {
                bool isAlive;
                string sexpr = null;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
                        sexpr = subscriptionExpression;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return sexpr;
            }
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
                    expressionParameters = this.subscriptionParameters;
                    result = DDS.ReturnCode.Ok;
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }
        
        public ReturnCode SetExpressionParameters(params string[] expressionParameters)
        {
            ReturnCode result = DDS.ReturnCode.Unsupported;
            
//            using (SequenceStringToArrMarshaler marshaler = new SequenceStringToArrMarshaler())
//            {
//                result = WriteLock();
//                if (result == DDS.ReturnCode.Ok)
//                {
//                    result = marshaler.CopyIn(expressionParameters);
//                    if (result == ReturnCode.Ok)
//                    {
//                        result = uResultToReturnCode(
//                                User.MultiTopic.Set(rlReq_UserPeer, marshaler.UserPtr, (uint) expressionParameters.Length));
//                        if (result == ReturnCode.Ok)
//                        {
//                            marshaler.CopyOut(ref this.subscriptionParameters); // Make deep copy.
//                        }
//                    }
//                    WriteUnlock();
//                }
//            }

            return result;
        }
        
        string ITopicDescriptionImpl.rlReq_TopicExpression
        {
            get
            {
                return subscriptionExpression;
            }
        }
        
        string[] ITopicDescriptionImpl.rlReq_TopicExpressionParameters
        {
            get
            {
                return subscriptionParameters;
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
