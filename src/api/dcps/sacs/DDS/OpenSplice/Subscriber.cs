// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2009 PrismTech Limited and its licensees.
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
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    internal class Subscriber : Entity, ISubscriber
    {
        private readonly SubscriberListenerHelper listenerHelper;

        internal Subscriber(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
            listenerHelper = new SubscriberListenerHelper();
        }

        internal Subscriber(IntPtr gapiPtr, SubscriberListenerHelper listenerHelper)
            : base(gapiPtr)
        {
            // Base class handles everything.
            this.listenerHelper = listenerHelper;
        }

        public ReturnCode SetListener(ISubscriberListener listener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Error;

            Gapi.gapi_subscriberListener gapiListener;
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (SubscriberListenerMarshaler marshaler = new SubscriberListenerMarshaler(ref gapiListener))
            {
                result = Gapi.Subscriber.set_listener(
                    GapiPeer,
                    marshaler.GapiPtr,
                    mask);
            }

            return result;
        }

        public IDataReader CreateDataReader(ITopicDescription topic)
        {
            IDataReader dataReader = null;
            SacsSuperClass superObj = (SacsSuperClass)topic;

            IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                GapiPeer,
                superObj.GapiPeer,
                IntPtr.Zero,
                IntPtr.Zero,
                StatusKind.Any);
            if (gapiPtr != IntPtr.Zero)
            {
                // TODO: check this and maybe do some error handling
                IDomainParticipant participant = topic.Participant;
                string name = topic.TypeName;
                ITypeSupport support = participant.GetTypeSupport(name);
                TypeSupport typeSupport = (TypeSupport)support;
                dataReader = typeSupport.CreateDataReader(gapiPtr);
            }
            return dataReader;
        }

        public IDataReader CreateDataReader(ITopicDescription topic,
            IDataReaderListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateDataReader(topic);
            }

            DataReader dataReader = null;
            SacsSuperClass superObj = (SacsSuperClass)topic;

            OpenSplice.Gapi.gapi_dataReaderListener gapiListener;
            DataReaderListenerHelper listenerHelper = new DataReaderListenerHelper();
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (DataReaderListenerMarshaler listenerMarshaler = new DataReaderListenerMarshaler(ref gapiListener))
            {
                IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                    GapiPeer,
                    superObj.GapiPeer,
                    IntPtr.Zero,
                    listenerMarshaler.GapiPtr,
                    mask);
                if (gapiPtr != IntPtr.Zero)
                {
                    // TODO: check this and maybe do some error handling
                    IDomainParticipant participant = topic.Participant;
                    string name = topic.TypeName;
                    ITypeSupport support = participant.GetTypeSupport(name);
                    TypeSupport typeSupport = (TypeSupport)support;
                    dataReader = typeSupport.CreateDataReader(gapiPtr);
                    dataReader.SetListener(listenerHelper);
                }
            }

            return dataReader;
        }

        public IDataReader CreateDataReader(ITopicDescription topic, ref DataReaderQos qos)
        {
            IDataReader dataReader = null;
            SacsSuperClass superObj = (SacsSuperClass)topic;

            using (IMarshaler marshaler = new DataReaderQosMarshaler(ref qos))
            {
                IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                    GapiPeer,
                    superObj.GapiPeer,
                    marshaler.GapiPtr,
                    IntPtr.Zero,
                    StatusKind.Any);
                if (gapiPtr != IntPtr.Zero)
                {
                    // TODO: check this and maybe do some error handling
                    IDomainParticipant participant = topic.Participant;
                    string name = topic.TypeName;
                    ITypeSupport support = participant.GetTypeSupport(name);
                    TypeSupport typeSupport = (TypeSupport)support;
                    dataReader = typeSupport.CreateDataReader(gapiPtr);
                }
            }

            return dataReader;
        }

        public IDataReader CreateDataReader(ITopicDescription topic, ref DataReaderQos qos,
            IDataReaderListener listener, StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateDataReader(topic, ref qos);
            }

            DataReader dataReader = null;
            SacsSuperClass superObj = (SacsSuperClass)topic;

            using (IMarshaler marshaler = new DataReaderQosMarshaler(ref qos))
            {
                OpenSplice.Gapi.gapi_dataReaderListener gapiListener;
                DataReaderListenerHelper listenerHelper = new DataReaderListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (DataReaderListenerMarshaler listenerMarshaler = new DataReaderListenerMarshaler(ref gapiListener))
                {
                    IntPtr gapiPtr = Gapi.Subscriber.create_datareader(
                        GapiPeer,
                        superObj.GapiPeer,
                        marshaler.GapiPtr,
                        listenerMarshaler.GapiPtr,
                        mask);
                    if (gapiPtr != IntPtr.Zero)
                    {
                        // TODO: check this and maybe do some error handling
                        IDomainParticipant participant = topic.Participant;
                        string name = topic.TypeName;
                        ITypeSupport support = participant.GetTypeSupport(name);
                        TypeSupport typeSupport = (TypeSupport)support;
                        dataReader = typeSupport.CreateDataReader(gapiPtr);
                        dataReader.SetListener(listenerHelper);
                    }
                }
            }

            return dataReader;
        }

        public ReturnCode DeleteDataReader(IDataReader dataReader)
        {
            DataReader dataReaderObj = (DataReader)dataReader;

            return Gapi.Subscriber.delete_datareader(
                GapiPeer,
                dataReaderObj.GapiPeer);
        }

        public IDataReader LookupDataReader(string topicName)
        {
            IntPtr gapiPtr = Gapi.Subscriber.lookup_datareader(
                GapiPeer,
                topicName);

            IDataReader dataReader = SacsSuperClass.fromUserData(gapiPtr) as IDataReader;
            return dataReader;
        }

        public ReturnCode DeleteContainedEntities()
        {
            return Gapi.Subscriber.delete_contained_entities(
                GapiPeer,
                null,
                IntPtr.Zero);
        }

        public ReturnCode GetDataReaders(out IDataReader[] readers)
        {
            return GetDataReaders(out readers, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode GetDataReaders(out IDataReader[] readers, SampleStateKind sampleStates,
            ViewStateKind viewStates, InstanceStateKind instanceStates)
        {
            readers = null;
            ReturnCode result = ReturnCode.Error;

            using (SequenceMarshaler<IDataReader, DataReader> marshaler = new SequenceMarshaler<IDataReader, DataReader>())
            {
                result = Gapi.Subscriber.get_datareaders(
                    GapiPeer,
                    marshaler.GapiPtr,
                    sampleStates,
                    viewStates,
                    instanceStates);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out readers);
                }
            }

            return result;
        }

        public ReturnCode NotifyDataReaders()
        {
            return Gapi.Subscriber.notify_datareaders(GapiPeer);
        }

        public ReturnCode SetQos(ref SubscriberQos qos)
        {
            using (IMarshaler marshaler = new SubscriberQosMarshaler(ref qos))
            {
                return OpenSplice.Gapi.Subscriber.set_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetQos(out SubscriberQos qos)
        {
            qos = new SubscriberQos();
            ReturnCode result = ReturnCode.Error;

            using (SubscriberQosMarshaler marshaler = new SubscriberQosMarshaler())
            {
                result = OpenSplice.Gapi.Subscriber.get_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode BeginAccess()
        {
            return OpenSplice.Gapi.Subscriber.begin_access(GapiPeer);
        }

        public ReturnCode EndAccess()
        {
            return OpenSplice.Gapi.Subscriber.end_access(GapiPeer);
        }

        public IDomainParticipant Participant
        {
            get
            {
                IntPtr gapiPtr = Gapi.Subscriber.get_participant(GapiPeer);

                IDomainParticipant domainParticipant = SacsSuperClass.fromUserData(gapiPtr) as IDomainParticipant;
                return domainParticipant;
            }
        }

        public ReturnCode SetDefaultDataReaderQos(ref DataReaderQos qos)
        {
            using (IMarshaler marshaler = new DataReaderQosMarshaler(ref qos))
            {
                return OpenSplice.Gapi.Subscriber.set_default_datareader_qos(
                GapiPeer,
                marshaler.GapiPtr);
            }
        }

        public ReturnCode GetDefaultDataReaderQos(out DataReaderQos qos)
        {
            qos = new DataReaderQos();
            ReturnCode result = ReturnCode.Error;

            using (DataReaderQosMarshaler marshaler = new DataReaderQosMarshaler())
            {
                result = OpenSplice.Gapi.Subscriber.get_default_datareader_qos(
                GapiPeer,
                marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        public ReturnCode CopyFromTopicQos(out DataReaderQos dataReaderQos, ref TopicQos topicQos)
        {
            dataReaderQos = new DataReaderQos();
            ReturnCode result = ReturnCode.Error;

            using (IMarshaler marshaler = new TopicQosMarshaler(ref topicQos))
            {
                using (DataReaderQosMarshaler dataReaderMarshaler = new
                    DataReaderQosMarshaler())
                {
                    result = OpenSplice.Gapi.Subscriber.copy_from_topic_qos(
                    GapiPeer,
                    dataReaderMarshaler.GapiPtr,
                    marshaler.GapiPtr);

                    if (result == ReturnCode.Ok)
                    {
                        dataReaderMarshaler.CopyOut(out dataReaderQos);
                    }
                }
            }

            return result;
        }
    }
}
