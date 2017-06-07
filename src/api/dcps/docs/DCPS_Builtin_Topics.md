Built-in Topics                                                                {#DCPS_Builtin_Topics}
===============

[TOC]

Built-in Topics                                                                {#DCPS_Builtin_Topics_Intro}
===============

As part of its operation, the middleware must discover and possibly keep track of
the presence of remote entities such as a new participant in the domain. This
information may also be important to the application, which may want to react to
this discovery, or else access it on demand.

To make this information accessible to the application, the DCPS specification
introduces a set of built-in topics and corresponding DataReader objects that can
then be used by the application. The information is then accessed as normal
application data. This approach avoids introducing a new API to access this
information and allows the application to become aware of any changes in those
values by means of any of the mechanisms presented in \ref DCPS_Modules_Infrastructure_Listener 
"Listeners" and \ref DCPS_Modules_Infrastructure_Waitset "Conditions and Waitsets".

\if isocpp2
The built-in data-readers all belong to a built-in Subscriber. This subscriber can be
retrieved by using the method dds::sub::builtin_subscriber with the DomainParticipant 
as parameter.<br>
The built-in DataReader objects can be retrieved by using the operation 
\ref dds::sub::find(const dds::sub::Subscriber& sub, const std::string &topic_name, FwdIterator begin, uint32_t max_size)
"dds::sub::find", with the BuiltinSubscriber and the topic name as parameter.
\else
The built-in data-readers all belong to a built-in Subscriber. This subscriber can be
retrieved by using the method get_builtin_subscriber provided by the
DomainParticipant. The built-in DataReader objects can be retrieved by using the operation
lookup_datareader, with the Subscriber and the topic name as parameter.
\endif

The QoS of the built-in Subscriber and DataReader objects is given by the following
table:
<table>
    <caption>Built-in Subscriber and DataReader QoS</caption>
    <tr>
        <th>QoS Policy</th><th>Value</th>
    </tr>
    <tr><td>\ref DCPS_QoS_UserData "USER_DATA"</td><td>empty</td></tr>
    <tr><td>\ref DCPS_QoS_TopicData "TOPIC_DATA"</td><td>empty</td></tr>
    <tr><td>\ref DCPS_QoS_GroupData "GROUP_DATA"</td><td>empty</td></tr>
    <tr><td>\ref DCPS_QoS_Durability "DURABILITY"</td><td>TRANSIENT
    <tr><td>\ref DCPS_QoS_DurabilityService "DURABILITY_SERVICE"</td><td>service_cleanup_delay = 0<br>
                                                                         history_kind = KEEP_LAST<br>
                                                                         history_depth = 1<br>
                                                                         max_samples = LENGTH_UNLIMITED<br>
                                                                         max_instances = LENGTH_UNLIMITED<br>
                                                                         max_samples_per_instance = LENGTH_UNLIMITED</td></tr>
    <tr><td>\ref DCPS_QoS_Presentation "PRESENTATION"</td><td>access_scope = TOPIC<br>
                                                              coherent_access = FALSE<br>
                                                              ordered_access = FALSE</td></tr>
    <tr><td>\ref DCPS_QoS_Deadline "DEADLINE"</td><td>Period = infinite</td></tr>
    <tr><td>\ref DCPS_QoS_LatencyBudget "LATENCY_BUDGET"</td><td>duration = 0</td></tr>
    <tr><td>\ref DCPS_QoS_Ownership "OWNERSHIP"</td><td>SHARED</td></tr>
    <tr><td>\ref DCPS_QoS_Liveliness "LIVELINESS"</td><td>kind = AUTOMATIC<br>
                                                          lease_duration = 0</td></tr>
    <tr><td>\ref DCPS_QoS_TimeBasedFilter "TIME_BASED_FILTER"</td><td>minimum_separation = 0</td></tr>
    <tr><td>\ref DCPS_QoS_Partition "PARTITION"</td><td>__BUILT-IN PARTITION__</td></tr>
    <tr><td>\ref DCPS_QoS_Reliability "RELIABILITY"</td><td>kind = RELIABLE<br>
                                                            max_blocking_time = 100 milliseconds<br>
                                                            synchronous = FALSE</td></tr>
    <tr><td>\ref DCPS_QoS_DestinationOrder "DESTINATION_ORDER"</td><td>BY_RECEPTION_TIMESTAMP
    <tr><td>\ref DCPS_QoS_History "HISTORY"</td><td>kind = KEEP_LAST<br>
                                                    depth = 1</td></tr>
    <tr><td>\ref DCPS_QoS_ResourceLimits "RESOURCE_LIMITS"</td><td>max_samples = LENGTH_UNLIMITED<br>
                                                                   max_instances = LENGTH_UNLIMITED<br>
                                                                   max_samples_per_instance = LENGTH_UNLIMITED</td></tr>
    <tr><td>\ref DCPS_QoS_ReaderDataLifecycle "READER_DATA_LIFECYCLE"</td><td>autopurge_nowriter_samples_delay = infinite<br>
                                                                              autopurge_disposed_samples_delay = infinite<br>
                                                                              invalid_sample_visibility = MINIMUM_INVALID_SAMPLES</td></tr>
    <tr><td>\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY"</td><td>autoenable_created_entities = TRUE</td></tr>
    <tr><td>SHARE (proprietary)</td><td>enable = FALSE<br>
                                                              name = NULL</td></tr>
    <tr><td>READER_DATA_LIFESPAN (proprietary)</td><td>used = FALSE<br>
                                                                                          duration = INFINITE</td></tr>
    <tr><td>USER_KEY (proprietary)</td><td>enable = FALSE<br>
                                                                   expression = NULL</td></tr>
</table>

Built-in entities have default listener settings as well. The built-in Subscriber and all
of its built-in Topics have nil listeners with all statuses appearing in their listener
masks. The built-in DataReaders have nil listeners with no statuses in their masks.

The information that is accessible about the remote entities by means of the built-in
topics includes all the QoS policies that apply to the corresponding remote Entity.
The QoS policies appear as normal 'data' fields inside the data read by means of
the built-in Topic. Additional information is provided to identify the Entity and
facilitate the application logic.

The tables below list the built-in topics, their names, and the additional information
(beyond the QoS policies that apply to the remote entity) that appears in the data
associated with the built-in topic.

ParticipantBuiltinTopicData                                                    {#DCPS_Builtin_Topics_ParticipantData}
---------------------------

The DCPSParticipant topic communicates the existence of DomainParticipants
by means of the ParticipantBuiltinTopicData datatype. Each
ParticipantBuiltinTopicData sample in a Domain represents a
DomainParticipant that participates in that Domain: a new
ParticipantBuiltinTopicData instance is created when a newly added
DomainParticipant is enabled, and it is disposed when that DomainParticipant is
deleted. An updated ParticipantBuiltinTopicData sample is written each
time the DomainParticipant modifies its UserDataQosPolicy.

<table>
    <caption>ParticipantBuiltinTopicData Members</caption>
    <tr>
        <th>Name</th><th>Type</th><th>Description</th>
    </tr>
    <tr><td>key</td><td>BuiltinTopicKey_t</td><td>Globally unique identifier of the participant</td></tr>
    <tr><td>user_data</td><td>UserDataQosPolicy</td><td>User-defined data attached to the participant via a QosPolicy</td></tr>
</table>

\if isocpp2
See also dds::topic::ParticipantBuiltinTopicData.
\endif

TopicBuiltinTopicData                                                          {#DCPS_Builtin_Topics_TopicData}
---------------------

The DCPSTopic topic communicates the existence of topics by means of the
TopicBuiltinTopicData datatype. Each TopicBuiltinTopicData sample in
a Domain represents a Topic in that Domain: a new TopicBuiltinTopicData
instance is created when a newly added Topic is enabled. However, the instance is
not disposed when a Topic is deleted by its participant because a topic lifecycle is
tied to the lifecycle of a Domain, not to the lifecycle of an individual participant.
An updated TopicBuiltinTopicData sample is written each time a Topic modifies one or
more of its QosPolicy values.

Information published in the DCPSTopicTopic is critical to the data distribution
service, therefore it cannot be disabled by means of the Domain/BuiltinTopics
element in the configuration file.

<table>
    <caption>TopicBuiltinTopicData Members</caption>
    <tr>
        <th>Name</th><th>Type</th><th>Description</th>
    </tr>
    <tr><td>key</td>               <td>BuiltinTopicKey_t</td>         <td>Global unique identifier of the Topic</td></tr>
    <tr><td>name</td>              <td>String</td>                    <td>Name of the Topic</td></tr>
    <tr><td>type_name</td>         <td>String</td>                    <td>Type name of the Topic (i.e. the fully scoped IDL name)</td></tr>
    <tr><td>durability</td>        <td>DurabilityQosPolicy</td>       <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>durability_service</td><td>DurabilityServiceQosPolicy</td><td>QosPolicy attached to the Topic</td></tr>
    <tr><td>deadline</td>          <td>DeadlineQosPolicy</td>         <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>latency_budget</td>    <td>LatencyBudgetQosPolicy</td>    <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>liveliness</td>        <td>LivelinessQosPolicy</td>       <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>reliability</td>       <td>ReliabilityQosPolicy</td>      <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>transport_priority</td><td>TransportPriorityQosPolicy</td><td>QosPolicy attached to the Topic</td></tr>
    <tr><td>lifespan</td>          <td>LifespanQosPolicy</td>         <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>destination_order</td> <td>DestinationOrderQosPolicy</td> <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>history</td>           <td>HistoryQosPolicy</td>          <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>resource_limits</td>   <td>ResourceLimitsQosPolicy</td>   <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>ownership</td>         <td>OwnershipQosPolicy</td>        <td>QosPolicy attached to the Topic</td></tr>
    <tr><td>topic_data</td>        <td>TopicDataQosPolicy</td>        <td>QosPolicy attached to the Topic</td></tr>
</table>

\if isocpp2
See also dds::topic::TopicBuiltinTopicData.
\endif

PublicationBuiltinTopicData                                                    {#DCPS_Builtin_Topics_PublicationData}
---------------------------

The DCPSPublication topic communicates the existence of datawriters by means
of the PublicationBuiltinTopicData datatype. Each
PublicationBuiltinTopicData sample in a Domain represents a datawriter in
that Domain: a new PublicationBuiltinTopicData instance is created when a
newly added DataWriter is enabled, and it is disposed when that DataWriter is
deleted. An updated PublicationBuiltinTopicData sample is written each
time the DataWriter (or the Publisher to which it belongs) modifies a QosPolicy that
applies to the entities connected to it. Also will it be updated when the writer looses
or regains its liveliness.

\if isocpp2
The PublicationBuiltinTopicData Topic is also used to return data through the
\ref dds::sub::matched_publication_data(const dds::sub::DataReader<T>& dr, const ::dds::core::InstanceHandle& h)
"dds::sub::matched_publications_data" operation.
\else
The PublicationBuiltinTopicData Topic is also used to return data through
the get_matched_publication_data operation on the DataReader.
\endif

<table>
    <caption>PublicationBuiltinTopicData Members</caption>
    <tr>
        <th>Name</th><th>Type</th><th>Description</th>
    </tr>
    <tr><td>key</td><td>BuiltinTopicKey_t</td><td>Global unique identifier of the DataWriter</td></tr>
    <tr><td>participant_key</td><td>BuiltinTopicKey_t</td><td>Global unique identifier of the Participant to which the DataWriter belongs</td></tr>
    <tr><td>topic_name</td><td>String</td><td>Name of the Topic used by the DataWriter</td></tr>
    <tr><td>type_name</td><td>String</td><td>Type name of the Topic used by the DataWriter</td></tr>
    <tr><td>durability</td><td>DurabilityQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>deadline</td><td>DeadlineQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>latency_budget</td><td>LatencyBudgetQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>liveliness</td><td>LivelinessQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>reliability</td><td>ReliabilityQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>lifespan</td><td>LifespanQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>destination_order</td><td>DestinationOrderQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>user_data</td><td>UserDataQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>ownership</td><td>OwnershipQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>ownership_strength</td><td>OwnershipStrengthQosPolicy</td><td>QosPolicy attached to the DataWriter</td></tr>
    <tr><td>presentation</td><td>PresentationQosPolicy</td><td>QosPolicy attached to the Publisher to which the DataWriter belongs</td></tr>
    <tr><td>partition</td><td>PartitionQosPolicy</td><td>QosPolicy attached to the Publisher to which the DataWriter belongs</td></tr>
    <tr><td>topic_data</td><td>TopicDataQosPolicy</td><td>QosPolicy attached to the Topic used by the DataWriter</td></tr>
    <tr><td>group_data</td><td>GroupDataQosPolicy</td><td>QosPolicy attached to the Publisher to which the DataWriter belongs</td></tr>
</table>

\if isocpp2
See also dds::topic::PublicationBuiltinTopicData.
\endif

SubscriptionBuiltinTopicData                                                   {#DCPS_Builtin_Topics_SubscriptionData}
----------------------------

The DCPSSubscription topic communicates the existence of datareaders by
means of the SubscriptionBuiltinTopicData datatype. Each
SubscriptionBuiltinTopicData sample in a Domain represents a datareader
in that Domain: a new SubscriptionBuiltinTopicData instance is created
when a newly added DataReader is enabled, and it is disposed when that
DataReader is deleted. An updated SubscriptionBuiltinTopicData sample is
written each time the DataReader (or the Subscriber to which it belongs) modifies a
QosPolicy that applies to the entities connected to it.

\if isocpp2
The SubscriptionBuiltinTopicData Topic is also used to return data through the
\ref dds::pub::matched_subscription_data(const dds::pub::DataWriter<T>& dw, const ::dds::core::InstanceHandle& h)
"dds::pub::matched_subscriptions_data" operation.
\else
The SubscriptionBuiltinTopicData Topic is also used to return data through
the get_matched_subscription_data operation on the DataWriter.
\endif

<table>
    <caption>SubscriptionBuiltinTopicData Members</caption>
    <tr>
        <th>Name</th><th>Type</th><th>Description</th>
    </tr>
    <tr><td>key</td><td>BuiltinTopicKey_t</td><td>Global unique identifier of the DataReader</td></tr>
    <tr><td>participant_key</td><td>BuiltinTopicKey_t</td><td>Global unique identifier of the Participant to which the DataReader belongs</td></tr>
    <tr><td>topic_name</td><td>String</td><td>Name of the Topic used by the DataReader</td></tr>
    <tr><td>type_name</td><td>String</td><td>Type name of the Topic used by the DataReader</td></tr>
    <tr><td>durability</td><td>DurabilityQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>deadline</td><td>DeadlineQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>latency_budget</td><td>LatencyBudgetQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>liveliness</td><td>LivelinessQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>reliability</td><td>ReliabilityQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>ownership</td><td>LifespanQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>destination_order</td><td>DestinationOrderQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>user_data</td><td>UserDataQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>time_based_filter</td><td>TimeBasedFilterQosPolicy</td><td>QosPolicy attached to the DataReader</td></tr>
    <tr><td>presentation</td><td>PresentationQosPolicy</td><td>QosPolicy attached to the Subscriber to which the DataReader belongs</td></tr>
    <tr><td>partition</td><td>PartitionQosPolicy</td><td>QosPolicy attached to the Subscriber to which the DataReader belongs</td></tr>
    <tr><td>topic_data</td><td>TopicDataQosPolicy</td><td>QosPolicy attached to the Topic used by theDataReader</td></tr>
    <tr><td>group_data</td><td>GroupDataQosPolicy</td><td>QosPolicy attached to the Subscriber to which the DataReader belongs</td></tr>
</table>

\if isocpp2
See also dds::topic::SubscriptionBuiltinTopicData.
\endif

Other built-in topics                                                          {#DCPS_Builtin_Topics_Others}
---------------------

There are a number of other built-in topics that have not been mentioned. These
topics (e.g. DCPSDelivery, DCPSHeartbeat and potentially some others) are
proprietary and for internal use only. Users are discouraged from doing anything
with these topics, so as not to interfere with internal mechanisms that rely on them.
The structure of these topics may change without notification.

