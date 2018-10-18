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

namespace DDS
{
    // ----------------------------------------------------------------------
    // Conditions
    // ----------------------------------------------------------------------
    /// <summary>
    /// Base class for all Conditions that maybe attached to an IWaitSet.
    /// </summary>
    /// <remarks>
    /// This class is the base class for all the conditions that may be attached to an IWaitSet.
    /// This base class is specialized in three classes by the Data Distribution Service:
    /// IGuardCondition, IStatusCondition and IReadCondition (also there is a
    /// IQueryCondition which is a specialized IReadCondition).
    /// Each Condition has a TriggerValue that can be true or false and is set by
    /// the Data Distribution Service (except an IGuardCondition) depending on the
    /// evaluation of the Condition.
    /// @see @ref DCPS_Modules_Infrastructure_Waitset
    /// </remarks>
    public interface ICondition
    {
        /// <summary>
        /// Each ICondition has a TriggerValue that can be true or false and is set
        /// by the DDS depending on the evaluation of the ICondition.
        /// </summary>
        /// <returns>The TriggerValue (true|false)</returns>
        bool GetTriggerValue();
    }

    /// <summary>
    /// An IWaitSet object allows an application to wait until one or more of the attached
    /// ICondition objects evaluates to true or until the timeout expires.
    /// </summary>
    /// <remarks>
    /// The IWaitSet has no factory and must be created by the application. It is directly
    /// created as an object by using the WaitSet constructor.
    /// @see @ref DCPS_Modules_Infrastructure_Waitset
    ///
    /// <code>
    /// /* Simplified example of the use of an WaitSet
    ///  * Defaults are used and possible errors are ignored. */
    ///
    /// /* Prepare Domain. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    ///
    /// /* Create waitset */
    /// IWaitSet waitset = new WaitSet();
    ///
    /// /* Add topic data type to the system. */
    /// DDS.ITypeSupport typesupport = new Space.FooTypeSupport();
    /// DDS.ReturnCode retcode = typesupport.RegisterType(participant, "Space.Foo");
    ///
    /// DDS.ITopic topic = participant.CreateTopic("SpaceFooTopic", "Space.Foo");
    ///
    /// /* Create typed datareader. */
    /// DDS.ISubscriber subscriber = participant.CreateSubscriber();
    /// Space.FooDataReader reader = (Space.FooDataReader)publisher.CreateDataReader(topic);
    ///
    /// /* Create querycondition */
    /// string[] params = new string[1];
    /// params[0] = "1";
    /// IQueryCondition condition = reader.CreateQueryCondition("field = %0", params);
    ///
    /// /* Attach condition to waitset */
    /// retcode = waitset.AttachCondition(condition);
    ///
    /// /* Wait for the condition to be triggered or a timeout occurs */
    /// Space.Foo[] samples = null;
    /// DDS.SampleInfo[] infos = null;
    ///
    /// DDS.ICondition[] conditions = new DDS.ICondition[1];
    ///
    /// retcode = waitset.Wait(ref conditions, new DDS.Duration(3, 0));
    /// if (retcode == DDS.ReturnCode.Ok) {
    ///     retcode = reader.ReadWithCondition(ref samples, ref infos, DDS.Length.Unlimited, condition);
    ///     ...
    /// }
    /// ...
    /// </code>
    /// </remarks>
    public interface IWaitSet
    {
        /// <summary>
        /// This operation allows an application thread to wait for the occurrence of at least one
        /// of the conditions that is attached to the IWaitSet.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation allows an application thread to wait for the occurrence of at least one
        /// of the conditions to evaluate to true that is attached to the WaitSet. If all of the
        /// conditions attached to the WaitSet have a TriggerValue of false, the wait operation will
        /// block the calling thread. The result of the operation is the continuation of the application
        /// thread after which the result is left in activeConditions. This is a reference to a sequence,
        /// which will contain the list of all the attached conditions that have a TriggerValue of true.
        /// </para><para>
        /// The parameter timeout specifies the maximum duration for the wait to block the calling
        /// application thread (when none of the attached conditions has a TriggerValue of true).
        /// In that case the return value is Timeout and the activeConditions sequence is left empty.
        /// Since it is not allowed for more than one application thread to be waiting on the same
        /// WaitSet, the operation returns immediately with the value PreconditionNotMet when the wait
        /// operation is invoked on a WaitSet which already has an application thread blocking on it.
        /// </para>
        /// </remarks>
        /// <param name="activeConditions">A sequence which is used to pass the list of all the attached
        /// conditions that have a TriggerValue of true.</param>
        /// <param name="timeout">The maximum duration to block for the wait, after which the application thread
        /// is unblocked. The special constant Infinite can be used when the maximum waiting time does not
        /// need to be bounded.</param>
        /// <returns>Possible return codes for the operation are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - At least one of the attached conditions has a TriggerValue of true.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode OutOfResources - The Data Distribution Service ran out of
        /// resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Timeout - The timeout has elapsed without any of the attached conditions
        /// becoming true.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - the WaitSet already has an application
        /// thread blocking on it.</item>
        /// </list>
        /// </returns>
        ReturnCode Wait(ref ICondition[] activeConditions, Duration timeout);
        /// <summary>
        /// This operation attaches a condition to the WaitSet.
        /// </summary>
        /// <remarks>
        /// This operation attaches an ICondition to the WaitSet. The parameter condition must be
        /// either an IReadCondition, IQueryCondition, IStatusCondition or IGuardCondition.
        /// To get this parameter see:
        /// <list type="bullet">
        /// <item>IReadCondition created by IDataReader.CreateReadCondition.</item>
        /// <item>IQueryCondition created by IDataReader.CreateQueryCondition.</item>
        /// <item>IStatusCondition retrieved by IEntity.StatusCondition on an IEntity.</item>
        /// <item>IGuardCondition created by the C# operation new.</item>
        /// </list>
        /// When an IGuardCondition is initially created, the TriggerValue is false.
        /// When an ICondition, whose TriggerValue evaluates to true, is attached to an
        /// IWaitSet that is currently being waited on (using the Wait operation), the IWaitSet
        /// will unblock immediately.
        /// </remarks>
        /// <param name="condition">The condition to be attached to the IWaitSet.
        /// The parameter must be either an IReadCondition, IQueryCondition, IStatusCondition or IGuardCondition</param>
        /// <returns>Possible return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Condition is attached to the IWaitSet.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter condition is not a valid ICondition.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode AttachCondition(ICondition condition);
        /// <summary>
        /// This operation detaches an ICondition from the IWaitSet.</summary>
        /// <remarks>
        /// If the ICondition was not attached to this IWaitSet, the operation returns PreconditionNotMet.
        /// </remarks>
        /// <param name="condition">The attached condition in the IWaitSet which is to be detached.</param>
        /// <returns>Possible return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The ICondition is detached from the IWaitSet.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter condition is not a valid ICondition.</item>
        /// <item>DDS.ReturnCode OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The ICondition was not attached to this IWaitSet.</item>
        /// </list>
        /// </returns>
        ReturnCode DetachCondition(ICondition condition);
        /// <summary>
        /// This operation retrieves the list of attached conditions.
        /// </summary>
        /// <remarks>
        /// This operation retrieves the list of attached conditions in the IWaitSet. The
        /// parameter attachedConditions is a reference to a sequence which afterwards
        /// will refer to the sequence of attached conditions.
        /// The resulting sequence will either be an empty sequence, meaning there were
        /// no conditions attached, or will contain a list of IReadCondition,
        /// IQueryCondition, IStatusCondition and IGuardCondition. These conditions
        /// previously have been attached by AttachCondition and were created by there
        /// respective create operation:
        /// <list type="bullet">
        /// <item>IReadCondition created by IDataReader.CreateReadCondition.</item>
        /// <item>IQueryCondition created by IDataReader.CreateQueryCondition.</item>
        /// <item>IStatusCondition retrieved by IEntity.StatusCondition on an IEntity.</item>
        /// <item>IGuardCondition created by the C# operation new.</item>
        /// </list>
        /// </remarks>
        /// <param name="attachedConditions">A reference to a sequence that will hold all the attached conditions
        /// on the IWaitSet</param>
        /// <returns>Possible return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The list of attached conditions is returned.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetConditions(ref ICondition[] attachedConditions);
    }

    /// <summary>
    /// An IGuardCondition object is a specific ICondition whose TriggerValue is
    /// completely under the control of the application. The IGuardCondition has no
    /// factory and must be created by the application. The IGuardCondition is directly
    /// created as an object by using the GuardCondition constructor. When a
    /// IGuardCondition is initially created, the TriggerValue is false. The purpose
    /// of the IGuardCondition is to provide the means for an application to manually
    /// wake up an IWaitSet. This is accomplished by attaching the IGuardCondition to
    /// the IWaitset and setting the TriggerValue by means of the
    /// IGuardCondition.SetTriggerValue operation.
    /// @see @ref DCPS_Modules_Infrastructure_Waitset
    /// </summary>
    public interface IGuardCondition : ICondition
    {
        /// <summary>
        /// This operation sets the TriggerValue of the IGuardCondition.
        /// </summary>
        /// <remarks>
        /// An IGuardCondition object is a specific ICondition which TriggerValue is
        /// completely under the control of the application. This operation must be used by the
        /// application to manually wake-up an IWaitSet. This operation sets the
        /// TriggerValue of the IGuardCondition to the parameter value.
        /// When an IGuardCondition is initially created, the TriggerValue is false.
        /// </remarks>
        /// <param name="value">The boolean value to which the IGuardCondition is set.</param>
        /// <returns>Possible return codes of the operation are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The specified TriggerValue has successfully been applied.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// </list>
        /// </returns>
        ReturnCode SetTriggerValue(bool value);
    }

    /// <summary>
    /// Specialized class of ICondition, which allows accesss and to be triggered on the related
    /// communication statuses of an IEntity
    /// </summary>
    /// <remarks>
    /// IEntity objects that have status attributes also have a StatusCondition, access is
    /// provided to the application by the GetStatusCondition operation.
    /// The communication statuses whose changes can be communicated to the application
    /// depend on the IEntity.
    /// <list type="table">
    /// <listheader><term>Entity</term><term>Status Kind</term><term>Status Name</term></listheader>
    /// <item><term rowspan="2">ITopic</term><term>DDS.StatusKind InconsistentTopic</term><term>InconsistentTopicStatus</term></item>
    /// <item><term>DDS.StatusKind AllDataDisposed</term></item>
    /// <item><term>ISubscriber</term><term>DDS.StatusKind DataOnReaders</term></item>
    /// <item><term rowspan="7">IDataReader</term><term>DDS.StatusKind SampleRejected</term><term>SampleRejectedStatus</term></item>
    /// <item><term>DDS.StatusKind LivelinessChanged</term><term>LivelinessChangedStatus</term></item>
    /// <item><term>DDS.StatusKind RequestedDeadlineMissed</term><term>RequestedDeadlineMissedStatus</term></item>
    /// <item><term>DDS.StatusKind RequestedIncompatibleQos</term><term>RequestedIncompatibleQosStatus</term></item>
    /// <item><term>DDS.StatusKind DataAvailable</term></item>
    /// <item><term>DDS.StatusKind SampleLost</term><term>SampleLostStatus</term></item>
    /// <item><term>DDS.StatusKind SubscriptionMatched</term><term>SubscriptionMatchedStatus</term></item>
    /// <item><term rowspan="4">IDataWriter</term><term>DDS.StatusKind LivelinessLost</term><term>LivelinessLostStatus</term></item>
    /// <item><term>DDS.StatusKind OfferedDeadlineMissed</term><term>OfferedDeadlineMissedStatus</term></item>
    /// <item><term>DDS.StatusKind OfferedIncompatibleQos</term><term>OfferedIncompatibleQosStatus</term></item>
    /// <item><term>DDS.StatusKind PublicationMatched</term><term>PublicationMatchedStatus</term></item>
    /// </list>
    /// <para>
    /// The TriggerValue of the IStatusCondition depends on the communication
    /// statuses of that IEntity (e.g., missed deadline) and also depends on the value of the
    /// IStatusCondition attribute mask (enabled statuses mask). An
    /// IStatusCondition can be attached to an IWaitSet in order to allow an application
    /// to suspend until the TriggerValue has become true.
    /// </para><para>
    /// The TriggerValue of an IStatusCondition will be true if one of the enabled
    /// StatusChangedFlags is set. That is, the TriggerValue is false only if all the
    /// values of the StatusChangedFlags are false.
    /// The sensitivity of the IStatusCondition to a particular communication status is
    /// controlled by the list of enabled statuses set on the condition by means of the
    /// IStatusCondition.SetEnabledStatuses operation.
    /// </para><para>
    /// When the enabled statuses are not changed by the IStatusCondition.SetEnabledStatuses
    /// operation, all statuses are enabled by default.
    /// </para>
    /// </remarks>
    public interface IStatusCondition : ICondition
    {
        /// <summary>
        /// This operation returns the list of enabled communication statuses of the IStatusCondition.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The TriggerValue of the IStatusCondition depends on the communication
        /// status of that IEntity (e.g., missed deadline, loss of information, etc.), ‘filtered’ by
        /// the set of EnabledStatuses on the IStatusCondition.
        /// </para><para>
        /// This operation returns the list of communication statuses that are taken into account
        /// to determine the TriggerValue of the IStatusCondition. This operation
        /// returns the statuses that were explicitly set on the last cal l to
        /// IStatusCondition.SetEnabledStatuses or, if IStatusCondition.SetEnabledStatuses was never called, the
        /// default list.
        /// </para><para>
        /// The result value is a bit mask in which each bit shows which status is taken into
        /// account for the IStatusCondition. The relevant bits represents one of the
        /// following statuses:
        /// <list type="bullet">
        /// <item>DDS.StatusKind InconsitentTopic</item>
        /// <item>DDS.StatusKind AllDataDisposed</item>
        /// <item>DDS.StatusKind OfferedDeadlineMissed</item>
        /// <item>DDS.StatusKind RequestedDeadlineMissed</item>
        /// <item>DDS.StatusKind OfferedIncompatibleQos</item>
        /// <item>DDS.StatusKind RequestedIncompatibleQos</item>
        /// <item>DDS.StatusKind SampleLost</item>
        /// <item>DDS.StatusKind SampleRejected</item>
        /// <item>DDS.StatusKind DataOnReaders</item>
        /// <item>DDS.StatusKind DataAvailable</item>
        /// <item>DDS.StatusKind LivelinessLost</item>
        /// <item>DDS.StatusKind LivelinessChanged</item>
        /// <item>DDS.StatusKind PublicationMatched</item>
        /// <item>DDS.StatusKind SubscriptionMatched</item>
        /// </list>
        /// Each status bit is declared as a constant and can be used in an AND operation to
        /// check the status bit against the result of type int.
        /// Not all statuses are relevant to all IEntity objects. See the respective Listener
        /// objects for each IEntity for more information.
        /// </remarks>
        /// <returns>DDS.StatusKind - A bit mask in which each bit shows which status is taken into account
        /// for the IStatusCondition</returns>
        StatusKind GetEnabledStatuses();

        /// <summary>
        /// This operation sets the list of communication statuses that are taken into account to
        /// determine the TriggerValue of the IStatusCondition.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The TriggerValue of the IStatusCondition depends on the communication
        /// status of that IEntity (e.g., missed deadline, loss of information, etc.), ‘filtered’ by
        /// the set of enabled statuses on the IStatusCondition.
        /// This operation sets the list of communication statuses that are taken into account to
        /// determine the TriggerValue of the IStatusCondition. This operation may
        /// change the TriggerValue of the IStatusCondition.
        /// </para><para>
        /// IWaitSet objects behaviour depend on the changes of the TriggerValue of their
        /// attached IConditions. Therefore, any IWaitSet to which the IStatusCondition
        /// is attached is potentially affected by this operation.
        /// If this function is not invoked, the default list of enabled statuses includes all
        /// the statuses.
        /// </para><para>
        /// The parameter mask is a bit mask in which each bit shows which status is taken into
        /// account for the IStatusCondition. The relevant bits represents one of the
        /// following statuses:
        /// <list type="bullet">
        /// <item>DDS.StatusKind InconsitentTopic</item>
        /// <item>DDS.StatusKind AllDataDisposed</item>
        /// <item>DDS.StatusKind OfferedDeadlineMissed</item>
        /// <item>DDS.StatusKind RequestedDeadlineMissed</item>
        /// <item>DDS.StatusKind OfferedIncompatibleQos</item>
        /// <item>DDS.StatusKind RequestedIncompatibleQos</item>
        /// <item>DDS.StatusKind SampleLost</item>
        /// <item>DDS.StatusKind SampleRejected</item>
        /// <item>DDS.StatusKind DataOnReaders</item>
        /// <item>DDS.StatusKind DataAvailable</item>
        /// <item>DDS.StatusKind LivelinessLost</item>
        /// <item>DDS.StatusKind LivelinessChanged</item>
        /// <item>DDS.StatusKind PublicationMatched</item>
        /// <item>DDS.StatusKind SubscriptionMatched</item>
        /// </list>
        /// Each status bit is declared as a constant and can be used in an OR operation to set
        /// the status bit in the parameter mask of type int.
        /// Not all statuses are relevant to all IEntity objects. See the respective Listener
        /// objects for each IEntity for more information.
        /// </remarks>
        /// <param name="mask">A bit mask in which each bit sets the status which is taken into
        /// account to determine the TriggerValue of the IStatusCondition</param>
        /// <returns>Possible return codes of the operation are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The list of communication statuses is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IStatusCondition has already been deleted.</item>
        /// </list>
        /// </returns>
        ReturnCode SetEnabledStatuses(StatusKind mask);

        /// <summary>
        /// This operation returns the IEntity associated with the IStatusCondition or a
        /// null reference.
        /// </summary>
        /// <remarks>
        /// This operation returns the IEntity associated with the IStatusCondition. Note
        /// that there is exactly one IEntity associated with each IStatusCondition. When
        /// the IEntity was already deleted (there is no associated IEntity any more), the
        /// value null is returned.</remarks>
        /// <returns>IEntity - The IEntity associated with the IStatusCondition.</returns>
        IEntity GetEntity();
    }

    /// <summary>
    /// IReadCondition objects allow an IDataReader to specify the data samples it is
    /// interested in
    /// </summary>
    /// <remarks>
    /// The IDataReader objects can create a set of IReadCondition (and
    /// IStatusCondition) objects which provide support (in conjunction with IWaitSet
    /// objects) for an alternative communication style between the Data Distribution
    /// Service and the application (i.e., state-based rather than event-based).
    /// IReadCondition objects allow an IDataReader to specify the data samples it is
    /// interested in (by specifying the desired sample-states, view-states, and
    /// instance-states); see the parameter definitions for IDataReader.CreateReadCondition
    /// operation. This allows the Data Distribution Service to trigger the condition
    /// only when suitable information is available. IReadCondition objects are to be
    /// used in conjunction with an IWaitSet. More than one IReadCondition may be attached
    /// to the same IDataReader.
    /// @see @ref DCPS_Modules_Infrastructure_Waitset
    ///
    /// <code>
    /// /* Simplified example of the creation of an ReadCondition
    ///  * Defaults are used and possible errors are ignored. */
    ///
    /// /* Prepare Domain. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    ///
    /// /* Create waitset */
    /// IWaitSet waitset = new WaitSet();
    ///
    /// /* Add topic data type to the system. */
    /// DDS.ITypeSupport typesupport = new Space.FooTypeSupport();
    /// DDS.ReturnCode retcode = typesupport.RegisterType(participant, "Space.Foo");
    ///
    /// DDS.ITopic topic = participant.CreateTopic("SpaceFooTopic", "Space.Foo");
    ///
    /// /* Create typed datareader. */
    /// DDS.ISubscriber subscriber = participant.CreateSubscriber();
    /// Space.FooDataReader reader = (Space.FooDataReader)publisher.CreateDataReader(topic);
    ///
    /// /* Create readcondition */
    /// IReadCondition condition = reader.CreateReadCondition(DDS.SampleStateKind.Read, DDS.ViewStateKind.New, DDS.InstanceStateKind.Alive);
    ///
    /// /* Attach condition to waitset */
    /// retcode = waitset.AttachCondition(condition);
    /// </code>
    /// </remarks>
    public interface IReadCondition : ICondition
    {
        /// <summary>
        /// This operation returns the set of SampleStates that are taken into account to
        /// determine the TriggerValue of the IReadCondition.
        /// </summary>
        /// <remarks>
        /// The SampleStates returned are the SampleStates specified when the
        /// IReadCondition was created. The SampleStates can be Read,
        /// NotRead or both (<see cref="DDS.SampleStateKind"/>).
        /// </remarks>
        /// <returns>The SampleStates specified when the IReadCondition was created.</returns>
        SampleStateKind GetSampleStateMask();

        /// <summary>
        /// This operation returns the set of ViewStates that are taken into account to
        /// determine the TriggerValue of the IReadCondition.
        /// </summary>
        /// <remarks>
        /// The ViewStates returned are the ViewStates specified when the
        /// IReadCondition was created. ViewStates can be New, NotNew or both (<see cref="DDS.ViewStateKind"/>).
        /// </remarks>
        /// <returns>The ViewStates specified when the IReadCondition was created.</returns>
        ViewStateKind GetViewStateMask();

        /// <summary>
        /// This operation returns the set of InstanceStates that are taken into account to
        /// determine the TriggerValue of the IReadCondition.
        /// </summary>
        /// <remarks>
        /// The InstanceStates returned are the InstanceStates specified when the
        /// IReadCondition was created. InstanceStates can be Alive NotAlivedDisposed
        /// NotAliveNoWriters or a combination of these (<see cref="DDS.InstanceStateKind"/>).
        /// </remarks>
        /// <returns>The InstanceStates specified when the IReadCondition was created.</returns>
        InstanceStateKind GetInstanceStateMask();

        /// <summary>
        /// This operation returns the IDataReader associated with the IReadCondition.
        /// </summary>
        /// <remarks>
        /// Note that there is exactly one IDataReader associated with each IReadCondition (i.e.
        /// the IDataReader that created the IReadCondition object).
        /// </remarks>
        /// <returns>The IDataReader associated with the IReadCondition.</returns>
        IDataReader GetDataReader();
    }

    /// <summary>
    /// IQueryCondition objects are specialized IReadCondition objects that allow the
    /// application to specify a filter on the locally available data.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The IDataReader objects accept a set of IQueryCondition objects for the IDataReader
    /// and provide support (in conjunction with IWaitSet objects) for an alternative communication
    /// style between the Data Distribution Service and the application (i.e., state-based rather
    /// than event-based).
    /// </para>
    /// <para><b><i>Query Function</i></b>
    /// <para>
    /// IQueryCondition objects allow an application to specify the data samples it is
    /// interested in (by specifying the desired sample-states, view-states, instance-states
    /// and query expression); see the parameter definitions for IDataReader's
    /// Read/Take operations. This allows the Data Distribution Service to trigger the
    /// condition only when suitable information is available. IQueryCondition objects
    /// are to be used in conjunction with an IWaitSet. More than one IQueryCondition
    /// may be attached to the same IDataReader.
    /// </para><para>
    /// The query (queryExpression) is similar to an SQL WHERE clause and can be
    /// parameterized by arguments that are dynamically changeable with the
    /// IQueryCondition.SetQueryParameters operation.
    /// @see @ref DCPS_Modules_Infrastructure_Waitset
    /// </para>
    ///
    /// <code>
    /// /* Simplified example of the creation of an QueryCondition
    ///  * Defaults are used and possible errors are ignored. */
    ///
    /// /* Prepare Domain. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    ///
    /// /* Create waitset */
    /// IWaitSet waitset = new WaitSet();
    ///
    /// /* Add topic data type to the system. */
    /// DDS.ITypeSupport typesupport = new Space.FooTypeSupport();
    /// DDS.ReturnCode retcode = typesupport.RegisterType(participant, "Space.Foo");
    ///
    /// DDS.ITopic topic = participant.CreateTopic("SpaceFooTopic", "Space.Foo");
    ///
    /// /* Create typed datareader. */
    /// DDS.ISubscriber subscriber = participant.CreateSubscriber();
    /// Space.FooDataReader reader = (Space.FooDataReader)publisher.CreateDataReader(topic);
    ///
    /// /* Create querycondition */
    /// string[] params = new string[1];
    /// params[0] = "1";
    /// IQueryCondition condition = reader.CreateQueryCondition("field = %0", params);
    ///
    /// /* Attach condition to waitset */
    /// retcode = waitset.AttachCondition(condition);
    /// </code>
    /// </remarks>
    public interface IQueryCondition : IReadCondition
    {
        /// <summary>
        /// This operation returns the query expression associated with the IQueryCondition.
        /// </summary>
        /// <remarks>
        /// That is, the expression specified when the IQueryCondition was created. The
        /// operation will return null when there was an internal error or when the
        /// IQueryCondition was already deleted.
        /// </remarks>
        /// <returns>The query expression associated with the IQueryCondition.</returns>
        string GetQueryExpression();

        /// <summary>
        /// This operation obtains the query parameters associated with the IQueryCondition
        /// </summary>
        /// <remarks>
        /// <para>
        /// That is, the parameters specified on the last successful call to
        /// IQueryCondition.SetQueryArguments or, if SetQueryArguments was never called,
        /// the arguments specified when the IQueryCondition were created.
        /// </para><para>
        /// The queryParameters parameter will contain a sequence of strings with the parameters
        /// used in the SQL expression (i.e., the %n tokens in the expression). The number of
        /// parameters in the result sequence will exactly match the number of %n tokens in
        /// the query expression associated with the IQueryCondition.
        /// </para>
        /// </remarks>
        /// <param name="queryParameters">A reference to a sequence of strings that will be
        /// used to store the parameters used in the SQL expression</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of query parameters applied to this IQueryCondition
        /// has successfully been copied into the specified queryParameters parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IQueryCondition has already been deleted.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQueryParameters(ref string[] queryParameters);

        /// <summary>
        /// This operation changes the query parameters associated with the IQueryCondition.
        /// </summary>
        /// <remarks>
        /// This operation changes the query parameters associated with the IQueryCondition.
        /// The parameter queryParameters is a sequence of strings which are the parameter values
        /// used in the SQL query string (i.e., the %n tokens in the expression).
        /// The number of values in queryParameters must be equal or greater than the highest
        /// referenced %n token in the queryExpression (e.g. if %1 and %8 are used as parameter
        /// in the queryExpression, the queryParameters should at least contain n+1 = 9 values).
        /// </remarks>
        /// <param name="queryParameters">A sequence of strings which are the parameters used in the SQL query string
        /// (i.e., the %n tokens in the expression).</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The query parameters associated with the IQueryCondition are changed.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The number of parameters in queryParameters does not match
        /// the number of %n tokens in the expression for this IQueryCondition or one of
        /// the parameters is an illegal parameter.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IQueryCondition has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQueryParameters(params string[] queryParameters);
    }

    // ----------------------------------------------------------------------
    // Factory
    // ----------------------------------------------------------------------
    /// @cond
    /// The interface IDomainParticipantFactory is not very interesting for
    /// the documentation. Instead of that, the DomainParticipantFactory class
    /// itself has been documented.
    ///
    public interface IDomainParticipantFactory
    {
        IDomainParticipant CreateParticipant(DomainId domainId);
        IDomainParticipant CreateParticipant(DomainId domainId,
            IDomainParticipantListener listener, StatusKind mask);
        IDomainParticipant CreateParticipant(DomainId domainId, DomainParticipantQos qos);
        IDomainParticipant CreateParticipant(DomainId domainId, DomainParticipantQos qos,
            IDomainParticipantListener listener, StatusKind mask);

        ReturnCode DeleteParticipant(IDomainParticipant participant);

        IDomainParticipant LookupParticipant(DomainId domainId);

        ReturnCode SetDefaultParticipantQos(DomainParticipantQos qos);
        ReturnCode GetDefaultParticipantQos(ref DomainParticipantQos qos);

        ReturnCode SetQos(DomainParticipantFactoryQos qos);
        ReturnCode GetQos(ref DomainParticipantFactoryQos qos);

        ReturnCode DetachAllDomains(bool blockOperations, bool deleteEntities);
    }
    /// @endcond

    // ----------------------------------------------------------------------
    // Entities
    // ----------------------------------------------------------------------
    /// <summary>
    /// This class is the abstract base class for all the DCPS objects. It acts as a generic class
    /// for IEntity objects.
    /// </summary>
    public interface IEntity
    {
        /// <summary>
        /// This operation enables the IEntity on which it is being called when the IEntity was created
        /// with the EntityFactoryQosPolicy set to false.
        /// </summary>
        /// <remarks>
        /// This operation enables the IEntity. Created IEntity objects can start in either an
        /// enabled or disabled state. This is controlled by the value of the
        /// EntityFactoryQosPolicy on the corresponding factory for the IEntity.
        /// Enabled entities are immediately activated at creation time meaning all their
        /// immutable QoS settings can no longer be changed. Disabled Entities are not yet
        /// activated, so it is still possible to change there immutable QoS settings. However,
        /// once activated the immutable QoS settings can no longer be changed.
        ///
        /// Creating disabled entities can make sense when the creator of the IEntity does not
        /// yet know which QoS settings to apply, thus allowing another piece of code to set the
        /// QoS later on. This is for example the case in the DLRL, where the ObjectHomes
        /// create all underlying DCPS entities but do not know which QoS settings to apply.
        /// The user can then apply the required QoS settings afterwards.
        ///
        /// The default setting of EntityFactoryQosPolicy is such that, by default, entities
        /// are created in an enabled state so that it is not necessary to explicitly call enable on
        /// newly created entities.
        ///
        /// The enable operation is idempotent. Calling enable on an already enabled
        /// IEntity returns OK and has no effect.
        ///
        /// If an IEntity has not yet been enabled, the only operations that can be invoked on it
        /// are: the ones to set, get or copy the QosPolicy settings, the ones that set (or get) the
        /// listener, the ones that get the IStatusCondition, the GetStatusChanges
        /// operation (although the status of a disabled entity never changes), and the factory
        /// operations that create, delete or lookup(This includes the LookupTopicDescription() but not FindTopic())
        /// other Entities. Other operations will return the error NotEnabled.
        ///
        /// Entities created from a factory that is disabled, are created disabled regardless of
        /// the setting of the EntityFactoryQosPolicy.
        /// Calling enable on an IEntity whose factory is not enabled will fail and return
        /// PreconditionNotMet.
        ///
        /// If the EntityFactoryQosPolicy has AutoenableCreatedEntities set to
        /// true, the enable operation on the factory will automatically enable all Entities
        /// created from the factory.
        ///
        /// The Listeners associated with an IEntity are not called until the IEntity is
        /// enabled. Conditions associated with an IEntity that is not enabled are "inactive",
        /// that is, have a TriggerValue which is false.
        ///
        /// In addition to the general description, the enable operation on a Subscriber has special meaning
        /// in specific usecases. This applies only to Subscribers with PresentationQoS coherent-access set to
        /// true with access-scope set to group.
        ///
        /// In this case the subscriber is always created in a disabled state, regardless of the factory's auto-enable
        /// created entities setting. While the subscriber remains disabled, DataReaders can be created that will
        /// participate in coherent transactions of the subscriber (See @ref DDS.ISubscriber.BeginAccess()) and
        /// @ref DDS.ISubscriber.EndAccess() operations for more information).
        ///
        /// All DataReaders will also be created in a disabled state. Coherency with group access-scope requires data
        /// to be delivered as a transaction, atomically, to all eligible readers. Therefore data should not be delivered
        /// to any single DataReader immediately after it's created, as usual, but only after the application has finished
        /// creating all DataReaders for a given Subscriber. At this point, the application should enable the Subscriber
        /// which in turn enables all its DataReaders.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Application enabled the IEntity (or it was already enabled)</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The factory of the IEntity is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode Enable();

        /// <summary>
        /// This property allows access to the IStatusCondition associated with the IEntity.
        /// </summary>
        /// <remarks>
        /// Each IEntity has a IStatusCondition associated with it. This operation allows
        /// access to the IStatusCondition associated with the IEntity. The returned
        /// condition can then be added to a WaitSet so that the application can wait for
        /// specific status changes that affect the IEntity.
        /// </remarks>
        IStatusCondition StatusCondition { get; }

        /// <summary>
        /// This operation returns a mask with the communication statuses in the IEntity that are triggered.
        /// </summary>
        /// <remarks>
        /// This operation returns a mask with the communication statuses in the IEntity that
        /// are triggered. That is the set of communication statuses whose value have changed
        /// since the last time the application called this operation. This operation shows
        /// whether a change has occurred even when the status seems unchanged because the
        /// status changed back to the original status.
        ///
        /// When the IEntity is first created or if the IEntity is not enabled, all
        /// communication statuses are in the un-triggered state so the mask returned by the
        /// operation is empty.
        ///
        /// The result value is a bit mask in which each bit shows which value has changed.
        /// - DDS.StatusKind InconsistentTopic
        /// - DDS.StatusKind OfferedDeadlineMissed
        /// - DDS.StatusKind RequestedDeadlineMissed
        /// - DDS.StatusKind OfferedIncompatibleQos
        /// - DDS.StatusKind RequestedIncompatibleQos
        /// - DDS.StatusKind SampleLost
        /// - DDS.StatusKind SampleRejected
        /// - DDS.StatusKind DataOnReaders
        /// - DDS.StatusKind DataAvailable
        /// - DDS.StatusKind LivelinessLost
        /// - DDS.StatusKind LivelinessChanged
        /// - DDS.StatusKind PublicationMatched
        /// - DDS.StatusKind SubscriptionMatched
        ///
        /// Each status bit is declared as a constant and can be used in an AND operation to
        /// check the status bit against the result of type StatusMask. Not all statuses are
        /// relevant to all IEntity objects. See the respective Listener interfaces for each
        /// IEntity for more information.
        /// </remarks>
        StatusKind StatusChanges { get; }

        /// <summary>
        /// This operation returns the InstanceHandle of the builtin topic sample that represents the specified IEntity.
        /// </summary>
        /// <remarks>
        /// The relevant state of some IEntity objects are distributed using builtin topics. Each
        /// builtin topic sample represents the state of a specific IEntity and has a unique
        /// instanceHandle. This operation returns the instanceHandle of the builtin
        /// topic sample that represents the specified IEntity.
        ///
        /// Some Entities (IPublisher and ISubscriber) do not have a corresponding
        /// builtin topic sample, but they still have an instanceHandle that uniquely
        /// identifies the IEntity. The instanceHandles obtained this way can also be used
        /// to check whether a specific IEntity is located in a specific IDomainParticipant.
        /// </remarks>
        InstanceHandle InstanceHandle { get; }
    }

    /// <summary>
    /// All the DCPS IEntity objects are attached to a IDomainParticipant. A IDomainParticipant
    /// represents the local membership of the application in a Domain.
    /// </summary>
    /// <remarks>A Domain is a distributed concept that links all the applications that must be able to
    /// communicate with each other. It represents a communication plane: only the
    /// Publishers and the Subscribers attached to the same Domain can interact.
    /// This class implements several functions:
    /// <list type="bullet">
    /// <item>it acts as a container for all other IEntity objects</item>
    /// <item>it acts as a factory for the IPublisher, ISubscriber, ITopic,IContentFilteredTopic
    /// and IMultiTopic objects</item>
    /// <item>it provides access to the built-in ITopic objects</item>
    /// <item>it provides information about ITopic objects</item>
    /// <item>It isolates applications within the same Domain (sharing the same domainId)
    /// from other applications in a different Domain on the same set of computers. In this way,
    /// several independent distributed applications can coexist in the same physical network without interfering,
    /// or even being aware of each other.</item>
    /// <item>It provides administration services in the Domain, offering operations, which allow the application
    /// to ignore locally any information about a given Participant, Publication, Subscription or Topic.</item>
    /// </list>
    /// </remarks>
    public interface IDomainParticipant : IEntity
    {
        /// <summary>
        /// This method creates a IPublisher with default values.
        /// </summary>
        /// <remarks>
        /// This operation creates a IPublisher with the default PublisherQos, a null IPublisherListener
        /// and 0 StatusKind mask.
        ///
        /// If the SetDefaultPublisherQos() method is called, then the default PublisherQos will be the
        /// QoS given to that method. Otherwise it will equal a new PublisherQos.
        ///
        /// To delete the IPublisher the operation DeletePublisher() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreatePublisher(PublisherQos qos, IPublisherListener listener, StatusKind mask) "CreatePublisher"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <returns>The newly created IPublisher. In case of an error, a null IPublisher is returned.</returns>
        IPublisher CreatePublisher();

        /// <summary>
        /// This method creates a IPublisher and if applicable, attaches the optionally specified IPublisherListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a IPublisher with the default PublisherQos, the given IPublisherListener
        /// and StatusKind mask. The IPublisherListener may be null and the mask may be 0.
        ///
        /// If the SetDefaultPublisherQos() method is called, then the default PublisherQos will be the
        /// QoS given to that method. Otherwise it will equal a new PublisherQos.
        ///
        /// To delete the IPublisher the operation DeletePublisher() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreatePublisher(PublisherQos qos, IPublisherListener listener, StatusKind mask) "CreatePublisher"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="listener">The IPublisherListener instance which will be attached to the new IPublisher.
        /// It is permitted to use null as the value of the listener: this behaves as a PublisherListener
        /// whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the PublisherListener
        /// for a certain status.</param>
        /// <returns>The newly created IPublisher. In case of an error, a null IPublisher is returned.</returns>
        IPublisher CreatePublisher(
                IPublisherListener listener,
                StatusKind mask);

        /// <summary>
        /// This method creates a IPublisher with the desired QosPolicy settings, but without an IPublisherListener.
        /// </summary>
        /// <remarks>
        /// This operation creates a IPublisher with the given PublisherQos, a null IPublisherListener
        /// and 0 StatusKind mask.
        ///
        /// In case the specified QosPolicy settings are not consistent, no IPublisher is
        /// created and null is returned. Null can also be returned
        /// when insufficient access rights exist for the partition(s) listed in the provided QoS
        ///
        /// To delete the IPublisher the operation DeletePublisher() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreatePublisher(PublisherQos qos, IPublisherListener listener, StatusKind mask) "CreatePublisher"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings for the new IPublisher.
        /// In case these settings are not self consistent, no IPublisher is created.</param>
        /// <returns>The newly created IPublisher. In case of an error, a null IPublisher is returned.</returns>
        IPublisher CreatePublisher(PublisherQos qos);

        /// <summary>
        /// This operation creates a IPublisher with the desired QosPolicy settings and if applicable,
        /// attaches the optionally specified IPublisherListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a IPublisher with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified IPublisherListener to it. When the
        /// PublisherListener is not applicable, null must be supplied instead.
        ///
        /// In case the specified QosPolicy settings are not consistent, no IPublisher is
        /// created and null is returned. Null can also be returned
        /// when insufficient access rights exist for the partition(s) listed in the provided QoS
        ///
        /// To delete the IPublisher the operation DeletePublisher() or
        /// DeleteContainedEntities() must be used.
        ///
        /// <i><b>Communication Status</b></i><br>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask, the associated
        /// IPublisherListener operation is invoked and the communication
        /// status is reset to false, as the listener implicitly accesses the status which is passed
        /// as a parameter to that operation. The fact that the status is reset prior to calling the
        /// listener means that if the application calls the Get<status_name>Status from
        /// inside the listener it will see the status already reset.
        ///
        /// The following statuses are applicable to the IPublisher
        /// - DDS.StatusKind OfferedDeadlineMissed (propagated)
        /// - DDS.StatusKind OfferedIncompatibleQos (propagated)
        /// - DDS.StatusKind LivelinessLost (propagated)
        /// - DDS.StatusKind PublicationMatched (propagated)
        ///
        /// Be aware that the PublicationMatched
        /// status are not applicable when the infrastructure does not have the
        /// information available to determine connectivity. This is the case when OpenSplice
        /// is configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return null.
        ///
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The special constant 0 can
        /// be used to indicate that the created entity should not respond to any of its available
        /// statuses. The DDS will therefore attempt to propagate these statuses to its factory.
        ///
        /// <i><b>Status Propagation</b></i><br>
        /// The Data Distribution Service will trigger the most specific and relevant Listener.<br>
        /// In other words, in case a communication status is also activated on the
        /// IDataWriterListener of a contained IDataWriter, the IDataWriterListener
        /// on that contained IDataWriter is invoked instead of the IPublisherListener.
        /// This means that a status change on a contained IDataWriter only invokes the
        /// IPublisherListener if the contained IDataWriter itself does not handle the
        /// trigger event generated by the status change.
        ///
        /// In case a communication status is not activated in the mask of the
        /// IPublisherListener, the IDomainParticipantListener of the containing
        /// IDomainParticipant is invoked (if attached and activated for the status that
        /// occurred). This allows the application to set a default behaviour in the
        /// IDomainParticipantListener of the containing IDomainParticipant and a
        /// IPublisher specific behaviour when needed. In case the
        /// IDomainParticipantListener is also not attached or the communication status
        /// is not activated in its mask, the application is not notified of the change.
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings for the new IPublisher.
        /// In case these settings are not self consistent, no IPublisher is created.</param>
        /// <param name="listener">The IPublisherListener instance which will be attached to the new IPublisher.
        /// It is permitted to use null as the value of the listener: this behaves as a PublisherListener
        /// whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the PublisherListener
        /// for a certain status.</param>
        /// <returns>The newly created IPublisher. In case of an error, a null IPublisher is returned.</returns>
        IPublisher CreatePublisher(
                PublisherQos qos,
                IPublisherListener listener,
                StatusKind mask);

        /// <summary>
        /// This operation deletes a IPublisher.
        /// </summary>
        /// <remarks>
        /// This operation deletes a IPublisher. A IPublisher cannot be deleted when it has
        /// any attached IDataWriter objects. When the operation is called on a IPublisher
        /// with IDataWriter objects, the operation returns PreconditionNotMet.
        /// When the operation is called on a different IDomainParticipant, as used when the IPublisher
        /// was created, the operation has no effect and returns PreconditionNotMet.
        /// </remarks>
        /// <param name="p">The IPublisher to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IPublisher is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter p is not a valid IPublisher.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the DDS has ran out of resources to complete this operation. </item>
        /// <item>DDS.ReturnCode PreconditionNotMet - the operation is called on a different IDomainParticipant,
        /// as used when the IPublisher was created, or the IPublisher contains one or more IDataWriter objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeletePublisher(IPublisher p);

        /// <summary>
        /// This method creates a ISubscriber with default values.
        /// </summary>
        /// <remarks>
        /// This operation creates a ISubscriber with the default SubscriberQos, a null ISubscriberListener
        /// and 0 StatusKind mask.
        ///
        /// If the SetDefaultSubscriberQos() method is called, then the default SubscriberQos will be the
        /// QoS given to that method. Otherwise it will equal a new SubscriberQos.
        ///
        /// To delete the ISubscriber the operation DeleteSubscriber() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask) "CreateSubscriber"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <returns>The newly created ISubscriber. In case of an error, a null ISubscriber is returned.</returns>
        ISubscriber CreateSubscriber();

        /// <summary>
        /// This method creates a ISubscriber and if applicable, attaches the optionally specified ISubscriberListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a ISubscriber with the default SubscriberQos, the given ISubscriberListener
        /// and StatusKind mask. The ISubscriberListener may be null and the mask may be 0.
        ///
        /// If the SetDefaultSubscriberQos() method is called, then the default SubscriberQos will be the
        /// QoS given to that method. Otherwise it will equal a new SubscriberQos.
        ///
        /// To delete the ISubscriber the operation DeleteSubscriber() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask) "CreateSubscriber"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="listener">The ISubscriberListener instance which will be attached to the new ISubscriber.
        /// It is permitted to use null as the value of the listener: this behaves as a ISubscriberListener
        /// whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the ISubscriberListener
        /// for a certain status.</param>
        /// <returns>The newly created ISubscriber. In case of an error, a null ISubscriber is returned.</returns>
        ISubscriber CreateSubscriber(
                ISubscriberListener listener,
                StatusKind mask);

        /// <summary>
        /// This method creates a ISubscriber with the desired QosPolicy settings, but without an ISubscriberListener.
        /// </summary>
        /// <remarks>
        /// This operation creates a ISubscriber with the given SubscriberQos, a null ISubscriberListener
        /// and 0 StatusKind mask.
        ///
        /// In case the specified QosPolicy settings are not consistent, no ISubscriber is
        /// created and null is returned. Null can also be returned
        /// when insufficient access rights exist for the partition(s) listed in the provided QoS
        ///
        /// To delete the ISubscriber the operation DeleteSubscriber() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask) "CreateSubscriber"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings for the new ISubscriber.
        /// In case these settings are not self consistent, no ISubscriber is created.</param>
        /// <returns>The newly created ISubscriber. In case of an error, a null ISubscriber is returned.</returns>
        ISubscriber CreateSubscriber(SubscriberQos qos);

        /// <summary>
        /// This operation creates a ISubscriber with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified ISubscriberListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a ISubscriber with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified ISubscriberListener to it. When the
        /// ISubscriberListener is not applicable, null must be supplied instead.
        ///
        /// In case the specified QosPolicy settings are not consistent, no ISubscriber is
        /// created and null is returned. Null can also be returned
        /// when insufficient access rights exist for the partition(s) listed in the provided QoS
        ///
        /// To delete the ISubscriber the operation DeleteSubscriber() or
        /// DeleteContainedEntities() must be used.
        ///
        /// <i><b>Communication Status</b></i><br>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask, the associated
        /// ISubscriberListener operation is invoked and the communication
        /// status is reset to false, as the listener implicitly accesses the status which is passed
        /// as a parameter to that operation. The fact that the status is reset prior to calling the
        /// listener means that if the application calls the Get<status_name>Status from
        /// inside the listener it will see the status already reset.
        ///
        /// The following statuses are applicable to the ISubscriber
        /// - DDS.StatusKind RequestedDeadlineMissed (propagated)
        /// - DDS.StatusKind RequestedIncompatibleQos (propagated)
        /// - DDS.StatusKind SampleLost (propagated)
        /// - DDS.StatusKind SampleRejected (propagated)
        /// - DDS.StatusKind DataAvailable (propagated)
        /// - DDS.StatusKind LivelinessChanged (propagated)
        /// - DDS.StatusKind SubscriptionMatched (propagated)
        /// - DDS.StatusKind DataOnReaders
        ///
        /// Be aware that the SubscriptionMatched
        /// status are not applicable when the infrastructure does not have the
        /// information available to determine connectivity. This is the case when OpenSplice
        /// is configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return null.
        ///
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The special constant 0 can
        /// be used to indicate that the created entity should not respond to any of its available
        /// statuses. The DDS will therefore attempt to propagate these statuses to its factory.
        ///
        /// <i><b>Status Propagation</b></i><br>
        /// The Data Distribution Service will trigger the most specific and relevant Listener.<br>
        /// In other words, in case a communication status is also activated on the
        /// IDataWriterListener of a contained IDataWriter, the IDataWriterListener
        /// on that contained IDataWriter is invoked instead of the ISubscriberListener.
        /// This means that a status change on a contained IDataWriter only invokes the
        /// ISubscriberListener if the contained IDataWriter itself does not handle the
        /// trigger event generated by the status change.
        ///
        /// In case a communication status is not activated in the mask of the
        /// ISubscriberListener, the IDomainParticipantListener of the containing
        /// IDomainParticipant is invoked (if attached and activated for the status that
        /// occurred). This allows the application to set a default behaviour in the
        /// IDomainParticipantListener of the containing IDomainParticipant and a
        /// ISubscriber specific behaviour when needed. In case the
        /// IDomainParticipantListener is also not attached or the communication status
        /// is not activated in its mask, the application is not notified of the change.
        /// </remarks>
        /// <remarks>
        /// This operation creates a ISubscriber with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified ISubscriberListener to it. When the
        /// ISubscriberListener is not applicable, a null ISubscriberListener must be supplied
        /// instead. To delete the ISubscriber the operation DeleteSubscriber() or
        /// DeleteContainedEntities() must be used.
        /// In case the specified QosPolicy settings are not consistent, no ISubscriber is
        /// created and a null ISubscriber is returned.
        /// </remarks>
        /// <param name="qos">a collection of QosPolicy settings for the new ISubscriber.
        /// In case these settings are not self consistent, no ISubscriber is created.</param>
        /// <param name="listener">a pointer to the ISubscriberListener instance which will be attached
        /// to the new ISubscriber. It is permitted to use null as the value of the listener: this
        /// behaves as a ISubscriberListener whose operations perform no action.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of the ISubscriberListener
        /// for a certain status.</param>
        /// <returns>The newly created ISubscriber. In case of an error a null ISubscriber is returned.</returns>
        ISubscriber CreateSubscriber(
                SubscriberQos qos,
                ISubscriberListener listener,
                StatusKind mask);

        /// <summary>
        /// This operation deletes a ISubscriber.
        /// </summary>
        /// <remarks>
        /// This operation deletes a ISubscriber. A ISubscriber cannot be deleted when it
        /// has any attached IDataReader objects. When the operation is called on a
        /// ISubscriber with IDataReader objects, the operation returns
        /// PreconditionNotMet. When the operation is called on a different
        /// IDomainParticipant, as used when the ISubscriber was created, the operation
        /// has no effect and returns PreconditionNotMet.
        /// </remarks>
        /// <param name="s"> The subscriber to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The ISubscriber is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter p is not a valid ISubscriber.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the DDS has ran out of resources to complete this operation. </item>
        /// <item>DDS.ReturnCode PreconditionNotMet - the operation is called on a different IDomainParticipant,
        /// as used when the ISubscriber was created, or the ISubscriber contains one or more IDataReader objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteSubscriber(ISubscriber s);

        /// <summary>
        /// This property returns the built-in ISubscriber associated with the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation returns the built-in ISubscriber associated with the
        /// IDomainParticipant. Each IDomainParticipant contains several built-in
        /// ITopic objects. The built-in ISubscriber contains the corresponding IDataReader
        /// objects to access them. All these IDataReader objects belong to a single built-in
        /// ISubscriber. Note that there is exactly one built-in ISubscriber associated with
        /// each IDomainParticipant.
        /// </remarks>
        /// <returns>The built-in ISubscriber associated with the IDomainParticipant.</returns>
        ISubscriber BuiltInSubscriber { get; }

        /// <summary>
        /// This operation creates a reference to a new or existing ITopic under the given name,
        /// for a specific type and uses default values for QoS and Listener.
        /// </summary>
        /// <remarks>
        /// This operation creates a ITopic with the default TopicQos, a null ITopicListener
        /// and 0 StatusKind mask.
        ///
        /// If the SetDefaultTopicQos() method is called, then the default TopicQos will be the
        /// QoS given to that method. Otherwise it will equal a new TopicQos.
        ///
        /// To delete the ITopic the operation DeleteTopic() or DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateTopic(string topicName, string typeName, TopicQos qos, ITopicListener listener, StatusKind mask) "CreateTopic"
        /// for:<br>
        /// - Type Support
        /// - Existing ITopic Name
        /// - Local Proxy
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="topicName">the name of the ITopic to be created. A new ITopic will only be created,
        /// when no ITopic, with the same name, is found within the IDomainParticipant.</param>
        /// <param name="typeName">a local alias of the data type, which must have been registered
        /// before creating the ITopic.</param>
        /// <returns>The new or existing ITopic. In case of an error, null is returned.</returns>
        ITopic CreateTopic(string topicName, string typeName);

        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type,
        /// with the default QosPolicy settings and if applicable, attaches the optionally specified
        /// TopicListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a ITopic with the default TopicQos, the given ITopicListener
        /// and StatusKind mask. The ITopicListener may be null and the mask may be 0.
        ///
        /// If the SetDefaultTopicQos() method is called, then the default TopicQos will be the
        /// QoS given to that method. Otherwise it will equal a new TopicQos.
        ///
        /// To delete the ITopic the operation DeleteTopic() or DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateTopic(string topicName, string typeName, TopicQos qos, ITopicListener listener, StatusKind mask) "CreateTopic"
        /// for:<br>
        /// - Type Support
        /// - Existing ITopic Name
        /// - Local Proxy
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="topicName">the name of the ITopic to be created. A new ITopic will only be created,
        /// when no ITopic, with the same name, is found within the IDomainParticipant.</param>
        /// <param name="typeName">a local alias of the data type, which must have been registered
        /// before creating the ITopic.</param>
        /// <param name="listener">the TopicListener instance which will be attached to the new ITopic.
        /// It is permitted to use null as the value of the listener: this behaves as a TopicListener
        /// whose operations perform no action.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of
        /// the TopicListener for a certain status.</param>
        /// <returns>The new or existing ITopic. In case of an error, null is returned.</returns>
        ITopic CreateTopic(
                string topicName,
                string typeName,
                ITopicListener listener,
                StatusKind mask);

        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type,
        /// with the desired QosPolicy settings and no Listener.
        /// </summary>
        /// <remarks>
        /// This operation creates a ITopic with the given TopicQos, a null ITopicListener
        /// and 0 StatusKind mask.
        ///
        /// To delete the ITopic the operation DeleteTopic() or DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateTopic(string topicName, string typeName, TopicQos qos, ITopicListener listener, StatusKind mask) "CreateTopic"
        /// for:<br>
        /// - Type Support
        /// - Existing ITopic Name
        /// - Local Proxy
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="topicName">the name of the ITopic to be created. A new ITopic will only be created,
        /// when no ITopic, with the same name, is found within the IDomainParticipant.</param>
        /// <param name="typeName">a local alias of the data type, which must have been registered
        /// before creating the ITopic.</param>
        /// <param name="qos">a collection of QosPolicy settings for the new ITopic.
        /// In case these settings are not self consistent, no ITopic is created.</param>
        /// <returns>The new or existing ITopic. In case of an error, null is returned.</returns>
        ITopic CreateTopic(string topicName, string typeName, TopicQos qos);

        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type,
        /// with the desired QosPolicy settings and if applicable, attaches the optionally specified
        /// TopicListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a reference to a new or existing ITopic under the given name,
        /// for a specific type, with the desired QosPolicy settings and if applicable, attaches
        /// the optionally specified TopicListener to it. When the TopicListener is not
        /// applicable, a null listener must be supplied instead. In case the specified
        /// QosPolicy settings are not consistent, no ITopic is created and a null ITopic is
        /// returned.
        ///
        /// To delete the ITopic the operation DeleteTopic() or DeleteContainedEntities() must be used.
        ///
        /// <i><b>Type Support</b></i><br>
        /// The ITopic is bound to the type typeName. Prior to creating the ITopic, the
        /// typeName must have been registered with the Data Distribution Service.
        /// Registering the typeName is done using the data type specific RegisterType
        /// operation (the simplest ITopic creation is used in this example).
        /// <code>
        /// DDS.DomainParticipantFactory factory     = DDS.DomainParticipantFactory.Instance;
        /// DDS.IDomainParticipant       participant = factory.CreateParticipant(DDS.DomainId.Default);
        /// Space.FooTypeSupport           typesupport = new Space.FooTypeSupport();
        /// DDS.ReturnCode retcode = typesupport.RegisterType(participant, "Space.Foo");
        ///
        /// DDS.ITopic topic = participant.CreateTopic("FoFoo", "Space.Foo");
        /// </code>
        ///
        /// <i><b>Existing ITopic Name</b></i><br>
        /// Before creating a new ITopic, this operation performs a
        /// LookupTopicDescription() for the specified topicName. When a ITopic is
        /// found with the same name in the current domain, the QoS and typeName of the
        /// found ITopic are matched against the parameters qos and typeName. When they
        /// are the same, no ITopic is created but a new proxy of the existing ITopic is returned.<br>
        /// When they are not exactly the same, no ITopic is created and null is
        /// returned.
        ///
        /// When a ITopic is obtained multiple times, it must also be deleted that same number
        /// of times using DeleteTopic() or calling DeleteContainedEntities() once to
        /// delete all the proxies.
        ///
        /// <i><b>Local Proxy</b></i><br>
        /// Since a Topic is a global concept in the system, access is provided through a local
        /// proxy. In other words, the reference returned is actually not a reference to a ITopic
        /// but to a locally created proxy. The Data Distribution Service propagates Topics
        /// and makes remotely created Topics locally available through this proxy. For each
        /// create, a new proxy is created. Therefore the Topic must be deleted the same
        /// number of times, as the Topic was created with the same topicName per
        /// Domain. In other words, each reference (local proxy) must be deleted separately.
        ///
        /// <i><b>Communication Status</b></i><br>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask, the associated
        /// ITopicListener operation is invoked and the communication
        /// status is reset to false, as the listener implicitly accesses the status which is passed
        /// as a parameter to that operation. The fact that the status is reset prior to calling the
        /// listener means that if the application calls the Get<status_name>Status from
        /// inside the listener it will see the status already reset.
        ///
        /// The following statuses are applicable to the ITopic
        /// - DDS.StatusKind InconsistentTopic
        ///
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The special constant 0 can
        /// be used to indicate that the created entity should not respond to any of its available
        /// statuses. The DDS will therefore attempt to propagate these statuses to its factory.
        ///
        /// <i><b>Status Propagation</b></i><br>
        /// In case a communication status is not activated in the mask of the
        /// ITopicListener, the IDomainParticipantListener of the containing
        /// IDomainParticipant is invoked (if attached and activated for the status that
        /// occurred). This allows the application to set a default behaviour in the
        /// IDomainParticipantListener of the containing IDomainParticipant and a
        /// ITopic specific behaviour when needed. In case the
        /// IDomainParticipantListener is also not attached or the communication status
        /// is not activated in its mask, the application is not notified of the change.
        /// </remarks>
        /// <param name="topicName">the name of the ITopic to be created. A new ITopic will only be created,
        /// when no ITopic, with the same name, is found within the IDomainParticipant.</param>
        /// <param name="typeName">a local alias of the data type, which must have been registered
        /// before creating the ITopic.</param>
        /// <param name="qos">a collection of QosPolicy settings for the new ITopic.
        /// In case these settings are not self consistent, no ITopic is created.</param>
        /// <param name="listener">the TopicListener instance which will be attached to the new ITopic.
        /// It is permitted to use null as the value of the listener: this behaves as a TopicListener
        /// whose operations perform no action.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of
        /// the TopicListener for a certain status.</param>
        /// <returns>The new or existing ITopic. In case of an error, null is returned.</returns>
        ITopic CreateTopic(
                string topicName,
                string typeName,
                TopicQos qos,
                ITopicListener listener,
                StatusKind mask);

        /// <summary>
        /// This operation deletes a ITopic
        /// </summary>
        /// <remarks>
        /// This operation deletes a ITopic. A ITopic cannot be deleted when there are any
        /// IDataReader, IDataWriter, IContentFilteredTopic or IMultiTopic objects,
        /// which are using the ITopic. When the operation is called on a ITopic referenced by
        /// any of these objects, the operation returns PreconditionNotMet.
        /// When the operation is called on a different IDomainParticipant, as used when
        /// the ITopic was created, the operation has no effect and returns PreconditionNotMet.
        ///
        /// <i><b>Local proxy</b></i><br>
        /// Since a Topic is a global concept in the system, access is provided through a local
        /// proxy. In other words, the reference is actually not a reference to a Topic but to the
        /// local proxy. The Data Distribution Service propagates Topics and makes remotely
        /// created Topics locally available through this proxy. Such a proxy is created by the
        /// CreateTopic or FindTopic operation. This operation will delete the local
        /// proxy. When a reference to the same Topic was created multiple times (either by
        /// CreateTopic or FindTopic), each reference (local proxy) must be deleted
        /// separately. When this proxy is the last proxy for this Topic, the Topic itself is also
        /// removed from the system. As mentioned, a proxy may only be deleted when there
        /// are no other entities attached to it. However, it is possible to delete a proxy while
        /// there are entities attached to a different proxy.
        /// </remarks>
        /// <param name="topic">The ITopic which is to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The ITopic is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter topic is not a valid ITopic.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is called on a different IDomainParticipant, as used when the
        /// ITopic was created, or the ITopic is still referenced by other objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteTopic(ITopic topic);

        /// <summary>
        /// This operation gives access to an existing (or ready to exist) enabled ITopic, based on its topicName.
        /// </summary>
        /// <remarks>
        /// This operation gives access to an existing ITopic, based on its topicName. The
        /// operation takes as arguments the topicName of the ITopic and a timeout.
        ///
        /// If a ITopic of the same topicName already exists, it gives access to this ITopic.
        /// Otherwise it waits (blocks the caller) until another mechanism creates it. This other
        /// mechanism can be another thread, a configuration tool, or some other Data
        /// Distribution Service utility. If after the specified timeout the ITopic can still not be
        /// found, the caller gets unblocked and null is returned.
        ///
        /// A ITopic obtained by means of FindTopic, must also be deleted by means of
        /// DeleteTopic so that the local resources can be released. If a ITopic is obtained
        /// multiple times it must also be deleted that same number of times using
        /// DeleteTopic or calling DeleteContainedEntities once to delete all the proxies.
        ///
        /// A ITopic that is obtained by means of FindTopic in a specific IDomainParticipant
        /// can only be used to create DataReaders and DataWriters in that IDomainParticipant if its
        /// corresponding TypeSupport has been registered to that same IDomainParticipant.
        ///
        /// <i><b>Local Proxy</b></i><br>
        /// Since a Topic is a global concept in the system, access is provided through a local
        /// proxy. In other words, the reference returned is actually not a reference to a Topic
        /// but to a locally created proxy. The Data Distribution Service propagates Topics
        /// and makes remotely created Topics locally available through this proxy. For each
        /// create, a new proxy is created. Therefore the Topic must be deleted the same
        /// number of times, as the ITopic was created with the same topicName per
        /// Domain. In other words, each reference (local proxy) must be deleted separately.
        /// </remarks>
        /// <param name="topicName">The name of te ITopic that the application wants access to.</param>
        /// <param name="timeout">The maximum duration to block for the IDomainParticipant FindTopic,
        /// after which the application thread is unblocked. The special constant Duration Infinite can be used
        /// when the maximum waiting time does not need to be bounded.</param>
        /// <returns>The ITopic that has been found. If an error occurs the operation returns a ITopic with a null value. </returns>
        ITopic FindTopic(string topicName, Duration timeout);

        /// <summary>
        /// This operation gives access to a locally-created ITopicDescription, with a matching name.
        /// </summary>
        /// <remarks>
        /// The operation LookupTopicDescription gives access to a locally-created
        /// ITopicDescription, based on its name. The operation takes as argument the name
        /// of the ITopicDescription.
        ///
        /// If one or more local ITopicDescription proxies of the same name already exist,
        /// a pointer to one of the already existing local proxies is returned: LookupTopicDescription
        /// will never create a new local proxy. That means that the proxy that is returned does not need
        /// to be deleted separately from its original. When no local proxy exists, it returns null.
        ///
        /// The operation never blocks. The operation LookupTopicDescription may be used to locate any
        /// locally-created ITopic, IContentFilteredTopic and IMultiTopic object.
        /// </remarks>
        /// <param name="name">The name of the ITopicDescription to look for.</param>
        /// <returns>The ITopicDescription it has found.If an error occurs the operation
        /// returns a ITopicDescription with a null value. </returns>
        ITopicDescription LookupTopicDescription(string name);

        /// <summary>
        /// This operation creates a IContentFilteredTopic for a IDomainParticipant in order to allow
        /// DataReaders to subscribe to a subset of the topic content.
        /// </summary>
        /// <remarks>
        /// This operation creates a IContentFilteredTopic for a IDomainParticipant in
        /// order to allow DataReaders to subscribe to a subset of the topic content. The base
        /// topic, which is being filtered is defined by the parameter relatedTopic. The
        /// resulting IContentFilteredTopic only relates to the samples published under the
        /// relatedTopic, which have been filtered according to their content. The resulting
        /// IContentFilteredTopic only exists at the IDataReader side and will never be
        /// published. The samples of the related topic are filtered according to the SQL
        /// expression (which is a subset of SQL) as defined in the parameter
        /// filterExpression.
        ///
        /// The filterExpression may also contain parameters, which appear as
        /// %n tokens in the expression which must be set by the sequence of strings defined by the
        /// parameter expressionParameters. The number of values in
        /// expressionParameters must be equal or greater than the highest referenced
        /// %n token in the filterExpression (e.g. if %1 and %8 are used as parameter in
        /// the filterExpression, the expressionParameters should at least contain
        /// n+1 = 9 values).
        ///
        /// The filterExpression is a string that specifies the criteria to select the data
        /// samples of interest. In other words, it identifies the selection of data from the
        /// associated Topics. It is an SQL expression where the WHERE clause gives the
        /// content filter.
        /// </remarks>
        /// <param name="name">The name of the IContentFilteredTopic.</param>
        /// <param name="relatedTopic">The base topic on which the filtering will be applied. Therefore,
        /// a filtered topic is based onn an existing ITopic.</param>
        /// <param name="filterExpression">The SQL expression (subset of SQL), which defines the filtering.</param>
        /// <param name="expressionParameters">A sequence of strings with the parameter value used in the SQL expression
        /// (i.e., the number of %n tokens in the expression).The number of values in expressionParameters
        /// must be equal or greater than the highest referenced %n token in the filterExpression
        /// (e.g. if %1 and %8 are used as parameter in the filterExpression, the expressionParameters should at least
        /// contain n+1 = 9 values)</param>
        /// <returns>The newly created IContentFilteredTopic. In case of an error a IContentFilteredTopic with a null
        /// value is returned.</returns>
        IContentFilteredTopic CreateContentFilteredTopic(
                string name,
                ITopic relatedTopic,
                string filterExpression,
                params string[] expressionParameters);

        /// <summary>
        /// This operation deletes a IContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// This operation deletes a IContentFilteredTopic.
        /// The deletion of a IContentFilteredTopic is not allowed if there are any existing
        /// IDataReader objects that are using the IContentFilteredTopic. If the
        /// DeleteContentFilteredTopic operation is called on a
        /// IContentFilteredTopic with existing IDataReader objects attached to it, it will
        /// return PreconditionNotMet.
        ///
        /// The DeleteContentFilteredTopic operation must be called on the same
        /// IDomainParticipant object used to create the IContentFilteredTopic. If
        /// DeleteContentFilteredTopic is called on a different IDomainParticipant
        /// the operation will have no effect and it will return PreconditionNotMet.
        /// </remarks>
        /// <param name="aContentFilteredTopic">The IContentFilteredTopic to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IContentFilteredTopic is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter aContentFilteredTopictopic is not a valid IContentFilteredTopic.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is called on a different IDomainParticipant, as used when the
        /// IContentFilteredTopic is being used by one or more IDataReader objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteContentFilteredTopic(IContentFilteredTopic aContentFilteredTopic);

        /// <summary>
        /// This operation creates a IMultiTopic for a IDomainParticipant in order to allow
        /// DataReaders to subscribe to a filtered/re-arranged combination and/or subset of
        /// the content of several topics.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This operation creates a IMultiTopic for a IDomainParticipant in order to allow
        /// DataReaders to subscribe to a filtered/re-arranged combination and/or subset of
        /// the content of several topics. Before the IMultiTopic can be created, the
        /// typeName of the IMultiTopic must have been registered prior to calling this
        /// operation. Registering is done, using the register_type operation from
        /// TypeSupport. The list of topics and the logic, which defines the selection,
        /// filtering, combining and re-arranging of the sample data, is defined by the SQL
        /// expression (subset of SQL) defined in subscription_expression. The
        /// subscription_expression may also contain parameters, which appear as %n
        /// tokens in the expression. These parameters are defined in
        /// expressionParameters. The number of values in expressionParameters
        /// must be equal or greater than the highest referenced %n token in the
        /// subscription_expression (e.g. if %1 and %8 are used as parameter in the
        /// subscription_expression, the expressionParameters should at least
        /// contain n+1 = 9 values).
        ///
        /// The subscription_expression is a string that specifies the criteria to select the
        /// data samples of interest. In other words, it identifies the selection and rearrangement
        /// of data from the associated Topics. It is an SQL expression where the SELECT
        /// clause provides the fields to be kept, the FROM part provides the names of the
        /// Topics that are searched for those fields, and the WHERE clause gives the content
        /// filter. The Topics combined may have different types but they are restricted in that
        /// the type of the fields used for the NATURAL JOIN operation must be the same.
        ///
        /// The IDataReader, which is associated with a IMultiTopic only accesses
        /// information which exist locally in the IDataReader, based on the Topics used in
        /// the subscription_expression. The actual IMultiTopic will never be
        /// produced, only the individual Topics.
        /// </remarks>
        /// <param name="name">the name of the multi topic.</param>
        /// <param name="typeName">the name of the type of the IMultiTopic. This
        ///	       typeName must have been registered using register_type prior to calling
        ///	       this operation.</param>
        /// <param name="subscriptionExpression">the SQL expression (subset of
        ///	       SQL), which defines the selection, filtering, combining and re-arranging of the
        ///	       sample data.</param>
        /// <param name="expressionParameters">the handle to a sequence
        ///	       of strings with the parameter value used in the SQL expression (i.e., the number
        ///	       of %n tokens in the expression). The number of values in
        ///	       expressionParameters must be equal or greater than the highest
        ///	       referenced %n token in the subscription_expression (e.g. if %1 and %8 are used
        ///	       as parameter in the subscription_expression, the
        ///	       expressionParameters should at least contain n+1 = 9 values).</param>
        /// <returns>The newly created IMultiTopic. In case of an error, null is returned.</returns>
        IMultiTopic CreateMultiTopic(
                string name,
                string typeName,
                string subscriptionExpression,
                params string[] expressionParameters);

        /// <summary>
        /// This operation deletes a IMultiTopic.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// The deletion of a IMultiTopic is not allowed if there are any existing IDataReader
        /// objects that are using the IMultiTopic. If the DeleteMultiTopic operation is
        /// called on a IMultiTopic with existing IDataReader objects attached to it, it will
        /// return PreconditionNotMet.
        ///
        /// The DeleteMultiTopic operation must be called on the same
        /// IDomainParticipant object used to create the IMultiTopic. If
        /// DeleteMultiTopic is called on a different IDomainParticipant the operation
        /// will have no effect and it will return PreconditionNotMet.
        /// </remarks>
        /// <param name="multiTopic">The IMultiTopic, which is to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IMultiTopic is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is called on a different
        ///     IDomainParticipant, as used when the IMultiTopic was created, or the
        ///     IMultiTopic is being used by one or more IDataReader objects.</item>
        /// </returns>
        ReturnCode DeleteMultiTopic(IMultiTopic multiTopic);

        /// <summary>
        /// This operation deletes all the IEntity objects that were created on the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the IEntity objects that were created on the
        /// IDomainParticipant. In other words, it deletes all IPublisher, ISubscriber,
        /// ITopic, IContentFilteredTopic and IMultiTopic objects. Prior to deleting each
        /// contained IEntity, this operation regressively calls the corresponding
        /// DeleteContainedEntities operation on each IEntity (if applicable). In
        /// other words, all IEntity objects in the IPublisher and ISubscriber are deleted,
        /// including the IDataWriter and IDataReader. Also the IQueryCondition and
        /// IReadCondition objects contained by the IDataReader are deleted.
        ///
        /// @note The operation will return PreconditionNotMet if the any of the
        /// contained entities is in a state where it cannot be deleted. This will occur, for
        /// example, if a contained IDataReader cannot be deleted because the application has
        /// called a read or take operation and has not called the corresponding
        /// ReturnLoan operation to return the loaned samples. In such cases, the operation
        /// does not roll back any entity deletions performed prior to the detection of the
        /// problem.
        ///
        /// <i><b>Topic</b></i><br>
        /// Since a Topic is a global concept in the system, access is provided through a local
        /// proxy. The Data Distribution Service propagates Topics and makes remotely
        /// created Topics locally available through this proxy. Such a proxy is created by the
        /// create_topic or FindTopic operation. When a reference to the same Topic
        /// was created multiple times (either by CreateTopic or FindTopic), all
        /// references (local proxies) are deleted. With the last proxy, the Topic itself is also
        /// removed from the system.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The contained IEntity objects are deleted and the application may delete the IDomainParticipant.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IIDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - One or more of the contained entities are
        /// 	        in a state where they cannot be deleted.</item>
        /// </returns>
        ReturnCode DeleteContainedEntities();

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a
        /// IDomainParticipant. The parameter qos contains the QosPolicy settings which
        /// is checked for self-consistency.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned Ok).
        /// </remarks>
        /// <param name="qos">New set of QosPolicy settings for the IDomainParticipant.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new DomainParticipantQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode SetQos(DomainParticipantQos qos);

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a
        /// IDomainParticipant on which this operation is used. This
        /// DomainparticipantQos is stored at the location pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination DomainParticipantQos struct in which the
        /// QosPolicy settings will be copied.</param>
        /// <returns>
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of QoS policy values applied to this IDomainParticipant
        /// has successfully been copied into the specified DomainParticipantQos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode GetQos(ref DomainParticipantQos qos);

        /// <summary>
        /// This property returns the IDomainParticipantListener currently attached to the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// Only one listener can be attached to the IDomainParticipant at any particular time. This property
        /// returns the listener that is currently attached to the IDomainParticipant. When no Listener was
        /// attached, null is returned.
        /// </remarks>
        /// <returns>returns the IDomainParticipantListener currently attached to the IDomainParticipant.</returns>
        IDomainParticipantListener Listener { get; }

        /// <summary>
        /// This operation attaches a IDomainParticipantListener to the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation attaches a IDomainParticipantListener to the
        /// IDomainParticipant. Only one IDomainParticipantListener can be
        /// attached to each IDomainParticipant. If a IDomainParticipantListener was
        /// already attached, the operation will replace it with the new one. When a_listener
        /// is null, it represents a listener that is treated as a NOOP1 for all statuses
        /// activated in the bit mask.
        /// </remarks>
        /// <param name="listener">The IDomainParticipantListener instance, which will be attached to the
        /// IDomainParticipant.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the IDomainParticipantListener
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IDomainParticipantListener is attached.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask);

        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        ReturnCode IgnoreParticipant(InstanceHandle handle);

        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        ReturnCode IgnoreTopic(InstanceHandle handle);

        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        ReturnCode IgnorePublication(InstanceHandle handle);

        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        ReturnCode IgnoreSubscription(InstanceHandle handle);

        /// <summary>
        /// This property returns the DomainId of the Domain to which this IDomainParticipant is attached.
        /// </summary>
        DomainId DomainId { get; }

        /// <summary>
        /// This operation asserts the liveliness for the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation will manually assert the liveliness for the IDomainParticipant.
        /// This way, the Data Distribution Service is informed that the IDomainParticipant
        /// is still alive. This operation only needs to be used when the IDomainParticipant
        /// contains DataWriters with the LivelinessQosPolicy set to
        /// MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, and it will only affect the
        /// liveliness of those DataWriters.
        ///
        /// Writing data via the write operation of a IDataWriter will assert the liveliness on
        /// the IDataWriter itself and its IDomainParticipant. Therefore,
        /// AssertLiveliness is only needed when not writing regularly.
        ///
        /// The liveliness should be asserted by the application, depending on the
        /// LivelinessQosPolicy.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The liveliness of this IDomainParticipant has successfully been asserted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The IDomainParticipant is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode AssertLiveliness();

        /// <summary>
        /// This operation sets the default PublisherQos of the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation sets the default PublisherQos of the IDomainParticipant (that is the struct with
        /// the QosPolicy settings) which is used for newly created IPublisher objects, in case no QoS is
        /// provided. The default PublisherQos is only used when the constant is supplied
        /// as parameter qos to specify the PublisherQos in the CreatePublisher operation. The
        /// PublisherQos is always self consistent, because its policies do not depend on
        /// each other. This means this operation never returns the InconsistentPolicy.
        /// The values set by this operation are returned by GetDefaultPublisherQos().
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings, which contains the new default QosPolicy
        /// settings for the newly created Publishers.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new default PublisherQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter qos is not a valid PublisherQos.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are currently not supported by openSplice.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultPublisherQos(PublisherQos qos);

        /// <summary>
        /// This operation gets the struct with the default IPublisher QosPolicy settings of the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation gets the struct with the default IPublisher QosPolicy settings of
        /// the IDomainParticipant (that is the PublisherQos) which is used for newly
        /// created IPublisher objects, in case no QoS is provided.
        /// The default PublisherQos is only used when the constant is supplied as
        /// parameter qos to specify the PublisherQos in the CreatePublisher
        /// operation. The application must provide the PublisherQos struct in which the
        /// QosPolicy settings can be stored and pass the qos reference to the operation. The
        /// operation writes the default QosPolicy settings to the struct referenced to by qos.
        /// Any settings in the struct are overwritten.
        ///
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultPublisherQos, or, if the call was never made,
        /// the default values as specified for each QosPolicy setting.
        /// </remarks>
        /// <param name="qos">A reference to the PublisherQos struct (provided by the application) in which
        /// the default QosPolicy settings for the IPublisher are written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The default IPublisher QosPolicy settings of this IDomainParticipant have successfully been
        /// copied into the specified qos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultPublisherQos(ref PublisherQos qos);

        /// <summary>
        /// This operation sets the default SubscriberQos of the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation sets the default SubscriberQos of the IDomainParticipant (that
        /// is the struct with the QosPolicy settings) which is used for newly created
        /// ISubscriber objects, in case no QoS was provided.
        /// The default SubscriberQos is only used when the constant is supplied as
        /// parameter qos to specify the SubscriberQos in the CreateSubscriber
        /// operation. The SubscriberQos is always self consistent, because its policies do
        /// not depend on each other. This means this operation never returns the
        /// InconsistentPolicy. The values set by this operation are returned
        /// by GetDefaultSubscriberQos().
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings, which contains the new default QosPolicy
        /// settings for the newly created Subscribers.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new default SubscriberQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter qos is not a valid SubscriberQos.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are currently not supported by openSplice.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultSubscriberQos(SubscriberQos qos);

        /// <summary>
        /// This operation gets the struct with the default ISubscriber QosPolicy settings of the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation gets the struct with the default ISubscriber QosPolicy settings of
        /// the IDomainParticipant (that is the SubscriberQos) which is used for newly
        /// created ISubscriber objects, in case the constant SUBSCRIBER_QOS_DEFAULT is
        /// used. The default SubscriberQos is only used when the constant is supplied as
        /// parameter qos to specify the SubscriberQos in the create_subscriber
        /// operation. The application must provide the QoS struct in which the policy can be
        /// stored and pass the qos reference to the operation. The operation writes the default
        /// QosPolicy to the struct referenced to by qos. Any settings in the struct are
        /// overwritten.
        ///
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to set_default_subscriber_qos, or, if the call was never made,
        /// the default values as specified for each QosPolicy.
        /// </remarks>
        /// <param name="qos">a reference to the QosPolicy struct (provided by the application) in which the default
        /// QosPolicy settings for the ISubscriber is written</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The default ISubscriber QosPolicy settings of this IDomainParticipant have successfully
        /// been copied into the specified SubscriberQos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos);

        /// <summary>
        /// This operation sets the default TopicQos of the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation sets the default TopicQos of the IDomainParticipant (that is the
        /// struct with the QosPolicy settings) which is used for newly created ITopic objects,
        /// in case no QoS was provided. The default TopicQos is only
        /// used when the constant is supplied as parameter qos to specify the TopicQos in the
        /// CreateTopic operation. This operation checks if the TopicQos is self
        /// consistent. If it is not, the operation has no effect and returns
        /// Inconsistentpolicy. The values set by this operation are returned by GetGefaultTopicQos.
        /// </remarks>
        /// <param name="qos">a collection of QosPolicy settings, which contains the new default QosPolicy settings
        /// for the newly created Topics.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new default TopicQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter qos is not a valid TopicQos.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the supported QosPolicy values are currently not supported
        /// by OpenSplice.</item>
        /// <item>DDS.ReturnCode InconsistentPolicy - the parameter qos contains conflicting QosPolicy settings, e.g a history depth that is
        /// higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultTopicQos(TopicQos qos);

        /// <summary>
        /// This operation gets the struct with the default ITopic QosPolicy settings of the IDomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation gets the struct with the default ITopic QosPolicy settings of the
        /// IDomainParticipant (that is the TopicQos) which is used for newly created
        /// ITopic objects, in case the constant TOPIC_QOS_DEFAULT is used. The default
        /// TopicQos is only used when the constant is supplied as parameter qos to specify
        /// the TopicQos in the CreateTopic operation. The application must provide the
        /// QoS struct in which the policy can be stored and pass the qos reference to the
        /// operation. The operation writes the default QosPolicy to the struct referenced to
        /// by qos. Any settings in the struct are overwritten.
        ///
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultTopicQos, or, if the call was never made, the
        /// default values as specified for each QosPolicy.
        /// </remarks>
        /// <param name="qos">A reference to the QosPolicy struct (provided by the application) in which the
        /// default QosPolicy settings for the ITopic is written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The default ITopic QosPolicy settings of this IDomainParticipant have successfully been copied
        /// into the specified TopicQos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultTopicQos(ref TopicQos qos);

        /// <summary>
        /// This operation checks whether or not the given IEntity represented by handle is created by the
        /// IDomainParticipant or any of its contained entities.
        /// </summary>
        /// <remarks>This operation checks whether or not the given IEntity represented by handle
        /// is created by the IDomainParticipant itself (ITopicDescription, IPublisher
        /// or ISubscriber) or created by any of its contained entities (IDataReader,
        /// IReadCondition, IQueryCondition, IDataWriter, etc.).
        ///
        /// Return value is true if a_handle represents an IEntity that is created by the
        /// IDomainParticipant or any of its contained Entities. Otherwise the return
        /// value is false</remarks>
        /// <param name="handle">An IEntity in the Data Distribution Service.</param>
        /// <returns>true if handle represents an IEntity that is created by the IDomainParticipant or any of its
        /// contained Entities. Otherwise the return value is false.</returns>
        bool ContainsEntity(InstanceHandle handle);

        /// <summary>
        /// This operation returns the value of the current time that the Data Distribution Service
        /// uses to time-stamp written data as well as received data in current_time.
        /// </summary>
        /// <remarks>
        /// This operation returns the value of the current time that the Data Distribution
        /// Service uses to time-stamp written data as well as received data in current_time.
        /// The input value of current_time is ignored by the operation.
        /// </remarks>
        /// <param name="currentTime">stores the value of currentTime as used by the Data Distribution Service.
        /// The input value of currentTime is ignored.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The value of the current time has been copied into current_time.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter current_time is not a valid reference.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDomainParticipant has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The IDomainParticipant is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode GetCurrentTime(out Time currentTime);

        /// <summary>
        /// This operation retrieves the list of DomainParticipants that have been discovered in the domain.
        /// </summary>
        /// <remarks>
        /// This operation retrieves the list of DomainParticipants that have been discovered in the domain and that the application
        /// has not indicated should be ignored by means of the IDomainParticipant IgnoreParticipant operation.
        ///
        /// The participant_handles sequence and its buffer may be pre-allocated by the
        /// application and therefore must either be re-used in a subsequent invocation of the
        /// GetDiscoveredParticipants operation or be released by
        /// calling free on the returned participant_handles. If the pre-allocated
        /// sequence is not big enough to hold the number of associated participants, the
        /// sequence will automatically be (re-)allocated to fit the required size.
        /// The handles returned in the participant_handles sequence are the ones that
        /// are used by the DDS implementation to locally identify the corresponding matched
        /// Participant entities. You can access more detailed information about a particular
        /// participant by passing its participantHandle to the GetDiscoveredParticipantData operation.
        /// </remarks>
        /// <param name="participantHandles">An array which is used to pass the list of all associated participants.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the list of associated participants has successfully been obtained.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDomainParticipant has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the information about associated participants</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDomainParticipant is not enabled.</item>
        /// <item>IllegalOperation- the operation is invoked on an inappropriate object.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDiscoveredParticipants (ref InstanceHandle[] participantHandles);

        /// <summary>
        /// This operation retrieves information on a IDomainParticipant that has been discovered on the network.
        /// </summary>
        /// <remarks>
        /// This operation retrieves information on a IDomainParticipant that has been discovered on the network. The participant
        /// must be in the same domain as the participant on which this operation is invoked and must not have been ignored by
        /// means of the IDomainParticipant IgnoreParticipant operation.
        ///
        /// The instance handle must correspond to a partition currently
        /// associated with the IDomainParticipant, otherwise the operation will fail and return
        /// Error. The operation GetDiscoveredParticipantData
        /// can be used to find more detailed information about a particular participant that is found with the
        /// GetDiscoveredParticipants operation.
        /// </remarks>
        /// <param name="data">The sample in which the information about the specified participant is to be stored.</param>
        /// <param name="handle">The handle to the participant whose information needs to be retrieved.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the information on the specified participant has been successfully retrieved</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDomainParticipant has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the information about associated participants</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDomainParticipant is not enabled.</item>
        /// <item>IllegalOperation- the operation is invoked on an inappropriate object.
        /// </list>
        /// </returns>
        ReturnCode GetDiscoveredParticipantData (ref ParticipantBuiltinTopicData data, InstanceHandle handle);

        /// <summary>
        /// This operation retrieves the list of Topics that have been discovered in the domain
        /// </summary>
        /// <remarks>
        /// This operation retrieves the list of Topics that have been discovered in the domain and that the application has not
        /// indicated should be ignored by means of the IDomainParticipant ignore_topic operation.
        ///
        /// The topic_handles sequence and its buffer may be pre-allocated by the
        /// application and therefore must either be re-used in a subsequent invocation of the
        /// GetDiscoveredTopics operation or be released by
        /// calling free on the returned topic_handles. If the pre-allocated
        /// sequence is not big enough to hold the number of associated participants, the
        /// sequence will automatically be (re-)allocated to fit the required size.
        /// The handles returned in the topic_handles sequence are the ones that
        /// are used by the DDS implementation to locally identify the corresponding matched
        /// ITopic entities. You can access more detailed information about a particular
        /// topic by passing its topic_handle to the GetDiscoveredTopicData operation.
        /// </remarks>
        /// <param name="topicHandles">An array which is used to pass the list of all associated topics.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the list of associated topic has successfully been obtained.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDomainParticipant has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the information about associated topics</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDomainParticipant is not enabled.</item>
        /// <item>IllegalOperation- the operation is invoked on an inappropriate object.
        /// </list>
        /// </returns>
        ReturnCode GetDiscoveredTopics (ref InstanceHandle[] topicHandles);

        /// <summary>
        /// This operation retrieves information on a ITopic that has been discovered on the network.
        /// </summary>
        /// <remarks>
        ///This operation retrieves information on a ITopic that has been discovered on the network. The topic must have been
        ///created by a participant in the same domain as the participant on which this operation is invoked and must not have been
        ///ignored by means of the IDomainParticipant ignore_topic operation.
        ///
        ///The topic_handle must correspond to a topic currently
        ///associated with the IDomainParticipant, otherwise the operation will fail and return Error.
        ///The operation GetDiscoveredTopicData can be used to find more detailed information about a particular topic that is found with the
        ///GetDiscoveredTopics operation.
        /// </remarks>
        /// <param name="data">The sample in which the information about the specified topic is to be stored.</param>
        /// <param name="handle">The handle to the topic whose information needs to be retrieved.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the information on the specified topic has been successfully retrieved</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDomainParticipant has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the information about associated topics</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDomainParticipant is not enabled.</item>
        /// <item>IllegalOperation- the operation is invoked on an inappropriate object.
        /// </list>
        /// </returns>
        ReturnCode GetDiscoveredTopicData (ref TopicBuiltinTopicData data, InstanceHandle handle);

        /// <summary>
        /// This operation looks up the property for a given key in the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation looks up the property for a given key
        /// in the DomainParticipant, returning the value belonging to this key
        /// If the property has not been set using setProperty,
        /// the default value of the property is returned.
        /// </remarks>
        /// <param name="name">The name of the property to request the value from</param>
        /// <param name="value">The value of the requested property.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the information on the specified property has been successfully retrieved</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDomainParticipant has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - if the name specifies an undefined property or the operation is not supported in this version.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDomainParticipant is not enabled.</item>
        /// <item>DDS.ReturnCode BadParameter - An invalid name or value has been specified.</item>
        /// </list>
        /// </returns>
        ReturnCode GetProperty(ref Property property);

        /// <summary>
        /// This operation sets the property specified by a key value pair.
        /// </summary>
        /// <remarks>
        /// This operation sets the property specified by a key value pair.
        /// Currently, the following property is defined:
        ///
        /// isolateNode
        /// The isolateNode property allows applications to isolate the federation from the rest of
        /// the Domain, i.e. at network level disconnect the node from the rest of the system.
        /// Additionally, they also need to be able to issue a request to reconnect their federation
        /// to the domain again after which the durability merge-policy that is configured needs to be
        /// applied.
        ///
        /// To isolate a federation, the application needs to set the isolateNode property value
        /// to 'true' and to (de)isolate the federation the same property needs to set to 'false'.
        /// The default value of the isolateNode property is 'false'.
        ///
        /// All data that is published after isolateNode is set to true will not be sent to the network
        /// and any data received from the network will be ignored. Be aware that data being processed
        /// by the network service at time of isolating a node may still be sent to the network due
        /// to asynchronous nature of network service internals.
        ///
        /// The value is interpreted as a boolean (i.e., it must be either 'true' or 'false').
        /// false (default): The federation is connected to the domain.
        /// true: The federation is disconnected from the domain meaning that data is not published
        /// on the network and data from the network is ignored.
        /// </remarks>
        /// <param name="name">The name of the property</param>
        /// <param name="value">The value of the property</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the information on the specified property has been successfully set</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDomainParticipant has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode Unsupported - if the name specifies an undefined property or the operation is not supported in this version.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDomainParticipant is not enabled.</item>
        /// <item>DDS.ReturnCode BadParameter - An invalid name or value has been specified.</item>
        /// </list>
        /// </returns>
        ReturnCode SetProperty(Property property);

    }

    public interface ITypeSupport
    {
        ReturnCode RegisterType(IDomainParticipant domain, string typeName);
        string TypeName { get; }
        string TypeDescriptor { get; }
        string KeyList { get; }
    }

    // ----------------------------------------------------------------------
    // Topics
    // ----------------------------------------------------------------------
    /// <summary>
    /// This class is an abstract class. It is the base class for ITopic, IContentFilteredTopic and IMultiTopic.
    /// The ITopicDescription property TypeName defines an unique data type that is
    /// made available to the Data Distribution Service via the TypeSupport.
    /// ITopicDescription has also a Name property that allows it to be retrieved locally.
    /// </summary>
    public interface ITopicDescription
    {
        /// <summary>
        /// Gets the name of the registered data type that is associated with this ITopicDescription.
        /// </summary>
        /// <returns>The name of the data type of the ITopicDescription.</returns>
        string TypeName { get; }
        /// <summary>
        /// Gets the name of this ITopicDescription.
        /// </summary>
        /// <returns>The name of the ITopicDescription.</returns>
        string Name { get; }
        /// <summary>
        /// Gets the IDomainParticipant associated with this ITopicDescription.
        /// </summary>
        /// <remarks>
        /// This operation returns the IDomainParticipant associated with this
        /// TopicDescription. Note that there is exactly one IDomainParticipant
        /// associated with each ITopicDescription. When the ITopicDescription was
        /// already deleted (there is no associated IDomainParticipant any more), null
        /// is returned.
        /// </remarks>
        /// <returns>The IDomainParticipant associated with the ITopicDescription or null.</returns>
        IDomainParticipant Participant { get; }
    }

    /// <summary>
    /// ITopic is the most basic description of the data to be published and subscribed.
    /// </summary>
    /// <remarks>
    /// A ITopic is identified by its name, which must be unique in the whole Domain. In
    /// addition (by virtue of extending ITopicDescription) it fully identifies the type of
    /// data that can be communicated when publishing or subscribing to the ITopic.
    /// ITopic is the only ITopicDescription that can be used for publications and
    /// therefore a specialized IDataWriter is associated to the ITopic.
    /// </remarks>
    public interface ITopic : IEntity, ITopicDescription
    {
        /// <summary>
        /// This operation obtains the InconsistentTopicStatus of the ITopic.
        /// </summary>
        /// <remarks>
        /// The InconsistentTopicStatus indicates that there exists an ITopic with
        /// the same name but with different characteristics. The InconsistentTopicStatus
        /// can also be monitored using a TopicListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="aStatus">The contents of the InconsistentTopicStatus of the ITopic will be
        /// copied into the location specified by aStatus.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current InconsistentTopicStatus of this ITopic has successfully been copied
        /// into the specified aStatus parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ITopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetInconsistentTopicStatus(ref InconsistentTopicStatus aStatus);
        /// <summary>
        /// This operation obtains the current set of QoS policies associated with the ITopic.
        /// </summary>
        /// <remarks>
        /// The operation returns the set of QoS policies currently associated with this ITopic in
        /// the provided qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to a TopicQos used to return the QoS policies of the ITopic.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of QoS policy values applied to this ITopic has successfully
        /// been copied into the specified TopicQos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ITopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref TopicQos qos);
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for an ITopic.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The parameter qos contains the QosPolicy settings which are to be applied to this
        /// ITopic. The provided QosPolicy settings are checked for self-consistency and mutability.
        /// When the application tries to change a QosPolicy setting for an enabled ITopic,
        /// which can only be set before the ITopic is enabled, the operation will fail and an
        /// ImmutablePolicy is returned.
        /// In other words, the application must provide the currently set QosPolicy settings
        /// in case of the immutable QosPolicy settings. Only the mutable QosPolicy
        /// settings can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and an InconsistentPolicy is returned.
        /// </para>
        /// <para>
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned OK).
        /// </para>
        /// </remarks>
        /// <param name="qos">The new set of QosPolicy settings for a ITopic.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new TopicQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ITopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter qos is not a valid topicQos. It contains a QosPolicy
        /// setting with an invalid Duration value or an enum value that is outside its legal boundaries.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are currently not supported by OpenSplice.</item>
        /// <item>DDS.ReturnCode ImmutablePolicy - The parameter qos contains an immutable QosPolicy setting with a different
        /// value than set during enabling of the ITopic.</item>
        /// <item>DDS.ReturnCode InconsistentPolicy - The parameter qos contains conflicting QosPolicy settings,e.g. a history depth
        /// that is higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(TopicQos qos);
        /// <summary>
        /// This property returns the TopicListener currently attached to the ITopic.
        /// </summary>
        /// <remarks>
        /// Only one listener can be attached to the ITopic at any particular time. This property
        /// returns the listener that is currently attached to the ITopic. When no listener is
        /// attached the null is returned.
        /// </remarks>
        /// <returns>returns the TopicListener currently attached to the ITopic.</returns>
        ITopicListener Listener { get; }
        /// <summary>
        /// This operation attaches an ITopicListener to the ITopic.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Only one ITopicListener can be attached to each ITopic. If an ITopicListener was
        /// already attached, the operation will replace it with the new one. When listener is
        /// null, it represents a listener that is treated as a No-Operation for all statuses
        /// activated in the bit mask.
        /// </para>
        /// <para>
        /// <i><b>Communication Status</b></i><br/>
        /// For each communication status, the StatusChanged flag is initially set to false.
        /// It becomes true when the communication status changes, and for each communication
        /// status activated in the provided mask parameter, the associated ITopicListener
        /// operation is invoked and the communication status is reset to false, as the
        /// listener implicitly accesses the status which is passed as an parameter to invoked
        /// listener operation. The status is reset prior to calling the listener operation,
        /// so if the application calls the get_<status_name> from inside the listener it
        /// will see that the status is already reset. An exception to this rule it the null
        /// listener, which does not reset the communication statuses for which it is invoked.
        /// </para>
        /// <para></para>
        /// For the ITopicListener the following statuses are applicable:
        /// <list type="bullet">
        /// <item>StatusKind.InconsistentTopic.</item>
        /// </list>
        /// <para>
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. With a mask set to 0 the caller indicates that
        /// the created entity not respond to any of its available statuses.
        /// The DDS will therefore attempt to propagate these statuses to its factory.
        /// </para>
        /// <para>
        /// <i><b>Status Propagation</b></i><br/>
        /// In case a communication status is not activated in the mask of the ITopicListener,
        /// the IDomainParticipantListener of the containing IDomainParticipant is
        /// invoked (if attached and activated for the status that occurred). This allows the
        /// application to set a default behaviour in the IDomainParticipantListener of the
        /// containing DomainParticipant and a Topic specific behaviour when needed. In
        /// case the IDomainParticipantListener is also not attached or the
        /// communication status is not activated in its mask, the application is not notified
        /// of the change.
        /// </para>
        /// </remarks>
        /// <param name="listener">The listener to be attached to the ITopic.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the TopicListener for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The TopicListener is attached.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ITopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(ITopicListener listener, StatusKind mask);
    }

    /// <summary>
    /// IContentFilteredTopic is a specialization of ITopicDescription that allows for content based subscriptions.
    /// </summary>
    /// <remarks>
    /// IContentFilteredTopic describes a more sophisticated subscription that indicates that the ISubscriber
    /// does not necessarily want to see all values of each instance published under the ITopic.
    /// Rather, it only wants to see the values whose contents satisfy certain criteria.
    /// Therefore this class must be used to request content-based subscriptions. The selection
    /// of the content is done using the SQL based filter with parameters to adapt the filter clause.
    /// </remarks>
    public interface IContentFilteredTopic : ITopicDescription
    {
        /// <summary>
        /// This operation returns the filterExpression associated with the IContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// The return string is the SQL filter expression which was specified when the
        /// the IContentFilteredTopic was created. The result is a string which specifies
        /// the criteria used to select the data samples of interest. It is similar to the
        /// WHERE clause of an SQL expression.
        /// </remarks>
        /// <returns>
        /// The string that specifies the criteria to select the data samples of interest.
        /// </returns>
        string GetFilterExpression();
        /// <summary>
        /// This operation obtains the expression parameters associated with the IContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation obtains the expression parameters associated with the
        /// IContentFilteredTopic. That is, the parameters specified on the last successful
        /// call to SetExpressionParameters, or if SetExpressionParameters
        /// was never called, the parameters specified when the IContentFilteredTopic was
        /// created.
        /// </para>
        /// <para>
        /// The resulting reference holds a sequence of strings with the parameters used in the
        /// SQL expression (i.e., the %n tokens in the expression). The number of parameters in
        /// the result sequence will exactly match the number of %n tokens in the filter
        /// expression associated with the IContentFilteredTopic.
        /// </para>
        /// </remarks>
        /// <param name="expressionParameters">A reference to a sequence of strings that will be used
        /// to store the parameters used in the SQL expression.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of expression parameters applied to this IContentFilteredTopic
        /// has successfully been copied into the specified expressionParameters parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IContentFilteredTopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetExpressionParameters(ref string[] expressionParameters);
        /// <summary>
        /// This operation changes the expression parameters associated with the IContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This operation changes the expression parameters associated with the
        /// IContentFilteredTopic. The parameter expressionParameters is a sequence
        /// of strings with the parameters used in the SQL expression. The number
        /// of values in expressionParameters must be equal or greater than the
        /// highest referenced %n token in the filterExpression (for example, if %1 and
        /// %8 are used as parameter in the filterExpression, the
        /// expressionParameters should at least contain n+1 = 9 values). This is the
        /// filter expression specified when the IContentFilteredTopic was created.
        /// </remarks>
        /// <param name="expressionParameters">A sequence of strings with the parameters used in the SQL expression.
        /// The number of values in expressionParameters must be equal or greater than the highest referenced
        /// %n token in the subscriptionExpression.</param>
        /// <returns>The possible return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new expression parameters are set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IContentFilteredTopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The number of parameters in expressionParameters does not match the
        /// number of tokens in the expression for this IContentFilteredTopic or one of the parameters
        /// is an illegal parameter.</item>
        /// </list>
        /// </returns>
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
        /// <summary>
        /// This property returns the ITopic associated with the IContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// That is, the ITopic specified when the IContentFilteredTopic was created. This
        /// ITopic is the base topic on which the filtering will be applied.
        /// </remarks>
        ITopic RelatedTopic { get; }
    }

    /// <summary>
    /// IMultiTopic is a specialization of ITopicDescription that allows subscriptions to combine, filter
    /// and/or rearrange data coming from several Topics.
    /// </summary>
    /// <remarks>
    /// @note The IMultiTopic is not yet implemented. It is scheduled for a future release.
    ///
    /// IMultiTopic allows a more sophisticated subscription that can select and combine
    /// data received from multiple Topics into a single data type (specified by the
    /// inherited typeName). The data will then be filtered (selection) and possibly
    /// re-arranged (aggregation and/or projection) according to an SQL expression with
    /// parameters to adapt the filter clause.
    /// </remarks>
    public interface IMultiTopic : ITopicDescription
    {
        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// This operation returns the subscription expression associated with the IMultiTopic.
        /// </summary>
        /// <remarks>
        /// This operation returns the subscription expression associated with the IMultiTopic.
        /// That is, the expression specified when the IMultiTopic was created.
        /// The subscription expression result is a string that specifies the criteria to select the
        /// data samples of interest. In other words, it identifies the selection and rearrangement
        /// of data from the associated ITopics. It is an SQL expression where the SELECT
        /// clause provides the fields to be kept, the FROM part provides the names of the
        /// Topics that are searched for those fields, and the WHERE clause gives the content
        /// filter. The ITopics combined may have different types but they are restricted in that
        /// the type of the fields used for the NATURAL JOIN operation must be the same
        /// </remarks>
        string SubscriptionExpression { get; }
        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// This operation obtains the expression parameters associated with the IMultiTopic.
        /// </summary>
        /// <remarks>
        /// This operation obtains the expression parameters associated with the IIMultiTopic.
        /// That is, the parameters specified on the last successful cal l to
        /// SetExpressionParameters, or if SetExpressionParameters was
        /// never called, the parameters specified when the MultiTopic was created.
        /// The resulting reference holds a sequence of strings with the values of the parameters
        /// used in the SQL expression (i.e., the %n tokens in the expression). The number of
        /// parameters in the result sequence will exactly match the number of %n tokens in the
        /// filter expression associated with the MultiTopic.
        /// </remarks>
        /// <param name="expressionParameters">A sequence of strings with the parameter value used in the SQL expression.</param>
        /// <returns>The possible return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the existing set of expression parameters applied to this IMultiTopic has
        /// successfully been copied into the specified expressionParameters parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IIMultiTopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        ReturnCode GetExpressionParameters(ref string[] expressionParameters);
        /// <summary>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <remarks>
        /// This operation changes the expression parameters associated with the IMultiTopic.
        /// The parameter expressionParameters is a handle to a sequence of strings with
        /// the parameters used in the SQL expression. The number of parameters in
        /// expressionParameters must exactly match the number of %n tokens in the
        /// subscription expression associated with the IMultiTopic. This is the subscription
        /// expression specified when the IMultiTopic was created.
        /// </remarks>
        /// <param name="expressionParameters">A sequence of strings with the parameter value used in the SQL expression.</param>
        /// <returns>The possible return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new expression parameters are set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occured.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IMultiTopic has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The number of parameters in expressionParameters does not match the
        /// number of tokens in the expression for this IMultiTopic or one of the parameters
        /// is an illegal parameter.</item>
        /// </list>
        /// </returns>
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
    }

    // ----------------------------------------------------------------------
    // IPublisher/ISubscriber, IDataWriter/IDataReader
    // ----------------------------------------------------------------------
    /// <summary>
    /// The IPublisher acts on behalf of one or more IDataWriter objects that belong to
    /// it. When it is informed of a change to the data associated with one of its
    /// IDataWriter objects, it decides when it is appropriate to actually process the
    /// sample-update message. In making this decision, it considers the PublisherQos
    /// and the DataWriterQos.
    /// </summary>
    ///
    public interface IPublisher : IEntity
    {
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <remarks>
        /// <seealso cref=" CreateDataWriter(ITopic topic, DataWriterQos qos ,IDataWriterListener listener, StatusKind mask)">
        /// </remarks>
        /// <param name="topic">The topic for which the IDataWriter is created.</param>
        /// <returns>The newly created IDataWriter. In case of an error a null value IDataWriter is returned.</returns>
        IDataWriter CreateDataWriter(ITopic topic);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters.
        /// </summary>
        /// <param name="topic">The topic for which the IDataWriter is created.</param>
        /// <param name="listener">The IDataWriterListener instance which will be attached to the new
        /// IDataWriter. It is permitted to use null as the value of the listener: this
        /// behaves as a IDataWriterListener whose operations perform no action.</param>
        /// <seealso cref=" CreateDataWriter(ITopic topic, DataWriterQos qos ,IDataWriterListener listener, StatusKind mask)">
        /// <param name="mask"> bit mask in which each bit enables the invocation of the IDataWriterListener
        /// for a certain status.</param>
        /// <returns>The newly created IDataWriter. In case of an error a null value IDataWriter is returned.</returns>
        IDataWriter CreateDataWriter(
                ITopic topic,
                IDataWriterListener listener, StatusKind mask);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default null for listeners and 0 mask for listener parameters.
        /// </summary>
        /// <remarks>
        /// <seealso cref=" CreateDataWriter(ITopic topic, DataWriterQos qos ,IDataWriterListener listener, StatusKind mask)">
        /// </remarks>
        /// <param name="topic">The topic for which the IDataWriter is created.</param>
        /// <param name="qos">The DataWriterQos for the new IDataWriter.
        /// In case these settings are not self consistent, no IDataWriter is created.</param>
        /// <returns>The newly created IDataWriter. In case of an error a null value IDataWriter is returned.</returns>
        IDataWriter CreateDataWriter(ITopic topic, DataWriterQos qos);
        /// <summary>
        /// This operation creates a IDataWriter with the desired DataWriterQos, for the
        /// desired ITopic and attaches the optionally specified IDataWriterListener to it.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation creates a IDataWriter with the desired DataWriterQos, for the
        /// desired ITopic and attaches the optionally specified IDataWriterListener to it.
        /// The returned IDataWriter is attached (and belongs) to the IPublisher on which
        /// this operation is being called. To delete the IDataWriter the operation
        /// DeleteDatawriter or DeleteContainedEntities must be used.
        /// </para><para>
        /// For an example see <see cref="Space.FooDataWriter"/> detailed description.
        /// </para>
        /// </remarks>
        /// <param name="topic">The topic for which the IDataWriter is created.</param>
        /// <param name="qos">The DataWriterQos for the new IDataWriter.
        /// In case these settings are not self consistent, no IDataWriter is created.</param>
        /// <param name="listener">The IDataWriterListener instance which will be attached to the new
        /// IDataWriter. It is permitted to use null as the value of the listener: this
        /// behaves as a IDataWriterListener whose operations perform no action.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the IDataWriterListener
        /// for a certain status.</param>
        /// <returns>The newly created IDataWriter. In case of an error a null value IDataWriter is returned.</returns>
        IDataWriter CreateDataWriter(
                ITopic topic,
                DataWriterQos qos,
                IDataWriterListener listener,
                StatusKind mask);
        /// <summary>
        /// This operation deletes a IDataWriter that belongs to the IPublisher.
        /// </summary>
        /// <remarks>
        /// This operation deletes a IDataWriter that belongs to the IPublisher. When the
        /// operation is called on a different IPublisher, as used when the IDataWriter was
        /// created, the operation has no effect and returns
        /// PreconditionNotMet. The deletion of the IDataWriter will automatically unregister all instances.
        /// Depending on the settings of WriterDataLifecycleQosPolicy, the deletion of the IDataWriter may also
        /// dispose of all instances.
        /// </remarks>
        /// <param name="dataWriter">The IDataWriter which is to be deleted.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IDataWriter is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IPublisher has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter dataWriter is not a valid IDataWriter reference.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is called on a different IPublisher, as used when the
        /// IDataWriter was created.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteDataWriter(IDataWriter dataWriter);
        /// <summary>
        /// This operation returns a previously created IDataWriter belonging to the IPublisher
        /// which is attached to a ITopic with the matching topicName.
        /// </summary>
        /// <remarks>
        /// This operation returns a previously created IDataWriter belonging to the
        /// IPublisher which is attached to a ITopic with the matching topicName. When
        /// multiple IDataWriter objects (which satisfy the same condition) exist, this
        /// operation will return one of them. It is not specified which one.
        /// </remarks>
        /// <param name="topicName">the name of the ITopic, which is attached to the IDataWriter to look for.</param>
        /// <returns>The IDataWriter found. When no such IDataWriter is found, a IDataWriter of null value is returned.</returns>
        IDataWriter LookupDataWriter(string topicName);
        /// <summary>
        /// This operation deletes all the IDataWriter objects that were created by means of
        /// one of the CreateDataWriter operations on the IPublisher.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the IDataWriter objects that were created by means of
        /// one of the CreateDatawriter operations on the IPublisher. In other words, it
        /// deletes all contained IDataWriter objects.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The contained IEntity objects are deleted and the application may delete the IPublisher.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IPublisher has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteContainedEntities();
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a IPublisher.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a IPublisher.
        /// The parameter qos contains the QosPolicy settings which is checked for
        /// self-consistency and mutability. When the application tries to change a QosPolicy
        /// setting for an enabled IPublisher, which can only be set before the IPublisher is
        /// enabled, the operation will fail and a ImmutablePolicy is returned. In
        /// other words, the application must provide the currently set QosPolicy settings in
        /// case of the immutable QosPolicy settings. Only the mutable QosPolicy settings
        /// can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and a InconsistentPolicy is
        /// returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned OK). If one or more of the partitions in the QoS
        /// structure have insufficient access rights configured then the SetQos function will
        /// fail with a PreconditionNotMet error code.
        /// </remarks>
        /// <param name="qos">The new set of QosPolicy settings for the IPublisher.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new PublisherQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IPublisher has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter qos is not a valid PublisherQos. It contains a
        /// QosPolicy setting with an enum value that is outside its legal boundaries.</item>
        /// <item>DDS.ReturnCode Unsupported - One ore more of the selected QosPolicy values are currently not
        /// supported by OpenSplice.</item>
        /// <item>DDS.ReturnCode ImmutablePolicy - The parameter qos contains an immutable Qospolicy setting with a
        /// different value than set during enabling of the IPublisher.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - Insufficient access rights exist for the partition(s) listed in the QoS structure</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(PublisherQos qos);
        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a IPublisher.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a IPublisher on
        /// which this operation is used. This PublisherQos is stored at the location pointed
        /// to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination PublisherQos struct in which the
        /// QosPolicy settings will be copied.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of QoS policy values applied to this IPublisher has successfully
        /// been copied into the specified PublisherQos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IPublisher has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref PublisherQos qos);
        /// <summary>
        /// This property returns the PublisherListener currently attached to the IPublisher.
        /// </summary>
        /// <remarks>
        /// Only one listener can be attached to the IPublisher at any particular time. This property
        /// returns the listener that is currently attached to the IPublisher.
        /// </remarks>
        /// <returns>returns the PublisherListener currently attached to the IPublisher.</returns>
       IPublisherListener Listener { get; }
        /// <summary>
        /// This operation attaches a PublisherListener to the IPublisher.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation attaches a PublisherListener to the IPublisher. Only one
        /// PublisherListener can be attached to each IPublisher. If a PublisherListener
        /// was already attached, the operation will replace it with the new one.
        /// When listener is null, it represents a listener that is
        /// treated as a No-Operation for all statuses activated in the bit mask.
        /// </para>
        /// <para><b><i>Communication Status</i></b></para>
        /// <para>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask, the associated
        /// PublisherListener operation is invoked and the communication status is
        /// reset to fase, as the listener implicitly accesses the status which is passed as a
        /// parameter to that operation. The status is reset prior to calling the listener, so if the
        /// application calls the Get<status_name>Status from inside the listener it will
        /// see the status already reset. An exception to this rule is the null
        /// listener, which does not reset the communication statuses for which it is invoked.
        /// The following statuses are applicable to the PublisherListener:
        /// <list type="bullet">
        /// <item>DDS.StatusKind OfferedDeadLineMissedStatus (propagated)</item>
        /// <item>DDS.StatusKind OfferedIncompatibleQosStatus (propagated)</item>
        /// <item>DDS.StatusKind LivelinessLostStatus (propagated)</item>
        /// <item>DDS.StatusKind PublicationMatchStatus (propagated)</item>
        /// </list>
        /// Be aware that the DDS.StatusKind.PublicationMatchStatus is not applicable when
        /// the infrastructure does not have the information available to determine connectivity.
        /// This is the case when OpenSplice is configured not to maintain discovery
        /// information in the Networking Service. In this case the operation will return
        /// DDS.ReturnCode Unsupported.
        /// </para>
        /// <para>
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The value 0 can be used to indicate that the
        /// created entity should not respond to any of its available statuses. The DDS will
        /// therefore attempt to propagate these statuses to its factory.
        /// </para>
        /// <para><b><i>Status Propagation</i></b></para>
        /// <para>
        /// The Data Distribution Service will trigger the most specific and relevant Listener.
        /// In other words, in case a communication status is also activated on the
        /// IDataWriterListener of a contained IDataWriter, the IDataWriterListener on that
        /// contained IDataWriter is invoked instead of the PublisherListener. This means that
        /// a status change on a contained IDataWriter only invokes the PublisherListener if the
        /// contained IDataWriter itself does not handle the trigger event generated by
        /// the status change.
        /// </para>
        /// <para>
        /// In case a status is not activated in the mask of the PublisherListener, the
        /// DomainParticipantListener of the containing IDomainParticipant is invoked
        /// (if attached and activated for the status that occurred). This allows the
        /// application to set a default behaviour in the DomainParticipantListener of the
        /// containing IDomainParticipant and an IPublisher specific behaviour when
        /// needed. In case the DomainParticipantListener is also not attached or
        /// the communication status is not activated in its mask, the application is not notified
        /// of the change.
        /// </para>
        /// </remarks>
        /// <param name="listener">The PublisherListener instance which will be attached to the publisher.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the PublisherListener
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The PublisherListener is attached.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IPublisher has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(IPublisherListener listener, StatusKind mask);
        /// <summary>
        /// This operation will suspend the dissemination of the publications by all contained IDataWriter objects.
        /// </summary>
        /// <remarks>
        /// This operation suspends the publication of all IDataWriter objects contained by
        /// this IPublisher. The data written, disposed or unregistered by a IDataWriter is
        /// stored in the history buffer of the IDataWriter and therefore, depending on its QoS
        /// settings, the following operations may block (see the operation descriptions for
        /// more information):
        /// <list type="bullet">
        /// <item>DDS.IDataWriter.Dispose</item>
        /// <item>DDS.IDataWriter.DisposeWithTimestamp</item>
        /// <item>DDS.IDataWriter.Write</item>
        /// <item>DDS.IDataWriter.WriteWithTimestamp</item>
        /// <item>DDS.IDataWriter.WriteDispose</item>
        /// <item>DDS.IDataWriter.WriteDisposeWithTimestamp</item>
        /// <item>DDS.IDataWriter.UnregisterInstance</item>
        /// <item>DDS.IDataWriter.UnregisterInstanceWithTimestamp</item>
        /// </list>
        /// <seealso cref="Space.FooDataWriter"/>
        /// Subsequent calls to this operation have no effect. When the IPublisher is deleted
        /// before ResumePublications is called, all suspended updates are discarded.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the IPublisher has been suspended</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IPublisher is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode SuspendPublications();
        /// <summary>
        /// This operation resumes a previously suspended publication.
        /// </summary>
        /// <remarks>
        /// If the IPublisher is suspended, this operation will resume the publication of all
        /// IDataWriter objects contained by this IPublisher. All data held in the history
        /// buffer of the IDataWriter's is actively published to the consumers. When the
        /// operation returns all IDataWriter's have resumed the publication of suspended
        /// updates.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the IPublisher has resumed publications</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IPublisher is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The IPublisher is not suspended.</item>
        /// </list>
        /// </returns>
        ReturnCode ResumePublications();
        /// <summary>
        /// This operation requests that the application will begin a ‘coherent set’ of
        /// modifications using IDataWriter objects attached to this IPublisher.
        /// The ‘coherent set’ will be completed by a matching call to IPublisher.EndCoherentChanges.
        /// </summary>
        /// <remarks>
        /// <para>
        /// A ‘coherent set’ is a set of modifications that must be propagated in such a way that
        /// they are interpreted at the receivers’ side as a consistent set of modifications; that is,
        /// the receiver will only be able to access the data after all the modifications in the set
        /// are available at the receiver end.
        /// A precondition for making coherent changes is that the PresentationQos of the
        /// IPublisher has its CoherentAccess attribute set to true. If this is not the
        /// case, the IPublisher will not accept any coherent start requests and return
        /// DDS.ReturnCode PreconditionNotMet.
        /// </para><para>
        /// A connectivity change may occur in the middle of a set of coherent changes; for
        /// example, the set of partitions used by the IPublisher or one of its connected
        /// ISubscribers may change, a late-joining IDataReader may appear on
        /// the network, or a communication failure may occur. In the event that such a change
        /// prevents an entity from receiving the entire set of coherent changes, that entity must
        /// behave as if it had received none of the set.
        /// These calls can be nested. In that case, the coherent set terminates only with the last
        /// call to IPublisher.EndCoherentChanges.
        /// </para><para>
        /// The support for ‘coherent changes’ enables a publishing application to change the
        /// value of several data-instances that could belong to the same or different topics and
        /// have those changes be seen ‘atomically’ by the readers. This is useful in cases where
        /// the values are inter-related (for example, if there are two data-instances representing
        /// the ‘altitude’ and ‘velocity vector’ of the same aircraft and both are changed, it may
        /// be useful to communicate those values in a way the reader can see both together;
        /// otherwise, it may e.g., erroneously interpret that the aircraft is on a collision course).
        /// </para>
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - a new coherent change has successfully been started.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - the IPublisher is not able to handle
        /// coherent changes because its PresentationQos has not set CoherentAccess to true.</item>
        /// </list>
        /// </returns>
        ReturnCode BeginCoherentChanges();
        /// <summary>
        /// This operation terminates the ‘coherent set’ initiated by the matching call to
        /// IPublisher.BeginCoherentChanges.
        /// </summary>
        /// <remarks>
        /// If there is no matching call to IPublisher.BeginCoherentChanges, the operation will return the error
        /// DDS.ReturnCode PreconditionNotMe.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the coherent change has successfully been closed.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - there is no matching IPublisher.BeginCoherentChanges
        /// call that can be closed.</item>
        /// </list>
        /// </returns>
        ReturnCode EndCoherentChanges();
        /// <summary>
        /// This operation blocks the calling thread until either all data written by all contained
        /// DataWriters is acknowledged by the local infrastructure, or until the duration
        /// specified by maxWait parameter elapses, whichever happens first.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation blocks the calling thread until either all data written by all contained
        /// DataWriters is acknowledged by the local infrastructure, or until the duration
        /// specified by maxWait parameter elapses, whichever happens first.
        /// Data is acknowledged by the local infrastructure when it does not need to be stored
        /// in its IDataWriter’s local history. When a locally-connected subscription (including
        /// the networking service) has no more resources to store incoming samples it will start
        /// to reject these samples, resulting in their source DataWriters to store them
        /// temporarily in their own local history to be retransmitted at a later moment in time.
        /// In such scenarios, the WaitForAcknowledgments operation will block until all
        /// contained DataWriters have retransmitted their entire history, which is therefore
        /// effectively empty, or until the maxWait timeout expires, whichever happens first.
        /// In the first case the operation will return Ok, in the latter it will return
        /// Timeout.
        /// </para><para>
        /// Be aware that in case the operation returns Ok, the data has only been
        /// acknowledged by the local infrastructure: it does not mean all remote subscriptions
        /// have already received the data. However, delivering the data to remote nodes is then
        /// the sole responsibility of the networking service: even when the publishing
        /// application would terminate, all data that has not yet been received may be
        /// considered ‘on-route’ and will therefore eventually arrive (unless the networking
        /// service itself will crash). In contrast, if an IDataWriter would still have data in its local
        /// history buffer when it terminates, this data is considered ‘lost’.
        /// This operation is intended to be used only if one or more of the contained
        /// DataWriters has its ReliabilityQosPolicyKind set to ReliableReliabilityQos
        /// Otherwise the operation will return immediately
        /// with Ok, since best-effort DataWriters will never store rejected samples
        /// in their local history: they will just drop them and continue business as usual.
        /// </para>
        /// </remarks>
        /// <param name="maxWait">the maximum duration to block for tacknowledgments,
        /// after which the application thread is unblocked. The special constant
        /// Duration.Infinte can be used when the maximum waiting time does not need to be bounded.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the data of all contained DataWriters has been acknowledged by
        /// the local infrastructure.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled- the IPublisher is not enabled.</item>
        /// <item>DDS.ReturnCode Timeout - not all data is acknowledged before maxWait elapsed.</item>
        /// </list>
        /// </returns>
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        /// <summary>
        /// This operation returns the IDomainParticipant associated with the IPublisher.
        /// </summary>
        /// <remarks>
        /// Note that there is exactly one IDomainParticipant associated with each
        /// IPublisher. When the IPublisher was already deleted (there is no associated
        /// IDomainParticipant any more), the null reference is returned.
        /// </remarks>
        /// <returns>The IDomainParticipant associated with the IPublisher.</returns>
        IDomainParticipant GetParticipant();
        /// <summary>
        /// This operation sets the default DataWriterQos of the IPublisher.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The default DataWriterQos is used when one of the IPublisher.CreateDataWriter
        /// operations is used to create the IDataWriter which does not have the DataWriterQos
        /// as parameter.
        /// </para><para>
        /// The SetDefaultDataWriterQos operation checks if the DataWriterQos is self
        /// consistent. If it is not, the operation has no effect and returns
        /// InconsistentPolicy. The values set by this operation are returned by GetDefaultDataWriterQos.
        /// </para>
        /// </remarks>
        /// <param name="qos">The DataWriterQos struct, which contains the new default DataWriterQos
        /// for the newly created DataWriters.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new default DataWriterQos is set.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - the parameter qos is not a valid DataWriterQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries.</item>
        /// <item>DDS.ReturnCode InconsistentPolicy - the parameter qos contains conflicting
        /// QosPolicy settings, e.g. a history depth that is higher than the specified resource
        /// limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultDataWriterQos(DataWriterQos qos);
        /// <summary>
        /// This operation gets the default DataWriterQos of the IPublisher.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The application must provide the DataWriterQos struct in which the QosPolicy
        /// settings can be stored and pass the qos reference to the operation. The operation
        /// writes the default DataWriterQos to the struct referenced to by qos. Any settings
        /// in the struct are overwritten.
        /// </para><para>
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultDatawriterQos, or, if the call was never made,
        /// the default values as specified for each QosPolicy setting.
        /// </para>
        /// </remarks>
        /// <param name="qos">A reference to the DataWriterQos struct (provided by the application) in which
        /// the default DataWriterQos for the IDataWriter is written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new default IDataWriter QosPolicy settings for the publisher have successfully
        /// been copied into the specified DataWriterQos policy.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultDataWriterQos(ref DataWriterQos qos);
        /// <summary>
        /// This operation will copy policies in topicQos to the corresponding policies in DataWriterQos.
        /// </summary>
        /// <remarks>
        /// This operation will copy the QosPolicy settings in topicQos to the
        /// corresponding QosPolicy settings in datawriterQos (replacing the values
        /// in DataWriterQos, if present). This will only apply to the common
        /// QosPolicy settings in each "IEntity" Qos.
        /// This is a convenience operation, useful in combination with the operations
        /// GetDefaultDataWriterQos and ITopic.GetQos. The operation
        /// CopyFromTopicQos can be used to merge the IDataWriter default
        /// QosPolicy settings with the corresponding ones on the TopicQos. The resulting
        /// DataWriterQos can then be used to create a new IDataWriter, or set its
        /// DataWriterQos.
        /// This operation does not check the resulting dataWriterQos for consistency.
        /// This is because the merged DataWriterQos may not be the final one, as the
        /// application can still modify some QosPolicy settings prior to applying the
        /// DataWriterQos to the IDataWriter.
        /// </remarks>
        /// <param name="dataWriterQos">The destination DataWriterQos struct to which the QosPolicy settings
        /// should be copied.</param>
        /// <param name="topicQos">The source TopicQos struct, which should be copied.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The QosPolicy settings are copied from the topic IDataWriter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode CopyFromTopicQos(ref DataWriterQos dataWriterQos, TopicQos topicQos);
    }

    /// <summary>
    /// IDataWriter allows the application to set the value of the sample to be published
    /// under a given ITopic.
    /// </summary>
    /// <remarks>
    /// A IDataWriter is attached to exactly one IPublisher which acts as a factory for it.
    /// A IDataWriter is bound to exactly one ITopic and therefore to exactly one data
    /// type. The ITopic must exist prior to the IDataWriter's creation.
    /// IDataWriter is an abstract class. It must be specialized for each particular
    /// application data type. For a fictional application data type Foo (defined in the
    /// module Space) the specialized class would be Space.FooDataWriter.
    /// <seealso cref="Space.FooDataWriter"/>
    /// </remarks>
    public interface IDataWriter : IEntity
    {
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a IDataWriter.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a IDataWriter.
        /// The parameter qos contains the struct with the QosPolicy settings which is
        /// checked for self-consistency and mutability. When the application tries to change a
        /// QosPolicy setting for an enabled IDataWriter, which can only be set before the
        /// IDataWriter is enabled, the operation will fail and a
        /// ImmutablePolicy is returned. In other words, the application must
        /// provide the presently set QosPolicy settings in case of the immutable QosPolicy
        /// settings. Only the mutable QosPolicy settings can be changed. When qos contains
        /// conflicting QosPolicy settings (not self-consistent), the operation will fail and a
        /// InconsistentPolicy is returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned OK).
        /// </remarks>
        /// <param name="qos">new set of QosPolicy settings for the IDataWriter.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the new default DataWriterQos is set</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - the parameter qos is not a valid DataWriterQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are
        /// currently not supported by OpenSplice.</item>
        /// <item>DDS.ReturnCode ImmutablePolicy - the parameter qos contains an immutable
        /// QosPolicy setting with a different value than set during enabling of the
        /// IDataWriter.</item>
        /// <item>DDS.ReturnCode InconsistentPolicy - the parameter qos contains an inconsistent QosPolicy settings,
        /// e.g. a history depth that is higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(DataWriterQos qos);
        /// <summary>
        /// This operation allows access to the existing list of QosPolicy settings for a IDataWriter.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing list of QosPolicy settings of a
        /// IDataWriter on which this operation is used. This DataWriterQos is stored at the
        /// location pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination DataWriterQos struct in which the
        /// QosPolicy settings will be copied into.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the existing set of QoS policy values applied to this IDataWriter
        /// has been successfully copied into the specified qos parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IPublisher has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref DataWriterQos qos);
        /// <summary>
        /// This property returns the IDataWriterListener currently attached to the IDataWriter.
        /// </summary>
        /// <remarks>
        /// Only one listener can be attached to the IDataWriter at any particular time. This property
        /// returns the listener that is currently attached to the IDataWriter.
        /// </remarks>
        /// <returns>returns the IDataWriterListener currently attached to the IDataWriter.</returns>
        IDataWriterListener Listener { get; }
        /// <summary>
        /// This operation attaches a IDataWriterListener to the IDataWriter.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Only one IDataWriterListener can be attached to each IDataWriter. If a IDataWriterListener
        /// was already attached, the operation will replace it with the new one.
        /// When listener is null, it represents a listener that is
        /// treated as a No-Operation for all statuses activated in the bit mask.
        /// </para>
        /// <para><b><i>Communication Status</i></b></para>
        /// <para>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask, the associated
        /// IDataWriterListener operation is invoked and the communication status is
        /// reset to fase, as the listener implicitly accesses the status which is passed as a
        /// parameter to that operation. The status is reset prior to calling the listener, so if the
        /// application calls the Get<status_name>Status from inside the listener it will
        /// see the status already reset. An exception to this rule is the null
        /// listener, which does not reset the communication statuses for which it is invoked.
        /// The following statuses are applicable to the IDataWriterListener:
        /// <list type="bullet">
        /// <item>DDS.StatusKind OfferedDeadLineMissedStatus</item>
        /// <item>DDS.StatusKind OfferedIncompatibleQosStatus</item>
        /// <item>DDS.StatusKind LivelinessLostStatus</item>
        /// <item>DDS.StatusKind PublicationMatchStatus</item>
        /// </list>
        /// Be aware that the DDS.StatusKind.PublicationMatchStatus is not applicable when
        /// the infrastructure does not have the information available to determine connectivity.
        /// This is the case when OpenSplice is configured not to maintain discovery
        /// information in the Networking Service. In this case the operation will return
        /// DDS.ReturnCode Unsupported.
        /// </para>
        /// <para>
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The value 0 can be used to indicate that the
        /// created entity should not respond to any of its available statuses. The DDS will
        /// therefore attempt to propagate these statuses to its factory.
        /// </para>
        /// <para><b><i>Status Propagation</i></b></para>
        /// <para>
        /// The Data Distribution Service will trigger the most specific and relevant Listener.
        /// In other words, in case a communication status is also activated on the
        /// IDataWriterListener of a contained IDataWriter, the IDataWriterListener on that
        /// contained IDataWriter is invoked instead of the PublisherListener. This means that
        /// a status change on a contained IDataWriter only invokes the PublisherListener if the
        /// contained IDataWriter itself does not handle the trigger event generated by
        /// the status change.
        /// </para>
        /// <para>
        /// In case a status is not activated in the mask of the IDataWriterListener, the
        /// PublisherListener of the containing IPublisher is invoked
        /// (if attached and activated for the status that occurred). This allows the
        /// application to set a default behaviour in the PublisherListener of the
        /// containing IPublisher and an IDataWriter specific behaviour when
        /// needed. In case the communication status is not activated in the mask of the
        /// PublisherListener as well, the communication status will be propagated to the
        /// DomainParticipantListener of the containing IDomainParticipant. In case the
        /// DomainParticipantListener is also not attached or the communication status is
        /// not activated in its mask, the application is not notified of the change.
        /// </para>
        /// </remarks>
        /// <param name="listener">The IDataWriterListener instance which will be attached to the DataWriter.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the IDataWriterListener
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IDataWriterListener is attached.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(IDataWriterListener listener, StatusKind mask);
        /// <summary>
        /// This property returns the ITopic which is associated with the IDataWriter.
        /// </summary>
        ITopic Topic { get; }
        /// <summary>
        /// This property returns the IPublisher to which the IDataWriter belongs.
        /// </summary>
        IPublisher Publisher { get; }
        /// <summary>
        /// This operation blocks the calling thread until either all data written by all contained
        /// IDataWriter is acknowledged by the local infrastructure, or until the duration
        /// specified by maxWait parameter elapses, whichever happens first.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Data is acknowledged by the local infrastructure when it does not need to be stored
        /// in its IDataWriter’s local history. When a locally-connected subscription (including
        /// the networking service) has no more resources to store incoming samples it will start
        /// to reject these samples, resulting in their source DataWriter to store them
        /// temporarily in its own local history to be retransmitted at a later moment in time.
        /// In such scenarios, the WaitForAcknowledgments operation will block until the
        /// IDataWriter has retransmitted its entire history, which is therefore
        /// effectively empty, or until the maxWait timeout expires, whichever happens first.
        /// In the first case the operation will return Ok, in the latter it will return
        /// Timeout.
        /// </para><para>
        /// Be aware that in case the operation returns Ok, the data has only been
        /// acknowledged by the local infrastructure: it does not mean all remote subscriptions
        /// have already received the data. However, delivering the data to remote nodes is then
        /// the sole responsibility of the networking service: even when the publishing
        /// application would terminate, all data that has not yet been received may be
        /// considered ‘on-route’ and will therefore eventually arrive (unless the networking
        /// service itself will crash). In contrast, if an IDataWriter would still have data in its local
        /// history buffer when it terminates, this data is considered ‘lost’.
        /// This operation is intended to be used only if the IDataWriter has its ReliabilityQosPolicyKind
        /// set to ReliableReliabilityQos Otherwise the operation will return immediately
        /// with Ok, since best-effort DataWriters will never store rejected samples
        /// in their local history: they will just drop them and continue business as usual.
        /// </para>
        /// </remarks>
        /// <param name="maxWait">the maximum duration to block for acknowledgments,
        /// after which the application thread is unblocked. The special constant
        /// Duration.Infinte can be used when the maximum waiting time does not need to be bounded.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the data of the IDataWriter has been acknowledged by
        /// the local infrastructure.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled- the IDataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode Timeout - not all data is acknowledged before maxWait elapsed.</item>
        /// </list>
        /// </returns>
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        /// <summary>
        /// This operation obtains the LivelinessLostStatus struct of the IDataWriter.
        /// </summary>
        /// <remarks>
        /// This struct contains the information whether the liveliness (that the IDataWriter
        /// has committed through its LivelinessQosPolicy) was respected.
        /// This means, that the status represents whether the IDataWriter failed to actively
        /// signal its liveliness within the offered liveliness period. If the liveliness is lost, the
        /// IDataReader objects will consider the IDataWriter as no longer alive.
        /// The LivelinessLostStatus can also be monitored using a
        /// IDataWriterListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to LivelinessLostStatus where the contents of the LivelinessLostStatus
        /// struct of the IDataWriter will be copied into.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current LivelinessLostStatus of this IDataWriter has successfully
        /// been copied into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetLivelinessLostStatus(ref LivelinessLostStatus status);
        /// <summary>
        /// This operation obtains the OfferedDeadlineMissedStatus struct of the IDataWriter.
        /// </summary>
        /// <remarks>
        /// This struct contains the information whether the deadline (that the
        /// IDataWriter has committed through its DeadlineQosPolicy) was respected for
        /// each instance.
        /// The OfferedDeadlineMissedStatus can also be monitored using a
        /// IDataWriterListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to OfferedDeadlineMissedStatus  where the contents of the
        /// OfferedDeadlineMissedStatus struct of the IDataWriter will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current OfferedDeadlineMissedStatus of this IDataWriter has successfully
        /// been copied into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetOfferedDeadlineMissedStatus(ref OfferedDeadlineMissedStatus status);
        /// <summary>
        /// This operation obtains the OfferedIncompatibleQosStatus struct of the IDataWriter.
        /// </summary>
        /// <remarks>
        /// This struct contains the information whether a QosPolicy setting
        /// was incompatible with the requested QosPolicy setting.
        /// This means, that the status represents whether a IDataReader object has been
        /// discovered by the IDataWriter with the same ITopic and a requested
        /// DataReaderQos that was incompatible with the one offered by the IDataWriter.
        /// The OfferedIncompatibleQosStatus can also be monitored using a
        /// IDataWriterListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to  OfferedIncompatibleQosStatus where the contents of the
        /// OfferedIncompatibleQosStatus struct of the IDataWriter will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current OfferedIncompatibleQosStatus of this IDataWriter has successfully
        /// been copied into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetOfferedIncompatibleQosStatus(ref OfferedIncompatibleQosStatus status);
        /// <summary>
        /// This operation obtains the PublicationMatchedStatus struct of the IDataWriter.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This object contains the information whether a new match has been discovered
        /// for the current publication, or whether an existing match has ceased to exist.
        /// </para><para>
        /// This means that the status represents that either an IDataReader object has been
        /// discovered by the IDataWriter with the same ITopic and a compatible Qos, or that a
        /// previously discovered IDataReader has ceased to be matched to the current
        /// IDataWriter. An IDataReader may cease to match when it gets deleted, when it
        /// changes its Qos to a value that is incompatible with the current IDataWriter or
        /// when either the IDataWriter or the IDataReader has chosen to put its matching
        /// counterpart on its ignore-list using the IgnoreSubcription or
        /// IgnorePublication operations on the IDomainParticipant.
        /// </para><para>
        /// The operation may fail if the infrastructure does not hold the information necessary
        /// to fill in the PublicationMatchedStatus. This is the case when OpenSplice is
        /// configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return Unsupported.
        /// </para><para>
        /// The OfferedIncompatibleQosStatus can also be monitored using a
        /// IDataWriterListener or by using the associated IStatusCondition.
        /// </para>
        /// </remarks>
        /// <param name="status">A reference to  OfferedIncompatibleQosStatus where the contents of the
        /// OfferedIncompatibleQosStatus struct of the IDataWriter will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current PublicationMatchedStatus of this IDataWriter has successfully
        /// been copied into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the
        /// information about “associated” subscriptions.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetPublicationMatchedStatus(ref PublicationMatchedStatus status);
        /// <summary>
        /// This operation asserts the liveliness for the IDataWriter.
        /// </summary>
        /// <remarks>
        /// This operation will manually assert the liveliness for the IDataWriter. This way,
        /// the Data Distribution Service is informed that the corresponding IDataWriter is
        /// still alive. This operation is used in combination with the LivelinessQosPolicy
        /// set to ManualByParticipantLivelinessQos or ManualByTopicLivelinessQos.
        /// Writing data via the write operation of a IDataWriter will assert the liveliness on
        /// the IDataWriter itself and its containing IDomainParticipant. Therefore,
        /// AssertLiveliness is only needed when not writing regularly.
        /// The liveliness should be asserted by the application, depending on the
        /// LivelinessQosPolicy. Asserting the liveliness for this IDataWriter can also
        /// be achieved by asserting the liveliness to the IDomainParticipant.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The liveliness of this IDataWriter has successfully been asserted.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The IDataWriter is not enabled. </item>
        /// </list>
        /// </returns>
        ReturnCode AssertLiveliness();
        /// <summary>
        /// This operation retrieves the list of subscriptions currently "associated" with the IDataWriter.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation retrieves the list of subscriptions currently "associated" with the
        /// IDataWriter. That is, subscriptions that have a matching Topic and compatible
        /// QoS that the application has not indicated should be “ignored” by means of the
        /// IgnoreSubscription operation on the DomainParticipant class.
        /// </para><para>
        /// The handles returned in the subscriptionHandles array are the ones that are
        /// used by the DDS implementation to locally identify the corresponding matched
        /// subscription entities. You can access more detailed information about a particular
        /// subscription by passing its subscriptionHandle to either the
        /// GetMatchedSubscriptionData operation or to the ReadInstance
        /// operation on the built-in reader for the “DCPSSubscription” topic.
        /// </para><para>
        /// Be aware that since an instance handle is an opaque datatype, it does not necessarily
        /// mean that the handles obtained from the GetMatchedSubscriptions
        /// operation have the same value as the ones that appear in the InstanceHandle
        /// field of the SampleInfo when retrieving the subscription info through
        /// corresponding "DCPSSubscriptions" built-in reader. You can’t just compare two
        /// handles to determine whether they represent the same subscription. If you want to
        /// know whether two handles actually do represent the same subscription, use both
        /// handles to retrieve their corresponding SubscriptionBuiltinTopicData
        /// samples and then compare the key field of both samples.
        /// </para><para>
        /// The operation may fail if the infrastructure does not locally maintain the
        /// connectivity information. This is the case when OpenSplice is configured not to
        /// maintain discovery information in the Networking Service. (See the description for
        /// the NetworkingService/Discovery/enabled property in the Deployment
        /// Manual for more information about this subject.) In such cases the operation will
        /// return ReturnCode Unsupported.
        /// </para>
        /// </remarks>
        /// <param name="subscriptionHandles">Reference to an array to store the list of
        /// associated subscriptions</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The list of associated subscriptions has successfully been obtained.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the
        /// information about “associated” subscriptions.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The IDataWriter is not enabled. </item>
        /// </list>
        /// </returns>
        ReturnCode GetMatchedSubscriptions(ref InstanceHandle[] subscriptionHandles);
        /// <summary>
        /// This operation retrieves information on the specified subscription that is currently
        /// “associated” with the IDataWriter.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation retrieves information on the specified subscription that is currently
        /// “associated” with the IDataWriter. That is, a subscription with a matching ITopic
        /// and compatible QoS that the application has not indicated should be “ignored” by
        /// means of the IgnoreSubscription operation on the IDomainParticipant
        /// class.
        /// </para><para>
        /// The subscriptionHandle must correspond to a subscription currently
        /// associated with the IDataWriter, otherwise the operation will fail and return
        /// ReturnCode BadParameter. The operation GetMatchedSubscriptions can
        /// be used to find the subscriptions that are currently matched with the IDataWriter.
        /// </para><para>
        /// The operation may also fail if the infrastructure does not hold the information
        /// necessary to fill in the subscriptionData. This is the case when OpenSplice is
        /// configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In such cases the
        /// operation will return ReturnCode Unsupported.
        /// </para>
        /// </remarks>
        /// <param name="subscriptionData"><Reference to the variable in which the subscription data
        /// will be returned.</param>
        /// <param name="subscriptionHandle">A handle to the subscription whose information needs
        /// to be retrieved.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The information on the specified subscription has successfully
        /// been retrieved.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the
        /// information about “associated” subscriptions.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the IDataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The IDataWriter is not enabled. </item>
        /// </list>
        /// </returns>
        ReturnCode GetMatchedSubscriptionData(
                ref SubscriptionBuiltinTopicData subscriptionData,
                InstanceHandle subscriptionHandle);

#if DOXYGEN_FOR_CS
//
// The above compile switch is never (and must never) be defined in normal compilation.
//
// QoS and Policy related enums are part of the generated code for builtin topics.
// They are repeated here for easy documentation generation.
//

        /// <summary>
        /// This operation informs the Data Distribution Service that the application will be
        /// modifying a particular instance (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.RegisterInstance(Foo instanceData)"/>
        /// </remarks>
        InstanceHandle RegisterInstance(<type> instanceData);

        /// <summary>
        /// This operation will inform the Data Distribution Service that the application will be
        /// modifying a particular instance and provides a value for the sourceTimestamp
        /// explicitly (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.RegisterInstanceWithTimestamp(Foo instanceData,
        /// Time sourceTimestamp)"/>
        /// </remarks>
        InstanceHandle RegisterInstanceWithTimestamp(
                <type> instanceData,
                Time sourceTimestamp);

        /// <summary>
        /// This operation informs the Data Distribution Service that the application will not be
        /// modifying a particular instance any more (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.UnregisterInstance(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle)"/>
        /// </remarks>
        ReturnCode UnregisterInstance(
                <type> instanceData,
                InstanceHandle instanceHandle);

        /// <summary>
        /// This operation will inform the Data Distribution Service that the application will not
        /// be modifying a particular instance any more and provides a value for the
        /// sourceTimestamp explicitly (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.UnregisterInstanceWithTimestamp(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle, DDS.Time sourceTimestamp)"/>
        /// </remarks>
        ReturnCode UnregisterInstanceWithTimestamp(
                <type> instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp);

        /// <summary>
        /// This operation modifies the value of a data instance (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.Write(Foo instanceData)"/>
        /// </remarks>
        ReturnCode Write(<type> instanceData);

        /// <summary>
        /// This operation modifies the value of a data instance (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.Write(Foo instanceData, DDS.InstanceHandle instanceHandle)"/>
        /// </remarks>
        ReturnCode Write(
                <type> instanceData,
                InstanceHandle instanceHandle);

        /// <summary>
        /// This operation modifies the value of a data instance (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.WriteWithTimestamp(Foo instanceData, DDS.Time sourceTimestamp)"/>
        /// </remarks>
        ReturnCode WriteWithTimestamp(
                <type> instanceData,
                Time sourceTimestamp);

        /// <summary>
        /// This operation modifies the value of a data instance and provides a value for the
        /// sourceTimestamp explicitly (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.WriteWithTimestamp(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle, DDS.Time sourceTimestamp)"/>
        /// </remarks>
        ReturnCode WriteWithTimestamp(
                <type> instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp);

        /// <summary>
        /// This operation requests the Data Distribution Service to mark the instance for deletion (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.Dispose(Foo instanceData, DDS.InstanceHandle instanceHandle)"/>
        /// </remarks>
        ReturnCode Dispose(
                <type> instanceData,
                InstanceHandle instanceHandle);

        /// <summary>
        /// This operation requests the Data Distribution Service to mark the instance for deletion
        /// and provides a value for the sourceTimestamp explicitly (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.DisposeWithTimestamp(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle, DDS.Time sourceTimestamp)"/>
        /// </remarks>
        ReturnCode DisposeWithTimestamp(
                <type> instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp);

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.WriteDispose(Foo instanceData)"/>
        /// </remarks>
        ReturnCode WriteDispose(
                <type> instanceData);

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.WriteDispose(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle)"/>
        /// </remarks>
        ReturnCode WriteDispose(
                <type> instanceData,
                InstanceHandle instanceHandle);

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion and provides a value for the sourceTimestamp explicitly (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.WriteDisposeWithTimestamp(Foo instanceData,
        /// Time sourceTimestamp)"/>
        /// </remarks>
        ReturnCode WriteDisposeWithTimestamp(
                <type> instanceData,
                Time sourceTimestamp);

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion and provides a value for the sourceTimestamp explicitly (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.WriteDisposeWithTimestamp(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle, DDS.Time sourceTimestamp)"/>
        /// </remarks>
        ReturnCode WriteDisposeWithTimestamp(
                <type> instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp);

        /// <summary>
        /// This operation retrieves the key value of a specific instance (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.GetKeyValue(ref Foo key, DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode GetKeyValue(
                ref <type> key,
                InstanceHandle instanceHandle);

        /// <summary>
        /// This operation returns the value of the instance handle which corresponds to the
        /// provided instanceData (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataWriter class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataWriter class.
        ///
        /// <seealso cref="Space.FooDataWriter.LookupInstance(Foo instanceData)"/>
        /// </remarks>
        InstanceHandle LookupInstance(<type> instanceData);

#endif

    }
    /// <summary>
    /// A ISubscriber is the object responsible for the actual reception of the data
    /// resulting from its subscriptions.
    /// </summary>
    /// <remarks>
    /// A ISubscriber acts on behalf of one or more IDataReader objects that are related
    /// to it. When it receives data (from the other parts of the system), it indicates to the
    /// application that data is available through its IDataReaderListener and by
    /// enabling related Conditions. The application can access the list of concerned
    /// IDataReader objects through the operation ISubscriber.GetDataReaders and then access the
    /// data available through operations on the IDataReader.
    /// </remarks>
    public interface ISubscriber : IEntity
    {
        /// <summary>
        /// This method creates a IDataReader with default values.
        /// </summary>
        /// <remarks>
        /// This operation creates a IDataReader with the default DataReaderQos, a null IDataReaderListener
        /// and 0 StatusKind mask.
        ///
        /// If the SetDefaultDataReaderQos() method is called, then the default DataReaderQos will be the
        /// QoS given to that method. Otherwise it will equal a new DataReaderQos.
        ///
        /// To delete the IDataReader the operation DeleteDataReader() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref CreateDataReader(ITopicDescription topic, DataReaderQos qos, IDataReaderListener listener, StatusKind mask) "CreateDataReader"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="topic">The TopicDescription for which the DataReader is created. This may be a
        ///                     Topic, MultiTopic or ContentFilteredTopic.</param>
        /// <returns>The newly created IDataReader. In case of an error, a null IDataReader is returned.</returns>
        IDataReader CreateDataReader(ITopicDescription topic);

        /// <summary>
        /// This method creates a IDataReader and if applicable, attaches the optionally specified IDataReaderListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a IDataReader with the default DataReaderQos, a null IDataReaderListener
        /// and 0 StatusKind mask.
        ///
        /// If the SetDefaultDataReaderQos() method is called, then the default DataReaderQos will be the
        /// QoS given to that method. Otherwise it will equal a new DataReaderQos.
        ///
        /// To delete the IDataReader the operation DeleteDataReader() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref CreateDataReader(ITopicDescription topic, DataReaderQos qos, IDataReaderListener listener, StatusKind mask) "CreateDataReader"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="topic">The TopicDescription for which the DataReader is created. This may be a
        ///                     Topic, MultiTopic or ContentFilteredTopic.</param>
        /// <param name="listener">The IDataReaderListener instance which will be attached to the new IDataReader.
        ///                     It is permitted to use null as the value of the listener: this behaves as a
        ///                     IDataReaderListener whose operations perform no action.</param>
        /// <param name="mask"> A bit-mask in which each bit enables the invocation of the IDataReaderListener.
        ///                     for a certain status.</param>
        /// <returns>The newly created IDataReader. In case of an error, a null IDataReader is returned.</returns>
        IDataReader CreateDataReader(
                ITopicDescription topic,
                IDataReaderListener listener, StatusKind mask);

        /// <summary>
        /// This method creates a IDataReader with the desired QosPolicy settings, but without an IDataReaderListener.
        /// </summary>
        /// <remarks>
        /// This operation creates a IDataReader with the default DataReaderQos, a null IDataReaderListener
        /// and 0 StatusKind mask.
        ///
        /// In case the specified QosPolicy settings are not consistent, no IDataReader is
        /// created and null is returned.
        ///
        /// The function CopyFromTopicQos() is available for convenience. That will provide a DataReaderQos
        /// that is consistent with the given TopicQos.
        ///
        /// To delete the IDataReader the operation DeleteDataReader() or
        /// DeleteContainedEntities() must be used.
        ///
        /// See
        /// @ref CreateDataReader(ITopicDescription topic, DataReaderQos qos, IDataReaderListener listener, StatusKind mask) "CreateDataReader"
        /// for:<br>
        /// - Communication Status
        /// - Status Propagation
        /// </remarks>
        /// <param name="topic">The TopicDescription for which the DataReader is created. This may be a
        ///                     Topic, MultiTopic or ContentFilteredTopic.</param>
        /// <param name="qos">  The struct with the QosPolicy settings for the new IDataReader, when these
        ///                     QosPolicy settings are not self consistent, no IDataReader is created.</param>
        /// <returns>The newly created IDataReader. In case of an error, a null IDataReader is returned.</returns>
        IDataReader CreateDataReader(ITopicDescription topic, DataReaderQos qos);

        /// <summary>
        /// This operation creates a IDataReader with the desired QosPolicy settings, for the
        /// desired ITopicDescription and attaches the optionally specified IDataReaderListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a IDataReader with the desired QosPolicy settings, for the
        /// desired ITopicDescription and attaches the optionally specified
        /// IDataReaderListener to it. The ITopicDescription may be a ITopic,
        /// IMultiTopic or IContentFilteredTopic. The returned IDataReader is attached
        /// (and belongs) to the ISubscriber.
        ///
        /// In case the specified QosPolicy settings are not consistent, no IDataReader is
        /// created and null is returned.
        ///
        /// The function CopyFromTopicQos() is available for convenience. That will provide a DataReaderQos
        /// that is consistent with the given TopicQos.
        ///
        /// To delete the IDataReader the operation DeleteDataReader() or
        /// DeleteContainedEntities() must be used.
        ///
        /// <i><b>Communication Status</b></i><br>
        /// For each communication status, the StatusChangedFlag flag is initially set to
        /// false. It becomes true whenever that communication status changes. For each
        /// communication status activated in the mask, the associated
        /// IPublisherListener operation is invoked and the communication
        /// status is reset to false, as the listener implicitly accesses the status which is passed
        /// as a parameter to that operation. The fact that the status is reset prior to calling the
        /// listener means that if the application calls the Get<status_name>Status from
        /// inside the listener it will see the status already reset.
        ///
        /// The following statuses are applicable to the IDataReader
        /// - DDS.StatusKind RequestedDeadlineMissed
        /// - DDS.StatusKind RequestedIncompatibleQos
        /// - DDS.StatusKind SampleLost
        /// - DDS.StatusKind SampleRejected
        /// - DDS.StatusKind DataAvailable
        /// - DDS.StatusKind LivelinessChanged
        /// - DDS.StatusKind SubscriptionMatched
        ///
        /// Be aware that the PublicationMatched
        /// status are not applicable when the infrastructure does not have the
        /// information available to determine connectivity. This is the case when OpenSplice
        /// is configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return null.
        ///
        /// Status bits are declared as a constant and can be used by the application in an OR
        /// operation to create a tailored mask. The special constant 0 can
        /// be used to indicate that the created entity should not respond to any of its available
        /// statuses. The DDS will therefore attempt to propagate these statuses to its factory.
        ///
        /// <i><b>Status Propagation</b></i><br>
        /// In case a communication status is not activated in the mask of the
        /// IDataReaderListener, the ISubscriberListener of the containing
        /// ISubscriber is invoked (if attached and activated for the status that
        /// occurred). This allows the application to set a default behaviour in the
        /// ISubscriberListener of the containing ISubscriber and a
        /// IDataReader specific behaviour when needed.
        ///
        /// In case the communication status is not activated in the mask of the
        /// ISubscriberListener either, the communication status will be propagated to the
        /// IDomainParticipantListener of the containing IDomainParticipant.
        ///
        /// In case the IDomainParticipantListener is also not attached or the communication
        /// status is not activated in its mask, the application is not notified of the change.
        /// </remarks>
        /// <param name="topic">The TopicDescription for which the DataReader is created. This may be a
        ///                     Topic, MultiTopic or ContentFilteredTopic.</param>
        /// <param name="qos">  The struct with the QosPolicy settings for the new IDataReader, when these
        ///                     QosPolicy settings are not self consistent, no IDataReader is created.</param>
        /// <param name="listener">The IDataReaderListener instance which will be attached to the new IDataReader.
        ///                     It is permitted to use null as the value of the listener: this behaves as a
        ///                     IDataReaderListener whose operations perform no action.</param>
        /// <param name="mask"> A bit-mask in which each bit enables the invocation of the IDataReaderListener.
        ///                     for a certain status.</param>
        /// <returns>The newly created IDataReader, or in case of an error a null value one.</returns>
        IDataReader CreateDataReader(
                ITopicDescription topic,
                DataReaderQos qos,
                IDataReaderListener listener,
                StatusKind mask);

        /// <summary>
        /// This operation deletes a IDataReader that belongs to the ISubscriber.
        /// </summary>
        /// <remarks>
        /// This operation deletes a IDataReader that belongs to the ISubscriber. When the
        /// operation is called on a different ISubscriber, as used when the IDataReader was
        /// created, the operation has no effect and returns PreconditionNotMet.
        ///
        /// The deletion of the IDataReader is not allowed if there are any IReadCondition or
        /// IQueryCondition objects that are attached to the IDataReader. In that case the operation returns
        /// PreconditionNotMet.
        /// </remarks>
        /// <param name="dataReader">The IDataReader which is to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the IDataReader is deleted</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - the parameter dataReader is not a valid IDataReader</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - the operation is called on a different ISubscriber,
        /// as used when the IDataReader was created, or the IDataReader contains one or more
        /// IReadCondition or IQueryCondition objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteDataReader(IDataReader dataReader);

        /// <summary>
        /// This operation deletes all the IDataReader objects that were created by means of
        /// the CreateDatareader operation on the ISubscriber.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the IDataReader objects that were created by means of
        /// the CreateDatareader operation on the ISubscriber. In other words, it deletes
        /// all contained IDataReader objects. Prior to deleting each IDataReader, this
        /// operation recursively calls the corresponding DeleteContainedEntities()
        /// operation on each IDataReader. In other words, all IDataReader objects in the
        /// ISubscriber are deleted, including the IQueryCondition and IReadCondition
        /// objects contained by the IDataReader.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The contained IEntity objects are deleted and the application may delete the ISubscriber</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode DeleteContainedEntities();

        /// <summary>
        /// This operation returns a previously created IDataReader belonging to the
        /// ISubscriber which is attached to a ITopic with the matching topicName.
        /// </summary>
        /// <remarks>
        /// This operation returns a previously created IDataReader belonging to the
        /// ISubscriber which is attached to a ITopic with the matching topicName. When
        /// multiple IDataReader objects (which satisfy the same condition) exist, this
        /// operation will return one of them. It is not specified which one.
        ///
        /// This operation may be used on the built-in ISubscriber, which returns the built-in
        /// IDataReader objects for the built-in Topics.
        /// </remarks>
        /// <param name="topicName">The name of the ITopic, which is attached to the IDataReader to look for.</param>
        /// <returns>A reference to the IDataReader found. If no such IDataReader is found the a null value one is
        /// returned.</returns>
        IDataReader LookupDataReader(string topicName);

        /// <summary>
        /// This operation allows the application to access the DataReader objects that contain samples.
        /// </summary>
        /// <remarks>
        /// This functionallity of this function is the same as calling
        /// GetDataReaders(ref IDataReader[] readers, SampleStateKind sampleStates, ViewStateKind viewStates, InstanceStateKind instanceStates)
        /// with all states set to Any.
        /// </remarks>
        /// <param name="readers">An array which is used to pass the list of all DataReaders
        ///         that contain samples.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - All appropriate listeners have been invoked.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ISubscriber has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The Data Distribution Service ran out of resources to
        ///         complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The ISubscriber is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is not invoked inside a BeginAccess() /
        //          EndAccess() block as required by its QosPolicy settings.</item>
        /// </returns>
        ReturnCode GetDataReaders(ref IDataReader[] readers);

        /// <summary>
        /// This operation allows the application to access the DataReader objects that contain
        /// samples with the specified SampleStates, ViewStates, and InstanceStates.
        /// </summary>
        /// <remarks>
        /// If the PresentationQosPolicy of the ISubscriber to which the IDataReader
        /// belongs has the AccessScope set to ‘GROUP’, this operation should only be
        /// invoked inside a BeginAccess() / EndAccess() block. Otherwise it will return the
        /// error DDS.ReturnCode PrecoditionNotMet.
        ///
        /// Depending on the setting of the PresentationQoSPolicy, the returned collection of
        /// IDataReader objects may be:
        /// - a ‘set’ containing each IDataReader at most once in no specified order,
        /// - a ‘list’ containing each IDataReader one or more times in a specific order.
        ///
        /// This difference is due to the fact that, in the second situation it is required to access
        /// samples belonging to different IDataReader objects in a particular order. In this case,
        /// the application should process each IDataReader in the same order it appears in the
        /// ‘list’ and read or take exactly one sample from each IDataReader.
        /// </remarks>
        /// <param name="readers">An array which is used to pass the list of all DataReaders that
        ///         contain samples of the specified SampleStates, ViewStates, and InstanceStates.</param>
        /// <param name="sampleStates">A mask, which selects only those readers that have samples with
        ///         the desired sample states.</param>
        /// <param name="viewStates">A mask, which selects only those readers that have samples with the
        ///         desired view states.</param>
        /// <param name="instanceStates">A mask, which selects only those readers that have samples with
        ///         the desired instance states.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - All appropriate listeners have been invoked.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ISubscriber has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The Data Distribution Service ran out of resources to
        ///         complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The ISubscriber is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is not invoked inside a BeginAccess() /
        //          EndAccess() block as required by its QosPolicy settings.</item>
        /// </returns>
        ReturnCode GetDataReaders(
                ref IDataReader[] readers,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation invokes the OnDataAvailable operation on
        /// IDataReaderListener objects which are attached to the contained IDataReader
        /// entities having new, available data.
        /// </summary>
        /// <remarks>
        /// This operation invokes the OnDataAvailable operation on the
        /// IDataReaderListener objects attached to contained IDataReader entities that
        /// have received information, but which have not yet been processed by those
        /// DataReaders.
        ///
        /// The NotifyDataReaders operation ignores the bit mask value of individual
        /// IDataReaderListener objects, even when the DDS.StatusKind DataAvailable bit
        /// has not been set on a IDataReader that has new, available data. The
        /// OnDataAvailable operation will still be invoked, when the
        /// DDS.StatusKind DataAvailable bit has not been set on a IDataReader, but will not
        /// propagate to the IDomainParticipantListener.
        ///
        /// When the IDataReader has attached a null listener, the event will be consumed
        /// and will not propagate to the IDomainParticipantListener. (Remember that a
        /// null listener is regarded as a listener that handles all its events as a NOOP).
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - All appropriate listeners have been invoked.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode NotifyDataReaders();

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a ISubscriber.
        /// </summary>
        /// <remarks>
        /// The parameter qos contains the QosPolicy settings which is checked for
        /// self-consistency and mutability. When the application tries to change a QosPolicy
        /// setting for an enabled ISubscriber, which can only be set before the ISubscriber
        /// is enabled, the operation will fail and a DDS.ReturnCode ImmutablePolicy is returned.
        /// In other words, the application must provide the presently set QosPolicy settings
        /// in case of the immutable QosPolicy settings. Only the mutable QosPolicy
        /// settings can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and a DDS.ReturnCode InconsistentPolicy
        /// is returned.
        ///
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned DDS.ReturnCode Ok).
        /// </remarks>
        /// <param name="qos">The new set of QosPolicy settings for the ISubscriber.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new SubscriberQos is set.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - the parameter qos is not a valid SubscriberQos.
        /// It contains a QosPolicy setting with an enum value that is outside its legal
        /// boundaries.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are
        /// currently not supported by OpenSplice.</item>
        /// <item>DDS.ReturnCode ImmutablePolicy - the parameter qos contains an immutable QosPolicy setting
        /// with a different value than set during enabling of the Subbscriber.</item>
        /// </returns>
        ReturnCode SetQos(SubscriberQos qos);

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a ISubscriber.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a ISubscriber
        /// on which this operation is used. This SubscriberQos is stored at the location
        /// pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination SubscriberQos struct in which the QosPolicy
        /// settings will be copied.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of QoS policy values applied to this ISubscriber
        /// has successfully been copied into the specified SubscriberQos parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref SubscriberQos qos);

        /// <summary>
        /// This property returns the ISubscriberListener currently attached to the ISubscriber.
        /// </summary>
        /// <remarks>
        /// Only one listener can be attached to the ISubscriber at any particular time. This property
        /// returns the listener that is currently attached to the ISubscriber.
        /// </remarks>
        /// <returns>returns the ISubscriberListener currently attached to the ISubscriber.</returns>
        ISubscriberListener Listener { get; }

        /// <summary>
        /// This operation attaches a ISubscriberListener to the ISubscriber.
        /// </summary>
        /// <remarks>
        /// This operation attaches a ISubscriberListener to the ISubscriber. Only one
        /// ISubscriberListener can be attached to each ISubscriber. If a ISubscriberListener
        /// was already attached, the operation will replace it with the new one.
        /// When listener is null, it represents a listener that is treated as a NOOP for all
        /// statuses activated in the bit mask.
        ///
        /// See
        /// @ref DDS.IDomainParticipant.CreateSubscriber(SubscriberQos qos, ISubscriberListener listener, StatusKind mask) "CreateSubscriber"
        /// for:<br>
        /// - Subscriber Communication Status
        /// - Subscriber Status Propagation
        /// </remarks>
        /// <param name="listener">The ISubscriberListener instance, which will be attached to the ISubscriber.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the ISubscriberListener
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The ISubscriberListener is attached.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources to
        /// complete this operation.</item>
        /// </returns>
        ReturnCode SetListener(ISubscriberListener listener, StatusKind mask);

        /// <summary>
        /// This operation indicates that the application will begin accessing a coherent and/or
        /// ordered set of modifications that spans multiple DataReaders attached to this
        /// Subscriber. The access will be completed by a matching call to EndAccess().
        /// </summary>
        /// <remarks>
        /// This operation indicates that the application is about to access a set of coherent
        /// and/or ordered samples in any of the IDataReader objects attached to the ISubscriber.
        /// The operation will effectively lock all of the Subscriber’s DataReader objects for
        /// any incoming modifications, so that the state of their history remains consistent for
        /// the duration of the access.
        ///
        /// The application is required to use this operation only if the PresentationQosPolicy of
        /// the Subscriber to which the DataReader belongs has the AccessScope set to
        /// ‘GROUP’. In the aforementioned case, the operation BeginAccess() must be called
        /// prior to calling any of the sample-accessing operations, namely:
        /// GetDatareaders on the Subscriber and Read(), Take(), and all their variants on any
        /// IDataReader. Otherwise the sample-accessing operations will return the error
        /// DDS.ReturnCode PreconditionNotMet. Once the application has finished accessing
        /// the data samples it must call EndAccess().
        ///
        /// It is not required for the application to call BeginAccess() / EndAccess() if the
        /// PresentationQosPolicy has the AccessScope set to something other than
        /// ‘GROUP’. Calling BeginAccess() / EndAccess() in this case is not considered an
        /// error and has no effect.
        ///
        /// The calls to BeginAccess() / EndAccess() may be nested. In that case, the
        /// application must call EndAccess() as many times as it called BeginAccess().
        ///
        /// Note that a coherent Subscriber should first be enabled, otherwise this operation
        /// will return an error. See @ref DDS.IEntity.Enable() for additional information.
        ///
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Access to coherent/ordered data has successfully started.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ISubscriber has already been deleted.</item>
        /// </returns>
        ReturnCode BeginAccess();

        /// <summary>
        /// This operation indicates that the application will stop accessing a coherent and/or
        /// ordered set of modifications that spans multiple DataReaders attached to this
        /// ISubscriber. This access must have been started by a matching call to
        /// BeginAccess().
        /// </summary>
        /// <remarks>
        /// Indicates that the application has finished accessing the data samples in IDataReader
        /// objects managed by the ISubscriber. This operation must be used to ‘close’ a
        /// corresponding BeginAccess(). The operation will effectively unlock all of the
        /// Subscriber’s DataReader objects for incoming modifications, so it is important to
        /// invoke it as quickly as possible to avoid an ever increasing backlog of
        /// modifications. After calling EndAccess the application should no longer access
        /// any of the Data or SampleInfo elements returned from the sample-accessing
        /// operations.
        ///
        /// This call must close a previous call to BeginAccess() otherwise the operation will
        /// return the error DDS.ReturnCode PreconditionNotMet.
        ///
        /// Please consult @ref DDS.IEntity.Enable() for additional information about coherent access
        ///
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Access to coherent/ordered data has successfully started.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The ISubscriber has already been deleted.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - No matching call to BeginAccess() has been detected.</item>
        /// </returns>
        ReturnCode EndAccess();

        /// <summary>
        /// This property allows access to the IDomainParticipant associated with the ISubscriber.
        /// </summary>
        IDomainParticipant Participant { get; }

        /// <summary>
        /// This operation sets the default DataReaderQos of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation sets the default DataReaderQos of the IDataReader (that is the
        /// struct with the QosPolicy settings). This QosPolicy is used for newly created
        /// IDataReader objects in case no QoS was provided
        /// parameter qos to specify the DataReaderQos in the CreateDataReader
        /// operation. This operation checks if the DataReaderQos is self consistent. If it is
        /// not, the operation has no effect and returns InconsistentPolicy.
        /// The values set by this operation are returned by GetDefaultDataReaderQos().
        /// </remarks>
        /// <param name="qos">The DataReaderQos struct, which containsthe new default QosPolicy
        /// settings for the newly created DataReaders.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new default DataReaderQos is set.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - the parameter qos is not a valid DataReaderQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries.</item>
        /// <item>DDS.ReturnCode InconsistentPolicy - the parameter qos contains conflicting
        /// QosPolicy settings, e.g. a history depth that is higher than the specified resource
        /// limits.</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are currently not
        /// supported by OpenSplice.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultDataReaderQos(DataReaderQos qos);

        /// <summary>
        /// This operation gets the default QosPolicy settings of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation gets the default QosPolicy settings of the IDataReader (that is the
        /// DataReaderQos) which is used for newly created IDataReader objects, in case
        /// no QoS was provided. The default DataReaderQos
        /// is only used when the constant is supplied as parameter qos to specify the
        /// DataReaderQos in the CreateDataReader operation. The application must
        /// provide the DataReaderQos struct in which the QosPolicy settings can be stored
        /// and pass the qos reference to the operation. The operation writes the default
        /// QosPolicy settings to the struct referenced to by qos. Any settings in the struct are
        /// overwritten.
        ///
        /// The values retrieved by this operation match the values specified on the last
        /// successful call to SetDefaultDataReaderQos, or, if the call was never made,
        /// the default values as specified for each QosPolicy setting
        /// </remarks>
        /// <param name="qos">A reference to the DataReaderQos struct(provided by the application)
        /// in which the default QosPolicy settings for the IDataReader are written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The default IDataReader QosPolicy settings of this ISubscriber
        /// have successfully been copied into the specified DataReaderQos parameter.</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// </returns>
        ReturnCode GetDefaultDataReaderQos(ref DataReaderQos qos);

        /// <summary>
        /// This operation will copy the policies in topicQos to the corresponding policies in datareaderQos.
        /// </summary>
        /// <remarks>
        /// This operation will copy the QosPolicy settings in topicQos to the
        /// corresponding QosPolicy settings in dataReaderQos (replacing the values
        /// in datareaderQos, if present).
        /// This is a convenience operation, useful in combination with the operations
        /// GetDefaultDataWriterQos and ITopic.GetQos. The operation
        /// CopyFromTopicQos can be used to merge the IDataReader default
        /// QosPolicy settings with the corresponding ones on the ITopic. The resulting
        /// DataReaderQos can then be used to create a new IDataReader, or set its
        /// DataReaderQos.
        ///
        /// This operation does not check the resulting dataReaderQos for self
        /// consistency. This is because the merged dataReaderQos may not be the
        /// final one, as the application can still modify some QosPolicy settings prior to
        /// applying the DataReaderQos to the IDataReader.
        /// </remarks>
        /// <param name="dataReaderQos">The destination DataReaderQos struct to which the QosPolicy settings
        /// will be copied.</param>
        /// <param name="topicQos">The source TopicQos, which will be copied.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The QosPolicy settings have successfully been copied from the TopicQos to DataReaderQos</item>
        /// <item>DDS.ReturnCode Error - an internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - the ISubscriber has already been deleted</item>
        /// <item>DDS.ReturnCode OutOfResources - the Data Distribution Service ran out of resources
        /// to complete this operation.</item>
        /// </returns>
        ReturnCode CopyFromTopicQos(ref DataReaderQos dataReaderQos, TopicQos topicQos);

    }

    /// <summary>
    /// A IDataReader allows the application:
    /// to declare data it wishes to receive (i.e., make a subscription)
    /// to access data received by the associated ISubscriber.
    /// A IDataReader refers to exactly one ITopicDescription (either a ITopic, a IContentFilteredTopic or a IMultiTopic)
    /// that identifies the samples to be read. The IDataReader may give access to several instances of the data type,
    /// which are distinguished from each other by their key.
    /// IDataReader is an abstract class. It is specialized for each particular application data type.
    /// For a fictional application data type Space (defined in the module SPACE)
    /// the specialized class would be SPACE.SpaceDataReader.
    /// </summary>
    public interface IDataReader : IEntity
    {
        /// <summary>
        /// This operation creates a new IReadCondition for the IDataReader.
        /// </summary>
        /// <remarks>
        /// The returned IReadCondition is attached (and belongs) to the IDataReader.
        /// When the operation fails, null is returned. To delete the IReadCondition the
        /// operation DeleteReadCondition or DeleteContainedEntities must be used.
        ///
        /// Samples with Any kind for SampleStates, ViewStates and InstanceStates will be read.
        /// </remarks>
        /// <returns>New IReadCondition. When the operation fails, null is returned.</returns>
        IReadCondition CreateReadCondition();

        /// <summary>
        /// This operation creates a new IReadCondition for the IDataReader.
        /// </summary>
        /// <remarks>
        /// The returned IReadCondition is attached (and belongs) to the IDataReader.
        /// When the operation fails, null is returned. To delete the IReadCondition the
        /// operation DeleteReadCondition or DeleteContainedEntities must be used.
        ///
        /// <b><i>State Masks</i></b><br>
        /// The result of the IReadCondition also depends on the selection of samples
        /// determined by three masks:
        /// - SampleStates is the mask, which selects only those samples with the desired
        /// sample states SampleStateKind Read, NotRead or both
        /// - ViewStates is the mask, which selects only those samples with the desired
        /// view states ViewStateKind New, NotNew or both
        /// - InstanceStates is the mask, which selects only those samples with the
        /// desired instance states InstanceStateKind Alive, NotAliveDisposed,
        /// NotAliveNoWriters or a combination of these.
        /// </remarks>
        /// <param name="sampleStates">A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>New IReadCondition. When the operation fails, null is returned.</returns>
        IReadCondition CreateReadCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation creates a new IQueryCondition for the IDataReader.
        /// </summary>
        /// <remarks>
        /// The returned IQueryCondition is attached (and belongs) to the IDataReader.
        /// When the operation fails, null is returned. To delete the IQueryCondition the
        /// operation DeleteReadCondition or DeleteContainedEntities must be used.
        ///
        /// Samples with Any kind for SampleStates, ViewStates and InstanceStates will be read.
        ///
        /// <b><i>SQL Expression</i></b><br>
        /// The SQL query string is set by queryExpression which must be a subset of the
        /// SQL query language. In this query expression, parameters may be used, which must
        /// be set in the sequence of strings defined by the parameter queryParameters. A
        /// parameter is a string which can define an integer, float, string or enumeration. The
        /// number of values in queryParameters must be equal or greater than the highest
        /// referenced %n token in the queryExpression (e.g. if %1 and %8 are used as
        /// parameters in the queryExpression , the queryParameters should at least
        /// contain n+1 = 9 values).
        /// </remarks>
        /// <param name="queryExpression">The query string, which must be a subset of the SQL query language.</param>
        /// <param name="queryParameters">A sequence of strings which are the parameter values used in the
        /// SQL query string (i.e., the tokens in the expression). The number of values in queryParameters
        /// must be equal or greater than the highest referenced %n token in the queryExpression
        /// (e.g.if %1 and %8 are used as parameters in the queryExpression, the queryParameters
        /// should at least contain n+1 = 9 values).</param>
        /// <returns>New IQueryCondition. When the operation fails, null is returned.</returns>
        IQueryCondition CreateQueryCondition(
                string queryExpression,
                params string[] queryParameters);

        /// <summary>
        /// This operation creates a new IQueryCondition for the IDataReader.
        /// </summary>
        /// <remarks>
        /// The returned IQueryCondition is attached (and belongs) to the IDataReader.
        /// When the operation fails, null is returned. To delete the IQueryCondition the
        /// operation DeleteReadCondition or DeleteContainedEntities must be used.
        ///
        /// <b><i>State Masks</i></b><br>
        /// The result of the IQueryCondition also depends on the selection of samples
        /// determined by three masks:
        /// - SampleStates is the mask, which selects only those samples with the desired
        /// sample states SampleStateKind Read, NotRead or both
        /// - ViewStates is the mask, which selects only those samples with the desired
        /// view states ViewStateKind New, NotNew or both
        /// - InstanceStates is the mask, which selects only those samples with the
        /// desired instance states InstanceStateKind Alive, NotAliveDisposed,
        /// NotAliveNoWriters or a combination of these.
        ///
        /// <b><i>SQL Expression</i></b><br>
        /// The SQL query string is set by queryExpression which must be a subset of the
        /// SQL query language. In this query expression, parameters may be used, which must
        /// be set in the sequence of strings defined by the parameter queryParameters. A
        /// parameter is a string which can define an integer, float, string or enumeration. The
        /// number of values in queryParameters must be equal or greater than the highest
        /// referenced %n token in the queryExpression (e.g. if %1 and %8 are used as
        /// parameters in the queryExpression , the queryParameters should at least
        /// contain n+1 = 9 values).
        /// </remarks>
        /// <param name="sampleStates">A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">A mask, which selects only those samples with the desired instance states.</param>
        /// <param name="queryExpression">The query string, which must be a subset of the SQL query language.</param>
        /// <param name="queryParameters">A sequence of strings which are the parameter values used in the
        /// SQL query string (i.e., the tokens in the expression). The number of values in queryParameters
        /// must be equal or greater than the highest referenced %n token in the queryExpression
        /// (e.g.if %1 and %8 are used as parameters in the queryExpression, the queryParameters
        /// should at least contain n+1 = 9 values).</param>
        /// <returns>New IQueryCondition. When the operation fails, null is returned.</returns>
        IQueryCondition CreateQueryCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates,
                string queryExpression,
                params string[] queryParameters);

        /// <summary>
        /// This operation deletes a IReadCondition or IQueryCondition which is attached to the IDataReader.
        /// </summary>
        /// This operation deletes a IReadCondition or IQueryCondition which is attached
        /// to the IDataReader. Since a IQueryCondition is a specialized IReadCondition,
        /// the operation can also be used to delete a IQueryCondition. A IReadCondition
        /// or IQueryCondition cannot be deleted when it is not attached to this IDataReader.
        /// When the operation is called on a IReadCondition or IQueryCondition which
        /// was not at t ached to this IDataReader, the operation returns
        /// PreconditionNotMet.
        /// <param name="condition">The IReadCondition or IQueryCondition which is to be deleted.
        /// </param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The IReadCondition or IQueryCondition is deleted.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter condition is not a valid DDS.IReadCondition reference.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The operation is called on a different IDataReader, as used when the
        ///  IReadCondition or IQueryCondition was created.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteReadCondition(IReadCondition condition);

        /// <summary>
        /// This operation deletes all the IEntity objects that were created by means of one of the
        /// Create operations on the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the IEntity objects that were created by means of one of
        /// the Create operations on the IDataReader. In other words, it deletes all
        /// IQueryCondition and IReadCondition objects contained by the IDataReader.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The contained IEntity objects are deleted and the application may delete the IDataReader</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteContainedEntities();

        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a IDataReader.
        /// The parameter qos contains the QosPolicy settings which is checked for
        /// self-consistency and mutability. When the application tries to change a QosPolicy
        /// setting for an enabled IDataReader, which can only be set before the IDataReader
        /// is enabled, the operation will fail and a ImmutablePolicy is returned.
        /// In other words, the application must provide the presently set QosPolicy settings
        /// in case of the immutable QosPolicy settings. Only the mutable QosPolicy
        /// settings can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and a InconsistentPolicy is
        /// returned.
        ///
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned Ok).
        /// </remarks>
        /// <param name="qos">the new set of QosPolicy settings for the IDataReader.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The new DDS.DataReaderQos is set.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode BadParameter - the parameter qos is not a valid DDS.DataReaderQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries</item>
        /// <item>DDS.ReturnCode Unsupported - one or more of the selected QosPolicy values are currently not
        /// supported by OpenSplice</item>
        /// <item>DDS.ReturnCode ImmutablePolicy - the parameter qos contains an immutable
        /// QosPolicy setting with a different value than set during enabling of the IDataReader</item>
        /// <item>DDS.ReturnCode InconsistentPolicy - the parameter qos contains conflicting
        /// QosPolicy settings, e.g. a history depth that is higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(DataReaderQos qos);

        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a IDataReader
        /// on which this operation is used. This DDS.DataReaderQos is stored at the location
        /// pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">a reference to DDS.DataReaderQos, where the QosPolicy settings of the IDataReader
        /// are to be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The existing set of QoSPolicy values applied to this IDataReader
        /// has successfully been copied into the specified DDS.DataReaderQos parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref DataReaderQos qos);

        /// <summary>
        /// This property returns the IDataReaderListener currently attached to the IDataReader.
        /// </summary>
        /// <remarks>
        /// Only one listener can be attached to the IDataReader at any particular time. This property
        /// returns the listener that is currently attached to the IDataReader.
        /// </remarks>
        /// <returns>returns the IDataReaderListener currently attached to the IDataReader.</returns>
        IDataReaderListener Listener { get; }

        /// <summary>
        /// This operation attaches a IDataReaderListener to the IDataReader.
        /// </summary>
        /// <param name="listener">The IDataReaderListener which will be attached to the IDataReader.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the IDataReaderListener
        /// for a certain status.</param>
        /// <returns></returns>
        ReturnCode SetListener(IDataReaderListener listener, StatusKind mask);

        /// <summary>
        /// This operation returns the DDS.ITopicDescription which is associated with the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation returns the DDS.ITopicDescription which is associated with the
        /// IDataReader, thus the DDS.ITopicDescription with which the IDataReader is
        /// created. If the IDataReader is already deleted, null is returned.
        /// </remarks>
        /// <returns>Returns the DDS.ITopicDescription associated with the IDataReader.</returns>
        ITopicDescription GetTopicDescription();

        /// <summary>
        /// This property returns the ISubscriber to which the IDataReader belongs.
        /// </summary>
        ISubscriber Subscriber { get; }

        /// <summary>
        /// This operation obtains the DDS.SampleRejectedStatus of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation obtains the DDS.SampleRejectedStatus struct of the IDataReader.
        /// This struct contains the information whether a received sample has been rejected.
        /// The DDS.SampleRejectedStatus can also be monitored using a
        /// IDataReaderListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to DDS.SampleRejectedStatus where the contents of the
        /// DDS.SampleRejectedStatus of the IDataReader will be copied into.</param>
        /// <returns>
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current DDS.SampleRejectedStatus of this IDataReader has successfully been copied
        /// into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetSampleRejectedStatus(ref SampleRejectedStatus status);

        /// <summary>
        /// This operation obtains the LivelinessChangedStatus struct of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This obtains returns the LivelinessChangedStatus struct of the IDataReader.
        /// This struct contains the information whether the liveliness of one or more
        /// IDataWriter objects that were writing instances read by the IDataReader has
        /// changed. In other words, some IDataWriter have become alive or not alive.
        /// The LivelinessChangedStatus can also be monitored using a
        /// IDataReaderListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to LivelinessChangedStatus where the contents of the
        ///  LivelinessChangedStatus of the IDataReader will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current  LivelinessChangedStatus of this IDataReader has successfully been copied
        /// into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetLivelinessChangedStatus(ref LivelinessChangedStatus status);

        /// <summary>
        /// This operation obtains the DDS.RequestedDeadlineMissedStatus struct of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation obtains the DDS.RequestedDeadlineMissedStatus struct of the
        /// IDataReader. This struct contains the information whether the deadline that the
        /// IDataReader was expecting through its DeadlineQosPolicy was not respected
        /// for a specific instance.
        /// The DDS.RequestedDeadlineMissedStatus can also be monitored using a
        /// IDataReaderListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to DDS.RequestedDeadlineMissedStatus where the contents of the
        /// DDS.RequestedDeadlineMissedStatus  of the IDataReader will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current DDS.RequestedDeadlineMissedStatus of this IDataReader has successfully been copied
        /// into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetRequestedDeadlineMissedStatus(ref RequestedDeadlineMissedStatus status);

        /// <summary>
        /// This operation obtains the DDS.RequestedIncompatibleQosStatus struct of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This operation obtains the DDS.RequestedIncompatibleQosStatus struct of the
        /// IDataReader. This struct contains the information whether a QosPolicy setting
        /// was incompatible with the offered QosPolicy setting.
        ///
        /// The Request/Offering mechanism is applicable between the IDataWriter and the
        /// IDataReader. If the QosPolicy settings between IDataWriter and IDataReader
        /// are inconsistent, no communication between them is established. In addition the
        /// IDataWriter will be informed via a RequestedIncompatibleQos DDS.StatusKind change
        /// and the IDataReader will be informed via an OfferedIncompatibleQos DDS.StatusKind change.
        /// The DDS.RequestedIncompatibleQosStatus can also be monitored using a
        /// IDataReaderListener or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status">A reference to DDS.RequestedIncompatibleQosStatus where the contents of the
        ///  DDS.RequestedIncompatibleQosStatus of the IDataReader will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current DDS.RequestedIncompatibleQosStatus of this IDataReader has successfully been copied
        /// into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetRequestedIncompatibleQosStatus(ref RequestedIncompatibleQosStatus status);

        /// <summary>
        /// This operation obtains the DDS.SubscriptionMatchedStatus struct of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This struct contains the information whether a new match has been
        /// discovered for the current subscription, or whether an existing match has ceased to
        /// exist.
        ///
        /// This means that the status represents that either a IDataWriter object has been
        /// discovered by the IDataReader with the same Topic and a compatible Qos, or that a
        /// previously discovered DataWriter has ceased to be matched to the current
        /// IDataReader. A IDataWriter may cease to match when it gets deleted, when it
        /// changes its Qos to a value that is incompatible with the current IDataReader or
        /// when either the IDataReader or the IDataWriter has chosen to put its matching
        /// counterpart on its ignore-list using the IgnorePublication or
        /// IgnoreSubcription operations on the IDomainParticipant.
        ///
        /// The operation may fail if the infrastructure does not hold the information necessary
        /// to fill in the SubscriptionMatchedStatus. This is the case when OpenSplice is
        /// configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return DDS.ReturnCode Unsupported.
        ///
        /// The SubscriptionMatchedStatus can also be monitored using a
        /// IDataReaderListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status"> reference to DDS.SubscriptionMatchedStatus where the contents of the
        ///  DDS.SubscriptionMatchedStatus of the IDataReader will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current DDS.SubscriptionMatchedStatus of this IDataReader has
        /// successfully been copied into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the
        /// information about “associated” publications.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetSubscriptionMatchedStatus(ref SubscriptionMatchedStatus status);

        /// <summary>
        /// This operation obtains the DDS.SampleLostStatus struct of the IDataReader.
        /// </summary>
        /// <remarks>
        /// This struct contains information whether samples have been lost. This only applies when
        /// the DDS.ReliabilityQosPolicyKind is set to ReliableReliabilityQos. If the
        /// DDS.ReliabilityQosPolicyKind is set to BestEffortReliabilityQos the Data Distribution Service
        /// will not report the loss of samples.
        ///
        /// The DDS.SampleLostStatus can also be monitored using a IDataReaderListener
        /// or by using the associated IStatusCondition.
        /// </remarks>
        /// <param name="status"> reference to DDS.SampleLostStatus where the contents of the
        ///  DDS.SampleLostStatus of the IDataReader will be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The current DDS.SampleLostStatus of this IDataReader has successfully been copied
        /// into the specified status parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetSampleLostStatus(ref SampleLostStatus status);

        /// <summary>
        /// This operation will block the application thread until all historical data is received.
        /// </summary>
        /// <remarks>
        /// This operation behaves differently for IDataReader objects which have a
        /// non-VolatileDurabilityQos kind in the DurabilityQosPolicy and for
        /// IDataReader objects which have a VolatileDurabilityQos kind in the
        /// DurabilityQosPolicy.
        ///
        /// As soon as an application enables a non-VolatileDurabilityQos
        /// IDataReader it will start receiving both historical data, i.e. the data that was
        /// written prior to the time the IDataReader joined the domain, as well as any new
        /// data written by the IDataWriter objects. There are situations where the application
        /// logic may require the application to wait until all historical data is received. This
        /// is the purpose of the WaitForHistoricalData operation.
        ///
        /// As soon as an application enables a VolatileDurabilityQos IDataReader it
        /// will not start receiving historical data but only new data written by the
        /// IDataWriter objects. By calling WaitForHistoricalData the IDataReader
        /// explicitly requests the Data Distribution Service to start receiving also the
        /// historical data and to wait until either all historical data is received, or the
        /// duration specified by the maxWait parameter has elapsed, whichever happens
        /// first.
        ///
        /// <b><i>Thread Blocking</i></b><br>
        /// The operation wait_for_historical_data blocks the calling thread until either
        /// all “historical” data is received, or the duration specified by the maxWait
        /// parameter elapses, whichever happens first. A return value of DDS.ReturnCode Ok
        /// indicates that all the “historical” data was received a return value of
        /// DDS.ReturnCode Timeout indicates that maxWait elapsed before all the data was
        /// received.
        /// </remarks>
        /// <param name="maxWait">the maximum duration to block for the operation, after which
        /// the application thread is unblocked. The special constant Duration Infinite can be used when
        /// the maximum waiting time does not need to be bounded.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The historical data is received </item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDataReader is not enabled.</item>
        /// <item>DDS.ReturnCode Timeout - Not all data is received before maxWait elapsed.</item>
        /// </list>
        /// </returns>
        ReturnCode WaitForHistoricalData(Duration maxWait);

        /// <summary>
        /// This operation retrieves the list of publications currently "associated" with the DataReader.
        /// </summary>
        /// <remarks>
        /// That is, publications that have a matching ITopic and compatible
        /// QoS that the application has not indicated should be “ignored” by means of the
        /// IgnorePublication operation on the IDomainParticipant.
        ///
        /// The handles returned in the publicationHandles array are the ones that are
        /// used by the DDS implementation to locally identify the corresponding matched
        /// IDataWriter entities. You can access more detailed information about a particular
        /// publication by passing its publicationHandle to either the
        /// GetMatchedPublicationData() operation or to the ReadInstance()
        /// operation on the built-in reader for the “DCPSPublication” topic.
        ///
        /// Be aware that since DDS.InstanceHandle is an opaque datatype, it does not
        /// necessarily mean that the handles obtained from the
        /// GetMatchedPublications() operation have the same value as the ones that
        /// appear in the InstanceHandle field of the SampleInfo when retrieving the
        /// publication info through corresponding "DCPSPublications" built-in reader. You
        /// can’t just compare two handles to determine whether they represent the same
        /// publication. If you want to know whether two handles actually do represent the
        /// same publication, use both handles to retrieve their corresponding
        /// PublicationBuiltinTopicData samples and then compare the key field of
        /// both samples.
        ///
        /// The operation may fail if the infrastructure does not locally maintain the
        /// connectivity information. This is the case when OpenSplice is configured not to
        /// maintain discovery information in the Networking Service. (See the description for
        /// the NetworkingService/Discovery/enabled property in the Deployment
        /// Manual for more information about this subject.) In this case the operation will
        /// return DDS.ReturnCode Unsupported.
        /// </remarks>
        /// <param name="publicationHandles">An array which is used to pass the list of all associated publications.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The list of associated publications has successfully been obtained.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the
        /// information about “associated” publications.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDataReader is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode GetMatchedPublications(ref InstanceHandle[] publicationHandles);

        /// <summary>
        /// This operation retrieves information on the specified publication that is currently
        /// “associated” with the IDataReader.
        /// </summary>
        /// <remarks>
        /// That is, a publication with a matching ITopic
        /// and compatible QoS that the application has not indicated should be “ignored” by
        /// means of the IgnorePublication operation on the IDomainParticipant.
        ///
        /// The publicationHandle must correspond to a publication currently associated
        /// with the IDataReader, otherwise the operation will fail and return DDS.ReturnCode
        /// BadParameter. The operation IDataReader.GetMatchedPublications() can
        /// be used to find the publications that are currently matched with the DataReader.
        ///
        /// The operation may also fail if the infrastructure does not hold the information
        /// necessary to fill in the publicationData. This is the case when OpenSplice is
        /// configured not to maintain discovery information in the Networking Service. (See
        /// the description for the NetworkingService/Discovery/enabled property in
        /// the Deployment Manual for more information about this subject.) In this case the
        /// operation will return DDS.ReturnCode Unsupported.
        /// </remarks>
        /// <param name="publicationData">The sample in which the information about the specified
        /// publication is to be stored.</param>
        /// <param name="publicationHandle">A handle to the publication whose information needs
        /// to be retrieved.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The information on the specified publication has successfully
        /// been retrieved.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode Unsupported - OpenSplice is configured not to maintain the
        /// information about “associated” publications.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The IDataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - the IDataReader is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode GetMatchedPublicationData(
                ref PublicationBuiltinTopicData publicationData,
                InstanceHandle publicationHandle);

#if DOXYGEN_FOR_CS
//
// The above compile switch is never (and must never) be defined in normal compilation.
//
// QoS and Policy related enums are part of the generated code for builtin topics.
// They are repeated here for easy documentation generation.
//

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Read(ref Space.Foo[] dataValues, ref DDS.SampleInfo[] sampleInfos)">
        /// </remarks>
        ReturnCode Read(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Read(ref Space.Foo[] dataValues, ref DDS.SampleInfo[] sampleInfos, int maxSamples)">
        /// </remarks>
        ReturnCode Read(
            ref <data> dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Read(
        ///    ref Space.Foo[] dataValues,
        ///    ref DDS.SampleInfo[] sampleInfos,
        ///    DDS.SampleStateKind sampleStates,
        ///    DDS.ViewStateKind viewStates,
        ///    DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode Read(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Read(
        ///    ref Space.Foo[] dataValues,
        ///    ref DDS.SampleInfo[] sampleInfos,
        ///    int maxSamples,
        ///    DDS.SampleStateKind sampleStates,
        ///    DDS.ViewStateKind viewStates,
        ///    DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode Read(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Take(ref Space.Foo[] dataValues, ref DDS.SampleInfo[] sampleInfos)">
        /// </remarks>
        ReturnCode Take(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Take(ref Space.Foo[] dataValues, ref DDS.SampleInfo[] sampleInfos, int maxSamples)">
        /// </remarks>
        ReturnCode Take(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Take(
        ///     ref Space.Foo[] dataValues,
        ///     ref DDS.SampleInfo[] sampleInfos,
        ///     DDS.SampleStateKind sampleStates,
        ///     DDS.ViewStateKind viewStates,
        ///     DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode Take(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.Take(
        ///         ref Space.Foo[] dataValues,
        ///         ref DDS.SampleInfo[] sampleInfos,
        ///         int maxSamples,
        ///         DDS.SampleStateKind sampleStates,
        ///         DDS.ViewStateKind viewStates,
        ///         DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode Take(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadWithCondition(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode ReadWithCondition(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadWithCondition(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode ReadWithCondition(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeWithCondition(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode TakeWithCondition(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeWithCondition(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode TakeWithCondition(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadNextSample(
        ///                ref Space.Foo dataValue,
        ///                ref DDS.SampleInfo sampleInfo)">
        /// </remarks>
        ReturnCode ReadNextSample(
                ref Space.Foo dataValue,
                ref SampleInfo sampleInfo);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeNextSample(
        ///                ref Space.Foo dataValue,
        ///                ref DDS.SampleInfo sampleInfo)">
        /// </remarks>
        ReturnCode TakeNextSample(
                ref Space.Foo dataValue,
                ref SampleInfo sampleInfo);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode ReadInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            int maxSamples,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode ReadInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadInstance(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.SampleStateKind sampleStates,
        ///                DDS.ViewStateKind viewStates,
        ///                DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode ReadInstance(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode TakeInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            int maxSamples,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode TakeInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeInstance(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.SampleStateKind sampleStates,
        ///                DDS.ViewStateKind viewStates,
        ///                DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode TakeInstance(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadNextInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode ReadNextInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadNextInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            int maxSamples,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode ReadNextInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadNextInstance(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.SampleStateKind sampleStates,
        ///                DDS.ViewStateKind viewStates,
        ///                DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode ReadNextInstance(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeNextInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode TakeNextInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeNextInstance(
        ///            ref Space.Foo[] dataValues,
        ///            ref DDS.SampleInfo[] sampleInfos,
        ///            int maxSamples,
        ///            DDS.InstanceHandle instanceHandle)">
        /// </remarks>
        ReturnCode TakeNextInstance(
            ref <data>[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeNextInstance(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.SampleStateKind sampleStates,
        ///                DDS.ViewStateKind viewStates,
        ///                DDS.InstanceStateKind instanceStates)">
        /// </remarks>
        ReturnCode TakeNextInstance(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadNextInstanceWithCondition(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode ReadNextInstanceWithCondition(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition);

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReadNextInstanceWithCondition(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode ReadNextInstanceWithCondition(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeNextInstanceWithCondition(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode TakeNextInstanceWithCondition(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition);

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader (abstract).
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.TakeNextInstanceWithCondition(
        ///                ref Space.Foo[] dataValues,
        ///                ref DDS.SampleInfo[] sampleInfos,
        ///                int maxSamples,
        ///                DDS.InstanceHandle instanceHandle,
        ///                DDS.IReadCondition readCondition)">
        /// </remarks>
        ReturnCode TakeNextInstanceWithCondition(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition);

        /// <summary>
        /// This operation indicates to the DataReader that the application is done accessing
        /// the dataValues and sampleInfos arrays (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.ReturnLoan(ref Space.Foo[] dataValues, ref DDS.SampleInfo[] sampleInfos)">
        /// </remarks>
        ReturnCode ReturnLoan(
                ref <data>[] dataValues,
                ref SampleInfo[] sampleInfos);

        /// <summary>
        /// This operation retrieves the key value of a specific instance (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.GetKeyValue(ref Space.Foo key, DDS.InstanceHandle handle)">
        /// </remarks>
        ReturnCode GetKeyValue(
                ref <data> key,
                InstanceHandle handle);

        /// <summary>
        /// This operation returns the handle which corresponds to the instance data (abstract).
        /// </summary>
        /// <remarks>
        /// This abstract operation is defined as a generic operation, which is implemented by
        /// the &lt;type&gt;DataReader class. Therefore, to use this operation, the data type
        /// specific implementation of this operation in its respective derived class must be
        /// used.
        ///
        /// For further explanation see the description for the fictional data type Space.Foo
        /// derived Space.FooDataReader class.
        ///
        /// <seealso cref="Space.FooDataReader.LookupInstance(Space.Foo instance)">
        /// </remarks>
        InstanceHandle LookupInstance(
                <data> instance);

#endif // DOXYGEN_FOR_CS

    }

    // listener interfaces

    public interface IListener
    {
        // Just an empty tagging interface.
    }
	/// <summary>Since a ITopic is an Entity, it has the ability to have a IListener
	/// associated with it. In this case, the associated IListener should be of type
	/// ITopicListener. This interface must be implemented by the
	/// application. A user-defined class must be provided by the application which must
	/// extend from the ITopicListener class.All operations for this interface must be implemented
	/// in the user-defined class, it is up to the application whether an operation is empty or
	/// contains some functionality.
	/// The ITopicListener provides a generic mechanism (actually a callback function) for the Data
	/// Distribution Service to notify the application of relevant asynchronous status change events,
	/// such as a missed deadline, violation of a QosPolicy setting, etc. The ITopicListener is
	/// related to changes in communication status StatusConditions.</summary>
	/// <example>
	/// <code>
	///	public class MyExampleTopicListener : DDS.TopicListener
	///	{
	///		public void OnInconsistentTopic(DDS.ITopic the_topic, DDS.InconsistentTopicStatus status)
	///		{
	///			Console.WriteLine("On_inconsistent_topic");
	///		}
	///	}
	/// </code>
	/// </example>
    public interface ITopicListener : IListener
    {
		/// <summary>
		/// This operation is called by the Data Distribution Service when the
		/// InconsistentTopicStatus changes.</summary>
		/// <remarks>
		/// The implementation may be left empty when this functionality is not needed.
		/// This operation will only be called when the relevant ITopicListener is installed and
		/// enabled with the DDS.StatusKind.InconsistentTopic. The InconsistentTopicStatus will change
		/// when another Topic exists with the same topic_name but different characteristics.
		/// </remarks>
		/// <param name="entityInterface">contain a pointer to the Topic on which the conflict
		/// occurred (this is an input to the application).</param>
		/// <param name="status">contain the InconsistentTopicStatus object (this is
		/// an input to the application).</param>
        void OnInconsistentTopic(ITopic entityInterface, InconsistentTopicStatus status);
    }

	/// <summary>Since a DataWriter is an Entity, it has the ability to have a IListener
	/// associated with it. In this case, the associated IListener should be of type
	/// IDataWriterListener. This interface must be implemented by the
	/// application. A user-defined class must be provided by the application which must
	/// extend from the DataWriterListener class.</summary>
	/// <remarks>All operations for this interface must be implemented in the user-defined class,
	/// it is up to the application whether an operation is empty or contains some functionality.
	/// The IDataWriterListener provides a generic mechanism (actually a callback function)
	/// for the Data Distribution Service to notify the application of relevant
	/// asynchronous status change events, such as a missed deadline, violation of
	/// a QosPolicy setting, etc. The IDataWriterListener is related to
	/// changes in communication status IStatusConditions.</remarks>
	/// <example><code>
	/// public class MyExampleDataWriterListener : DDS.DataWriterListener
	/// {
	/// 	public MyExampleDataWriterListener()
	/// 	{
	/// 	}
	/// 	public void OnOfferedDeadlineMissed(DDS.IDataWriter writer, DDS.OfferedDeadlineMissedStatus status)
	/// 	{
	/// 		Console.WriteLine("OnOfferedDeadlineMissed");
	/// 	}
	///
	/// 	public void OnOfferedIncompatibleQos(DDS.IDataWriter writer, DDS.OfferedIncompatibleQosStatus status)
	/// 	{
	/// 		Console.WriteLine("OnOfferedIncompatibleQos");
	/// 	}
	///
	/// 	public void OnLivelinessLost(DDS.IDataWriter writer, DDS.LivelinessLostStatus status)
	/// 	{
	/// 		Console.WriteLine("OnLivelinessLost");
	/// 	}
	///
	/// 	public void OnPublicationMatched(DDS.IDataWriter writer, DDS.PublicationMatchedStatus status)
	/// 	{
	/// 		Console.WriteLine("OnPublicationMatched");
	/// 	}
	/// }
	/// </code>
	/// </example>
    public interface IDataWriterListener : IListener
    {
		/// <summary>This operation is called by the Data Distribution Service when the
		/// OfferedDeadlineMissedStatus changes.</summary>
		/// <remarks> This operation will only be called when the relevant
		/// IDataWriterListener is installed and enabled for the offered
		/// deadline missed status (DDS.StatusKind.OfferedDeadlineMissed). The offered deadline
		/// missed status will change when the deadline that the IDataWriter has
		/// committed through its DeadlineQosPolicy was not respected for a
		/// specific instance.</remarks>
		/// <param name="entityInterface">contain a pointer to the IDataWriter on which
		/// the OfferedDeadlineMissedStatus has changed (this is an input to the application)</param>
		/// <param name="status">contain the OfferedDeadlineMissedStatus object (this is
		/// an input to the application).</param>
        void OnOfferedDeadlineMissed(IDataWriter entityInterface, OfferedDeadlineMissedStatus status);
		/// <summary>This operation called by the Data Distribution Service when the
		/// OfferedIncompatibleQosStatus changes.</summary>
		/// <remarks>This operation will only be called when the relevant IDataWriterListener
		/// is installed and enabled for the DDS.StatusKind.OfferedIncompatibleQos. The incompatible
		/// Qos status will change when a IDataReader object has been discovered by the IDataWriter
		/// with the same ITopic and a requested DataReaderQos that was incompatible with the
		/// one offered by the IDataWriter.</remarks>
		/// <param name="entityInterface">contain a pointer to the IDataWriter on which the
		/// OfferedIncompatibleQosStatus has changed (this is an input to the application).</param>
		/// <param name="status">contain the OfferedIncompatibleQosStatus object (this is
		/// an input to the application).</param>
        void OnOfferedIncompatibleQos(IDataWriter entityInterface, OfferedIncompatibleQosStatus status);
		/// <summary>This operation is called by the Data Distribution Service when the
		/// LivelinessLostStatus changes.</summary>
		/// <remarks>This operation will only be called when the relevant
		/// IDataWriterListener is installed and enabled for the liveliness lost status
		/// (LivelinessLostStatus).
		/// The liveliness lost status will change when the liveliness that the DataWriter has
		/// committed through its LivelinessQosPolicy was not respected. In other words,
		/// the IDataWriter failed to actively signal its liveliness within the offered liveliness
		/// period. As a result, the IDataReader objects will consider the IDataWriter as no
		/// longer “alive”.</remarks>
		/// <param name="entityInterface">contains a pointer to the IDataWriter on which
		/// the LivelinessLostStatus has changed (this is an input to
		/// the application).</param>
		/// <param name="status">contains the LivelinessLostStatus object (this is an input
		/// to the application).</param>
        void OnLivelinessLost(IDataWriter entityInterface, LivelinessLostStatus status);
		/// <summary>
		/// This operation is called by the Data Distribution Service when a new match has
		/// been discovered for the current publication, or when an existing match has
		/// ceased to exist.</summary>
		/// <remarks>Usually this means that a new IDataReader that matches the ITopic
		/// and that has compatible Qos as the current IDataWriter has either been discovered,
		/// or that a previously discovered IDataReader has ceased to be matched to the current
		/// IDataWriter. A IDataReader may cease to match when it gets deleted, when it
		/// changes its Qos to a value that is incompatible with the current IDataWriter or
		/// when either the IDataWriter or the IDataReader has chosen to put its matching
		/// counterpart on its ignore-list using the DDS.IDomainParticipant.IgnoreSubscription
		/// or DDS.IDomainParticipant.IgnorePublication operations.
		/// it will only be called when the relevant IDataWriterListener is installed and enabled
		/// for the PublicationMatchedStatus.</remarks>
		/// <param name="entityInterface">contains a pointer to the IDataWriter for which a
		/// match has been discovered (this is an input to the application provided by the
		/// Data Distribution Service).</param>
		/// <param name="status">contains the PublicationMatchedStatus object
		/// (this is an input to the application provided by the Data Distribution Service).</param>

        void OnPublicationMatched(IDataWriter entityInterface, PublicationMatchedStatus status);
    }
	/// <summary> Since a Publisher is an Entity, it has the ability to have a Listener
	/// associated with it. In this case, the associated Listener should be of type
	/// IPublisherListener. This interface must be implemented by the
	/// application. A user-defined class must be provided by the application which must
	/// extend from the IPublisherListener class.
	/// All operations for this interface must be implemented in the user-defined class, it is
	/// up to the application whether an operation is empty or contains some functionality.
	/// The IPublisherListener provides a generic mechanism (actually a
	/// callback function) for the Data Distribution Service to notify the application of
	/// relevant asynchronous status change events, such as a missed deadline, violation of
	/// a QosPolicy setting, etc. The IPublisherListener is related to
	/// changes in communication status StatusConditions.
	/// </summary>
	/// <example>
	/// <code>
	///	public class MyPublisherListener : DDS.IPublisherListener
	///	{
	///		public void OnPublicationMatched(DDS.IDataWriter writer, DDS.PublicationMatchedStatus status)
	///		{
	///			Console.WriteLine("OnPublicationMatched");
	///		}
	///	}
	/// </code>
	/// </example>
    public interface IPublisherListener : IDataWriterListener
    {
    }
	/// <summary>Since a IDataReader is an Entity, it has the ability to have a IListener
	/// associated with it. In this case, the associated IListener should be of type
	/// IDataReaderListener. This interface must be implemented by the
	/// application. A user-defined class must be provided by the application which must
	/// extend from the IDataReaderListener class.</summary>
	/// <remarks>All operations for this interface must be implemented in the user-defined
	/// class, it is up to the application whether an operation is empty or contains some
	/// functionality. The IDataReaderListener provides a generic mechanism (actually a
	/// callback function) for the Data Distribution Service to notify the application of
	/// relevant asynchronous status change events, such as a missed deadline, violation of
	/// a QosPolicy setting, etc. The IDataReaderListener is related to
	/// changes in communication status IStatusConditions.</remarks>
	/// <example><code>
	///public class MyExampleDataReaderListener : DDS.IDataReaderListener
	/// {
	///	public MyExampleDataReaderListener()
	/// 	{
	/// 	}
	///	public void OnRequestedDeadlineMissed(DDS.IDataReader reader, DDS.RequestedDeadlineMissedStatus status)
	/// 	{
	/// 		Console.WriteLine("OnRequestedDeadlineMissed");
	/// 	}
	///
	/// 	public void OnRequestedIncompatibleQos(DDS.IDataReader reader, DDS.RequestedIncompatibleQosStatus status)
	/// 	{
	/// 		Console.WriteLine("OnRequestedIncompatibleQos");
	/// 	}
	///
	/// 	public void OnSampleRejected(DDS.IDataReader reader, DDS.SampleRejectedStatus status)
	/// 	{
	/// 		Console.WriteLine("OnSampleRejected");
	/// 	}
	///
	/// 	public void OnLivelinessChanged(DDS.IDataReader reader, DDS.LivelinessChangedStatus status)
	/// 	{
	/// 		Console.WriteLine("OnLivelinessChanged");
	/// 	}
	///
	/// 	public void OnDataAvailable(DDS.IDataReader reader)
	/// 	{
	/// 		Console.WriteLine("OnDataAvailableCalled");
	/// 	}
	///
	/// 	public void OnSubscriptionMatched(DDS.IDataReader reader, DDS.SubscriptionMatchedStatus status)
	/// 	{
	/// 		Console.WriteLine("OnSubscriptionMatchCalled");
	/// 	}
	///
	/// 	public void OnSampleLost(DDS.IDataReader reader, DDS.SampleLostStatus status)
	/// 	{
	/// 		Console.WriteLine("OnSampleLostCalled");
	/// 	}
	/// }
	/// </code>
	/// </example>
    public interface IDataReaderListener : IListener
    {

		/// <summary>
		/// This operation called by the Data Distribution Service when the deadline
		/// that the IDataReader was expecting through its DeadlineQosPolicy was not
		/// respected for a specific instance.
		/// </summary>
		/// <remarks>
		/// The implementation may be left empty when this
		/// functionality is not needed. This operation will only be called when the relevant
		/// IDataReaderListener is installed and enabled for the
		/// DDS.StatusKind.RequestedDeadlineMissed
		/// </remarks>
		/// <param name="entityInterface">contain a pointer to the IDataReader for which
		/// the deadline was missed (this is an input to the application provided by the Data
		/// Distribution Service). </param>
		/// <param name="status">contain the RequestedDeadlineMissedStatus object (this is
		/// an input to the application  provided by the Data Distribution Service). </param>
        void OnRequestedDeadlineMissed(IDataReader entityInterface, RequestedDeadlineMissedStatus status);

		/// <summary>This operation is called by the Data Distribution Service when the
		/// DDS.StatusKind.RequestedIncompatibleQos changes. </summary>
		/// <remarks> The implementation may be left empty when this functionality is not
		/// needed. This operation will only be called when the relevant IDataReaderListener
		/// is installed and enabled for the DDS.StatusKind.RequestedIncompatibleQos. The
		/// Data Distribution Service will provide a reference to the IDataReader in the
		/// parameter <paramref name="entityInterface"/> and the RequestedIncompatibleQosStatus object in the
		/// parameter <paramref name="status"/> , for use by the application.
		///
		/// When the IDataReaderListener on the IDataReader is not enabled with the
		/// DDS.StatusKind.RequestedIncompatibleQos, the DDS.StatusKind.RequestedIncompatibleQos
		/// change will propagate to the ISubscriberListener of the ISubscriber
		/// (if enabled) or to the IDomainParticipantListener of the
		/// IDomainParticipant (if enabled).
		/// <param name="entityInterface">reader the IDataReader provided by the Data Distribution Service.</param>
		/// <param name="status">the RequestedIncompatibleQosStatus object provided by the
		/// Data Distribution Service.</param>
        void OnRequestedIncompatibleQos(IDataReader entityInterface, RequestedIncompatibleQosStatus status);

		/// <summary>This operation called by the Data Distribution Service when a (received)
		/// sample has been rejected.</summary>
		/// <remarks> Samples may be rejected by the IDataReader when it runs out of
		/// ResourceLimitsQosPolicy to store incoming samples. Usually this means that old samples need
		/// to be ‘consumed’ (for example by ‘taking’ them instead of ‘reading’ them) to make
		/// room for newly incoming samples.
		/// The implementation may be left empty when this functionality is not needed. This
		/// operation will only be called when the relevant IDataReaderListener is installed
		/// and enabled with the DDS.StatusKind.SampleLost.</remarks>
		/// <param name="entityInterface">contains a pointer to the IDataReader for which a sample
		///  has been rejected (this is an input to the application provided by the
		/// Data Distribution Service).</param>
		/// <param name="status"> contains the SampleRejectedStatus object (this is an
		/// input to the application provided by the Data Distribution Service).
        void OnSampleRejected(IDataReader entityInterface, SampleRejectedStatus status);

		/// <summary>
		/// This operation is called by the Data Distribution Service when the liveliness of
		/// one or more IDataWriter objects that were writing instances read through this
		/// IDataReader has changed.
		/// </summary>
		/// <remarks>
		/// In other words, some IDataWriter have become
		/// “alive” or “not alive”. The implementation may be left empty when this
		/// functionality is not needed. This operation will only be called when the relevant
		/// IDataReaderListener is installed and enabled for the
		/// DDS.StatusKind.LivelinessChanged.
		/// </remarks>
		/// <param name="entityInterface">contain a pointer to the IDataReader for which
		/// the liveliness of one or more IDataWriter objects has changed (this is an input
		/// to the application provided by the Data Distribution Service).
		/// </param>
		/// <param name="status">contain the LivelinessChangedStatus object (this
		///  is an input to the application provided by the Data Distribution Service).
		/// </param>
		void OnLivelinessChanged(IDataReader entityInterface, LivelinessChangedStatus status);

		/// <summary>
		/// This operation is called by the Data Distribution Service when new data is
		/// available for this IDataReader.
		/// </summary>
		/// <remarks>
	    /// The implementation may be left empty when this functionality is not
		/// needed. This operation will only be called when the relevant IDataReaderListener
		/// is installed and enabled for the DDS.StatusKind.DataAvailable.
		/// The Data Distribution Service will provide a reference to the IDataReader in the
		/// parameter <paramref name="entityInterface"/> for use by the application.
		/// The statuses DDS.StatusKind.DataOnReaders and DDS.StatusKind.DataAvailable will
		/// occur together. In case these status changes occur, the Data Distribution
		/// Service will look for an attached and activated ISubscriberListener or
		/// IDomainParticipantListener (in that order) for the enabled
		/// DDS.StatusKind.DataOnReaders. In case the DDS.StatusKind.DataOnReaders can not be
		/// handled, the Data Distribution Service will look for an attached and activated
		/// IDataReaderListener, ISubscriberListener or IDomainParticipantListener for the enabled
		/// DDS.StatusKind.DataAvailable (in that order).
		/// Note that if DataOnReaders is called, then the Data Distribution Service
		/// will not try to call DataAvailable, however, the application can force a call
		/// to the IDataReader objects that have data by means of the ISubscriber.NotifyDataReaders
		/// operation.
		/// </remarks>
		/// <param name="entityInterface">contain a pointer to the IDataReader for which
		/// data is available (this is an input to the application provided by the Data
		/// Distribution Service).</param>
		void OnDataAvailable(IDataReader entityInterface);

		/// <summary> This operation is called by the Data Distribution Service
		///  when a new match has been discovered for the current subscription, or
		///  when an existing match has ceased to exist. </summary>
		/// <remarks> Usually this means that a new IDataWriter that matches
		/// the ITopic and that has compatible Qos as the current IDataReader has
		/// either been discovered, or that a previously discovered IDataWriter has
		/// ceased to be matched to the current IDataReader. A IDataWriter may cease to
		/// match when it gets deleted, when it changes its Qos to a value that is incompatible
		/// with the current IDataReader or when either the IDataReader or the IDataWriter
		/// has chosen to put its matching counterpart on its ignore-list using the
		/// DDS.IDomainParticipant.IgnoreSubscription or DDS.IDomainParticipant.IgnorePublication operations.
		///
		/// The implementation of this IListener operation may be left empty when this
		/// functionality is not needed: it will only be called when the relevant
		/// IDataReaderListener is installed and enabled for the DDS.StatusKind.SubscriptionMatched.</remarks>
		///
		/// <param name="entityInterface">contains a pointer to the IDataReader for which
		/// a match has been discovered (this is an input to the application provided by the
		/// Data Distribution Service). </param>
		/// <param name="status"> contains the SubscriptionMatchedStatus object (this
		///  is an input to the application provided by the Data Distribution Service).</param>

        void OnSubscriptionMatched(IDataReader entityInterface, SubscriptionMatchedStatus status);

		/// <summary>NOTE: This operation is not yet implemented. It is scheduled for a future release.</summary>
		/// <param name="entityInterface"> the IDataReader the Listener is applied to</param>
		/// <param name="status"> the SampleLostStatus status</param>
        void OnSampleLost(IDataReader entityInterface, SampleLostStatus status);
    }

	/// <summary>Since a Subscriber is an Entity, it has the ability to have a Listener
	/// associated with it. In this case, the associated Listener should be of type
	/// ISubscriberListener. This interface must be implemented by the
	/// application. A user-defined class must be provided by the application which must
	/// extend from the SubscriberListener class. All operations for this interface must be
	/// implemented in the user-defined class, it is up to the application whether an operation is
	/// empty or contains some functionality.
	/// The ISubscriberListener provides a generic mechanism (actually a callback function) for
	/// the Data Distribution Service to notify the application of relevant asynchronous status
	/// change events, such as a missed deadline, violation of a QosPolicy setting, etc. The
	/// ISubscriberListener is related to changes in communication status StatusConditions.</summary>
	/// <example><code>
	/// public class MySubscriberListener : DDS.ISubscriberListener
	/// {
	/// 	public MySubscriberListener()
	/// 	{
	/// 	}
	/// 	public void OnDataOnReaders(DDS.ISubscriber entityInterface)
	/// 	{
	/// 		Console.WriteLine("OnDataAvailableCalled");
	/// 	}
	/// }
	/// </code>
	/// </example>
    public interface ISubscriberListener : IDataReaderListener
    {
		/// <summary> This operation called by the Data Distribution Service when new data is
		///  available for this Subscriber.</summary>
		/// <remarks> The implementation may be left empty when this functionality is not needed.
		/// This operation will only be called when the relevant ISubscriberListener
		/// is installed and enabled with the DDS.StatusKind.DataOnReaders.
		/// The statuses OnDataOnReaders() and OnDataAvailable() will occur together.
		/// In case these status changes occur, the Data Distribution Service will look for
		/// an attached and activated ISubscriberListener or IDomainParticipantListener
		/// (in that order) for the enabled DDS.StatusKind.DataOnReaders. In case the DDS.StatusKind.DataOnReaders
		/// can not be handled, the Data Distribution Service will look for an attached and activated
		/// IDataReaderListener, ISubscriberListener or IDomainParticipantListener for the enabled
		/// DDS.StatusKind.DataAvailable (in that order). Note that if OnDataOnReaders is called, then the Data Distribution
		/// Service will not try to call OnDataAvailable, however, the application can force a call
		/// to the callback function OnDataAvailable of IDataReaderListener objects that have
		/// data by means of the Subscriber.NotifyDatareaders() operation.</remarks>
		/// <param name="entityInterface">contain a pointer to the ISubscriber for which data is available (this is
		/// an input to the application provided by the Data Distribution Service).</param>
        void OnDataOnReaders(ISubscriber entityInterface);
    }
	///<summary>Since a DomainParticipant is an Entity, it has the ability to have a Listener
	/// associated with it. In this case, the associated Listener should be of type
	/// IDomainParticipantListener. This interface must be implemented by the
	/// application. A user-defined class must be provided by the application which must
	/// extend from the IDomainParticipantListener class.
	/// All operations for this interface must be implemented in the user-defined class, it is
	/// up to the application whether an operation is empty or contains some functionality.
	/// The IDomainParticipantListener provides a generic mechanism (actually a callback function)
	/// for the Data Distribution Service to notify the application of relevant asynchronous status
	/// change events, such as a missed deadline, violation of QosPolicy setting, etc. The
	/// IDomainParticipantListener is related to changes in communication status StatusConditions.
	/// </summary>
	/// <example>
	/// <code>
	/// public class MyExampleParticipantListener : DDS.IDomainParticipantListener
	/// {
	/// 	public MyExampleParticipantListener()
	/// 	{
	/// 	}
	/// 	public void OnDataAvailable(DDS.IDataReader reader)
	/// 	{
	/// 		Console.WriteLine("OnDataAvailable");
	/// 	}
	/// 	public void DDS.ITopicListener.OnInconsistentTopic(DDS.ITopic entityInterface, DDS.InconsistentTopicStatus status)
	/// 	{
	/// 		Console.WriteLine("OnInconsistentTopic");
	/// 	}
	/// }
	/// </code>
	/// </example>
    public interface IDomainParticipantListener : ITopicListener, IPublisherListener, ISubscriberListener
    {
    }

    /// @cond
    /// The interface IQosProvider is not very interesting for
    /// the documentation. Instead of that, the QosProvider class
    /// itself has been documented.
    public interface IQosProvider {


        ReturnCode
        GetParticipantQos (
            ref DomainParticipantQos participantQos,
            string id);

        ReturnCode
        GetTopicQos (
            ref TopicQos topicQos,
            string id);

        ReturnCode
        GetSubscriberQos (
            ref SubscriberQos subscriberQos,
            string id);

        ReturnCode
        GetDataReaderQos (
            ref DataReaderQos datareaderQos,
            string id);

        ReturnCode
        GetPublisherQos (
            ref PublisherQos publisherQos,
            string id);

        ReturnCode
        GetDataWriterQos (
            ref DataWriterQos datawriterQos,
            string id);

        // Future expansion will allow the user to share QoSs over DDS
        //
        // ReturnCode_t
        // subscribe ();
        //
        // ReturnCode_t
        // publish ();
    }
    /// @endcond


} // end namespace DDS
