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
using System.Text;

namespace DDS
{
    // NOTE: Since we cannot derive from multiple base classes, we just reimplement the
    // virtual methods from the aggregate interface IDomainParticipantListener
    public abstract class DomainParticipantListener : IDomainParticipantListener
    {
        //ITopicListener
        public virtual void OnInconsistentTopic(ITopic entityInterface, InconsistentTopicStatus status)
        {
        }

        //IDataWriterListener
        public virtual void OnOfferedDeadlineMissed(IDataWriter entityInterface, OfferedDeadlineMissedStatus status)
        {
        }
        public virtual void OnOfferedIncompatibleQos(IDataWriter entityInterface, OfferedIncompatibleQosStatus status)
        {
        }
        public virtual void OnLivelinessLost(IDataWriter entityInterface, LivelinessLostStatus status)
        {
        }
        public virtual void OnPublicationMatched(IDataWriter entityInterface, PublicationMatchedStatus status)
        {
        }

        //ISubscriberListener
        public virtual void OnDataOnReaders(ISubscriber entityInterface)
        {
        }

        //IDataReaderListener
        public virtual void OnRequestedDeadlineMissed(IDataReader entityInterface, RequestedDeadlineMissedStatus status)
        {
        }
        public virtual void OnRequestedIncompatibleQos(IDataReader entityInterface, RequestedIncompatibleQosStatus status)
        {
        }
        public virtual void OnSampleRejected(IDataReader entityInterface, SampleRejectedStatus status)
        {
        }
        public virtual void OnLivelinessChanged(IDataReader entityInterface, LivelinessChangedStatus status)
        {
        }
        public virtual void OnDataAvailable(IDataReader entityInterface)
        {
        }
        public virtual void OnSubscriptionMatched(IDataReader entityInterface, SubscriptionMatchedStatus status)
        {
        }
        public virtual void OnSampleLost(IDataReader entityInterface, SampleLostStatus status)
        {
        }
    }
}
