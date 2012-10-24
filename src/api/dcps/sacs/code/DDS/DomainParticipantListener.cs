// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
