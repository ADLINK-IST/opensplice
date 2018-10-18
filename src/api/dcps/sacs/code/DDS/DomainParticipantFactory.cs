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
using System.Collections.Generic;
using DDS.OpenSplice;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Kernel;

namespace DDS
{
    /// <summary>
    /// The purpose of this class is to allow the creation and destruction of
    /// IDomainParticipant objects. DomainParticipantFactory itself has no
    /// factory. It is a pre-existing singleton object that can be accessed by means of the
    /// Instance property on the DomainParticipantFactory class.
    /// </summary>
    public class DomainParticipantFactory : IDomainParticipantFactory
    {
        //
        // Attribute containing the singleton 'self' reference.
        //
        private static DomainParticipantFactory singletonSelf = null;
        private static readonly object singleton_lock = new object();
        private DDS.DomainParticipantFactoryQos myQos = new DDS.DomainParticipantFactoryQos();
        private DDS.DomainParticipantQos defaultParticipantQos = new DDS.DomainParticipantQos();
        private List<DomainParticipant> participantList = new List<DomainParticipant>();

        // Attribute containing the delegates to the individual Listener functions.

        /// <summary>
        /// This operation returns the reference to the singleton DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// The operation is idempotent, that is, it can be called multiple times without
        /// side-effects and it returns the same DomainParticipantFactory instance.
        ///
        /// The operation is static and must be called upon its class.
        /// <code>
        /// DDS.DomainParticipantFactory factory = null;
        /// factory = DomainParticipantFactory.GetInstance();
        /// </code>
        /// </remarks>
        /// @return DomainParticipantFactory singleton
        private static DomainParticipantFactory GetInstance()
        {
            // GetInstance() is called very infrequently so a simple locked singleton
            // approach is used.
            lock(singleton_lock)
            {
                if (singletonSelf == null) // If singleton doesn't exist, create it.
                {
                    if (DDS.OpenSplice.User.u.userInitialise() == V_RESULT.OK)
                    {
                        ReportStack.Start();
                        singletonSelf = new DomainParticipantFactory();
                        ReportStack.Flush(null, singletonSelf == null);
                        if (singletonSelf != null) {
                            AppDomain.CurrentDomain.ProcessExit += new EventHandler(ProcessExit);
                        }
                    }
                }
                return singletonSelf;
            }
        }

        /// <summary>
        /// This property is used to get the DomainParticipantFactory singleton
        /// object.
        /// </summary>
        /// <remarks>
        /// The operation is idempotent, that is, it can be called multiple times without
        /// side-effects and it returns the same DomainParticipantFactory instance.
        ///
        /// <code>
        /// DDS.DomainParticipantFactory factory = null;
        /// factory = DomainParticipantFactory.Instance;
        /// </code>
        /// </remarks>
        /// @return DomainParticipantFactory singleton
        public static DomainParticipantFactory Instance
        {
            get { return GetInstance(); }
        }

        /// @cond
        /// Constructor for DomainParticipantFactory.
        /// Only to be used by the static <code>GetInstance</code> operation.
        protected DomainParticipantFactory()
        {
            DDS.ReturnCode result;

            // Init default ParticipantFactoryQos and default ParticipantQos.
            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
            {
                result = marshaler.CopyIn(QosManager.defaultDomainParticipantQos);
                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref defaultParticipantQos);
                }
            }
            myQos.EntityFactory = new EntityFactoryQosPolicy();
            myQos.EntityFactory.AutoenableCreatedEntities = true;
        }
        /// @endcond

        /// <summary>
        /// This operation creates a new IDomainParticipant which will join the domain
        /// identified by domainId (or DDS.DomainId.Default).
        /// </summary>
        /// <remarks>
        /// It will use default DDS.DomainParticipantQos, a null listener and 0 mask.
        ///
        /// See
        /// @ref DDS.DomainParticipantFactory.CreateParticipant(DomainId domainId, DomainParticipantQos qos, IDomainParticipantListener listener, StatusKind mask) "CreateParticipant"
        /// for:<br>
        /// - Identifying the Domain
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// @param domainId The ID of the Domain to which the IDomainParticipant is joined.
        ///                 This should be the ID as specified in the configuration file.
        /// @return The newly created IDomainParticipant. In case of an error a null is returned.
        public IDomainParticipant CreateParticipant(
            DomainId domainId)
        {
            return CreateParticipant(domainId, defaultParticipantQos, null, 0);
        }

        /// <summary>
        /// This operation creates a new IDomainParticipant which will join the domain
        /// identified by domainId (or DDS.DomainId.Default), and attaches the specified
        /// IDomainParticipantListener to it and uses the given communication StatusKind mask.
        /// </summary>
        /// <remarks>
        /// It will use default DDS.DomainParticipantQos.
        ///
        /// See
        /// @ref DDS.DomainParticipantFactory.CreateParticipant(DomainId domainId, DomainParticipantQos qos, IDomainParticipantListener listener, StatusKind mask) "CreateParticipant"
        /// for:<br>
        /// - Identifying the Domain
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// @param domainId The ID of the Domain to which the IDomainParticipant is joined.
        ///                 This should be the ID as specified in the configuration file.
        /// @param listener A IDomainParticipantListener instance which will be attached to
        ///                 the new IDomainParticipant. It is permitted to use null as the
        ///                 value of the listener: this behaves as a IDomainParticipantListener
        ///                 whose operations perform no action.
        /// @param mask     A bit-mask in which each bit enables the invocation of the
        ///                 IDomainParticipantListener for a certain status.
        /// @return The newly created IDomainParticipant. In case of an error a null is returned.
        public IDomainParticipant CreateParticipant(
            DomainId domainId,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            return CreateParticipant(domainId, defaultParticipantQos, listener, mask);
        }

        /// <summary>
        /// This operation creates a new IDomainParticipant which will join the domain
        /// identified by domainId (or DDS.DomainId.Default), with the desired
        /// DomainParticipantQos.
        /// </summary>
        /// <remarks>
        /// It will use a null listener and 0 mask.
        ///
        /// See
        /// @ref DDS.DomainParticipantFactory.CreateParticipant(DomainId domainId, DomainParticipantQos qos, IDomainParticipantListener listener, StatusKind mask) "CreateParticipant"
        /// for:<br>
        /// - Identifying the Domain
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// @param domainId The ID of the Domain to which the IDomainParticipant is joined.
        ///                 This should be the ID as specified in the configuration file.
        /// @param qos      a DomainParticipantQos for the new IDomainParticipant. When
        ///                 this set of QosPolicy settings is inconsistent, no
        ///                 IDomainParticipant is created.
        /// @return The newly created IDomainParticipant. In case of an error a null is returned.
        public IDomainParticipant CreateParticipant(
            DomainId domainId,
            DomainParticipantQos qos)
        {
            return CreateParticipant(domainId, qos, null, 0);
        }

        /// <summary>
        /// This operation creates a new IDomainParticipant which will join the domain
        /// identified by domainId, with the desired DomainParticipantQos and attaches
        /// the specified IDomainParticipantListener to it and uses the given communication
        /// StatusKind mask.
        /// </summary>
        /// <remarks>
        /// <i><b>Identifying the Domain</b></i><br>
        /// The IDomainParticipant will attach to the Domain that is specified by the
        /// domainId parameter. This parameter consists of an integer specified in the Id tag
        /// in the configuration file. Note that to make multiple connections to a Domain (create
        /// multiple Participants for the same Domain) within a single process, all of the
        /// Participants must use the same identification (i.e. all use the same domain Id).
        ///
        /// The constant DDS.DomainId.Default can be used for this parameter. If this is done
        /// the value of Id tag from the configuration file specified by the environment variable
        /// called OSPL_URI will be used.
        ///
        /// It is recommended to use this domain Id in conjunction with the OSPL_URI
        /// environment variable instead of hard-coding a domain Id into your application,
        /// since this gives you much more flexibility in the deployment phase of your product.<br>
        /// See also Section 1.3.2.1, The OSPL_URI environment variable, in the Deployment
        /// Guide.
        ///
        /// <i><b>Communication Status</b></i><br>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask , the associated
        /// IDomainParticipantListener operation is invoked and the communication
        /// status is reset to false , as the listener implicitly accesses the status which is passed
        /// as a parameter to that operation. The fact that the status is reset prior to calling the
        /// listener means that if the application calls the Get<status_name>Status from
        /// inside the listener it will see the status already reset.
        ///
        /// The following statuses are applicable to the IDomainParticipant
        /// - DDS.StatusKind InconsistentTopic (propagated)
        /// - DDS.StatusKind OfferedDeadlineMissed (propagated)
        /// - DDS.StatusKind RequestedDeadlineMissed (propagated)
        /// - DDS.StatusKind OfferedIncompatibleQos (propagated)
        /// - DDS.StatusKind RequestedIncompatibleQos (propagated)
        /// - DDS.StatusKind SampleLost (propagated)
        /// - DDS.StatusKind SampleRejected (propagated)
        /// - DDS.StatusKind DataOnReaders (propagated)
        /// - DDS.StatusKind DataAvailable (propagated)
        /// - DDS.StatusKind LivelinessLost (propagated)
        /// - DDS.StatusKind LivelinessChanged (propagated)
        /// - DDS.StatusKind PublicationMatched (propagated)
        /// - DDS.StatusKind SubscriptionMatched (propagated)
        ///
        /// Be aware that the PublicationMatched and SubscriptionMatched
        /// statuses are not applicable when the infrastructure does not have the
        /// information available to determine connectivity. This is the case when OpenSplice
        /// is configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return NULL.
        ///
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The special constant 0 can
        /// be used to indicate that the created entity should not respond to any of its available
        /// statuses. The DDS will therefore attempt to propagate these statuses to its factory.
        ///
        /// <i><b>Status Propagation</b></i><br>
        /// The Data Distribution Service will trigger the most specific and relevant Listener.<br>
        /// In other words, in case a communication status is also activated on the Listener of
        /// a contained entity, the Listener on that contained entity is invoked instead of the
        /// IDomainParticipantListener. This means that a status change on a contained
        /// entity only invokes the IDomainParticipantListener if the contained entity
        /// itself does not handle the trigger event generated by the status change.
        ///
        /// The statuses DataOnReaders and DataAvailable are
        /// “Read Communication Statuses” and are an exception to all other plain
        /// communication statuses: they have no corresponding status structure that can be
        /// obtained with a Get<status_name>Status operation and they are mutually
        /// exclusive. When new information becomes available to a IDataReader, the Data
        /// Distribution Service will first look in an attached and activated
        /// ISubscriberListener or IDomainParticipantListener (in that order) for the
        /// DataOnReaders. In case the DataOnReaders can not be
        /// handled, the Data Distribution Service will look in an attached and activated
        /// IDataReaderListener, ISubscriberListener or IDomainParticipantListener for
        /// the DataAvailable (in that order).
        /// </remarks>
        /// @param domainId The ID of the Domain to which the IDomainParticipant is joined.
        ///                 This should be the ID as specified in the configuration file.
        /// @param qos      a DomainParticipantQos for the new IDomainParticipant. When
        ///                 this set of QosPolicy settings is inconsistent, no
        ///                 IDomainParticipant is created.
        /// @param listener A IDomainParticipantListener instance which will be attached to
        ///                 the new IDomainParticipant. It is permitted to use null as the
        ///                 value of the listener: this behaves as a IDomainParticipantListener
        ///                 whose operations perform no action.
        /// @param mask     A bit-mask in which each bit enables the invocation of the
        ///                 IDomainParticipantListener for a certain status.
        /// @return The newly created IDomainParticipant. In case of an error a null is returned.
        public IDomainParticipant CreateParticipant(
            DomainId domainId,
            DomainParticipantQos qos,
            IDomainParticipantListener listener,
            StatusKind mask)
        {
            DomainParticipant participant = null;
            ReturnCode result;

            ReportStack.Start();
            result = QosManager.checkQos(qos);
            if (result == DDS.ReturnCode.Ok)
            {
                if (domainId != DDS.DomainId.Invalid)
                {
                    lock(singleton_lock)
                    {
                        participant = new OpenSplice.DomainParticipant();
                        result = participant.init(domainId, qos);
                        if (result == ReturnCode.Ok)
                        {
                            result = participant.SetListener(listener, mask);
                        }
                        else
                        {
                            participant = null;
                        }

                        if (result == ReturnCode.Ok)
                        {
                            participantList.Add(participant);
                            if (myQos.EntityFactory.AutoenableCreatedEntities)
                            {
                                result = participant.Enable();
                            }
                        }
                    }
                }
                else
                {
                    ReportStack.Report(DDS.ReturnCode.BadParameter,
                                "DomainParticipant is using an invalid domain identifier (" + domainId + ").");
                }
            }

            if (result != ReturnCode.Ok && participant != null)
            {
                // Ignore result because we prefer the original error.
                DeleteParticipant(participant);
                participant = null;
            }

            ReportStack.Flush(null, result != ReturnCode.Ok);
            return participant;
        }

        /// <summary>
        /// This operation deletes an IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// An IDomainParticipant cannot
        /// be deleted when it has any attached Entity objects. When the operation is called
        /// on a IDomainParticipant with existing Entity objects, the operation returns
        /// DDS.ReturnCode PreconditionNotMet.
        /// </remarks>
        /// @param a_participant The IDomainParticipant that is to be deleted.
        /// @return DDS.ReturnCode Ok - The IDomainParticipant is deleted
        /// @return DDS.ReturnCode Error - An internal error has occurred
        /// @return DDS.ReturnCode BadParameter - The given IDomainParticipant is not valid
        /// @return DDS.ReturnCode OutOfResources - The Data Distribution Service ran out of
        ///                                         resources to complete this operation.
        /// @return DDS.ReturnCode PreconditionNotMet - The IDomainParticipant contains one
        ///                                             or more Entity objects.
        public ReturnCode DeleteParticipant(IDomainParticipant a_participant)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            ReportStack.Start();
            DomainParticipant participant = a_participant as DomainParticipant;
            if (participant != null)
            {
                lock(singleton_lock)
                {
                    if (participantList.Remove(participant))
                    {
                        result = participant.wlReq_deinit();
                        if (result != DDS.ReturnCode.Ok)
                        {
                            participantList.Add(participant);
                        }
                    }
                    else
                    {
                        /* Since there is only one DomainParticipantFactory, the participant
                         * has to be created by this one. If we can't find it anymore, it
                         * must have beel deleted already.
                         */
                        result = DDS.ReturnCode.BadParameter;
                        ReportStack.Report(result, "DomainParticipant was already deleted.");
                    }
                }
            }
            else
            {
                result = ReturnCode.BadParameter;
                ReportStack.Report(result, "Participant parameter 'a_participant' is null.");
            }

            ReportStack.Flush(null, result != DDS.ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// This operation retrieves a previously created IDomainParticipant belonging to
        /// the specified domainId.
        /// </summary>
        /// <remarks>
        /// The domainId used to search for a specific IDomainParticipant must be
        /// identical to the domainId that was used to create that specific
        /// IDomainParticipant.
        ///
        /// If multiple IDomainParticipant entities belonging to the specified domainId
        /// exist, then the operation will return one of them. It is not specified which one.
        ///
        /// If no IDomainParticipant was found, the operation will return NULL.
        /// </remarks>
        /// @param domainId The ID of the Domain to which the IDomainParticipant is joined.
        ///                 This should be the ID as specified in the configuration file.
        /// @return The retrieved IDomainParticipant. If no such IDomainParticipant is found
        ///         a null is returned.
        public IDomainParticipant LookupParticipant(DomainId domainId)
        {
            DomainParticipant participant;

            if (domainId == DDS.DomainId.Default) {
                domainId = DDS.OpenSplice.User.u.GetDomainIdFromEnvUri();
            }
            lock(singleton_lock)
            {
                participant = participantList.Find(
                    delegate (DomainParticipant a_participant)
                    {
                        return a_participant.DomainId == domainId;
                    }
                );
            }
            return participant;
        }

        /// <summary>
        /// This operation sets the default DomainParticipantQos of the DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation sets the default DomainParticipantQos of the DomainParticipantFactory
        /// (that is the struct with the QosPolicy settings) which is used for newly created
        /// IDomainParticipant objects, in case no QoS was provided in the CreateParticipant operation.
        ///
        /// The DomainParticipantQos is always self consistent, because its policies do not
        /// depend on each other. <br>
        /// This means this operation never returns the ReturnCode InconsistentPolicy.
        ///
        /// The values set by this operation are returned by GetDefaultParticipantQos().
        /// </remarks>
        /// @param qos The DomainParticipantQos which contains the new default
        ///            DomainParticipantQos for the newly created DomainParticipants
        /// @return DDS.ReturnCode Ok - The new default DomainParticipantQos is set
        /// @return DDS.ReturnCode Error - An internal error has occured
        /// @return DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation
        public ReturnCode SetDefaultParticipantQos(DomainParticipantQos qos)
        {
            ReturnCode result;

            lock(singleton_lock)
            {
                ReportStack.Start();
                result = QosManager.checkQos(qos);
                if (result == DDS.ReturnCode.Ok) {
                    using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                        new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
                    {
                        result = marshaler.CopyIn(qos);
                        if (result == ReturnCode.Ok)
                        {
                            marshaler.CopyOut(ref defaultParticipantQos);
                            // Listener scheduling qospolicy is not part of User Layer, so obtain it separately from participant.
                            if (qos.ListenerScheduling.SchedulingClass == null)
                            {
                                return ReturnCode.BadParameter;
                            }
                            this.defaultParticipantQos.ListenerScheduling.SchedulingClass.Kind = qos.ListenerScheduling.SchedulingClass.Kind;
                            if (qos.ListenerScheduling.SchedulingPriorityKind == null)
                            {
                                return ReturnCode.BadParameter;
                            }
                            this.defaultParticipantQos.ListenerScheduling.SchedulingPriorityKind.Kind = qos.ListenerScheduling.SchedulingPriorityKind.Kind;
                            this.defaultParticipantQos.ListenerScheduling.SchedulingPriority = qos.ListenerScheduling.SchedulingPriority;
                        }
                    }
                }
                ReportStack.Flush(null, result != ReturnCode.Ok);
            }
            return result;
        }

        /// <summary>
        /// This operation gets the default DomainParticipantQos of the DomainParticipantFactory
        /// </summary>
        /// <remarks>
        /// This operation gets the default DomainParticipantQos of the
        /// DomainParticipantFactory (that is the struct with the QosPolicy settings)
        /// which is used for newly created IDomainParticipant objects, in case no QoS is used
        /// in the CreateParticipant operation.
        ///
        /// The application must provide the DomainParticipantQos struct in which the
        /// QosPolicy settings can be stored and provide a reference to the struct. The
        /// operation writes the default QosPolicy settings to the struct referenced to by qos.<br>
        /// Any settings in the struct are overwritten.<br>
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultParticipantQos(), or, if the call was never
        /// made, the default values as specified for each QosPolicy setting.
        /// </remarks>
        /// @param qos A reference to the DomainParticipantQos in which the default
        ///            DomainParticipantQos for the IDomainParticipant is written
        /// @return DDS.ReturnCode Ok - The new default DomainParticipantQos is set
        /// @return DDS.ReturnCode Error - An internal error has occured
        /// @return DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation
        public ReturnCode GetDefaultParticipantQos(ref DomainParticipantQos qos)
        {
            ReturnCode result;

            using (OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler marshaler =
                    new OpenSplice.CustomMarshalers.DomainParticipantQosMarshaler())
            {
                lock(singleton_lock)
                {
                    ReportStack.Start();
                    result = marshaler.CopyIn(defaultParticipantQos);
                    if (result == ReturnCode.Ok)
                    {
                        marshaler.CopyOut(ref qos);
                        // Listener scheduling qospolicy is not part of User Layer, so obtain it separately from participant.
                        if (qos.ListenerScheduling.SchedulingClass == null)
                        {
                            qos.ListenerScheduling.SchedulingClass = new SchedulingClassQosPolicy();
                        }
                        qos.ListenerScheduling.SchedulingClass.Kind = this.defaultParticipantQos.ListenerScheduling.SchedulingClass.Kind;
                        if (qos.ListenerScheduling.SchedulingPriorityKind == null)
                        {
                            qos.ListenerScheduling.SchedulingPriorityKind = new SchedulingPriorityQosPolicy();
                        }
                        qos.ListenerScheduling.SchedulingPriorityKind.Kind = this.defaultParticipantQos.ListenerScheduling.SchedulingPriorityKind.Kind;
                        qos.ListenerScheduling.SchedulingPriority = this.defaultParticipantQos.ListenerScheduling.SchedulingPriority;
                    }
                    ReportStack.Flush(null, result != ReturnCode.Ok);
                }
            }

            return result;
        }

        /// <summary>
        /// This operation replaces the existing set of QoS settings for the DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a
        /// DomainParticipantFactory. The parameter qos must contain the struct with
        /// the QosPolicy settings.
        ///
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned Ok).
        /// </remarks>
        /// @param qos The new set of Qos policy settings for the DomainParticipantFactory
        /// @return DDS.ReturnCode Ok - The new DomainParticipantFactoryQos is set
        /// @return DDS.ReturnCode Error - An internal error has occured
        /// @return DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation
        public ReturnCode SetQos(DomainParticipantFactoryQos qos)
        {
            lock(singleton_lock)
            {
                myQos.EntityFactory.AutoenableCreatedEntities = qos.EntityFactory.AutoenableCreatedEntities;
                return ReturnCode.Ok;
            }
        }

        /// <summary>
        /// This operation obtains the QoS settings for the DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a
        /// DomainParticipantFactory on which this operation is used. This
        /// DomainParticipantFactoryQos is stored at the location pointed to by the qos
        /// parameter.
        /// </remarks>
        /// @param qos A reference to the destination DomainParticipantFactoryQos,
        ///            in which the Qos policies will be copied
        /// @return DDS.ReturnCode Ok - The new DomainParticipantFactoryQos is set
        /// @return DDS.ReturnCode Error - An internal error has occured
        /// @return DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation
        public ReturnCode GetQos(ref DomainParticipantFactoryQos qos)
        {
            lock(singleton_lock)
            {
                if (qos == null) qos = new DomainParticipantFactoryQos();
                if (qos.EntityFactory == null) qos.EntityFactory = new EntityFactoryQosPolicy();
                qos.EntityFactory.AutoenableCreatedEntities = myQos.EntityFactory.AutoenableCreatedEntities;
                return ReturnCode.Ok;
            }
        }

        /// <summary>
        /// This operation deletes all of the Entity objects that were created on the
        /// DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation deletes all of the Entity objects that were created on the
        /// DomainParticipantFactory (it deletes all contained IDomainParticipant
        /// objects). Prior to deleting each contained Entity, this operation regressively calls
        /// the corresponding DeleteContainedEntities operation on each Participant.<br>
        /// In other words, this operation cleans up all Entity objects in the
        /// process.
        ///
        /// @note The operation will return DDS.ReturnCode PreconditionNotMet if the any of the
        /// contained entities is in a state where it cannot be deleted. This will occur, for
        /// example, if a contained IDataReader cannot be deleted because the application has
        /// called a read or take operation and has not called the corresponding
        /// ReturnLoan operation to return the loaned samples. In such cases, the operation
        /// does not roll back any entity deletions performed prior to the detection of the
        /// problem.
        /// </remarks>
        /// @return DDS.ReturnCode Ok - The contained Entity objects are deleted and the application may
        ///                             delete the DomainParticipant.
        /// @return DDS.ReturnCode Error - An internal error has occurred.
        /// @return DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.
        /// @return DDS.ReturnCode PreconditionNotMet - One or more of the contained entities are
        ///                                             in a state where they cannot be deleted.
        public ReturnCode DeleteContainedEntities()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            lock(singleton_lock)
            {
                foreach (DomainParticipant p in participantList) {
                    result = p.DeleteContainedEntities();
                    if (result == DDS.ReturnCode.Ok)
                    {
                        result = p.deinit();
                    }
                    if (result != DDS.ReturnCode.Ok)
                    {
                        break;
                    }
                }
            }

            if (result == DDS.ReturnCode.Ok)
            {
                participantList.Clear();
            }

            return ReturnCode.Ok;
        }

        /// <summary>
        /// This operation safely detaches the application from all domains it is currently
        /// participating in.
        /// </summary>
        /// <remarks>
        /// @note This is a proprietary OpenSplice extension.
        ///
        /// This operation safely detaches the application from all domains it is currently
        /// participating in. When this operation has been performed successfully,
        /// the application is no longer connected to any Domain.
        /// - For Federated domains finishing this operation successfully means that all shared
        /// memory segments have been safely un-mapped from the application process.
        /// - For SingleProcess mode domains this means all services for all domains have been
        /// stopped. This allows graceful termination of the OSPL services that run as threads
        /// within the application. Graceful termination of services in this mode would for
        /// instance allow durability flushing of persistent data and networking termination
        /// announcement over the network.
        ///
        /// When this call returns further access to all domains will be denied and it will
        /// not be possible for the application to open or re-open any DDS domain.
        ///
        /// The behavior of the DetachAllDomains operation is determined by the blockOperations
        /// and deleteEntities parameters:<br>
        /// - blockOperations:
        ///     This parameter specifies if the application wants any DDS operation to be blocked
        ///     or not while detaching. When true, any DDS operation called during this operation
        ///     will be blocked and remain blocked forever (so also after the detach operation has
        ///     completed and returns to the caller). When false, any DDS operation called during
        ///     this operation may return AlreadyDeleted. Please note that a listener
        ///     callback is not considered an operation in progress. Of course, if a DDS operation
        ///     is called from within the listener callback, that operation will be blocked
        ///     during the detaching if this attribute is set to true.
        /// - deleteEntities:
        ///     This parameter specifies if the application wants the DDS entities created by
        ///     the application to be deleted (synchronously) while detaching from the domain or
        ///     not. If true, all application entities are guaranteed to be deleted when the call
        ///     returns. If false, application entities will not explicitly be deleted by this
        ///     operation. In case of federated mode, the splice-daemon will delete them
        ///     asynchronously after this operation has returned. In case of SingleProcess mode
        ///     this attribute is ignored and clean up will always be performed, as this cannot
        ///     be delegated to a different process.
        ///
        /// @note In federated mode when the DetachAllDomains operation is called with
        /// blockOperations is false and deleteEntities is false then the DDS operations
        /// which are in progress and which are waiting for some condition to become true
        /// or waiting for an event to occur while the detach operation is performed may be
        /// blocked.
        /// </remarks>
        /// @param blockOperations
        ///   Indicates whether the application wants any operations that are called while
        ///   detaching to be blocked or not.
        /// @param deleteEntities
        ///   Indicates whether the application DDS entities in the 'connected' domains must
        ///   be deleted synchronously during detaching.
        /// @return DDS.ReturnCode Ok - The application is detached from all domains
        public ReturnCode DetachAllDomains(bool blockOperations, bool deleteEntities)
        {
            uint flags = 0;

            if (blockOperations) {
                flags |= DDS.OpenSplice.User.u.BLOCK_OPERATIONS;
            }
            if (deleteEntities) {
                flags |= DDS.OpenSplice.User.u.DELETE_ENTITIES;
            }

			return SacsSuperClass.uResultToReturnCode(DDS.OpenSplice.User.u.userDetach(flags));
        }

        /// @cond
        static void ProcessExit(object sender, EventArgs args)
        {
            try {
	        DDS.OpenSplice.User.u.userDetach(DDS.OpenSplice.User.u.DELETE_ENTITIES);
            } catch (Exception e) {
                Console.WriteLine(e.ToString());
            }
        }
        /// @endcond
    }
}
