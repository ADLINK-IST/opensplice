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

        /// <summary>
        /// This operation creates a DomainParticipant using the specified URI.
        /// </summary>
        /// <param name="domainId">The specified URI used to create the DomainParticipant</param>
        /// <returns></returns>
        public IDomainParticipant CreateParticipant(
            string domainId)
        {
            return CreateParticipant(domainId, null, 0);
        }

        /// <summary>
        /// This operation creates a new DomainParticipant which will join the domain 
        /// identified by domainId, and attaches the optionally specified 
        /// DomainParticipantListener to it.
        /// </summary>
        /// <param name="domainId">The ID of the Domain to which the 
        /// DomainParticipant is joined. This should be a URI to the location of the 
        /// configuration file that identifies the configuration details of the Domain.</param>
        /// <param name="listener">The DomainParticipantListener instance which will be attached to the new 
        /// DomainParticipant. It is permitted to use null as the value of the listener: 
        /// this behaves as a DomainParticipantListener whose operations perform no action.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of the 
        /// DomainParticipantListener for a certain status.</param>
        /// <returns>The newly created DomainParticipant. In case of an error a null is returned.</returns>
        public IDomainParticipant CreateParticipant(
            string domainId,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            IDomainParticipant participant = null;

            if (listener != null)
            {
                OpenSplice.Gapi.gapi_domainParticipantListener gapiListener;
                DomainParticipantListenerHelper listenerHelper = new DomainParticipantListenerHelper();
                listenerHelper.Listener = listener;
                listenerHelper.CreateListener(out gapiListener);
                lock (listener)
                {
                    using (DomainParticipantListenerMarshaler listenerMarshaler = new DomainParticipantListenerMarshaler(ref gapiListener))
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
                            IntPtr.Zero);
                        if (gapiParticipant != IntPtr.Zero)
                        {
                            participant = new OpenSplice.DomainParticipant(gapiParticipant, listenerHelper);
                        }
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
                    IntPtr.Zero);

                if (gapiParticipant != IntPtr.Zero)
                {
                    participant = new OpenSplice.DomainParticipant(gapiParticipant);
                }
            }

            return participant;
        }

        /// <summary>
        /// This operation creates a new DomainParticipant using the specified URI and QoS settings.
        /// </summary>
        /// <param name="domainId">The ID of the Domain to which the 
        /// DomainParticipant is joined. This should be a URI to the location of the 
        /// configuration file that identifies the configuration details of the Domain.</param>
        /// <param name="qos">a DomainParticipantQos for the new DomainParticipant. 
        /// When this set of QosPolicy settings is inconsistent, no DomainParticipant is created.</param>
        /// <returns>The newly created DomainParticipant. In case of an error a null is returned.</returns>
        public IDomainParticipant CreateParticipant(
            string domainId,
            DomainParticipantQos qos)
        {
            return CreateParticipant(domainId, qos, null, 0);
        }

        /// <summary>
        /// This operation creates a new DomainParticipant which will join the domain 
        /// identified by domainId, and attaches the optionally specified 
        /// DomainParticipantListener to it.
        /// </summary>
        /// <param name="domainId">The ID of the Domain to which the 
        /// DomainParticipant is joined. This should be a URI to the location of the 
        /// configuration file that identifies the configuration details of the Domain.</param>
        /// <param name="listener">The DomainParticipantListener instance which will be attached to the new 
        /// DomainParticipant. It is permitted to use null as the value of the listener: 
        /// this behaves as a DomainParticipantListener whose operations perform no action.</param>
        /// <param name="qos">a DomainParticipantQos for the new DomainParticipant. 
        /// When this set of QosPolicy settings is inconsistent, no DomainParticipant is created.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of the 
        /// DomainParticipantListener for a certain status.</param>
        /// <returns>The newly created DomainParticipant. In case of an error a null is returned.</returns>
        public IDomainParticipant CreateParticipant(
            string domainId,
            DomainParticipantQos qos,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            IDomainParticipant participant = null;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
            {
                if (marshaler.CopyIn(qos) == ReturnCode.Ok)
                {
                    if (listener != null)
                    {
                        OpenSplice.Gapi.gapi_domainParticipantListener gapiListener;
                        DomainParticipantListenerHelper listenerHelper = new DomainParticipantListenerHelper();
                        listenerHelper.Listener = listener;
                        listenerHelper.CreateListener(out gapiListener);
                        lock (listener)
                        {
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
                            IntPtr.Zero);
                        if (gapiParticipant != IntPtr.Zero)
                        {
                            participant = new OpenSplice.DomainParticipant(gapiParticipant);
                        }
                    }
                }
            }

            return participant;
        }

        /// <summary>
        /// This operation deletes the specified DomainParticipant.
        /// </summary>
        /// <param name="a_participant"> The DomainParticipant which is to be deleted.</param>
        /// <returns>Return codes can be Ok,Error,BadParameter,OutOfResources or PreconditionNotMet</returns>
        public ReturnCode DeleteParticipant(IDomainParticipant a_participant)
        {
            ReturnCode result = ReturnCode.BadParameter;

            DomainParticipant participant = a_participant as DomainParticipant;
            if (participant != null)
            {
                result = OpenSplice.Gapi.DomainParticipantFactory.delete_participant_w_action(
                    GapiPeer,
                    participant.GapiPeer,
                    DeleteEntityAction,
                    IntPtr.Zero);
            }

            return result;
        }

        /// <summary>
        /// This operation looks up an existing DomainParticipant based on its domainId.
        /// </summary>
        /// <param name="domainId">the ID of the Domain for which a joining DomainParticipant 
        /// should be retrieved. This should be a URI to the location of the configuration file that 
        /// identifies the configuration details of the Domain.</param>
        /// <returns>The retrieved DomainParticipant. If no such DomainParticipant is found 
        /// a null is returned.</returns>
        public IDomainParticipant LookupParticipant(string domainId)
        {
            IntPtr gapiDP = OpenSplice.Gapi.DomainParticipantFactory.lookup_participant(GapiPeer, domainId);
            IDomainParticipant participant = (IDomainParticipant)SacsSuperClass.fromUserData(gapiDP);
            return participant;
        }

        /// <summary>
        /// This operation specifies the default settings for the DomainParticipantQos, 
        /// that can be used by newly created DomainParticipants that do not have 
        /// a particular preference for all individual policies. 
        /// </summary>
        /// <param name="qos">The DomainParticipantQos which contains the new default 
        /// DomainParticipantQos for the newly created DomainParticipants</param>
        /// <returns>Possible return codes are: Ok,Error and OutOfResources.</returns>
        public ReturnCode SetDefaultParticipantQos(DomainParticipantQos qos)
        {
            ReturnCode result;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
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
        
        /// <summary>
        /// This operation obtains the default settings for the DomainParticipantQos, that can be used 
        /// by newly created DomainParticipants that do not have a particular preference for all individual policies. 
        /// </summary>
        /// <param name="qos">A reference to the DomainParticipantQos in which the default DomainParticipantQos
        /// for the DomainParticipant is written.</param>
        /// <returns>Possible return codes are: Ok,Error and outOfResources.</returns>
        public ReturnCode GetDefaultParticipantQos(ref DomainParticipantQos qos)
        {
            ReturnCode result;

            using (DomainParticipantQosMarshaler marshaler = new DomainParticipantQosMarshaler())
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

        /// <summary>
        /// This operation specifies the QoS settings for the DomainParticipantFactory.
        /// </summary>
        /// <param name="qos">The new set of Qos policy settings for the DomainParticipantFactory.</param>
        /// <returns>Possible return codes are: Ok,Error and OutOfResources.</returns>
        public ReturnCode SetQos(DomainParticipantFactoryQos qos)
        {
            ReturnCode result;

            using (DomainParticipantFactoryQosMarshaler marshaler = new DomainParticipantFactoryQosMarshaler())
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

        /// <summary>
        /// This operation obtains the QoS settings for the DomainParticipantFactory.
        /// </summary>
        /// <param name="qos">A reference to the destination DomainParticipantFactoryQos, 
        /// in which the Qos policies will be copied.</param>
        /// <returns>Possible values are: Ok,Error, OutOfResources.</returns>
        public ReturnCode GetQos(ref DomainParticipantFactoryQos qos)
        {
            ReturnCode result;

            using (DomainParticipantFactoryQosMarshaler marshaler = new DomainParticipantFactoryQosMarshaler())
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
