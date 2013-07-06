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
using System.Runtime.InteropServices;
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    /// <summary>
    /// The purpose of this class is to allow the creation and destruction of 
    /// DomainParticipant objects. DomainParticipantFactory itself has no 
    /// factory. It is a pre-existing singleton object that can be accessed by means of the 
    /// Instance property on the DomainParticipantFactory class. 
    /// </summary>
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
        private static DomainParticipantFactory GetInstance()
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

        /// <summary>
        /// [Non-standard] Property for DomainParticipantFactory. 
        /// This property is used to return the DomainParticipantFactory singleton
        /// object.
        /// </summary>
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


        public IDomainParticipant CreateParticipant(
            DomainId domainId)
        {
            return CreateParticipant(domainId, null, 0);
        }

        public IDomainParticipant CreateParticipant(
            DomainId domainId,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            IDomainParticipant participant = null;
            string className = null;

             
            if (listener != null)
            {
                OpenSplice.Gapi.gapi_domainParticipantListener gapiListener;
                DomainParticipantListenerHelper listenerHelper = new DomainParticipantListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                using (DomainParticipantListenerMarshaler listenerMarshaler = 
                        new DomainParticipantListenerMarshaler(ref gapiListener))
                {
                    // Invoke the corresponding gapi function.
                    IntPtr gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                            GapiPeer,
                            domainId,
                            OpenSplice.Gapi.NativeConstants.GapiParticipantQosDefault,
                            listenerMarshaler.GapiPtr,
                            mask,
                            IntPtr.Zero,
                            IntPtr.Zero,
                            IntPtr.Zero,
                            className);
                    if (gapiParticipant != IntPtr.Zero)
                    {
                        participant = new OpenSplice.DomainParticipant(gapiParticipant, listenerHelper);
                    }
                }
            }
            else
            {
                // Invoke the corresponding gapi function.
                IntPtr gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                    GapiPeer,
                    domainId,
                    OpenSplice.Gapi.NativeConstants.GapiParticipantQosDefault,
                    IntPtr.Zero,
                    mask,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    className);

                if (gapiParticipant != IntPtr.Zero)
                {
                    participant = new OpenSplice.DomainParticipant(gapiParticipant);
                }
            }
            
            if (participant != null)
            {
                DomainParticipantFactoryQos dpfQos = null;
                ReturnCode result = GetQos(ref dpfQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpfQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        participant.Enable();
                    }
                }
            }       
                
            return participant;
        }

        public IDomainParticipant CreateParticipant(
            DomainId domainId,
            DomainParticipantQos qos)
        {
            return CreateParticipant(domainId, qos, null, 0);
        }

        
        public IDomainParticipant CreateParticipant(
            DomainId domainId,
            DomainParticipantQos qos,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            IDomainParticipant participant = null;
            string className = null;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        OpenSplice.Gapi.gapi_domainParticipantListener gapiListener;
                        DomainParticipantListenerHelper listenerHelper = new DomainParticipantListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
                        using (DomainParticipantListenerMarshaler listenerMarshaler = 
                                new DomainParticipantListenerMarshaler(ref gapiListener))
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
                                    IntPtr.Zero,
                                    className);
                            if (gapiParticipant != IntPtr.Zero)
                            {
                                participant = new OpenSplice.DomainParticipant(gapiParticipant, listenerHelper);
                            }
                        }
                    }
                    else
                    {
                        // Invoke the corresponding gapi function.
                        IntPtr gapiParticipant = OpenSplice.Gapi.DomainParticipantFactory.create_participant(
                            GapiPeer,
                            domainId,
                            marshaler.GapiPtr,
                            IntPtr.Zero,
                            mask,
                            IntPtr.Zero,
                            IntPtr.Zero,
                            IntPtr.Zero,
                            className);
                        if (gapiParticipant != IntPtr.Zero)
                        {
                            participant = new OpenSplice.DomainParticipant(gapiParticipant);
                        }
                    }
                }
            }

            if (participant != null)
            {
                DomainParticipantFactoryQos dpfQos = null;
                ReturnCode result = GetQos(ref dpfQos);
                if (result == ReturnCode.Ok)
                {
                    if (dpfQos.EntityFactory.AutoenableCreatedEntities)
                    {
                        participant.Enable();
                    }
                }
            }       
                
            return participant;
        }
      
        public ReturnCode DeleteParticipant(IDomainParticipant a_participant)
        {
            ReturnCode result = ReturnCode.BadParameter;

            DomainParticipant participant = a_participant as DomainParticipant;
            if (participant != null)
            {
                result = OpenSplice.Gapi.DomainParticipantFactory.delete_participant(
                    GapiPeer,
                    participant.GapiPeer);
            }

            return result;
        }
        
        public IDomainParticipant LookupParticipant(DomainId domainId)
        {
            IntPtr gapiDP = OpenSplice.Gapi.DomainParticipantFactory.lookup_participant(GapiPeer, domainId);
            IDomainParticipant participant = (IDomainParticipant)SacsSuperClass.fromUserData(gapiDP);
            return participant;
        }
        
        public ReturnCode SetDefaultParticipantQos(DomainParticipantQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    // Invoke the corresponding gapi function.
                    result = OpenSplice.Gapi.DomainParticipantFactory.set_default_participant_qos(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }
            return result;
        }
                
        public ReturnCode GetDefaultParticipantQos(ref DomainParticipantQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
            {
                // Invoke the corresponding gapi function.
                result = OpenSplice.Gapi.DomainParticipantFactory.get_default_participant_qos(GapiPeer, marshaler.GapiPtr);

                // When no error occured, copy the QoS settings from the gapi Qos representation.
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }

        
        public ReturnCode SetQos(DomainParticipantFactoryQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantFactoryQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DomainParticipantFactoryQosMarshaler())
            {
                result = marshaler.CopyIn(qos);
                if (result == ReturnCode.Ok)
                {
                    // Invoke the corresponding gapi function.
                    result = OpenSplice.Gapi.DomainParticipantFactory.set_qos(GapiPeer, marshaler.GapiPtr);
                }
            }

            return result;
        }
        
        public ReturnCode GetQos(ref DomainParticipantFactoryQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantFactoryQosMarshaler marshaler = 
                    new OpenSplice.CustomMarshalers.DomainParticipantFactoryQosMarshaler())
            {
                // Invoke the corresponding gapi function.
                result = OpenSplice.Gapi.DomainParticipantFactory.get_qos(GapiPeer, marshaler.GapiPtr);

                // When no error occured, copy the QoS settings from the gapi Qos representation.
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref qos);
                }
            }

            return result;
        }

        internal new void DeleteEntityAction(IntPtr entityData, IntPtr userData)
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
