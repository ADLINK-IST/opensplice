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
using System.Runtime.InteropServices;
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    public class DomainParticipantFactory : SacsSuperClass, IDomainParticipantFactory
    {
        /**
         * Attribute containing the singleton 'self' reference.
         */
        private static DomainParticipantFactory singletonSelf = null;
        private static readonly object singleton_lock = new object();
         

        /**
         * Attribute containing the delegates to the individual Listener functions.
         */


        /**
         * This operation returns the reference to the singleton DomainParticipantFactory.
         */
        public static DomainParticipantFactory GetInstance()
        {
            // GetInstance() is called very infrequntly so a simple locked singleton
            // approach is used.
            lock(singleton_lock)
            {
              if (singletonSelf == null) // If singleton doesn't exist, create it.
              {
                  IntPtr gapiPtr = OpenSplice.Gapi.DomainParticipantFactory.get_instance();

                  if (!gapiPtr.Equals(null)) // Wrap Gapi entity in C# object.
                  {
                      singletonSelf = new DomainParticipantFactory(gapiPtr);
                  }
              }
              return singletonSelf;
            }
        }

        /**
         * [Non-standard] Property for DomainParticipantFactory.
         */
        public static DomainParticipantFactory Instance
        {
            get { return GetInstance(); }
        }
        
        
        /**
         * Constructor for DomainParticipantFactory. Only to be used by the static 
         * <code>get_instance</code> operation.
         */
        protected DomainParticipantFactory(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        /**
         * This operation creates a new DomainParticipant using the specified URI. 
         */
        public IDomainParticipant CreateParticipant(
            string domainId)
        {
            IntPtr gapiParticipant;
            DDS.IDomainParticipant participant = null;

            // Invoke the corresponding gapi function.
            gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                GapiPeer,
                domainId,
                OpenSplice.Gapi.NativeConstants.GapiParticipantQosDefault,
                IntPtr.Zero,
                0,
                IntPtr.Zero,
                IntPtr.Zero,
                IntPtr.Zero);

            if (gapiParticipant != IntPtr.Zero)
            {
                participant = new OpenSplice.DomainParticipant(gapiParticipant);
            }

            return participant;
        }

        public IDomainParticipant CreateParticipant(
            string domainId,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateParticipant(domainId);
            }

            IDomainParticipant participant = null;

            OpenSplice.Gapi.gapi_domainParticipantListener gapiListener;
            DomainParticipantListenerHelper listenerHelper = new DomainParticipantListenerHelper();
            listenerHelper.Listener = listener;
            listenerHelper.CreateListener(out gapiListener);
            using (DomainParticipantListenerMarshaler listenerMarshaler = new DomainParticipantListenerMarshaler(ref gapiListener))
            {
                // Invoke the corresponding gapi function.
                IntPtr gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                    GapiPeer,
                    domainId,
                    IntPtr.Zero,
                    listenerMarshaler.GapiPtr,
                    mask,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero);
                if (gapiParticipant != IntPtr.Zero)
                {
                    participant = new OpenSplice.DomainParticipant(gapiParticipant, listenerHelper);
                }
            }

            return participant;
        }

        /**
         * This operation creates a new DomainParticipant using the specified URI and QoS settings. 
         */
        public IDomainParticipant CreateParticipant(
            string domainId,
            ref DomainParticipantQos qos)
        {
            IDomainParticipant participant = null;

            using (IMarshaler marshaler = new DomainParticipantQosMarshaler(ref qos))
            {
                // Invoke the corresponding gapi function.
                IntPtr gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                    GapiPeer,
                    domainId,
                    marshaler.GapiPtr,
                    IntPtr.Zero,
                    0,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero);
                if (gapiParticipant != IntPtr.Zero)
                {
                    participant = new OpenSplice.DomainParticipant(gapiParticipant);
                }
            }

            return participant;
        }

        public IDomainParticipant CreateParticipant(
            string domainId,
            ref DomainParticipantQos qos,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            // just in case they use this with null
            if (listener == null)
            {
                return CreateParticipant(domainId, ref qos);
            }

            IDomainParticipant participant = null;

            using (IMarshaler marshaler = new DomainParticipantQosMarshaler(ref qos))
            {
                OpenSplice.Gapi.gapi_domainParticipantListener gapiListener;
                DomainParticipantListenerHelper listenerHelper = new DomainParticipantListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (DomainParticipantListenerMarshaler listenerMarshaler = new DomainParticipantListenerMarshaler(ref gapiListener))
                {
                    // Invoke the corresponding gapi function.
                    IntPtr gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                        GapiPeer,
                        domainId,
                        marshaler.GapiPtr,
                        listenerMarshaler.GapiPtr,
                        mask,
                        IntPtr.Zero,
                        IntPtr.Zero,
                        IntPtr.Zero);
                    if (gapiParticipant != IntPtr.Zero)
                    {
                        participant = new OpenSplice.DomainParticipant(gapiParticipant, listenerHelper);
                    }
                }
            }

            return participant;
        }

        /**
         * This operation deletes the specified DomainParticipant. 
         */
        public ReturnCode DeleteParticipant(IDomainParticipant a_participant)
        {
            DomainParticipant participant = (DomainParticipant)a_participant;
            return OpenSplice.Gapi.DomainParticipantFactory.delete_participant_w_action(
                GapiPeer,
                participant.GapiPeer,
                DeleteEntityAction,
                IntPtr.Zero);
        }

        /**
         * This operation looks up an existing DomainParticipant based on its domainId. 
         */
        public IDomainParticipant LookupParticipant(string domainId)
        {
            IntPtr gapiDP = OpenSplice.Gapi.DomainParticipantFactory.lookup_participant(GapiPeer, domainId);
            IDomainParticipant participant = (IDomainParticipant)SacsSuperClass.fromUserData(gapiDP);
            return participant;
        }

        /**
         * This operation specifies the default settings for the DomainParticipantQos,
         * that can be used by newly created DomainParticipants that do not have a particular
         * preference for all individual policies. 
         */
        public ReturnCode SetDefaultParticipantQos(ref DomainParticipantQos qos)
        {
            ReturnCode result;

            using (IMarshaler marshaler = new DomainParticipantQosMarshaler(ref qos))
            {
                // Invoke the corresponding gapi function.
                result = OpenSplice.Gapi.DomainParticipantFactory.set_default_participant_qos(
                    GapiPeer, 
                    marshaler.GapiPtr);
            }

            return result;
        }

        /**
         * This operation obtains the default settings for the DomainParticipantQos,
         * that can be used by newly created DomainParticipants that do not have a particular
         * preference for all individual policies. 
         */
        public ReturnCode GetDefaultParticipantQos(out DomainParticipantQos qos)
        {
            qos = new DomainParticipantQos();
            ReturnCode result;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
            {
                // Invoke the corresponding gapi function.
                result = OpenSplice.Gapi.DomainParticipantFactory.get_default_participant_qos(GapiPeer, marshaler.GapiPtr);

                // When no error occured, copy the QoS settings from the gapi Qos representation.
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        /**
         * This operation specifies the QoS settings for the DomainParticipantFactory.
         */
        public ReturnCode SetQos(ref DomainParticipantFactoryQos qos)
        {
            ReturnCode result;

            using (IMarshaler marshaler = new DomainParticipantFactoryQosMarshaler(ref qos))
            {
                // Invoke the corresponding gapi function.
                result = OpenSplice.Gapi.DomainParticipantFactory.set_qos(GapiPeer, marshaler.GapiPtr);
            }

            return result;
        }

        /**
         * This operation obtains the QoS settings for the DomainParticipantFactory.
         */
        public ReturnCode GetQos(out DomainParticipantFactoryQos qos)
        {
            qos = new DomainParticipantFactoryQos();

            ReturnCode result;

            using (DomainParticipantFactoryQosMarshaler marshaler = new DomainParticipantFactoryQosMarshaler())
            {
                // Invoke the corresponding gapi function.
                result = OpenSplice.Gapi.DomainParticipantFactory.get_qos(GapiPeer, marshaler.GapiPtr);

                // When no error occured, copy the QoS settings from the gapi Qos representation.
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(out qos);
                }
            }

            return result;
        }

        internal void DeleteEntityAction(IntPtr entityData, IntPtr userData)
        {
            // Translate the UserData pointer into a valid C# language object.
            SacsSuperClass entity = SacsSuperClass.fromUserData(entityData);

            // If the UserData contained a valid object, then destruct it.
            if (entity != null)
            {
                entity.Dispose();
            }
        }

    }

}
