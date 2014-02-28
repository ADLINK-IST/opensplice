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
// Csharp backend
// PTF C# mapping for IDL
// File /Users/Jcm/Documents/Ecllipse_WS/CSharpDDS/generated/dds_dcps.cs
// Generated on 2008-11-11 13:36:00
// from dds_dcps.idl

using System;
using System.Runtime.InteropServices;

namespace DDS
{
    // ----------------------------------------------------------------------
    // Conditions
    // ----------------------------------------------------------------------
    /// <summary>
    /// Base class for all Conditions that maybe attached to a WaitSet.
    /// </summary>
    /// <remarks>
    /// This class is the base class for all the conditions that may be attached to a WaitSet.
    /// This base class is specialized in three classes by the Data Distribution Service:
    /// GuardCondition, StatusCondition and ReadCondition (also there is a
    /// QueryCondition which is a specialized ReadCondition).
    /// Each Condition has a trigger_value that can be TRUE or FALSE and is set by
    /// the Data Distribution Service (except a GuardCondition) depending on the
    /// evaluation of the Condition.
    /// </remarks>
    public interface ICondition
    {
        /// <summary>
        /// Each Condition has a trigger_value that can be TRUE or FALSE and is set
        /// by the DDS depending on the evaluation of the Condition.
        /// </summary>
        /// <returns>the trigger_value (true|false)</returns>
        bool GetTriggerValue();
    }

    /// <summary>
    /// A WaitSet object allows an application to wait until one or more of the attached 
    /// Condition objects evaluates to TRUE or until the timeout expires. 
    /// The WaitSet has no factory and must be created by the application. It is directly 
    /// created as an object by using WaitSet constructors.
    /// </summary>
    public interface IWaitSet
    {
        /// <summary>
        /// This operation allows an application thread to wait for the occurrence of at least one 
        /// of the conditions that is attached to the WaitSet.
        /// </summary>
        /// <remarks>This operation allows an application thread to wait for the occurrence of at least one
        /// of the conditions to evaluate to TRUE that is attached to the WaitSet. If all of the
        /// conditions attached to the WaitSet have a trigger_value of FALSE, the wait
        /// operation will block the calling thread. The result of the operation is the
        /// continuation of the application thread after which the result is left in
        /// activeConditions. This is a reference to a sequence, which will contain the list
        /// of all the attached conditions that have a trigger_value of TRUE. The
        /// activeConditions sequence and its buffer may be pre-allocated by the
        /// application and therefore must either be re-used in a subsequent invocation of the
        /// wait operation or be released by invoking its destructor either implicitly or
        /// explicitly. If the pre-allocated sequence is not big enough to hold the number of
        /// triggered Conditions, the sequence will automatically be (re-)allocated to fit the
        /// required size. The parameter timeout specifies the maximum duration for the
        /// wait to block the calling application thread (when none of the attached conditions
        /// has a trigger_value of TRUE) . In that case the return value is
        /// Timeout and the activeConditions sequence is left empty. Since it
        /// is not allowed for more than one application thread to be waiting on the same
        /// WaitSet, the operation returns immediately with the value
        /// PreconditionNotMet when the wait operation is invoked on a
        /// WaitSet which already has an application thread blocking on it.
        /// </remarks>
        /// <param name="activeConditions">a sequence which is used to pass the list of all the attached 
        /// conditions that have a trigger_value of true.</param>
        /// <param name="timeout">the maximum duration to block for the wait, after which the application thread 
        /// is unblocked. The special constant Infinite can be used when the maximum waiting time does not 
        /// need to be bounded.</param>
        /// <returns>Possible return codes for the operation are: 
        /// <list type="bullet">
        /// <item>Ok - at least one of the attached conditions has a trigger_value of TRUE.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of 
        /// resources to complete this operation.</item>
        /// <item>Timeout - the timeout has elapsed without any of the attached conditions
        /// becoming TRUE.</item>
        /// <item>PreconditionNotMet - the WaitSet already has an application 
        /// thread blocking on it.</item>
        /// </list> 
        /// </returns>
        ReturnCode Wait(ref ICondition[] activeConditions, Duration timeout);
        /// <summary>
        /// This operation attaches a condition to the WaitSet.
        /// </summary>
        /// <remarks>
        /// This operation attaches a Condition to the WaitSet. The parameter cond must be
        /// either a ReadCondition, QueryCondition, StatusCondition or GuardCondition. 
        /// To get this parameter see:
        /// <list type="bullet">
        /// <item>ReadCondition created by create_readcondition.</item>
        /// <item>QueryCondition created by create_querycondition.</item>
        /// <item>StatusCondition retrieved by get_statuscondition on an Entity.</item>
        /// <item>GuardCondition created by the C# operation new.</item>
        /// </list>
        /// When a GuardCondition is initially created, the trigger_value is FALSE.
        /// When a Condition, whose trigger_value evaluates to TRUE, is attached to a
        /// WaitSet that is currently being waited on (using the wait operation), the WaitSet
        /// will unblock immediately.
        /// </remarks>
        /// <param name="condition">The condition to be attached to the WaitSet. 
        /// The parameter must be either a ReadCondition, QueryCondition, StatusCondition or GuardCondition</param>
        /// <returns>Possible return codes are:
        /// <list type="bullet">
        /// <item>Ok - the Condition is attached to the WaitSet.</item>
        /// <item>Error - an internal error has occured.</item>
        /// <item>BadParameter - the parameter condition is not a valid ICondition.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode AttachCondition(ICondition condition);
        /// <summary>This operation detaches a Condition from the WaitSet.</summary>
        /// <remarks>
        /// This operation detaches a Condition from the WaitSet. If the Condition was
        /// not attached to this WaitSet, the operation returns PreconditionNotMet.
        /// </remarks>
        /// <param name="condition">The attached condition in the WaitSet which is to be detached.</param>
        /// <returns>Possible return codes are: 
        /// <list type="bullet">
        /// <item>Ok - the Condition is detached from the WaitSet.</item>
        /// <item>Error - an internal error has occured.</item>
        /// <item>BadParameter - the parameter condition is not a valid ICondition.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// <item>PreconditionNotMet - the Condition was not attached to this WaitSet.</item>
        /// </list>
        /// </returns>
        ReturnCode DetachCondition(ICondition condition);
        /// <summary>
        /// This operation retrieves the list of attached conditions.
        /// </summary>
        /// <remarks>
        /// This operation retrieves the list of attached conditions in the WaitSet. The
        /// parameter attached_conditions is a reference to a sequence which afterwards
        /// will refer to the sequence of attached conditions. The attached_conditions
        /// sequence and its buffer may be pre-allocated by the application and therefore must
        /// either be re-used in a subsequent invocation of the get_conditions operation or
        /// be released by invoking its destructor either implicitly or explicitly. If the
        /// pre-allocated sequence is not big enough to hold the number of triggered
        /// Conditions, the sequence will automatically be (re-)allocated to fit the required
        /// size. The resulting sequence will either be an empty sequence, meaning there were
        /// no conditions attached, or will contain a list of ReadCondition,
        /// QueryCondition, StatusCondition and GuardCondition. These conditions
        /// previously have been attached by attach_condition and were created by there
        /// respective create operation:
        /// <list type="bullet">
        /// <item>ReadCondition created by create_readcondition.</item>
        /// <item>QueryCondition created by create_querycondition.</item>
        /// <item>StatusCondition retrieved by get_statuscondition on an Entity.</item>
        /// <item>GuardCondition created by the C# operation new.</item>
        /// </list>
        /// </remarks>
        /// <param name="attachedConditions">A reference to a sequence that will hold all the attached conditions
        /// on the WaitSet</param>
        /// <returns>Possible return codes are: 
        /// <list type="bullet">
        /// <item>Ok - the list of attached conditions is returned.</item>
        /// <item>Error - an internal error has occured.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetConditions(ref ICondition[] attachedConditions);
    }

    /// <summary>
    /// A GuardCondition object is a specific Condition whose trigger_value is
    /// completely under the control of the application. The GuardCondition has no
    /// factory and must be created by the application. The GuardCondition is directly
    /// created as an object by using the GuardCondition constructor. When a
    /// GuardCondition is initially created, the trigger_value is FALSE. The purpose
    /// of the GuardCondition is to provide the means for an application to manually
    /// wake up a WaitSet. This is accomplished by attaching the GuardCondition to
    /// the Waitset and setting the trigger_value by means of the
    /// set_trigger_value operation.
    /// </summary>
    public interface IGuardCondition : ICondition
    {
        /// <summary>
        /// his operation sets the trigger_value of the GuardCondition.
        /// </summary>
        /// <remarks>A GuardCondition object is a specific Condition which trigger_value is
        /// completely under the control of the application. This operation must be used by the
        /// application to manually wake-up a WaitSet. This operation sets the
        /// trigger_value of the GuardCondition to the parameter value. The
        /// GuardCondition is directly created using the GuardCondition constructor.
        /// When a GuardCondition is initially created, the trigger_value is FALSE.</remarks>
        /// <param name="value">the Boolean value to which the GuardCondition is set.</param>
        /// <returns>Possible return codes of the operation are:
        /// <list type="bullet">
        /// <item>Ok - the specified trigger_value has successfully been applied.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// </list>
        /// </returns>
        ReturnCode SetTriggerValue(bool value);
    }

    /// <summary>
    /// Specialized class of Condition, and indicates the condition that may be attached to
    /// a WaitSet.
    /// </summary>
    /// <remarks>
    /// Entity objects that have status attributes also have a StatusCondition, access is 
    /// provided to the application by the GetStatusCondition operation.
    /// The communication statuses whose changes can be communicated to the application 
    /// depend on the Entity.
    /// The trigger_value of the StatusCondition depends on the communication
    /// statuses of that Entity (e.g., missed deadline) and also depends on the value of the
    /// StatusCondition attribute mask (enabled_statuses mask). A
    /// StatusCondition can be attached to a WaitSet in order to allow an application
    /// to suspend until the trigger_value has become TRUE.
    /// The trigger_value of a StatusCondition will be TRUE if one of the enabled
    /// StatusChangedFlags is set. That is, trigger_value==FALSE only if all the
    /// values of the StatusChangedFlags are FALSE.
    /// The sensitivity of the StatusCondition to a particular communication status is
    /// controlled by the list of enabled_statuses set on the condition by means of the
    /// set_enabled_statuses operation.
    /// When the enabled_statuses are not changed by the SetEnabledStatuses
    /// operation, all statuses are enabled by default.
    /// </remarks>
    public interface IStatusCondition : ICondition
    {
        /// <summary>
        /// This operation returns the list of enabled communication statuses of the StatusCondition.
        /// </summary>
        /// <returns>StatusKind - a bit mask in which each bit shows which status is taken into account 
        /// for the StatusCondition</returns>
        StatusKind GetEnabledStatuses();
        /// <summary>
        /// This operation sets the list of communication statuses that are taken into account to 
        /// determine the trigger_value of the StatusCondition.
        /// </summary>
        /// <param name="mask">A bit mask in which each bit sets the status which is taken into
        /// account to determine the trigger_value of the StatusCondition</param>
        /// <returns>Possible return codes of the operation are: 
        /// <list type="bullet">
        /// <item>Ok - the list of communication statuses is set.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the StatusCondition has already been deleted.</item>
        /// </list>
        /// </returns>
        ReturnCode SetEnabledStatuses(StatusKind mask);
        /// <summary>
        /// This operation returns the Entity associated with the StatusCondition or a
        /// null Entity.
        /// </summary>
        /// <remarks>This operation returns the Entity associated with the StatusCondition. Note
        /// that there is exactly one Entity associated with each StatusCondition. When
        /// the Entity was already deleted (there is no associated Entity any more), the
        /// NULL value IEntity is returned.</remarks>
        /// <returns>IEntity - The Entity associated with the StatusCondition.</returns>
        IEntity GetEntity();
    }

    /// <remarks>
    /// The DataReader objects can create a set of ReadCondition (and
    /// StatusCondition) objects which provide support (in conjunction with WaitSet
    /// objects) for an alternative communication style between the Data Distribution
    /// Service and the application (i.e., state-based rather than event-based).
    /// ReadCondition objects allow an DataReader to specify the data samples it is
    /// interested in (by specifying the desired sample-states, view-states, and
    /// instance-states); see the parameter definitions for DataReader's
    /// create_readcondition operation. This allows the Data Distribution Service to
    /// trigger the condition only when suitable information is available. ReadCondition
    /// objects are to be used in conjunction with a WaitSet. More than one
    /// ReadCondition may be attached to the same DataReader.
    /// </remarks>
    public interface IReadCondition : ICondition
    {
        /// <summary>
        /// This operation returns the set of sample_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <remarks>
        /// This operation returns the set of sample_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// The sample_states returned are the sample_states specified when the
        /// ReadCondition was created. sample_states can be Read,
        /// NotRead or both.
        /// </remarks>
        /// <returns>The sample_states specified when the ReadCondition was created.</returns>
        SampleStateKind GetSampleStateMask();
        /// <summary>
        /// This operation returns the set of view_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <remarks>
        /// This operation returns the set of view_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// The view_states returned are the view_states specified when the
        /// ReadCondition was created. view_states can be New, NotNew or both.
        /// </remarks>
        /// <returns>The view_states specified when the ReadCondition was created.</returns>
        ViewStateKind GetViewStateMask();
        /// <summary>
        /// This operation returns the set of instance_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <remarks>
        /// This operation returns the set of instance_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// The instance_states returned are the instance_states specified when the
        /// ReadCondition was created. instance_states can be
        /// Alive,NotAlivedDisposed,NotAliveNoWriters or a combination of these.
        /// </remarks>
        /// <returns>The instance_states specified when the ReadCondition was created.</returns>
        InstanceStateKind GetInstanceStateMask();
        /// <summary>
        /// This operation returns the DataReader associated with the ReadCondition.
        /// </summary>
        /// <remarks>This operation returns the DataReader associated with the ReadCondition. Note
        /// that there is exactly one DataReader associated with each ReadCondition (i.e.
        /// the DataReader that created the ReadCondition object).
        /// </remarks>
        /// <returns>The DataReader associated with the ReadCondition.</returns>
        IDataReader GetDataReader();
    }
    /// <summary>
    /// QueryCondition objects are specialized ReadCondition objects that allow the
    /// application to specify a filter on the locally available data. 
    /// </summary>
    /// <remarks>The DataReader objects accept a set of QueryCondition objects for the DataReader and provide support
    /// (in conjunction with WaitSet objects) for an alternative communication style
    /// between the Data Distribution Service and the application (i.e., state-based rather
    /// than event-based).
    /// QueryCondition objects allow an application to specify the data samples it is
    /// interested in (by specifying the desired sample-states, view-states, instance-states
    /// and query expression); see the parameter definitions for DataReader's
    /// read/take operations. This allows the Data Distribution Service to trigger the
    /// condition only when suitable information is available. QueryCondition objects
    /// are to be used in conjunction with a WaitSet. More than one QueryCondition
    /// may be attached to the same DataReader.
    /// The query (query_expression) is similar to an SQL WHERE clause and can be
    /// parameterized by arguments that are dynamically changeable with the
    /// SetQueryParameters operation.
    /// </remarks>
    public interface IQueryCondition : IReadCondition
    {
        /// <summary>
        /// This operation returns the query expression associated with the QueryCondition.
        /// </summary>
        /// <remarks>
        /// This operation returns the query expression associated with the QueryCondition.
        /// That is, the expression specified when the QueryCondition was created. The
        /// operation will return NULL when there was an internal error or when the
        /// QueryCondition was already deleted. If there were no parameters, an empty
        /// sequence is returned.
        /// </remarks>
        /// <returns>The query expression associated with the QueryCondition.</returns>
        string GetQueryExpression();
        /// <summary>
        /// This operation obtains the queryParameters associated with the QueryCondition
        /// </summary>
        /// <remarks>
        /// This operation obtains the query_parameters associated with the
        /// QueryCondition. That is, the parameters specified on the last successful call to
        /// set_query_arguments or, if set_query_arguments was never called, the
        /// arguments specified when the QueryCondition were created.
        /// The resulting handle contains a sequence of strings with the parameters used in the
        /// SQL expression (i.e., the %n tokens in the expression). The number of parameters in
        /// the result sequence will exactly match the number of %n tokens in the query
        /// expression associated with the QueryCondition.
        /// </remarks>
        /// <param name="queryParameters">A reference to a sequence of strings that will be 
        /// used to store the parameters used in the SQL expression</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - the existing set of query parameters applied to this QueryCondition 
        /// has successfully been copied into the specified queryParameters parameter.</item>
        /// <item>Error - an internal error has occured.</item>
        /// <item>AlreadyDeleted - the QueryCondition has already been deleted.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQueryParameters(ref string[] queryParameters);
        /// <summary>
        /// This operation changes the query parameters associated with the QueryCondition.
        /// </summary>
        /// <remarks>
        /// This operation changes the query paramet e r s associated with the QueryCondition. 
        /// The parameter queryParameters is a sequence of strings which are the parameter values
        /// used in the SQL query string (i.e., the %n tokens in the expression). 
        /// The number of values in queryParameters must be equal or greater than the highest 
        /// referenced %n token in the query_expression (e.g. if %1 and %8 are used as parameter 
        /// in the query_expression, the queryParameters should at least contain n+1 = 9 values).
        /// </remarks>
        /// <param name="queryParameters">A sequence of strings which are the parameters used in the SQL query string
        /// (i.e., the %n tokens in the expression).</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - the query parameters associated with the QueryCondition are changed.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>BadParameter - the number of parameters in queryParameters does not match 
        /// the number of %n tokens in the expression for this QueryCondition or one of 
        /// the parameters is an illegal parameter.</item>
        /// <item>AlreadyDeleted - the QueryCondition has already been deleted.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQueryParameters(params string[] queryParameters);
    }

    // ----------------------------------------------------------------------
    // Factory
    // ----------------------------------------------------------------------

    /// <summary>
    /// The purpose of this class is to allow the creation and destruction of 
    /// DomainParticipant objects. DomainParticipantFactory itself has no 
    /// factory. It is a pre-existing singleton object that can be accessed by means of the 
    /// Instance property on the DomainParticipantFactory class. 
    /// The pre-defined value TheParticipantFactory can also be used as an alias for the singleton
    /// factory returned by the operation get_instance.
    /// </summary>
    public interface IDomainParticipantFactory
    {
        /// <summary>
        /// This operation creates a DomainParticipant using the specified ID.
        /// </summary>
        /// <remarks>This operation behaves similarly to the spec operation but substitutes default values
        /// for the missing parameters. These are: default Qos for Qos parameters and null listeners with 0
        /// mask for listener parameters.
        /// </remarks>
        /// <param name="domainId">The specified ID used to create the DomainParticipant</param>
        /// <returns>The newly created DomainParticipant. In case of an error a null is returned.</returns>
        IDomainParticipant CreateParticipant(DomainId domainId);
        /// <summary>
        /// This operation creates a DomainParticipant using the specified ID, listener and mask for listener parameter.
        /// </summary>
        /// <remarks>This operation behaves similarly to the spec operation but substitutes default values
        /// for the missing parameters. These are: default Qos for Qos parameters and null listeners with 0
        /// mask for listener parameters.
        /// <param name="domainId"></param>
        /// <param name="listener"></param>
        /// <param name="mask"></param>
        /// <returns>The newly created DomainParticipant. In case of an error a null is returned.</returns>
        IDomainParticipant CreateParticipant(DomainId domainId,
            IDomainParticipantListener listener, StatusKind mask);
        IDomainParticipant CreateParticipant(DomainId domainId, DomainParticipantQos qos);
        /// <summary>
        /// This operation creates a new DomainParticipant which will join the domain 
        /// identified by domainId, and attaches the optionally specified 
        /// DomainParticipantListener to it.
        /// </summary>
        /// <param name="domainId">The ID of the Domain to which the 
        /// DomainParticipant is joined.</param>
        /// <param name="listener">The DomainParticipantListener instance which will be attached to the new 
        /// DomainParticipant. It is permitted to use null as the value of the listener: 
        /// this behaves as a DomainParticipantListener whose operations perform no action.</param>
        /// <param name="qos">a DomainParticipantQos for the new DomainParticipant. 
        /// When this set of QosPolicy settings is inconsistent, no DomainParticipant is created.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of the 
        /// DomainParticipantListener for a certain status.</param>
        /// <returns>The newly created DomainParticipant. In case of an error a null is returned.</returns>
        IDomainParticipant CreateParticipant(DomainId domainId, DomainParticipantQos qos,
            IDomainParticipantListener listener, StatusKind mask);
        /// <summary>
        /// This operation deletes the specified DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation deletes a DomainParticipant. A DomainParticipant cannot
        /// be deleted when it has any attached Entity objects. When the operation is called
        /// on a DomainParticipant with existing Entity objects, the operation returns
        /// PreconditionNotMet.
        /// </remarks>
        /// <param name="participant"> The DomainParticipant which is to be deleted.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The DomainParticipant is deleted.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>BadParameter - The parameter participant is not a valid IDomainParticipant.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>PreconditionNotMet - The DomainParticipant contains one or more Entity objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteParticipant(IDomainParticipant participant);
        /// <summary>
        /// This operation looks up an existing DomainParticipant based on its domainId.
        /// </summary>
        /// <remarks>
        /// This operation retrieves a previously created DomainParticipant belonging to
        /// the specified domainId. If no such DomainParticipant exists, the operation will
        /// return NULL.
        /// If multiple DomainParticipant entities belonging to the specified domainId
        /// exist, then the operation will return one of them. It is not specified which one.
        /// </remarks>
        /// <param name="domainId">the ID of the Domain for which a joining DomainParticipant 
        /// should be retrieved.</param>
        /// <returns>The retrieved DomainParticipant. If no such DomainParticipant is found 
        /// a null is returned.</returns>
        IDomainParticipant LookupParticipant(DomainId domainId);
        /// <summary>
        /// This operation sets the default DomainParticipantQos of the DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation sets the default DomainParticipantQos of the DomainParticipantFactory 
        /// (that is the struct with the QosPolicy settings) which is used for newly created 
        /// DomainParticipant objects, in case the constant PARTICIPANT_QOS_DEFAULT is used. 
        /// The default DomainParticipantQos is only used when the constant is supplied as parameter 
        /// qos to specify the DomainParticipantQos in the CreateParticipant operation. The 
        /// DomainParticipantQos is always self consistent, because its policies do not 
        /// depend on each other. This means this operation never returns the ReturnCode InconsistentPolicy.
        /// The values set by this operation are returned by GetDefaultParticipantQos.
        /// </remarks>
        /// <param name="qos">The DomainParticipantQos which contains the new default 
        /// DomainParticipantQos for the newly created DomainParticipants</param>
        /// <returns>Possible return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The new default DomainParticipantQos is set.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultParticipantQos(DomainParticipantQos qos);
        /// <summary>
        /// This operation gets the default DomainParticipantQos of the DomainParticipantFactory
        /// </summary>
        /// <remarks>
        /// This operation gets the default DomainParticipantQos of the
        /// DomainParticipantFactory (that is the struct with the QosPolicy settings)
        /// which is used for newly created DomainParticipant objects, in case the constant
        /// PARTICIPANT_QOS_DEFAULT is used. The default DomainParticipantQos is
        /// only used when the constant is supplied as parameter qos to specify the
        /// DomainParticipantQos in the CreateParticipant operation. The
        /// application must provide the DomainParticipantQos struct in which the
        /// QosPolicy settings can be stored and provide a reference to the struct. The
        /// operation writes the default QosPolicy settings to the struct referenced to by qos.
        /// Any settings in the struct are overwritten.
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultParticipantQos, or, if the call was never
        /// made, the default values as specified for each QosPolicy setting.
        /// </remarks>
        /// <param name="qos">A reference to the DomainParticipantQos in which the default DomainParticipantQos
        /// for the DomainParticipant is written.</param>
        /// <returns>Possible return codes are:
        /// <list type="bullet">
        /// <item>Ok - The the default DomainParticipant QosPolicy settings of this DomainParticipantFactory 
        /// have successfully been copied into the specified DomainParticipantQos parameter.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultParticipantQos(ref DomainParticipantQos qos);
        /// <summary>
        /// This operation replaces the existing set of QoS settings for the DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a
        /// DomainParticipantFactory. The parameter qos must contain the struct with
        /// the QosPolicy settings.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned Ok).
        /// </remarks>
        /// <param name="qos">The new set of Qos policy settings for the DomainParticipantFactory.</param>
        /// <returns>Possible return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The new DomainParticipantFactoryQos is set.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(DomainParticipantFactoryQos qos);
        /// <summary>
        /// This operation obtains the QoS settings for the DomainParticipantFactory.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a
        /// DomainParticipantFactory on which this operation is used. This
        /// DomainparticipantFactoryQos is stored at the location pointed to by the qos
        /// parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination DomainParticipantFactoryQos, 
        /// in which the Qos policies will be copied.</param>
        /// <returns>Possible return values are: 
        /// <list type="bullet">
        /// <item>Ok - the existing set of QoS policy values applied to this DomainParticipantFactory 
        /// has successfully been copied into the specified DomainParticipantFactoryQos parameter.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref DomainParticipantFactoryQos qos);
    }

    // ----------------------------------------------------------------------
    // Entities
    // ----------------------------------------------------------------------    
    /// <summary>
    /// This class is the abstract base class for all the DCPS objects. It acts as a generic class 
    /// for Entity objects.
    /// </summary>
    public interface IEntity
    {
        /// <summary>
        /// This operation enables the Entity on which it is being called when the Entity was created 
        /// with the EntityFactoryQosPolicy set to FALSE.
        /// </summary>
        /// <remarks>
        /// This operation enables the Entity. Created Entity objects can start in either an
        /// enabled or disabled state. This is controlled by the value of the
        /// EntityFactoryQosPolicy on the corresponding factory for the Entity.
        /// Enabled entities are immediately activated at creation time meaning all their
        /// immutable QoS settings can no longer be changed. Disabled Entities are not yet
        /// activated, so it is still possible to change there immutable QoS settings. However,
        /// once activated the immutable QoS settings can no longer be changed.
        /// Creating disabled entities can make sense when the creator of the Entity does not
        /// yet know which QoS settings to apply, thus allowing another piece of code to set the
        /// QoS later on. This is for example the case in the DLRL, where the ObjectHomes
        /// create all underlying DCPS entities but do not know which QoS settings to apply.
        /// The user can then apply the required QoS settings afterwards.
        /// The default setting of EntityFactoryQosPolicy is such that, by default, entities
        /// are created in an enabled state so that it is not necessary to explicitly call enable on
        /// newly created entities.
        /// The enable operation is idempotent. Calling enable on an already enabled
        /// Entity returns OK and has no effect.
        /// If an Entity has not yet been enabled, the only operations that can be invoked on it
        /// are: the ones to set, get or copy the QosPolicy settings, the ones that set (or get) the
        /// listener, the ones that get the StatusCondition, the GetStatusChanges
        /// operation (although the status of a disabled entity never changes), and the factory
        /// operations that create, delete or lookup(This includes the LookupTopicDescription but not FindTopic)
        /// other Entities. Other operations will return the error NotEnabled.
        /// Entities created from a factory that is disabled, are created disabled regardless of
        /// the setting of the EntityFactoryQosPolicy.
        /// Calling enable on an Entity whose factory is not enabled will fail and return
        /// PreconditionNotMet.
        /// If the EntityFactoryQosPolicy has autoenable_created_entities set to
        /// TRUE, the enable operation on the factory will automatically enable all Entities
        /// created from the factory.
        /// The Listeners associated with an Entity are not called until the Entity is
        /// enabled. Conditions associated with an Entity that is not enabled are "inactive",
        /// that is, have a trigger_value which is FALSE.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The Application enabled the Entity (or it was already enabled)</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>OutOfResources - the DDS ran out of resources to complete this operation.</item>
        /// <item>PreconditionNotMet - The factory of the Entity is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode Enable();
        /// <summary>
        /// This property allows access to the StatusCondition associated with the Entity.
        /// </summary>
        /// <remarks>
        /// Each Entity has a StatusCondition associated with it. This operation allows
        /// access to the StatusCondition associated with the Entity. The returned
        /// condition can then be added to a WaitSet so that the application can wait for
        /// specific status changes that affect the Entity.
        /// </remarks>
        IStatusCondition StatusCondition { get; }
        /// <summary>
        /// This operation returns a mask with the communication statuses in the Entity that are triggered.
        /// </summary>
        /// <remarks>
        /// This operation returns a mask with the communication statuses in the Entity that
        /// are triggered. That is the set of communication statuses whose value have changed
        /// since the last time the application called this operation. This operation shows
        /// whether a change has occurred even when the status seems unchanged because the
        /// status changed back to the original status.
        /// When the Entity is first created or if the Entity is not enabled, all
        /// communication statuses are in the un-triggered state so the mask returned by the
        /// operation is empty.
        /// The result value is a bit mask in which each bit shows which value has changed.
        /// Each status bit is declared as a constant and can be used in an AND operation to
        /// check the status bit against the result of type StatusMask. Not all statuses are
        /// relevant to all Entity objects. See the respective Listener interfaces for each
        /// Entity for more information.
        /// </remarks>
        StatusKind StatusChanges { get; }
        /// <summary>
        /// This operation returns the InstanceHandle of the builtin topic sample that represents the specified Entity.
        /// </summary>
        /// <remarks>
        /// The relevant state of some Entity objects are distributed using builtin topics. Each
        /// builtin topic sample represents the state of a specific Entity and has a unique
        /// instance_handle. This operation returns the instance_handle of the builtin
        /// topic sample that represents the specified Entity.
        /// Some Entities (Publisher and Subscriber) do not have a corresponding
        /// builtin topic sample, but they still have an instance_handle that uniquely
        /// identifies the Entity. The instance_handles obtained this way can also be used
        /// to check whether a specific Entity is located in a specific DomainParticipant.
        /// </remarks>
        InstanceHandle InstanceHandle { get; }
    }

    /// <summary>
    /// All the DCPS Entity objects are attached to a DomainParticipant. A DomainParticipant 
    /// represents the local membership of the application in a Domain.
    /// </summary>
    /// <remarks>A Domain is a distributed concept that links all the applications that must be able to 
    /// communicate with each other. It represents a communication plane: only the 
    /// Publishers and the Subscribers attached to the same Domain can interact. 
    /// This class implements several functions:
    /// <list type="bullet">
    /// <item>it acts as a container for all other Entity objects</item>
    /// <item>it acts as a factory for the Publisher, Subscriber, Topic,ContentFilteredTopic 
    /// and MultiTopic objects</item>
    /// <item>it provides access to the built-in Topic objects</item>
    /// <item>it provides information about Topic objects</item>
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
        /// This method creates a Publisher. It behaves similarly to the most complete operation, but it 
        /// uses default values for the ommitted parameters. These are the default Qos for Qos parameters, null
        /// for listeners with 0 mask for listener parameters.
        /// </summary>
        /// <returns>The created Publisher.</returns>
        IPublisher CreatePublisher();
        /// <summary>
        /// This method creates a Publisher. It behaves similarly to the most complete operation, but it 
        /// uses default values for the ommitted parameters. These are the default Qos for Qos parameters, null
        /// for listeners with 0 mask for listener parameters.
        /// </summary>
        /// <param name="listener"></param>
        /// <param name="mask"></param>
        /// <returns>The created Publisher.</returns>
        IPublisher CreatePublisher(
                IPublisherListener listener, 
                StatusKind mask);
        /// <summary>
        /// This method creates a Publisher. It behaves similarly to the most complete operation, but it 
        /// uses default values for the ommitted parameters. These are the default Qos for Qos parameters, null
        /// for listeners with 0 mask for listener parameters.
        /// </summary>
        /// <param name="qos"></param>
        /// <returns>The created Publisher.</returns>
        IPublisher CreatePublisher(PublisherQos qos);
        /// <summary>
        /// This operation creates a Publisher with the desired QosPolicy settings and if applicable, 
        /// attaches the optionally specified PublisherListener to it.
        /// </summary>
        /// <param name="qos">A collection of QosPolicy settings for the new Publisher.
        /// In case these settings are not self consistent, no Publisher is created.</param>
        /// <param name="listener">The PublisherListener instance which will be attached to the new Publisher.
        /// It is permitted to use null as the value of the listener: this behaves as a PublisherListener 
        /// whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of the PublisherListener 
        /// for a certain status.</param>
        /// <returns>The newly created Publisher. In case of an error, a null Publisher is returned.</returns>
        IPublisher CreatePublisher(
                PublisherQos qos,
                IPublisherListener listener, 
                StatusKind mask);
        /// <summary>
        /// This operation deletes a Publisher.
        /// </summary>
        /// <remarks>
        /// This operation deletes a Publisher. A Publisher cannot be deleted when it has
        /// any attached DataWriter objects. When the operation is called on a Publisher
        /// with DataWriter objects, the operation returns PreconditionNotMet. 
        /// When the operation is called on a different DomainParticipant, as used when the Publisher 
        /// was created, the operation has no effect and returns PreconditionNotMet.
        /// </remarks>
        /// <param name="p">The Publisher to be deleted.</param>
        /// <returns>Rerurn values are:
        /// <list type="bullet">
        /// <item>Ok - The Publisher is deleted.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>BadParameter - The parameter p is not a valid IPublisher.</item>
        /// <item>AlreadyDeleted - The DomainParticipant has already been deleted.</item>
        /// <item>OutOfResources - the DDS has ran out of resources to complete this operation. </item>
        /// <item>PreconditionNotMet - the operation is called on a different DomainParticipant, 
        /// as used when the Publisher was created, or the Publisher contains one or more DataWriter objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeletePublisher(IPublisher p);
        /// <summary>
        /// This method creates a Subscriber. It behaves similarly to the most complete operation, but it 
        /// uses default values for the ommitted parameters. These are the default Qos for Qos parameters, null
        /// for listeners with 0 mask for listener parameters.
        /// </summary>
        /// <returns>The created Subscriber.</returns>
        ISubscriber CreateSubscriber();
        /// <summary>
        /// This method creates a Subscriber. It behaves similarly to the most complete operation, but it 
        /// uses default values for the ommitted parameters. These are the default Qos for Qos parameters, null
        /// for listeners with 0 mask for listener parameters.
        /// </summary>
        /// <param name="listener"></param>
        /// <param name="mask"></param>
        /// <returns>The created Subscriber.</returns>
        ISubscriber CreateSubscriber(
                ISubscriberListener listener, 
                StatusKind mask);
        /// <summary>
        /// This method creates a Subscriber. It behaves similarly to the most complete operation, but it 
        /// uses default values for the ommitted parameters. These are the default Qos for Qos parameters, null
        /// for listeners with 0 mask for listener parameters.
        /// </summary>
        /// <param name="qos"></param>
        /// <returns>The created Subscriber.</returns>
        ISubscriber CreateSubscriber(SubscriberQos qos);
        /// <summary>
        /// This operation creates a Subscriber with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified SubscriberListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a Subscriber with the desired QosPolicy settings and if
        /// applicable, attaches the optionally specified SubscriberListener to it. When the
        /// SubscriberListener is not applicable, a null SubscriberListener must be supplied
        /// instead. To delete the Subscriber the operation DeleteSubscriber or
        /// DeleteContainedEntities must be used.
        /// In case the specified QosPolicy settings are not consistent, no Subscriber is
        /// created and a null Subscriber is returned.
        /// </remarks>
        /// <param name="qos">a collection of QosPolicy settings for the new Subscriber. 
        /// In case these settings are not self consistent, no Subscriber is created.</param>
        /// <param name="listener">a pointer to the SubscriberListener instance which will be attached 
        /// to the new Subscriber. It is permitted to use null as the value of the listener: this
        /// behaves as a SubscriberListener whose operations perform no action.</param>
        /// <param name="mask">a bit-mask in which each bit enables the invocation of the SubscriberListener 
        /// for a certain status.</param>
        /// <returns>The newly created Subscriber.In case of an error a null Subscriber is returned.</returns>
        ISubscriber CreateSubscriber(
                SubscriberQos qos,
                ISubscriberListener listener, 
                StatusKind mask);
        /// <summary>
        /// This operation deletes a Subscriber.
        /// </summary>
        /// <remarks>
        /// This operation deletes a Subscriber. A Subscriber cannot be deleted when it
        /// has any attached DataReader objects. When the operation is called on a
        /// Subscriber with DataReader objects, the operation returns
        /// PreconditionNotMet. When the operation is called on a different
        /// DomainParticipant, as used when the Subscriber was created, the operation
        /// has no effect and returns PreconditionNotMet.
        /// </remarks>
        /// <param name="s"> The subscriber to be deleted.</param>
        /// <returns>Rerurn values are:
        /// <list type="bullet">
        /// <item>Ok - The Subscriber is deleted.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>BadParameter - The parameter p is not a valid ISubscriber.</item>
        /// <item>AlreadyDeleted - The DomainParticipant has already been deleted.</item>
        /// <item>OutOfResources - the DDS has ran out of resources to complete this operation. </item>
        /// <item>PreconditionNotMet - the operation is called on a different DomainParticipant, 
        /// as used when the Subscriber was created, or the Publisher contains one or more DataReader objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteSubscriber(ISubscriber s);
        /// <summary>
        /// This property returns the built-in Subscriber associated with the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation returns the built-in Subscriber associated with the
        /// DomainParticipant. Each DomainParticipant contains several built-in
        /// Topic objects. The built-in Subscriber contains the corresponding DataReader
        /// objects to access them. All these DataReader objects belong to a single built-in
        /// Subscriber. Note that there is exactly one built-in Subscriber associated with
        /// each DomainParticipant.
        /// </remarks>
        /// <returns>The built-in Subscriber associated with the DomainParticipant.</returns>
        ISubscriber BuiltInSubscriber { get; }
        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type, 
        /// with the desired QosPolicy settings and if applicable, attaches the optionally specified 
        /// TopicListener to it. It behaves very similarly to the most complete operation but it uses default 
        /// values for the omitted parameters. For QosPolicy parameters the default QosPolicy settings, null for listeners
        /// and 0 as mask for listener parameters.
        /// </summary>
        /// <param name="topicName"></param>
        /// <param name="typeName"></param>
        /// <returns>The new or existing ITopic.</returns>
        ITopic CreateTopic(string topicName, string typeName);
        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type, 
        /// with the desired QosPolicy settings and if applicable, attaches the optionally specified 
        /// TopicListener to it. It behaves very similarly to the most complete operation but it uses default 
        /// values for the omitted parameters. For QosPolicy parameters the default QosPolicy settings, null for listeners
        /// and 0 as mask for listener parameters.
        /// </summary>
        /// <param name="topicName"></param>
        /// <param name="typeName"></param>
        /// <param name="listener"></param>
        /// <param name="mask"></param>
        /// <returns>The new or existing ITopic.</returns>
        ITopic CreateTopic(
                string topicName, 
                string typeName,
                ITopicListener listener, 
                StatusKind mask);
        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type, 
        /// with the desired QosPolicy settings and if applicable, attaches the optionally specified 
        /// TopicListener to it. It behaves very similarly to the most complete operation but it uses default 
        /// values for the omitted parameters. For QosPolicy parameters the default QosPolicy settings, null for listeners
        /// and 0 as mask for listener parameters.
        /// </summary>
        /// <param name="topicName"></param>
        /// <param name="typeName"></param>
        /// <param name="qos"></param>
        /// <returns>The new or existing ITopic.</returns>
        ITopic CreateTopic(string topicName, string typeName, TopicQos qos);
        /// <summary>
        /// This operation returns a new or existing ITopic for the given name for a specific type, 
        /// with the desired QosPolicy settings and if applicable, attaches the optionally specified 
        /// TopicListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a reference to a new or existing Topic under the given name,
        /// for a specific type, with the desired QosPolicy settings and if applicable, attaches
        /// the optionally specified TopicListener to it. When the TopicListener is not
        /// applicable, a null listener must be supplied instead. In case the specified
        /// QosPolicy settings are not consistent, no Topic is created and a null ITopic is
        /// returned. To delete the Topic the operation DeleteTopic or DeleteContainedEntities must be used.
        /// </remarks>
        /// <param name="topicName">the name of the Topic to be created. A new Topic will only be created, 
        /// when no Topic, with the same name, is found within the DomainParticipant.</param>
        /// <param name="typeName">a local alias of the data type, which must have been registered 
        /// before creating the Topic.</param>
        /// <param name="qos">a collection of QosPolicy settings for the new Topic. 
        /// In case these settings are not self consistent, no Topic is created.</param>
        /// <param name="listener">the TopicListener instance which will be attached to the new Topic. 
        /// It is permitted to use null as the value of the listener: this behaves as a TopicListener
        /// whose operations perform no action.</param>
        /// <param name="mask"></param>
        /// <returns></returns>
        ITopic CreateTopic(
                string topicName, 
                string typeName, 
                TopicQos qos,
                ITopicListener listener, 
                StatusKind mask);
        /// <summary>
        /// This operation deletes a Topic
        /// </summary>
        /// <remarks>
        /// This operation deletes a Topic. A Topic cannot be deleted when there are any
        /// DataReader, DataWriter, ContentFilteredTopic or MultiTopic objects,
        /// which are using the Topic. When the operation is called on a Topic referenced by
        /// any of these objects, the operation returns PreconditionNotMet.
        /// When the operation is called on a different DomainParticipant, as used when
        /// the Topic was created, the operation has no effect and returns PreconditionNotMet.
        /// </remarks>
        /// <param name="topic">The Topic which is to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The Topic is deleted.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>BadParameter - The parameter topic is not a valid ITopic.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>PreconditionNotMet - The operation is called on a different DomainParticipant, as used when the 
        /// Topic was created, or the Topic is still referenced by other objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteTopic(ITopic topic);
        /// <summary>
        /// This operation gives access to an existing (or ready to exist) enabled Topic, based on its topicName.
        /// </summary>
        /// <remarks>
        /// This operation gives access to an existing Topic, based on its topic_name. The
        /// operation takes as arguments the topic_name of the Topic and a timeout.
        /// If a Topic of the same topic_name already exists, it gives access to this Topic.
        /// Otherwise it waits (blocks the caller) until another mechanism creates it. This other
        /// mechanism can be another thread, a configuration tool, or some other Data
        /// Distribution Service utility. If after the specified timeout the Topic can still not be
        /// found, the caller gets unblocked and the NULL pointer is returned.
        /// A Topic obtained by means of find_topic, must also be deleted by means of
        /// delete_topic so that the local resources can be released. If a Topic is obtained
        /// multiple times it must also be deleted that same number of times using
        /// delete_topic or calling delete_contained_entities once to delete all the proxies.
        /// A Topic that is obtained by means of find_topic in a specific DomainParticipant 
        /// can only be used to create DataReaders and DataWriters in that DomainParticipant if its 
        /// corresponding TypeSupport has been registered to that same DomainParticipant.
        /// </remarks>
        /// <param name="topicName">The name of te Topic that the application wants access to.</param>
        /// <param name="timeout">The maximum duration to block for the DomainParticipant FindTopic, 
        /// after which the application thread is unblocked. The special constant Duration Infinite can be used 
        /// when the maximum waiting time does not need to be bounded.</param>
        /// <returns>The Topic that has been found. If an error occurs the operation returns a Topic with a null value. </returns>
        ITopic FindTopic(string topicName, Duration timeout);
        /// <summary>
        /// This operation gives access to a locally-created TopicDescription, with a matching name.
        /// </summary>
        /// <remarks>
        /// The operation lookup_topicdescription gives access to a locally-created
        /// TopicDescription, based on its name. The operation takes as argument the name
        /// of the TopicDescription.
        /// If one or more local TopicDescription proxies of the same name already exist, 
        /// a pointer to one of the already existing local proxies is returned: LookupTopicDescription 
        /// will never create a new local proxy. That means that the proxy that is returned does not need 
        /// to be deleted separately from its original. When no local proxy exists, it returns null. 
        /// The operation never blocks. The operation LookupTopicDescription may be used to locate any
        /// locally-created Topic, ContentFilteredTopic and MultiTopic object.
        /// </remarks>
        /// <param name="name">The name of the TopicDescription to look for.</param>
        /// <returns>The TopicDescription it has found.If an error occurs the operation 
        /// returns a TopicDescription with a null value. </returns>
        ITopicDescription LookupTopicDescription(string name);
        /// <summary>
        /// This operation creates a ContentFilteredTopic for a DomainParticipant in order to allow 
        /// DataReaders to subscribe to a subset of the topic content.
        /// </summary>
        /// <remarks>
        /// This operation creates a ContentFilteredTopic for a DomainParticipant in
        /// order to allow DataReaders to subscribe to a subset of the topic content. The base
        /// topic, which is being filtered is defined by the parameter relatedTopic. The
        /// resulting ContentFilteredTopic only relates to the samples published under the
        /// relatedTopic, which have been filtered according to their content. The resulting
        /// ContentFilteredTopic only exists at the DataReader side and will never be
        /// published. The samples of the related_topic are filtered according to the SQL
        /// expression (which is a subset of SQL) as defined in the parameter
        /// filterExpression .The filter_expression may also contain parameters, which appear as
        /// %n tokens in the expression which must be set by the sequence of strings defined by the
        /// parameter expressionParameters. The number of values in
        /// expressionParameters must be equal or greater than the highest referenced
        /// %n token in the filter_expression (e.g. if %1 and %8 are used as parameter in
        /// the filter_expression, the expression_parameters should at least contain
        /// n+1 = 9 values).
        /// The filterExpression is a string that specifies the criteria to select the data
        /// samples of interest. In other words, it identifies the selection of data from the
        /// associated Topics. It is an SQL expression where the WHERE clause gives the
        /// content filter.
        /// </remarks>
        /// <param name="name">The name of the ContentFilteredTopic.</param>
        /// <param name="relatedTopic">The base topic on which the filtering will be applied. Therefore, 
        /// a filtered topic is based onn an existing Topic.</param>
        /// <param name="filterExpression">The SQL expression (subset of SQL), which defines the filtering.</param>
        /// <param name="expressionParameters">A sequence of strings with the parameter value used in the SQL expression
        /// (i.e., the number of %n tokens in the expression).The number of values in expressionParameters 
        /// must be equal or greater than the highest referenced %n token in the filterExpression 
        /// (e.g. if %1 and %8 are used as parameter in the filterExpression, the expressionParameters should at least
        /// contain n+1 = 9 values)</param>
        /// <returns>The newly created ContentFilteredTopic. In case of an error a ContentFilteredTopic with a null
        /// value is returned.</returns>
        IContentFilteredTopic CreateContentFilteredTopic(
                string name,
                ITopic relatedTopic,
                string filterExpression,
                params string[] expressionParameters);
        /// <summary>
        /// This operation deletes a ContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// This operation deletes a ContentFilteredTopic.
        /// The deletion of a ContentFilteredTopic is not allowed if there are any existing
        /// DataReader objects that are using the ContentFilteredTopic. If the
        /// DeleteContentFilteredTopic operation is called on a
        /// ContentFilteredTopic with existing DataReader objects attached to it, it will
        /// return PreconditionNotMet.
        /// The DeleteContentFilteredTopic operation must be called on the same
        /// DomainParticipant object used to create the ContentFilteredTopic. If
        /// DeleteContentFilteredTopic is called on a different DomainParticipant
        /// the operation will have no effect and it will return PreconditionNotMet.
        /// </remarks>
        /// <param name="aContentFilteredTopic">The ContentFilteredTopic to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The ContentFilteredTopic is deleted.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>BadParameter - The parameter aContentFilteredTopictopic is not a valid IContentFilteredTopic.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>PreconditionNotMet - The operation is called on a different DomainParticipant, as used when the 
        /// ContentFilteredTopic is being used by one or more DataReader objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteContentFilteredTopic(IContentFilteredTopic aContentFilteredTopic);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="typeName"></param>
        /// <param name="subscriptionExpression"></param>
        /// <param name="expressionParameters"></param>
        /// <returns></returns>
        IMultiTopic CreateMultiTopic(
                string name,
                string typeName,
                string subscriptionExpression,
                params string[] expressionParameters);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="multiTopic"></param>
        /// <returns></returns>
        ReturnCode DeleteMultiTopic(IMultiTopic multiTopic);
        /// <summary>
        /// This operation deletes all the Entity objects that were created on the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the Entity objects that were created on the
        /// DomainParticipant. In other words, it deletes all Publisher, Subscriber,
        /// Topic, ContentFilteredTopic and MultiTopic objects. Prior to deleting each
        /// contained Entity, this operation regressively calls the corresponding
        /// delete_contained_entities operation on each Entity (if applicable). In
        /// other words, all Entity objects in the Publisher and Subscriber are deleted,
        /// including the DataWriter and DataReader. Also the QueryCondition and
        /// ReadCondition objects contained by the DataReader are deleted.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The contained Entity objects are deleted and the application may delete the DomainParticipant.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode DeleteContainedEntities();
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a
        /// DomainParticipant. The parameter qos contains the QosPolicy settings which
        /// is checked for self-consistency.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned Ok).
        /// </remarks>
        /// <param name="qos">New set of QosPolicy settings for the DomainParticipant.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The new DomainParticipantQos is set.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode SetQos(DomainParticipantQos qos);
        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a
        /// DomainParticipant on which this operation is used. This
        /// DomainparticipantQos is stored at the location pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination DomainParticipantQos struct in which the 
        /// QosPolicy settings will be copied.</param>
        /// <returns>
        /// <list type="bullet">
        /// <item>Ok - The existing set of QoS policy values applied to this DomainParticipant 
        /// has successfully been copied into the specified DomainParticipantQos parameter.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode GetQos(ref DomainParticipantQos qos);
        /// <summary>
        /// This operation attaches a DomainParticipantListener to the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation attaches a DomainParticipantListener to the
        /// DomainParticipant. Only one DomainParticipantListener can be
        /// attached to each DomainParticipant. If a DomainParticipantListener was
        /// already attached, the operation will replace it with the new one. When a_listener
        /// is the NULL pointer, it represents a listener that is treated as a NOOP1 for all statuses
        /// activated in the bit mask.
        /// </remarks>
        /// <param name="listener">The DomainParticipantListener instance, which will be attached to the 
        /// DomainParticipant.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the DomainParticipantListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The DomainParticipantListener is attached.</item>
        /// <item>Error - An internal error has occured.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(IDomainParticipantListener listener, StatusKind mask);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        ReturnCode IgnoreParticipant(InstanceHandle handle);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        ReturnCode IgnoreTopic(InstanceHandle handle);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        ReturnCode IgnorePublication(InstanceHandle handle);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        ReturnCode IgnoreSubscription(InstanceHandle handle);
        /// <summary>
        /// This property returns the DomainId of the Domain to which this DomainParticipant is attached.
        /// </summary>
        DomainId DomainId { get; }
        /// <summary>
        /// This operation asserts the liveliness for the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation will manually assert the liveliness for the DomainParticipant.
        /// This way, the Data Distribution Service is informed that the DomainParticipant
        /// is still alive. This operation only needs to be used when the DomainParticipant
        /// contains DataWriters with the LivelinessQosPolicy set to
        /// MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, and it will only affect the
        /// liveliness of those DataWriters.
        /// Writing data via the write operation of a DataWriter will assert the liveliness on
        /// the DataWriter itself and its DomainParticipant. Therefore,
        /// AssertLiveliness is only needed when not writing regularly.
        /// The liveliness should be asserted by the application, depending on the
        /// LivelinessQosPolicy.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The liveliness of this DomainParticipant has successfully been asserted.</item>
        /// <item>Error - An internal error has occured.</item>       
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>NotEnabled - The DomainParticipant is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode AssertLiveliness();
        /// <summary>
        /// This operation sets the default PublisherQos of the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation sets the default PublisherQos of the DomainParticipant (that is the struct with 
        /// the QosPolicy settings) which is used for newly created Publisher objects, in case the constant 
        /// PUBLISHER_QOS_DEFAULT is used. The default PublisherQos is only used when the constant is supplied 
        /// as parameter qos to specify the PublisherQos in the CreatePublisher operation. The
        /// PublisherQos is always self consistent, because its policies do not depend on
        /// each other. This means this operation never returns the InconsistentPolicy. 
        /// The values set by this operation are returned by GetGefaultPublisherQos.
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings, which contains the new default QosPolicy 
        /// settings for the newly created Publishers.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The new default PublisherQos is set.</item>
        /// <item>Error - An internal error has occured.</item>       
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The parameter qos is not a valid PublisherQos.</item>
        /// <item>Unsupported - one or more of the selected QosPolicy values are currently not supported by openSplice.</item>        
        /// </list>
        /// </returns>
        ReturnCode SetDefaultPublisherQos(PublisherQos qos);
        /// <summary>
        /// This operation gets the struct with the default Publisher QosPolicy settings of the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation gets the struct with the default Publisher QosPolicy settings of
        /// the DomainParticipant (that is the PublisherQos) which is used for newly
        /// created Publisher objects, in case the constant PUBLISHER_QOS_DEFAULT is
        /// used. The default PublisherQos is only used when the constant is supplied as
        /// parameter qos to specify the PublisherQos in the CreatePublisher
        /// operation. The application must provide the PublisherQos struct in which the
        /// QosPolicy settings can be stored and pass the qos reference to the operation. The
        /// operation writes the default QosPolicy settings to the struct referenced to by qos.
        /// Any settings in the struct are overwritten.
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultPublisherQos, or, if the call was never made,
        /// the default values as specified for each QosPolicy setting
        /// </remarks>
        /// <param name="qos">A reference to the PublisherQos struct (provided by the application) in which 
        /// the default QosPolicy settings for the Publisher are written.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The default Publisher QosPolicy settings of this DomainParticipant have successfully been
        /// copied into the specified qos parameter.</item>
        /// <item>Error - An internal error has occured.</item>       
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultPublisherQos(ref PublisherQos qos);
        /// <summary>
        /// This operation sets the default SubscriberQos of the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation sets the default SubscriberQos of the DomainParticipant (that
        /// is the struct with the QosPolicy settings) which is used for newly created
        /// Subscriber objects, in case the constant SUBSCRIBER_QOS_DEFAULT is used.
        /// The default SubscriberQos is only used when the constant is supplied as
        /// parameter qos to specify the SubscriberQos in the CreateSubscriber
        /// operation. The SubscriberQos is always self consistent, because its policies do
        /// not depend on each other. This means this operation never returns the
        /// InconsistentPolicy. The values set by this operation are returned
        /// by GetDefaultSubscriberQos.
        /// </remarks>
        /// <param name="qos">A collection of QosPolicy settings, which contains the new default QosPolicy 
        /// settings for the newly created Subscribers.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The new default SubscriberQos is set.</item>
        /// <item>Error - An internal error has occured.</item>       
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The parameter qos is not a valid SubscriberQos.</item>
        /// <item>Unsupported - one or more of the selected QosPolicy values are currently not supported by openSplice.</item>        
        /// </list>
        /// </returns>
        ReturnCode SetDefaultSubscriberQos(SubscriberQos qos);
        /// <summary>
        /// This operation gets the struct with the default Subscriber QosPolicy settings of the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation gets the struct with the default Subscriber QosPolicy settings of
        /// the DomainParticipant (that is the SubscriberQos) which is used for newly
        /// created Subscriber objects, in case the constant SUBSCRIBER_QOS_DEFAULT is
        /// used. The default SubscriberQos is only used when the constant is supplied as
        /// parameter qos to specify the SubscriberQos in the create_subscriber
        /// operation. The application must provide the QoS struct in which the policy can be
        /// stored and pass the qos reference to the operation. The operation writes the default
        /// QosPolicy to the struct referenced to by qos. Any settings in the struct are
        /// overwritten.
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to set_default_subscriber_qos, or, if the call was never made,
        /// the default values as specified for each QosPolicy.
        /// </remarks>
        /// <param name="qos">a reference to the QosPolicy struct (provided by the application) in which the default 
        /// QosPolicy settings for the Subscriber is written</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The default Subscriber QosPolicy settings of this DomainParticipant have successfully 
        /// been copied into the specified SubscriberQos parameter.</item>
        /// <item>Error - An internal error has occured.</item>       
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultSubscriberQos(ref SubscriberQos qos);
        /// <summary>
        /// This operation sets the default TopicQos of the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation sets the default TopicQos of the DomainParticipant (that is the
        /// struct with the QosPolicy settings) which is used for newly created Topic objects,
        /// in case the constant TOPIC_QOS_DEFAULT is used. The default TopicQos is only
        /// used when the constant is supplied as parameter qos to specify the TopicQos in the
        /// CreateTopic operation. This operation checks if the TopicQos is self
        /// consistent. If it is not , the operation has no effect and returns
        /// Inconsistentpolicy. The values set by this operation are returned by GetGefaultTopicQos.
        /// </remarks>
        /// <param name="qos">a collection of QosPolicy settings, which contains the new default QosPolicy settings 
        /// for the newly created Topics.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The new default TopicQos is set.</item>
        /// <item>Error - An internal error has occured.</item>   
        /// <item>BadParameter - The parameter qos is not a valid TopicQos.</item>
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>Unsupported - one or more of the supported QosPolicy values are currently not supported
        /// by OpenSplice.</item>
        /// <item>InconsistentPolicy - the parameter qos contains conflicting QosPolicy settings, e.g a history depth that is 
        /// higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultTopicQos(TopicQos qos);
        /// <summary>
        /// This operation gets the struct with the default Topic QosPolicy settings of the DomainParticipant.
        /// </summary>
        /// <remarks>
        /// This operation gets the struct with the default Topic QosPolicy settings of the
        /// DomainParticipant (that is the TopicQos) which is used for newly created
        /// Topic objects, in case the constant TOPIC_QOS_DEFAULT is used. The default
        /// TopicQos is only used when the constant is supplied as parameter qos to specify
        /// the TopicQos in the CreateTopic operation. The application must provide the
        /// QoS struct in which the policy can be stored and pass the qos reference to the
        /// operation. The operation writes the default QosPolicy to the struct referenced to
        /// by qos. Any settings in the struct are overwritten.
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultTopicQos, or, if the call was never made, the
        /// default values as specified for each QosPolicy.
        /// </remarks>
        /// <param name="qos">A reference to the QosPolicy struct (provided by the application) in which the 
        /// default QosPolicy settings for the Topic is written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The default Topic QosPolicy settings of this DomainParticipant have successfully been copied 
        /// into the specified TopicQos parameter.</item>
        /// <item>Error - An internal error has occured.</item>       
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultTopicQos(ref TopicQos qos);
        /// <summary>
        /// This operation checks whether or not the given Entity represented by handle is created by the 
        /// DomainParticipant or any of its contained entities.
        /// </summary>
        /// <remarks>This operation checks whether or not the given Entity represented by handle
        /// is created by the DomainParticipant itself (TopicDescription, Publisher
        /// or Subscriber) or created by any of its contained entities (DataReader,
        /// ReadCondition, QueryCondition, DataWriter, etc.).
        /// Return value is TRUE if a_handle represents an Entity that is created by the
        /// DomainParticipant or any of its contained Entities. Otherwise the return
        /// value is FALSE</remarks>
        /// <param name="handle">An Entity in the Data Distribution Service.</param>
        /// <returns>true if handle represents an Entity that is created by the DomainParticipant or any of its
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
        /// <item>Ok - The value of the current time has been copied into current_time.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>BadParameter - The parameter current_time is not a valid reference.</item>      
        /// <item>AlreadyDeleted - The DomaiParticipant has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>NotEnabled - The DomainParticipant is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode GetCurrentTime(out Time currentTime);
        
        [Obsolete("This is a proprietary OpenSplice DDS function that is not supposed to be used by applications.")]
        ITypeSupport GetTypeSupport(string registeredName);
        [Obsolete("This is a proprietary OpenSplice DDS function that is not supposed to be used by applications.")]
        ITypeSupport LookupTypeSupport(string registeredTypeName);
        
        /// <summary>
		/// This operation retrieves the list of DomainParticipants that have been discovered in the domain.
		/// </summary>
		/// <remarks>
		///This operation retrieves the list of DomainParticipants that have been discovered in the domain and that the application
		///has not indicated should be ignored by means of the DomainParticipant ignore_participant operation.
		///The participant_handles sequence and its buffer may be pre-allocated by the
		///application and therefore must either be re-used in a subsequent invocation of the
		///get_discovered_participants operation or be released by
		///calling free on the returned participant_handles. If the pre-allocated
		///sequence is not big enough to hold the number of associated participants, the
		///sequence will automatically be (re-)allocated to fit the required size.
		///The handles returned in the participant_handles sequence are the ones that
		///are used by the DDS implementation to locally identify the corresponding matched
		///Participant entities. You can access more detailed information about a particular
		///participant by passing its participant_handle to the get_discovered_participant_data operation.
		/// </remarks>
		/// <param name="participantHandles"></param>
		/// <returns>Return values are:
		/// <list type="bullet">
		/// <item>Ok - the list of associated participants has successfully been obtained.</item>
		/// <item>Error - an internal error has occurred.</item>
		/// <item>AlreadyDeleted - the DomainParticipant has already been deleted</item>
		/// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
		/// <item>Unsupported - OpenSplice is configured not to maintain the information about associated participants</item>
		/// <item>NotEnabled - the DomainParticipant is not enabled.</item>
		/// <item>IllegalOperation- the operation is invoked on an inappropriate object.</item>
		/// </list>
		/// </returns>
        ReturnCode GetDiscoveredParticipants (ref InstanceHandle[] participantHandles);
        
        /// <summary>
		/// This operation retrieves information on a DomainParticipant that has been discovered on the network.
		/// </summary>
		/// <remarks>
		///This operation retrieves information on a DomainParticipant that has been discovered on the network. The participant
		///must be in the same domain as the participant on which this operation is invoked and must not have been ignored by
		///means of the DomainParticipant ignore_participant operation.
		///The partition_handle must correspond to a partition currently
		///associated with the DomainParticipant, otherwise the operation will fail and return
		///Error. The operation get_discovered_participant_data
		///can be used to find more detailed information about a particular participant that is found with the 
		///get_discovered_participants operation.
		/// </remarks>
        /// <param name="participantData"></param>
        /// <param name="participantHandle"></param>
		/// <returns>Return values are:
		/// <list type="bullet">
		/// <item>Ok - the information on the specified participant has been successfully retrieved</item>
		/// <item>Error - an internal error has occurred.</item>
		/// <item>AlreadyDeleted - the DomainParticipant has already been deleted</item>
		/// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
		/// <item>Unsupported - OpenSplice is configured not to maintain the information about associated participants</item>
		/// <item>NotEnabled - the DomainParticipant is not enabled.</item>
		/// <item>IllegalOperation- the operation is invoked on an inappropriate object.
		/// </list>
		/// </returns>
        ReturnCode GetDiscoveredParticipantData (ref ParticipantBuiltinTopicData data, InstanceHandle handle);
        /// <summary>
		/// This operation retrieves the list of Topics that have been discovered in the domain
		/// </summary>
		/// <remarks>
		///This operation retrieves the list of Topics that have been discovered in the domain and that the application has not
		///indicated should be ignored by means of the DomainParticipant ignore_topic operation.
		///The topic_handles sequence and its buffer may be pre-allocated by the
		///application and therefore must either be re-used in a subsequent invocation of the
		///get_discovered_topics operation or be released by
		///calling free on the returned topic_handles. If the pre-allocated
		///sequence is not big enough to hold the number of associated participants, the
		///sequence will automatically be (re-)allocated to fit the required size.
		///The handles returned in the topic_handles sequence are the ones that
		///are used by the DDS implementation to locally identify the corresponding matched
		///Topic entities. You can access more detailed information about a particular
		///topic by passing its topic_handle to the get_discovered_topic_data operation.
		/// </remarks>
		/// <param name="topicHandles"></param>
		/// <returns>Return values are:
		/// <list type="bullet">
		/// <item>Ok - the list of associated topic has successfully been obtained.</item>
		/// <item>Error - an internal error has occurred.</item>
		/// <item>AlreadyDeleted - the DomainParticipant has already been deleted</item>
		/// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
		/// <item>Unsupported - OpenSplice is configured not to maintain the information about associated topics</item>
		/// <item>NotEnabled - the DomainParticipant is not enabled.</item>
		/// <item>IllegalOperation- the operation is invoked on an inappropriate object.
		/// </list>
		/// </returns>
        ReturnCode GetDiscoveredTopics (ref InstanceHandle[] topicHandles);
        
        /// <summary>
		/// This operation retrieves information on a Topic that has been discovered on the network.
		/// </summary>
		/// <remarks>
		///This operation retrieves information on a Topic that has been discovered on the network. The topic must have been
		///created by a participant in the same domain as the participant on which this operation is invoked and must not have been
		///ignored by means of the DomainParticipant ignore_topic operation.
		///The topic_handle must correspond to a topic currently
		///associated with the DomainParticipant, otherwise the operation will fail and return Error. 
		///The operation get_discovered_topic_data can be used to find more detailed information about a particular topic that is found with the 
		///get_discovered_topics operation.
		/// </remarks>
		/// <param name="topicData"></param>
		/// <param name="topicHandle"></param>
		/// <returns>Return values are:
		/// <list type="bullet">
		/// <item>Ok - the information on the specified topic has been successfully retrieved</item>
		/// <item>Error - an internal error has occurred.</item>
		/// <item>AlreadyDeleted - the DomainParticipant has already been deleted</item>
		/// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
		/// <item>Unsupported - OpenSplice is configured not to maintain the information about associated topics</item>
		/// <item>NotEnabled - the DomainParticipant is not enabled.</item>
		/// <item>IllegalOperation- the operation is invoked on an inappropriate object.
		/// </list>
		/// </returns>
        ReturnCode GetDiscoveredTopicData (ref TopicBuiltinTopicData data, InstanceHandle handle);
        
        
    }

    public interface ITypeSupport
    {
        ReturnCode RegisterType(IDomainParticipant domain, string typeName);
        string TypeName { get; }
        string[] Description { get; }
        string KeyList { get; }
    }

    // ----------------------------------------------------------------------
    // Topics
    // ----------------------------------------------------------------------
    /// <summary>
    /// This class is an abstract class . It is the base class for Topic,ContentFilteredTopic and MultiTopic.
    /// The TopicDescription attribute type_name defines an unique data type that is
    /// made available to the Data Distribution Service via the TypeSupport.
    /// TopicDescription has also a name that allows it to be retrieved locally.
    /// </summary>
    public interface ITopicDescription
    {
        /// <summary>
        /// This property returns the registered name of the data type associated with the TopicDescription.
        /// </summary>
        /// <returns>The name of the data type of the TopicDescription.</returns>
        string TypeName { get; }
        /// <summary>
        /// This property returns the name used to create the TopicDescription.
        /// </summary>
        /// <returns>The name of the TopicDescription.</returns>
        string Name { get; }
        /// <summary>
        /// This property returns the DomainParticipant associated with the TopicDescription or NULL.
        /// </summary>
        /// <remarks>
        /// This operation returns the DomainParticipant associated with the
        /// TopicDescription. Note that there is exactly one DomainParticipant
        /// associated with each TopicDescription. When the TopicDescription was
        /// already deleted (there is no associated DomainParticipant any more), NULL
        /// is returned.
        /// </remarks>
        /// <returns>The DomainParticipant associated with the TopicDescription or NULL.</returns>
        IDomainParticipant Participant { get; }
    }

    /// <summary>
    /// Topic is the most basic description of the data to be published and subscribed.
    /// </summary>
    /// <remarks>
    /// A Topic is identified by its name, which must be unique in the whole Domain. In
    /// addition (by virtue of extending TopicDescription) it fully identifies the type of
    /// data that can be communicated when publishing or subscribing to the Topic.
    /// Topic is the only TopicDescription that can be used for publications and
    /// therefore a specialized DataWriter is associated to the Topic.
    /// </remarks>
    public interface ITopic : IEntity, ITopicDescription
    {
        /// <summary>
        /// This operation obtains the InconsistentTopicStatus of the Topic.
        /// </summary>
        /// <remarks>
        /// This operation obtains the InconsistentTopicStatus of the Topic. The 
        /// InconsistentTopicStatus can also be monitored using a TopicListener or
        /// by using the associated StatusCondition.
        /// </remarks>
        /// <param name="aStatus">the contents of the InconsistentTopicStatus struct of the Topic will be 
        /// copied into the location specified by status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The current InconsistentTopicStatus of this Topic has successfully been copied 
        /// into the specified aStatus parameter.</item>
        /// <item>Error - An internal error has occured.</item>         
        /// <item>AlreadyDeleted - The Topic has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetInconsistentTopicStatus(ref InconsistentTopicStatus aStatus);
        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a Topic.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a Topic on which
        /// this operation is used. This TopicQos is stored at the location pointed to by the qos
        /// parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination TopicQos struct in which the QosPolicy 
        /// settings will be copied.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The existing set of QoS policy values applied to this Topic has successfully 
        /// been copied into the specified TopicQos parameter.</item>
        /// <item>Error - An internal error has occured.</item>         
        /// <item>AlreadyDeleted - The Topic has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref TopicQos qos);
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a Topic.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a Topic. The
        /// parameter qos contains the struct with the QosPolicy settings which is checked
        /// for self-consistency and mutability. When the application tries to change a
        /// QosPolicy setting for an enabled Topic, which can only be set before the Topic
        /// is enabled, the operation will fail and an ImmutablePolicy is returned.
        /// In other words, the application must provide the currently set QosPolicy settings
        /// in case of the immutable QosPolicy settings. Only the mutable QosPolicy
        /// settings can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and an InconsistentPolicy is returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned OK).
        /// </remarks>
        /// <param name="qos">The new set of QosPolicy settings for a Topic.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The new TopicQos is set.</item>
        /// <item>Error - An internal error has occured.</item>         
        /// <item>AlreadyDeleted - The Topic has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The parameter qos is not a valid topicQos. It contains a QosPolicy 
        /// setting with an invalid Duration value or an enum value that is outside its legal boundaries.</item>
        /// <item>Unsupported - one or more of the selected QosPolicy values are currently not supported by OpenSplice.</item>
        /// <item>ImmutablePolicy - The parameter qos contains an immutable QosPolicy setting with a different 
        /// value than set during enabling of the Topic.</item>
        /// <item>InconsistentPolicy - The parameter qos contains conflicting QosPolicy settings,e.g. a history depth
        ///  that is higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(TopicQos qos);
        /// <summary>
        /// This operation attaches a TopicListener to the Topic.
        /// </summary>
        /// <remarks>
        /// This operation attaches a TopicListener to the Topic. Only one
        /// TopicListener can be attached to each Topic. If a TopicListener was already
        /// attached, the operation will replace it with the new one. When listener is NULL , 
        /// it represents a listener that is treated as a NOOP for all statuses 
        /// activated in the bit mask.
        /// </remarks>
        /// <param name="listener">The listener to be attached to the Topic.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the TopicListener for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The TopicListener is attached.</item>
        /// <item>Error - An internal error has occured.</item>         
        /// <item>AlreadyDeleted - The Topic has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(ITopicListener listener, StatusKind mask);
    }

    /// <summary>
    /// ContentFilteredTopic is a specialization of TopicDescription that allows for content based subscriptions. 
    /// </summary>
    /// <remarks>
    /// ContentFilteredTopic describes a more sophisticated subscription that indicates the Subscriber 
    /// does not necessarily want to see all values of each instance published under the Topic. 
    /// Rather, it only wants to see the values whose contents satisfy certain criteria. 
    /// Therefore this class must be used to request content-based subscriptions. The selection 
    /// of the content is done using the SQL based filter with parameters to adapt the filter clause.
    /// </remarks>
    public interface IContentFilteredTopic : ITopicDescription
    {
        /// <summary>
        /// This operation returns the filter_expression associated with the ContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// This operation returns the filter_expression associated with the ContentFilteredTopic. 
        /// That is, the expression specified when the ContentFilteredTopic was created.
        /// </remarks>
        /// <returns>The string that specifies the criteria to select the data samples of interest. 
        /// It is similar to the WHERE clause of an SQL expression.</returns>
        string GetFilterExpression();
        /// <summary>
        /// This operation obtains the expression parameters associated with the ContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// This operation obtains the expression parameters associated with the
        /// ContentFilteredTopic. That is, the parameters specified on the last successful
        /// call to SetExpressionParameters, or if SetExpressionParameters
        /// was never called, the parameters specified when the ContentFilteredTopic was
        /// created.
        /// The resulting reference holds a sequence of strings with the parameters used in the
        /// SQL expression (i.e., the %n tokens in the expression). The number of parameters in
        /// the result sequence will exactly match the number of %n tokens in the filter
        /// expression associated with the ContentFilteredTopic.
        /// </remarks>
        /// <param name="expressionParameters">A reference to a sequence of strings that will be used 
        /// to store the parameters used in the SQL expression.</param>
        /// <returns>Return values are: 
        /// <list type="bullet">
        /// <item>Ok - The existing set of expression parameters applied to this ContentFilteredTopic 
        /// has successfully been copied into the specified expressionParameters parameter.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The ContentFilteredTopic has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetExpressionParameters(ref string[] expressionParameters);
        /// <summary>
        /// This operation changes the expression parameters associated with the ContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// This operation changes the expression parameters associated with the
        /// ContentFilteredTopic. The parameter expressionParameters is a handle
        /// to a sequence of strings with the parameters used in the SQL expression. The
        /// number of values in expressionParameters must be equal or greater than the
        /// highest referenced %n token in the filter_expression (for example, if %1 and
        /// %8 are used as parameter in the filter_expression, the
        /// expressionParameters should at least contain n+1 = 9 values). This is the
        /// filter expression specified when the ContentFilteredTopic was created.
        /// </remarks>
        /// <param name="expressionParameters">A sequence of strings with the parameters used in the SQL expression.
        /// The number of values in expressionParameters must be equal or greater than the highest referenced 
        /// %n token in the subscriptionExpression.</param>
        /// <returns>The possible return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The new expression parameters are set.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The ContentFilteredTopic has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The number of parameters in expression_parameters does not match the 
        /// number of tokens in the expression for this ContentFilteredTopic or one of the parameters
        /// is an illegal parameter.</item>          
        /// </list>
        /// </returns>
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
        /// <summary>
        /// This property returns the Topic associated with the ContentFilteredTopic.
        /// </summary>
        /// <remarks>
        /// This operation returns the Topic associated with the ContentFilteredTopic.
        /// That is, the Topic specified when the ContentFilteredTopic was created. This
        /// Topic is the base topic on which the filtering will be applied.
        /// </remarks>
        ITopic RelatedTopic { get; }
    }

    /// <summary>
    /// MultiTopic is a specialization of TopicDescription that allows subscriptions to combine, filter 
    /// and/or rearrange data coming from several Topics.
    /// </summary>
    /// <remarks>
    /// MultiTopic allows a more sophisticated subscription that can select and combine
    /// data received from multiple Topics into a single data type (specified by the
    /// inherited type_name). The data will then be filtered (selection) and possibly
    /// re-arranged (aggregation and/or projection) according to an SQL expression with
    /// parameters to adapt the filter clause.
    /// </remarks>
    public interface IMultiTopic : ITopicDescription
    {
        /// <summary>
        /// This property is not yet implemented. It is scheduled for a future release.
        /// </summary>
        string SubscriptionExpression { get; }
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="expressionParameters"></param>
        /// <returns></returns>
        ReturnCode GetExpressionParameters(ref string[] expressionParameters);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="expressionParameters"></param>
        /// <returns></returns>
        ReturnCode SetExpressionParameters(params string[] expressionParameters);
    }

    // ----------------------------------------------------------------------  
    // Publisher/Subscriber, DataWriter/DataReader
    // ---------------------------------------------------------------------- 
    /// <summary>
    /// The Publisher acts on behalf of one or more DataWriter objects that belong to
    /// it. When it is informed of a change to the data associated with one of its
    /// DataWriter objects, it decides when it is appropriate to actually process the
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
        /// <param name="topic"></param>
        /// <returns></returns>
        IDataWriter CreateDataWriter(ITopic topic);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <param name="topic"></param>
        /// <param name="listener"></param>
        /// <param name="mask"></param>
        /// <returns></returns>
        IDataWriter CreateDataWriter(
                ITopic topic,
                IDataWriterListener listener, StatusKind mask);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <param name="topic"></param>
        /// <param name="qos"></param>
        /// <returns></returns>
        IDataWriter CreateDataWriter(ITopic topic, DataWriterQos qos);
        /// <summary>
        /// This operation creates a DataWriter with the desired DataWriterQos, for the
        /// desired Topic and attaches the optionally specified DataWriterListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a DataWriter with the desired DataWriterQos, for the
        /// desired Topic and attaches the optionally specified DataWriterListener to it.
        /// The returned DataWriter is attached (and belongs) to the Publisher on which
        /// this operation is being called. To delete the DataWriter the operation
        /// DeleteDatawriter or DeleteContained_entities must be used.
        /// </remarks>
        /// <param name="topic">The topic for which the DataWriter is created.</param>
        /// <param name="qos">The DataWriterQos for the new DataWriter. 
        /// In case these settings are not self consistent, no DataWriter is created.</param>
        /// <param name="listener">The DataWriterListener instance which will be attached to the new
        /// DataWriter. It is permitted to use NULL as the value of the listener: this
        /// behaves as a DataWriterListener whose operations perform no action.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the DataWriterListener  
        /// for a certain status.</param>
        /// <returns>The newly created DataWriter. In case of an error a null value DataWriter is returned.</returns>
        IDataWriter CreateDataWriter(
                ITopic topic, 
                DataWriterQos qos,
                IDataWriterListener listener, 
                StatusKind mask);
        /// <summary>
        /// This operation deletes a DataWriter that belongs to the Publisher.
        /// </summary>
        /// <remarks>
        /// This operation deletes a DataWriter that belongs to the Publisher. When the
        /// operation is called on a different Publisher, as used when the DataWriter was
        /// created, the operation has no effect and returns
        /// PreconditionNotMet. The deletion of the DataWriter will automatically unregister all instances. 
        /// Depending on the settings of WriterDataLifecycleQosPolicy, the deletion of the DataWriter may also
        /// dispose of all instances.
        /// </remarks>
        /// <param name="dataWriter">The DataWriter which is to be deleted.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The DataWriter is deleted.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The Publisher has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The parameter dataWriter is not a valid IDataWriter reference.</item>      
        /// <item>PreconditionNotMet - The operation is called on a different Publisher, as used when the 
        /// DataWriter was created.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteDataWriter(IDataWriter dataWriter);
        /// <summary>
        /// This operation returns a previously created DataWriter belonging to the Publisher 
        /// which is attached to a Topic with the matching topicName.
        /// </summary>
        /// <remarks>
        /// This operation returns a previously created DataWriter belonging to the
        /// Publisher which is attached to a Topic with the matching topicName. When
        /// multiple DataWriter objects (which satisfy the same condition) exist, this
        /// operation will return one of them. It is not specified which one.
        /// </remarks>
        /// <param name="topicName">the name of the Topic, which is attached to the DataWriter to look for.</param>
        /// <returns>The DataWriter found. When no such DataWriter is found, a DataWriter of null value is returned.</returns>
        IDataWriter LookupDataWriter(string topicName);
        /// <summary>
        /// This operation deletes all the DataWriter objects that were created by means of 
        /// one of the CreateDataWriter operations on the Publisher.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the DataWriter objects that were created by means of
        /// one of the CreateDatawriter operations on the Publisher. In other words, it
        /// deletes all contained DataWriter objects.
        /// </remarks>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The contained Entity objects are deleted and the application may delete the Publisher.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The Publisher has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteContainedEntities();
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a Publisher.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a Publisher.
        /// The parameter qos contains the QosPolicy settings which is checked for
        /// self-consistency and mutability. When the application tries to change a QosPolicy
        /// setting for an enabled Publisher, which can only be set before the Publisher is
        /// enabled, the operation will fail and a ImmutablePolicy is returned. In
        /// other words, the application must provide the currently set QosPolicy settings in
        /// case of the immutable QosPolicy settings. Only the mutable QosPolicy settings
        /// can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and a InconsistentPolicy is
        /// returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned OK).
        /// </remarks>
        /// <param name="qos">The new set of QosPolicy settings for the Publisher.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The new PublisherQos is set.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The Publisher has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The parameter qos is not a valid PublisherQos. It contains a 
        /// QosPolicy setting with an enum value that is outside its legal boundaries.</item> 
        /// <item>Unsupported - One ore more of the selected QosPolicy values are currently not 
        /// supported by OpenSplice.</item>
        /// <item>ImmutablePolicy - The parameter qos contains an immutable Qospolicy setting with a 
        /// different value than set during enabling of the Publisher.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(PublisherQos qos);
        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a Publisher.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a Publisher on
        /// which this operation is used. This PublisherQos is stored at the location pointed
        /// to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination PublisherQos struct in which the 
        /// QosPolicy settings will be copied.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The existing set of QoS policy values applied to this Publisher has successfully
        /// been copied into the specified PublisherQos parameter.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The Publisher has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref PublisherQos qos);
        /// <summary>
        /// This operation attaches a PublisherListener to the Publisher.
        /// </summary>
        /// <remarks>
        /// This operation attaches a PublisherListener to the Publisher. Only one
        /// PublisherListener can be attached to each Publisher. If a PublisherListener 
        /// was already attached, the operation will replace it with the new one. 
        /// When listener is NULL , it represents a listener that is
        /// treated as a NOOP for all statuses activated in the bit mask.
        /// </remarks>
        /// <param name="listener">The PublisherListener instance which will be attached to the publisher.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the PublisherListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The PublisherListener is attached.</item>
        /// <item>Error - An internal error has occured.</item> 
        /// <item>AlreadyDeleted - The Publisher has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(IPublisherListener listener, StatusKind mask);
        /// <summary>
        /// This operation will suspend the dissemination of the publications by all contained DataWriter objects.
        /// </summary>
        /// <remarks>
        /// This operation suspends the publication of all DataWriter objects contained by
        /// this Publisher. The data written, disposed or unregistered by a DataWriter is
        /// stored in the history buffer of the DataWriter and therefore, depending on its QoS
        /// settings, the following operations may block (see the operation descriptions for
        /// more information):
        /// <list type="bullet">
        /// <item>DDS.DataWriter.dispose</item>
        /// <item>DDS.DataWriter.dispose_w_timestamp</item>
        /// <item>DDS.DataWriter.write</item>
        /// <item>DDS.DataWriter.write_w_timestamp</item>
        /// <item>DDS.DataWriter.writedispose</item>
        /// <item>DDS.DataWriter.writedispose_w_timestamp</item>
        /// <item>DDS.DataWriter.unregister_instance</item>
        /// <item>DDS.DataWriter.unregister_instance_w_timestamp</item>
        /// </list>
        /// Subsequent calls to this operation have no effect. When the Publisher is deleted
        /// before ResumePublications is called, all suspended updates are discarded.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - the Publisher has been suspended</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the Publisher has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// <item>NotEnabled- the Publisher is not enabled.</item>
        /// </list>
        /// </returns>
        ReturnCode SuspendPublications();
        /// <summary>
        /// This operation resumes a previously suspended publication.
        /// </summary>
        /// <remarks>
        /// If the Publisher is suspended, this operation will resume the publication of all
        /// DataWriter objects contained by this Publisher. All data held in the history
        /// buffer of the DataWriter's is actively published to the consumers. When the
        /// operation returns all DataWriter's have resumed the publication of suspended
        /// updates.
        /// </remarks>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - the Publisher has resumed publications</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the Publisher has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// <item>NotEnabled- the Publisher is not enabled.</item>
        /// <item>PreconditionNotMet - The Publisher is not suspended.</item>
        /// </list>
        /// </returns>
        ReturnCode ResumePublications();
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        ReturnCode BeginCoherentChanges();
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        ReturnCode EndCoherentChanges();
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="maxWait"></param>
        /// <returns></returns>
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        /// <summary>
        /// This operation returns the DomainParticipant associated with the Publisher.
        /// </summary>
        /// <returns>The DomainParticipant associated with the Publisher or a Publisher of null value if an error occurs.</returns>
        IDomainParticipant GetParticipant();
        /// <summary>
        /// This operation sets the default DataWriterQos of the Publisher.
        /// </summary>
        /// <remarks>
        /// This operation sets the default DataWriterQos of the Publisher (that is the
        /// struct with the QosPolicy settings) which is used for newly created DataWriter
        /// objects, in case the constant DATAWRITER_QOS_DEFAULT is used. The default
        /// DataWriterQos is only used when the constant is supplied as parameter qos to
        /// specify the DataWriterQos in the create_datawriter operation. The
        /// SetDefaultDataWriterQos operation checks if the DataWriterQos is self
        /// consistent. If it is not, the operation has no effect and returns
        /// InconsistentPolicy. The values set by this operation are returned by GetDefaultDataWriterQos.
        /// </remarks>
        /// <param name="qos">The DataWriterQos struct, which contains the new default DataWriterQos 
        /// for the newly created DataWriters.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The new default DataWriterQos is set.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Publisher has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// <item>BadParameter - the parameter qos is not a valid DataWriterQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries.</item>
        /// <item>InconsistentPolicy - the parameter qos contains conflicting
        /// QosPolicy settings, e.g. a history depth that is higher than the specified resource
        /// limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultDataWriterQos(DataWriterQos qos);
        /// <summary>
        /// This operation gets the default DataWriterQos of the Publisher.
        /// </summary>
        /// <remarks>
        /// This operation gets the default DataWriterQos of the Publisher (that is the
        /// struct with the QosPolicy settings) which is used for newly created DataWriter
        /// objects, in case the constant DATAWRITER_QOS_DEFAULT is used. The default
        /// DataWriterQos is only used when the constant is supplied as parameter qos to
        /// specify the DataWriterQos in the CreateDataWriter operation. The
        /// application must provide the DataWriterQos struct in which the QosPolicy
        /// settings can be stored and pass the qos reference to the operation. The operation
        /// writes the default DataWriterQos to the struct referenced to by qos. Any settings 
        /// in the struct are overwritten.
        /// The values retrieved by this operation match the set of values specified on the last
        /// successful call to SetDefaultDatawriterQos, or, if the call was never made,
        /// the default values as specified for each QosPolicy setting.
        /// </remarks>
        /// <param name="qos">A reference to the DataWriterQos struct (provided by the application) in which
        /// the default DataWriterQos for the DataWriter is written.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The new default DataWriter QosPolicy settings for the publisher have successfully
        /// been copied into the specified DataWriterQos policy.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Publisher has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetDefaultDataWriterQos(ref DataWriterQos qos);
        /// <summary>
        /// This operation will copy policies in topicQos to the corresponding policies in dataWriterQos.
        /// </summary>
        /// <remarks>
        /// This operation will copy the QosPolicy settings in a_topic_qos to the
        /// corresponding QosPolicy settings in a_datawriter_qos (replacing the values
        /// in dataWriterQos, if present). This will only apply to the common
        /// QosPolicy settings in each "Entity" Qos.
        /// This is a convenience operation, useful in combination with the operations
        /// GetDefaultDataWriterQos and Topic.GetQos. The operation
        /// CopyFromTopicQos can be used to merge the DataWriter default
        /// QosPolicy settings with the corresponding ones on the TopicQos. The resulting
        /// DataWriterQos can then be used to create a new DataWriter, or set its
        /// DataWriterQos.
        /// This operation does not check the resulting dataWriterQos for consistency.
        /// This is because the merged datawWriterQos may not be the final one, as the
        /// application can still modify some QosPolicy settings prior to applying the
        /// DataWriterQos to the DataWriter.
        /// </remarks>
        /// <param name="dataWriterQos">The destination DataWriterQos struct to which the QosPolicy settings 
        /// should be copied.</param>
        /// <param name="topicQos">The source TopicQos struct, which should be copied.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The QosPolicy settings are copied from the topic DataWriter.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Publisher has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode CopyFromTopicQos(ref DataWriterQos dataWriterQos, TopicQos topicQos);
    }

    /// <summary>
    /// DataWriter allows the application to set the value of the sample to be published
    /// under a given Topic.
    /// A DataWriter is attached to exactly one Publisher which acts as a factory for it.
    /// A DataWriter is bound to exactly one Topic and therefore to exactly one data
    /// type. The Topic must exist prior to the DataWriter's creation.
    /// DataWriter is an abstract class. It must be specialized for each particular
    /// application data type. For a fictional application data type Foo (defined in the
    /// module SPACE) the specialized class would be SPACE.FooDataWriter.
    /// </summary>
    public interface IDataWriter : IEntity
    {
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a DataWriter.
        /// The parameter qos contains the struct with the QosPolicy settings which is
        /// checked for self-consistency and mutability. When the application tries to change a
        /// QosPolicy setting for an enabled DataWriter, which can only be set before the
        /// DataWriter is enabled, the operation will fail and a
        /// ImmutablePolicy is returned. In other words, the application must
        /// provide the presently set QosPolicy settings in case of the immutable QosPolicy
        /// settings. Only the mutable QosPolicy settings can be changed. When qos contains
        /// conflicting QosPolicy settings (not self-consistent), the operation will fail and a
        /// InconsistentPolicy is returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned OK).
        /// </remarks>
        /// <param name="qos">new set of QosPolicy settings for the DataWriter.</param>
        /// <returns>Return values are: 
        /// <list type="bullet">
        /// <item>Ok - the new default DataWriterQos is set</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Publisher has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// <item>BadParameter - the parameter qos is not a valid DataWriterQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries.</item>
        /// <item>Unsuppported - one or more of the selected QosPolicy values are
        /// currently not supported by OpenSplice.</item>
        /// <item>ImmutablePolicy - the parameter qos contains an immutable
        /// QosPolicy setting with a different value than set during enabling of the
        /// DataWriter.</item>
        /// <item>InconsistentPolicy - the parameter qos contains an inconsistent QosPolicy settings, 
        /// e.g. a history depth that is higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(DataWriterQos qos);
        /// <summary>
        /// This operation allows access to the existing list of QosPolicy settings for a DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing list of QosPolicy settings of a
        /// DataWriter on which this operation is used. This DataWriterQos is stored at the
        /// location pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination DataWriterQos struct in which the 
        /// QosPolicy settings will be copied into.</param>
        /// <returns>Return values are: 
        /// 
        /// </returns>
        ReturnCode GetQos(ref DataWriterQos qos);
        /// <summary>
        /// This operation attaches a DataWriterListener to the DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation attaches a DataWriterListener to the DataWriter. Only one
        /// DataWriterListener can be attached to each DataWriter. If a
        /// DataWriterListener was already attached, the operation will replace it with the
        /// new one. When listener is NULL , it represents a listener that is
        /// treated as a NOOP for all statuses activated in the bit mask.
        /// </remarks>
        /// <param name="listener"> The DataWriterListener instance that will be attached to the
        /// DataWriter.</param>
        /// <param name="mask">a bit mask in which each bit enables the invocation of the DataWriterListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - the DataWriterListener is attached</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode SetListener(IDataWriterListener listener, StatusKind mask);
        /// <summary>
        /// This property returns the Topic which is associated with the DataWriter.
        /// </summary>
        ITopic Topic { get; }
        /// <summary>
        /// This property returns the Publisher to which the DataWriter belongs.
        /// </summary>
        IPublisher Publisher { get; }
        /// <summary>
        /// This operation is not yet implemented It is scheduled for a future release.
        /// </summary>
        /// <param name="maxWait"></param>
        /// <returns>Return codes are: Unsupported</returns>
        ReturnCode WaitForAcknowledgments(Duration maxWait);
        /// <summary>
        /// This operation obtains the LivelinessLostStatus struct of the DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation obtains the LivelinessLostStatus struct of the DataWriter.
        /// This struct contains the information whether the liveliness (that the DataWriter
        /// has committed through its LivelinessQosPolicy) was respected.
        /// This means, that the status represents whether the DataWriter failed to actively
        /// signal its liveliness within the offered liveliness period. If the liveliness is lost, the
        /// DataReader objects will consider the DataWriter as no longer alive.
        /// The LivelinessLostStatus can also be monitored using a
        /// DataWriterListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to LivelinessLostStatus where the contents of the LivelinessLostStatus  
        /// struct of the DataWriter will be copied into.</param>
        /// <returns>Return values are: 
        /// <list type="bullet">
        /// <item>Ok - The current LivelinessLostStatus of this DataWriter has successfully 
        /// been copied into the specified status parameter.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetLivelinessLostStatus(ref LivelinessLostStatus status);
        /// <summary>
        /// This operation obtains the OfferedDeadlineMissedStatus struct of the DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation obtains the OfferedDeadlineMissedStatus struct of the
        /// DataWriter. This struct contains the information whether the deadline (that the
        /// DataWriter has committed through its DeadlineQosPolicy) was respected for
        /// each instance.
        /// The OfferedDeadlineMissedStatus can also be monitored using a
        /// DataWriterListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to OfferedDeadlineMissedStatus  where the contents of the   
        /// OfferedDeadlineMissedStatus struct of the DataWriter will be copied into.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The current OfferedDeadlineMissedStatus of this DataWriter has successfully 
        /// been copied into the specified status parameter.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetOfferedDeadlineMissedStatus(ref OfferedDeadlineMissedStatus status);
        /// <summary>
        /// This operation obtains the OfferedIncompatibleQosStatus struct of the DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation obtains the OfferedIncompatibleQosStatus struct of the
        /// DataWriter. This struct contains the information whether a QosPolicy setting
        /// was incompatible with the requested QosPolicy setting.
        /// This means, that the status represents whether a DataReader object has been
        /// discovered by the DataWriter with the same Topic and a requested
        /// DataReaderQos that was incompatible with the one offered by the DataWriter.
        /// The OfferedIncompatibleQosStatus can also be monitored using a
        /// DataWriterListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to  OfferedIncompatibleQosStatus where the contents of the   
        ///  OfferedIncompatibleQosStatus struct of the DataWriter will be copied into.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The current OfferedIncompatibleQosStatus of this DataWriter has successfully 
        /// been copied into the specified status parameter.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetOfferedIncompatibleQosStatus(ref OfferedIncompatibleQosStatus status);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="status"></param>
        /// <returns></returns>
        ReturnCode GetPublicationMatchedStatus(ref PublicationMatchedStatus status);
        /// <summary>
        /// This operation asserts the liveliness for the DataWriter.
        /// </summary>
        /// <remarks>
        /// This operation will manually assert the liveliness for the DataWriter. This way,
        /// the Data Distribution Service is informed that the corresponding DataWriter is
        /// still alive. This operation is used in combination with the LivelinessQosPolicy
        /// set to MANUAL_BY_PARTICIPANT_LIVELINESS_QOS or
        /// MANUAL_BY_TOPIC_LIVELINESS_QOS. 
        /// Writing data via the write operation of a DataWriter will assert the liveliness on
        /// the DataWriter itself and its containing DomainParticipant. Therefore,
        /// assert_liveliness is only needed when not writing regularly.
        /// The liveliness should be asserted by the application, depending on the
        /// LivelinessQosPolicy. Asserting the liveliness for this DataWriter can also
        /// be achieved by asserting the liveliness to the DomainParticipant.
        /// </remarks>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The liveliness of this DataWriter has successfully been asserted.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// <item>NotEnabled - The DataWriter is not enabled. </item>
        /// </list>
        /// </returns>
        ReturnCode AssertLiveliness();
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="subscriptionHandles"></param>
        /// <returns></returns>
        ReturnCode GetMatchedSubscriptions(ref InstanceHandle[] subscriptionHandles);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="subscriptionData"></param>
        /// <param name="subscriptionHandle"></param>
        /// <returns></returns>
        ReturnCode GetMatchedSubscriptionData(
                ref SubscriptionBuiltinTopicData subscriptionData,
                InstanceHandle subscriptionHandle);
    }
    /// <summary>
    /// A Subscriber is the object responsible for the actual reception of the data
    /// resulting from its subscriptions.
    /// A Subscriber acts on behalf of one or more DataReader objects that are related
    /// to it. When it receives data (from the other parts of the system), it indicates to the
    /// application that data is available through its DataReaderListener and by
    /// enabling related Conditions. The application can access the list of concerned
    /// DataReader objects through the operation get_datareaders and then access the
    /// data available through operations on the DataReader.
    /// </summary>
    public interface ISubscriber : IEntity
    {
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <param name="topic"></param>
        /// <returns></returns>
        IDataReader CreateDataReader(ITopicDescription topic);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <param name="topic"></param>
        /// <param name="listener"></param>
        /// <param name="mask"></param>
        /// <returns></returns>
        IDataReader CreateDataReader(
                ITopicDescription topic,
                IDataReaderListener listener, StatusKind mask);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <param name="topic"></param>
        /// <param name="qos"></param>
        /// <returns></returns>
        IDataReader CreateDataReader(ITopicDescription topic, DataReaderQos qos);
        /// <summary>
        /// This operation creates a DataReader with the desired QosPolicy settings, for the
        /// desired TopicDescription and attaches the optionally specified DataWriterListener to it.
        /// </summary>
        /// <remarks>
        /// This operation creates a DataReader with the desired QosPolicy settings, for the
        /// desired TopicDescription and attaches the optionally specified
        /// DataReaderListener to it. The TopicDescription may be a Topic,
        /// MultiTopic or ContentFilteredTopic. The returned DataReader is attached
        /// (and belongs) to the Subscriber. To delete the DataReader the operation
        /// DeleteDatareader or DeleteContainedEntities must be used.
        /// </remarks>
        /// <param name="topic">The TopicDescription for which the DataReader is created. 
        /// This may be a Topic, MultiTopic or ContentFilteredTopic.</param>
        /// <param name="qos">The struct with the QosPolicy settings for the new DataReader, 
        /// when these QosPolicy settings are not self consistent, no DataReader is created.</param>
        /// <param name="listener">The DataReaderListener instance which will be attached to the new
        /// DataReader. It is permitted to use NULL as the value of the listener: this
        /// behaves as a DataWriterListener whose operations perform no action.</param>
        /// <param name="mask">A bit-mask in which each bit enables the invocation of 
        /// the DataReaderListener for a certain status.</param>
        /// <returns>The newly created DataReader, or in case of an error a null value one.</returns>
        IDataReader CreateDataReader(
                ITopicDescription topic, 
                DataReaderQos qos,
                IDataReaderListener listener, 
                StatusKind mask);
        /// <summary>
        /// This operation deletes a DataReader that belongs to the Subscriber.
        /// </summary>
        /// <remarks>
        /// This operation deletes a DataReader that belongs to the Subscriber. When the
        /// operation is called on a different Subscriber, as used when the DataReader was
        /// created, the operation has no effect and returns PreconditionNotMet. 
        /// The deletion of the DataReader is not allowed if there are any ReadCondition or 
        /// QueryCondition objects that are attached to the DataReader. In that case the operation returns
        /// PreconditionNotMet.
        /// </remarks>
        /// <param name="dataReader">The DataReader which is to be deleted.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - the DataReader is deleted</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Subscriber has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// <item>BadParameter - the parameter a_datareader is not a valid IDataReader</item>
        /// <item>PreconditionNotMet - the operation is called on a different Subscriber, 
        /// as used when the DataReader was created, or the DataReader contains one or more 
        /// ReadCondition or QueryCondition objects.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteDataReader(IDataReader dataReader);
        /// <summary>
        /// This operation deletes all the DataReader objects that were created by means of
        /// the CreateDatareader operation on the Subscriber.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the DataReader objects that were created by means of
        /// the CreateDatareader operation on the Subscriber. In other words, it deletes
        /// all contained DataReader objects. Prior to deleting each DataReader, this
        /// operation recursively calls the corresponding DeleteContainedEntities
        /// operation on each DataReader. In other words, all DataReader objects in the
        /// Subscriber are deleted, including the QueryCondition and ReadCondition
        /// objects contained by the DataReader.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The contained Entity objects are deleted and the application may delete the Subscriber</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Subscriber has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode DeleteContainedEntities();
        /// <summary>
        /// This operation returns a previously created DataReader belonging to the
        /// Subscriber which is attached to a Topic with the matching topic_name.
        /// </summary>
        /// <remarks>
        /// This operation returns a previously created DataReader belonging to the
        /// Subscriber which is attached to a Topic with the matching topic_name. When
        /// multiple DataReader objects (which satisfy the same condition) exist, this
        /// operation will return one of them. It is not specified which one.
        /// This operation may be used on the built-in Subscriber, which returns the built-in
        /// DataReader objects for the built-in Topics.
        /// </remarks>
        /// <param name="topicName">The name of the Topic, which is attached to the DataReader to look for.</param>
        /// <returns>A reference to the DataReader found. If no such DataReader is found the a null value one is 
        /// returned.</returns>
        IDataReader LookupDataReader(string topicName);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="readers"></param>
        /// <returns></returns>
        ReturnCode GetDataReaders(ref IDataReader[] readers);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="readers"></param>
        /// <param name="sampleStates"></param>
        /// <param name="viewStates"></param>
        /// <param name="instanceStates"></param>
        /// <returns></returns>
        ReturnCode GetDataReaders(
                ref IDataReader[] readers,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);
        /// <summary>
        /// This operation invokes the on_data_available operation on
        /// DataReaderListener objects which are attached to the contained DataReader
        /// entities having new, available data.
        /// </summary>
        /// <remarks>
        /// This operation invokes the on_data_available operation on the
        /// DataReaderListener objects attached to contained DataReader entities that
        /// have received information, but which have not yet been processed by those
        /// DataReaders.
        /// The notify_datareaders operation ignores the bit mask value of individual
        /// DataReaderListener objects, even when the DATA_AVAILABLE_STATUS bit
        /// has not been set on a DataReader that has new, available data. The
        /// on_data_available operation will still be invoked, when the
        /// DATA_AVAILABLE_STATUS bit has not been set on a DataReader, but will not
        /// propagate to the DomainParticipantListener.
        /// When the DataReader has attached a NULL listener, the event will be consumed
        /// and will not propagate to the DomainParticipantListener. (Remember that a
        /// NULL listener is regarded as a listener that handles all its events as a NOOP).
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - All appropriate listeners have been invoked.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Subscriber has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to complete this operation.</item>
        /// </returns>
        ReturnCode NotifyDataReaders();
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a Subscriber.
        /// </summary>
        /// <param name="qos">The new set of QosPolicy settings for the Subscriber.</param>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a Subscriber.
        /// The parameter qos contains the QosPolicy settings which is checked for
        /// self-consistency and mutability. When the application tries to change a QosPolicy
        /// setting for an enabled Subscriber, which can only be set before the Subscriber
        /// is enabled, the operation will fail and a RETCODE_IMMUTABLE_POLICY is returned.
        /// In other words, the application must provide the presently set QosPolicy settings
        /// in case of the immutable QosPolicy settings. Only the mutable QosPolicy
        /// settings can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and a RETCODE_INCONSISTENT_POLICY is
        /// returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned RETCODE_OK).
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The new SubscriberQos is set.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// <item>BadParameter - the parameter qos is not a valid SubscriberQos.
        /// It contains a QosPolicy setting with an enum value that is outside its legal
        /// boundaries.</item>
        /// <item>Unsupported - one or more of the selected QosPolicy values are
        /// currently not supported by OpenSplice.</item>
        /// <item>ImmutablePolicy - the parameter qos contains an immutable QosPolicy setting 
        /// with a different value than set during enabling of the Subbscriber.</item>
        /// </returns>
        ReturnCode SetQos(SubscriberQos qos);
        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a Subscriber.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a Subscriber
        /// on which this operation is used. This SubscriberQos is stored at the location
        /// pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">A reference to the destination SubscriberQos struct in which the QosPolicy 
        /// settings will be copied.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The existing set of QoS policy values applied to this Subscriber
        /// has successfully been copied into the specified SubscriberQos parameter.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref SubscriberQos qos);
        /// <summary>
        /// This operation attaches a SubscriberListener to the Subscriber.
        /// </summary>
        /// <remarks>
        /// This operation attaches a SubscriberListener to the Subscriber. Only one
        /// SubscriberListener can be attached to each Subscriber. If a SubscriberListener 
        /// was already attached, the operation will replace it with the new one. 
        /// When a_listener is NULL , it represents a listener that is treated as a NOOP for all 
        /// statuses activated in the bit mask.
        /// </remarks>
        /// <param name="listener">The SubscriberListener instance, which will be attached to the Subscriber.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the SubscriberListener 
        /// for a certain status.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The SubscriberListener is attached.</item>
        /// <item>Error - an internal error has occurred</item>
        /// <item>AlreadyDeleted - the DataWriter has already been deleted.</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources to 
        /// complete this operation.</item>
        /// </returns>
        ReturnCode SetListener(ISubscriberListener listener, StatusKind mask);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        ReturnCode BeginAccess();
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <returns></returns>
        ReturnCode EndAccess();
        /// <summary>
        /// This property allows access to the DomainParticipant associated with the Subscriber.
        /// </summary>
        IDomainParticipant Participant { get; }
        /// <summary>
        /// This operation sets the default DataReaderQos of the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation sets the default DataReaderQos of the DataReader (that is the
        /// struct with the QosPolicy settings). This QosPolicy is used for newly created
        /// DataReader objects in case the constant DATAREADER_QOS_DEFAULT is used as
        /// parameter qos to specify the DataReaderQos in the CreateDataReader
        /// operation. This operation checks if the DataReaderQos is self consistent. If it is
        /// not, the operation has no effect and returns InconsistentPolicy.
        /// The values set by this operation are returned by GetDefaultDataReaderQos.
        /// </remarks>
        /// <param name="qos">The DataReaderQos struct, which containsthe new default QosPolicy 
        /// settings for the newly created DataReaders.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>Ok - The new default DataReaderQos is set.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Subscriber has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// <item>BadParameter - the parameter qos is not a valid DataReaderQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries.</item>
        /// <item>InconsistentPolicy - the parameter qos contains conflicting
        /// QosPolicy settings, e.g. a history depth that is higher than the specified resource
        /// limits.</item>
        /// <item>Unsupported - one or more of the selected QosPolicy values are currently not 
        /// supported by OpenSplice.</item>
        /// </list>
        /// </returns>
        ReturnCode SetDefaultDataReaderQos(DataReaderQos qos);
        /// <summary>
        /// This operation gets the default QosPolicy settings of the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation gets the default QosPolicy settings of the DataReader (that is the
        /// DataReaderQos) which is used for newly created DataReader objects, in case
        /// the constant DATAREADER_QOS_DEFAULT is used. The default DataReaderQos
        /// is only used when the constant is supplied as parameter qos to specify the
        /// DataReaderQos in the CreateDataReader operation. The application must
        /// provide the DataReaderQos struct in which the QosPolicy settings can be stored
        /// and pass the qos reference to the operation. The operation writes the default
        /// QosPolicy settings to the struct referenced to by qos. Any settings in the struct are
        /// overwritten.
        /// The values retrieved by this operation match the values specified on the last
        /// successful call to SetDefaultDataReaderQos, or, if the call was never made,
        /// the default values as specified for each QosPolicy setting
        /// </remarks>
        /// <param name="qos">A reference to the DataReaderQos struct(provided by the application) 
        /// in which the default QosPolicy settings for the DataReader are written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The default DataReader QosPolicy settings of this Subscriber 
        /// have successfully been copied into the specified DataReaderQos parameter.</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Subscriber has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// </returns>
        ReturnCode GetDefaultDataReaderQos(ref DataReaderQos qos);
        /// <summary>
        /// This operation will copy the policies in a_topic_qos to the corresponding policies in datareaderQos.
        /// </summary>
        /// <remarks>
        /// This operation will copy the QosPolicy settings in topicQos to the
        /// corresponding QosPolicy settings in dataReaderQos (replacing the values
        /// in datareaderQos, if present).
        /// This is a convenience operation, useful in combination with the operations
        /// GetDefaultDataWriterQos and Topic.get_qos. The operation
        /// CopyFromTopicQos can be used to merge the DataReader default
        /// QosPolicy settings with the corresponding ones on the Topic. The resulting
        /// DataReaderQos can then be used to create a new DataReader, or set its
        /// DataReaderQos.
        /// This operation does not check the resulting dataReaderQos for self
        /// consistency. This is because the merged dataReaderQos may not be the
        /// final one, as the application can still modify some QosPolicy settings prior to
        /// applying the DataReaderQos to the DataReader.
        /// </remarks>
        /// <param name="dataReaderQos">The destination DataReaderQos struct to which the QosPolicy settings 
        /// will be copied.</param>
        /// <param name="topicQos">The source TopicQos, which will be copied.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The QosPolicy settings have successfully been copied from the TopicQos to DataReaderQos</item>
        /// <item>Error - an internal error has occurred.</item>
        /// <item>AlreadyDeleted - the Subscriber has already been deleted</item>
        /// <item>OutOfResources - the Data Distribution Service ran out of resources 
        /// to complete this operation.</item>
        /// </returns>
        ReturnCode CopyFromTopicQos(ref DataReaderQos dataReaderQos, TopicQos topicQos);
    }

    /// <summary>
    /// A DataReader allows the application:
    /// to declare data it wishes to receive (i.e., make a subscription)
    /// to access data received by the associated Subscriber. 
    /// A DataReader refers to exactly one TopicDescription (either a Topic, a ContentFilteredTopic or a MultiTopic) 
    /// that identifies the samples to be read. The DataReader may give access to several instances of the data type, 
    /// which are distinguished from each other by their key. 
    /// DataReader is an abstract class. It is specialized for each particular application data type. 
    /// For a fictional application data type Foo (defined in the module SPACE) 
    /// the specialized class would be SPACE.FooDataReader.
    /// </summary>
    public interface IDataReader : IEntity
    {
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <returns></returns>
        IReadCondition CreateReadCondition();
        /// <summary>
        /// This operation creates a new ReadCondition for the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation creates a new ReadCondition for the DataReader. The returned
        /// ReadCondition is attached (and belongs) to the DataReader. When the
        /// operation fails, NULL is returned. To delete the ReadCondition the
        /// operation delete_readcondition or DeleteContainedEntities must be used.
        /// </remarks>
        /// <param name="sampleStates">a mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">a mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">a mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Returns the ReadCondition, it it fails it returns null.</returns>
        IReadCondition CreateReadCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates);
        /// <summary>
        /// This operations behaves similarly to the most complete operation, and it substitutes default values
        /// for the missing parameters. Default for QoS for QoS parameters, null for listeners and 0 mask for
        /// listener parameters.
        /// </summary>
        /// <param name="queryExpression"></param>
        /// <param name="queryParameters"></param>
        /// <returns></returns>
        IQueryCondition CreateQueryCondition(
                string queryExpression,
                params string[] queryParameters);
        /// <summary>
        /// This operation creates a new QueryCondition for the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation creates a new QueryCondition for the DataReader. The returned
        /// QueryCondition is attached (and belongs) to the DataReader. When the
        /// operation fails, the NULL pointer is returned. To delete the QueryCondition the
        /// operation delete_readcondition or delete_contained_entities must be
        /// used.
        /// </remarks>
        /// <param name="sampleStates">a mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">a mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">a mask, which selects only those samples with the desired instance states.</param>
        /// <param name="queryExpression">the query string, which must be a subset of the SQL query language.</param>
        /// <param name="queryParameters">a sequence of strings which are the parameter values used in the 
        /// SQL query string (i.e., the tokens in the expression). The number of values in queryParameters 
        /// must be equal or greater than the highest referenced %n token in the queryExpression 
        /// (e.g.if %1 and %8 are used as parameters in the queryExpression, the queryParameters 
        /// should at least contain n+1 = 9 values).</param>
        /// <returns>Returns the QueryCondition. When it fails it returns a null QueryCondition.</returns>
        IQueryCondition CreateQueryCondition(
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates,
                string queryExpression,
                params string[] queryParameters);
        /// <summary>
        /// This operation deletes a ReadCondition or QueryCondition which is attached to the DataReader.
        /// </summary>
        /// This operation deletes a ReadCondition or QueryCondition which is attached
        /// to the DataReader. Since a QueryCondition is a specialized ReadCondition,
        /// the operation can also be used to delete a QueryCondition. A ReadCondition
        /// or QueryCondition cannot be deleted when it is not attached to this DataReader.
        /// When the operation is called on a ReadCondition or QueryCondition which
        /// was not at t ached to this DataReader, the operation returns
        /// PreconditionNotMet.
        /// <param name="condition">The ReadCondition or QueryCondition which is to be deleted.
        /// </param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The ReadCondition or QueryCondition is deleted.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - The parameter condition is not a valid IReadCondition reference.</item>
        /// <item>PreconditionNotMet - The operation is called on a different DataReader, as used when the 
        ///  ReadCondition or QueryCondition was created.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteReadCondition(IReadCondition condition);
        /// <summary>
        /// This operation deletes all the Entity objects that were created by means of one of the 
        /// Create operations on the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation deletes all the Entity objects that were created by means of one of
        /// the Create operations on the DataReader. In other words, it deletes all
        /// QueryCondition and ReadCondition objects contained by the DataReader.
        /// </remarks>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The contained Entity objects are deleted and the application may delete the DataReader</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode DeleteContainedEntities();
        /// <summary>
        /// This operation replaces the existing set of QosPolicy settings for a DataReader.
        /// </summary>
        /// <remarks>
        /// This operation replaces the existing set of QosPolicy settings for a DataReader.
        /// The parameter qos contains the QosPolicy settings which is checked for
        /// self-consistency and mutability. When the application tries to change a QosPolicy
        /// setting for an enabled DataReader, which can only be set before the DataReader
        /// is enabled, the operation will fail and a ImmutablePolicy is returned.
        /// In other words, the application must provide the presently set QosPolicy settings
        /// in case of the immutable QosPolicy settings. Only the mutable QosPolicy
        /// settings can be changed. When qos contains conflicting QosPolicy settings (not
        /// self-consistent), the operation will fail and a InconsistentPolicy is
        /// returned.
        /// The set of QosPolicy settings specified by the qos parameter are applied on top of
        /// the existing QoS, replacing the values of any policies previously set (provided, the
        /// operation returned Ok).
        /// </remarks>
        /// <param name="qos">the new set of QosPolicy settings for the DataReader.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The new DataReaderQos is set.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>BadParameter - the parameter qos is not a valid DataReaderQos.
        /// It contains a QosPolicy setting with an invalid Duration value or an enum
        /// value that is outside its legal boundaries</item>
        /// <item>Unsupported - one or more of the selected QosPolicy values are currently not
        /// supported by OpenSplice</item>
        /// <item>ImmutablePolicy - the parameter qos contains an immutable
        /// QosPolicy setting with a different value than set during enabling of the DataReader</item>
        /// <item>InconsistentPolicy - the parameter qos contains conflicting
        /// QosPolicy settings, e.g. a history depth that is higher than the specified resource limits.</item>
        /// </list>
        /// </returns>
        ReturnCode SetQos(DataReaderQos qos);
        /// <summary>
        /// This operation allows access to the existing set of QoS policies for a DataReader.
        /// </summary>
        /// <remarks>
        /// This operation allows access to the existing set of QoS policies of a DataReader
        /// on which this operation is used. This DataReaderQos is stored at the location
        /// pointed to by the qos parameter.
        /// </remarks>
        /// <param name="qos">a reference to DataReaderQos, where the QosPolicy settings of the DataReader
        /// are to be copied into.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>Ok - The existing set of QoSPolicy values applied to this DataReader
        /// has successfully been copied into the specified DataReaderQos parameter.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetQos(ref DataReaderQos qos);
        /// <summary>
        /// This operation attaches a DataReaderListener to the DataReader.
        /// </summary>
        /// <param name="listener">The DataReaderListener which will be attached to the DataReader.</param>
        /// <param name="mask">A bit mask in which each bit enables the invocation of the DataReaderListener 
        /// for a certain status.</param>
        /// <returns></returns>
        ReturnCode SetListener(IDataReaderListener listener, StatusKind mask);
        /// <summary>
        /// This operation returns the TopicDescription which is associated with the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation returns the TopicDescription which is associated with the
        /// DataReader, thus the TopicDescription with which the DataReader is
        /// created. If the DataReader is already deleted, NULL is returned.
        /// </remarks>
        /// <returns>Returns the TopicDescription associated with the DataReader.</returns>
        ITopicDescription GetTopicDescription();
        /// <summary>
        /// This property returns the Subscriber to which the DataReader belongs.
        /// </summary>
        ISubscriber Subscriber { get; }
        /// <summary>
        /// This operation obtains the SampleRejectedStatus of the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation obtains the SampleRejectedStatus struct of the DataReader.
        /// This struct contains the information whether a received sample has been rejected.
        /// The SampleRejectedStatus can also be monitored using a
        /// DataReaderListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to SampleRejectedStatus where the contents of the 
        /// SampleRejectedStatus of the DataReader will be copied into.</param>
        /// <returns>
        /// <list type="bullet">
        /// <item>Ok - The current SampleRejectedStatus of this DataReader has successfully been copied 
        /// into the specified status parameter.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetSampleRejectedStatus(ref SampleRejectedStatus status);
        /// <summary>
        /// This operation obtains the LivelinessChangedStatus struct of the DataReader.
        /// </summary>
        /// <remarks>
        /// This obtains returns the LivelinessChangedStatus struct of the DataReader.
        /// This struct contains the information whether the liveliness of one or more
        /// DataWriter objects that were writing instances read by the DataReader has
        /// changed. In other words, some DataWriter have become alive or not alive.
        /// The LivelinessChangedStatus can also be monitored using a
        /// DataReaderListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to LivelinessChangedStatus where the contents of the 
        ///  LivelinessChangedStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The current  LivelinessChangedStatus of this DataReader has successfully been copied 
        /// into the specified status parameter.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetLivelinessChangedStatus(ref LivelinessChangedStatus status);
        /// <summary>
        /// This operation obtains the RequestedDeadlineMissedStatus struct of the DataReader.        
        /// </summary>
        /// <remarks>
        /// This operation obtains the RequestedDeadlineMissedStatus struct of the
        /// DataReader. This struct contains the information whether the deadline that the
        /// DataReader was expecting through its DeadlineQosPolicy was not respected
        /// for a specific instance.
        /// The RequestedDeadlineMissedStatus can also be monitored using a
        /// DataReaderListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to RequestedDeadlineMissedStatus where the contents of the 
        /// RequestedDeadlineMissedStatus  of the DataReader will be copied into.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The current RequestedDeadlineMissedStatus of this DataReader has successfully been copied 
        /// into the specified status parameter.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetRequestedDeadlineMissedStatus(ref RequestedDeadlineMissedStatus status);
        /// <summary>
        /// This operation obtains the RequestedIncompatibleQosStatus struct of the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation obtains the RequestedIncompatibleQosStatus struct of the
        /// DataReader. This struct contains the information whether a QosPolicy setting
        /// was incompatible with the offered QosPolicy setting.
        /// The Request/Offering mechanism is applicable between the DataWriter and the
        /// DataReader. If the QosPolicy settings between DataWriter and DataReader
        /// are inconsistent, no communication between them is established. In addition the
        /// DataWriter will be informed via a REQUESTED_INCOMPATIBLE_QOS status
        /// change and the DataReader will be informed via an OFFERED_INCOMPATIBLE_QOS status change.
        /// The RequestedIncompatibleQosStatus can also be monitored using a
        /// DataReaderListener or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status">A reference to RequestedIncompatibleQosStatus where the contents of the 
        ///  RequestedIncompatibleQosStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The current RequestedIncompatibleQosStatus of this DataReader has successfully been copied 
        /// into the specified status parameter.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetRequestedIncompatibleQosStatus(ref RequestedIncompatibleQosStatus status);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="status"></param>
        /// <returns></returns>
        ReturnCode GetSubscriptionMatchedStatus(ref SubscriptionMatchedStatus status);
        /// <summary>
        /// This operation obtains the SampleLostStatus struct of the DataReader.
        /// </summary>
        /// <remarks>
        /// This operation obtains the SampleLostStatus struct of the DataReader. This
        /// struct contains information whether samples have been lost. This only applies when
        /// the ReliabilityQosPolicy is set to RELIABLE. If the
        /// ReliabilityQosPolicy is set to BEST_EFFORT the Data Distribution Service
        /// will not report the loss of samples.
        /// The SampleLostStatus can also be monitored using a DataReaderListener
        /// or by using the associated StatusCondition.
        /// </remarks>
        /// <param name="status"> reference to SampleLostStatus where the contents of the 
        ///  SampleLostStatus of the DataReader will be copied into.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The current SampleLostStatus of this DataReader has successfully been copied 
        /// into the specified status parameter.</item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        ReturnCode GetSampleLostStatus(ref SampleLostStatus status);
        /// <summary>
        /// This operation will block the application thread until all historical data is received.
        /// </summary>
        /// <remarks>
        /// This operation behaves differently for DataReader objects which have a
        /// non-VOLATILE_DURABILITY_QOS DurabilityQosPolicy and for
        /// DataReader obj ects which have a VOLATILE_DURABILITY_QOS
        /// DurabilityQosPolicy.
        /// As soon as an application enables a non-VOLATILE_DURABILITY_QOS
        /// DataReader it will start receiving both historical data, i.e. the data that was
        /// written prior to the time the DataReader joined the domain, as well as any new
        /// data written by the DataWriter objects. There are situations where the application
        /// logic may require the application to wait until all historical data is received. This
        /// is the purpose of the WaitForHistoricalData operation. 
        /// As soon as an application enables a VOLATILE_DURABILITY_QOS DataReader it
        /// will not start receiving historical data but only new data written by the
        /// DataWriter objects. By calling WaitForHistoricalData the DataReader
        /// explicitly requests the Data Distribution Service to start receiving also the
        /// historical data and to wait until either all historical data is received, or the
        /// duration specified by the max_wait parameter has elapsed, whichever happens
        /// first.
        /// </remarks>
        /// <param name="maxWait">the maximum duration to block for the operation, after which 
        /// the application thread is unblocked. The special constant Duration Infinite can be used when 
        /// the maximum waiting time does not need to be bounded.</param>
        /// <returns>Return codes are: 
        /// <list type="bullet">
        /// <item>Ok - The historical data is received </item>
        /// <item>Error - An internal error has occured.</item> </item>
        /// <item>AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// <item>NotEnabled - the DataReader is not enabled.</item>
        /// <item>Timeout - Not all data is received before maxWait elapsed.</item>
        /// </list>
        /// </returns>
        ReturnCode WaitForHistoricalData(Duration maxWait);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="publicationHandles"></param>
        /// <returns></returns>
        ReturnCode GetMatchedPublications(ref InstanceHandle[] publicationHandles);
        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="publicationData"></param>
        /// <param name="publicationHandle"></param>
        ReturnCode GetMatchedPublicationData(
                ref PublicationBuiltinTopicData publicationData,
                InstanceHandle publicationHandle);
    }

    // listener interfaces

    public interface ITopicListener
    {
        void OnInconsistentTopic(ITopic entityInterface, InconsistentTopicStatus status);
    }

    public interface IDataWriterListener
    {
        void OnOfferedDeadlineMissed(IDataWriter entityInterface, OfferedDeadlineMissedStatus status);
        void OnOfferedIncompatibleQos(IDataWriter entityInterface, OfferedIncompatibleQosStatus status);
        void OnLivelinessLost(IDataWriter entityInterface, LivelinessLostStatus status);
        void OnPublicationMatched(IDataWriter entityInterface, PublicationMatchedStatus status);
    }

    public interface IPublisherListener : IDataWriterListener
    {
    }

    public interface IDataReaderListener
    {
        void OnRequestedDeadlineMissed(IDataReader entityInterface, RequestedDeadlineMissedStatus status);
        void OnRequestedIncompatibleQos(IDataReader entityInterface, RequestedIncompatibleQosStatus status);
        void OnSampleRejected(IDataReader entityInterface, SampleRejectedStatus status);
        void OnLivelinessChanged(IDataReader entityInterface, LivelinessChangedStatus status);
        void OnDataAvailable(IDataReader entityInterface);
        void OnSubscriptionMatched(IDataReader entityInterface, SubscriptionMatchedStatus status);
        void OnSampleLost(IDataReader entityInterface, SampleLostStatus status);
    }

    public interface ISubscriberListener : IDataReaderListener
    {
        void OnDataOnReaders(ISubscriber entityInterface);
    }

    public interface IDomainParticipantListener : ITopicListener, IPublisherListener, ISubscriberListener
    {
    }
    
    /// <summary>
    /// Create a QosProvider fetching QoS configuration from the specified
    /// URI. For instance, the following code:
    ///
    /// <pre><code>
    /// QosProvider xml_file_provider("file://somewhere/on/disk/qos-config.xml");
    /// QosProvider json_file_provider("file://somewhere/on/disk/json-config.json");
    /// QosProvider json_http_provider("http:///somewhere.org/here/json-config.json");
    /// </code></pre>
    ///
    /// The URI determines the how the Qos configuration is fetched and the
    /// format in which it is represented. This specification requires compliant
    /// implementations to support at least one file based configuration using
    /// the XML syntax defined as part of the DDS for CCM specification (formal/12.02.01).
    ///
    /// constructor (
    ///     in string uri,
    ///     in string name);
    /// </summary>
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


} // end namespace DDS
