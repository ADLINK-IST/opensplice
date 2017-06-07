Release Notes                                              {#release_notes}
====================

[TOC]

Available in 6.5.1                                                                              {#release_notes_10}
====================
- Fixed implementation of the CoherentSet and CoherentAccess class, which will provided topic, 
  instance or group coherency. 
 
Available in 6.4.3p1                                                                            {#release_notes_9}
===============
- Added new ISO C++ Streams DCPS API and documentation. See \ref streams_mainpage for more information.

Available in 6.4.2p1                                                                            {#release_notes_8}
===============
- Improved ISO C++ documentation layout and content.

Available in 6.4.1p1                                                                            {#release_notes_7}
================
- Added operator= overload to dds::sub::Sample.

Available in 6.4.0p4                                                                            {#release_notes_6}
====================
- [QoS Provider](#release_notes_qosprovider640p4)
- [Shared Samples](#release_notes_sharedsamples640p4)

QoS Provider                                                                      {#release_notes_qosprovider640p4}
----------------------------------------
The QoS provider API allows users to extract (by name) pre-configured QoS policy values from an XML resource file,
so that these QoS values can be used by their applications to create DDS entities without the need to hard-code their QoS settings into the application code.

- Construct a QosProvider
~~~~~~~~~~~~~~~{.cpp}
                            //URI                           //Profile
dds::core::QosProvider qp("file:///home/user/defaults.xml", "DefaultQosProfile");
// OR
dds::core::QosProvider qp("file:///home/user/defaults.xml");
~~~~~~~~~~~~~~~

- Using QosProvider
~~~~~~~~~~~~~~~{.cpp}
dds::domain::qos::DomainParticipantQos dpq  = qp.participant_qos();
// OR
dds::domain::qos::DomainParticipantQos dpq  = qp.participant_qos("DefaultQosProfile::DerivedQos");

dds::sub::qos::SubscriberQos sq = qp.subscriber_qos();
dds::pub::qos::PublisherQos pq = qp.publisher_qos("DefaultQosProfile::DerivedQos");
dds::sub::qos::DataReaderQos drq = qp.datareader_qos();
dds::pub::qos::DataWriterQos dwq = qqp.datawriter_qos("Qos")
~~~~~~~~~~~~~~~

Shared Samples                                                                     {#release_notes_sharedsamples640p4}
----------------------------------------
The SharedSamples class allows data that has been taken or read into a LoanedSamples object
to persist even when the LoanedSamples object has gone out of scope. Usually when a LoanedSamples
object goes out of scope the containing data loan would be returned however, passing it into a
SharedSamples object created in a parent scope will prevent this.

- Using SharedSamples
~~~~~~~~~~~~~~~{.cpp}
dds::sub::SharedSamples<SAMPLE> sharedSamples;
{
    dds::sub::LoanedSamples<SAMPLE> loanedSamples = reader.read();
    sharedSamples = loanedSamples;
}
//loanedSamples is now out of scope but it's data is still contained in sharedSamples at this point
~~~~~~~~~~~~~~~

Available in 6.4.0                                                                            {#release_notes_5}
====================
- [New IDL type mapping](#release_notes_newtypes640)
- [Find functions, built-in topics](#release_notes_findandlookups640)
- [Entity operations](#release_notes_entity640)

New IDL type mapping                                                 {#release_notes_newtypes640}
----------------------------------------
Introduced the new DDS-PSM-CXX specification IDL type mapping. This allows simplified usage of IDL generated
types through full compatibility with the STL. All ISO C++ example code has been updated to demonstrate the new
usage with the exception of built-in topics (see known issues).

The table below shows the old generated types and their equivalent in the new mapping:

IDL Type                | Old mapping             | New mapping
------------------------|-------------------------|------------------------
struct                  | struct                  | class
sequence<T>             | IDL to C++ sequence     | std::vector<T>
array                   | C++ array               | dds::core::array<T, n>
boolean                 | unsigned char           | bool
string                  | const char*             | std::string

Find functions, Built-in topics                                                 {#release_notes_findandlookups640}
----------------------------------------
Added support for obtaining the built-in Subscriber as well as the ability to find the DataReaders
of a Subscriber based on Topic name. This allows retrieval of the built-in DataReaders.

- dds::sub::builtin_subscriber(dds::domain::DomainParticipant)
    - Retrieves the built-in Subscriber for the specified domain
- dds::sub::find(various parameters)
    - Retrieves a DataReader from a Subscriber with a specified Topic name. Supplying the built-in
    subscriber will allow retrieval of the built-in DataReaders.
- dds::domain::find(uint32_t id)
    - Retrieves the DomainParticipant with the specified id.
- dds::pub::find(various parameters)
    - Retrieves a DataWriter from a Publisher with a specified Topic name.
- dds::topic::find(const dds::domain::DomainParticipant& dp, const std::string& topic_name)
    - Retrieves a Topic from a DomainParticipant with a specified Topic name.


Entity operations                                                                      {#release_notes_entity640}
------------------------
Added support for the following operations on Entities:

- retain()
    - Indicates that an Entity should be retained in memory even if it goes out of scope. Can later be retrieved using the find api. Calling close() on the Entity will override retain().

Available in 6.3.2                                                                            {#release_notes_4}
====================
- [Performance enhancements](#release_notes_performance632)
- [Support for Sun studio](#release_notes_sunstudio632)
- [Entity operations](#release_notes_entity632)

Performance enhancements                                                                      {#release_notes_performance632}
------------------------
Several improvements have been made to performance of WaitSets, as well as read and take operations on DataReader, up to 40% faster than in 6.3.1

Support for Sun studio                                                                      {#release_notes_sunstudio632}
------------------------
ISO C++ can now compile on Sun studio and has been tested with version 5.12

Entity operations                                                                      {#release_notes_entity632}
------------------------
Added support for the following operations on Entities

- close()
    - Manually close a Entity
- enable()
    - Manually enable Entity if its been created with dds::core::policy::EntityFactory::ManuallyEnable()

Available in 6.3.1                                                                            {#release_notes_3}
====================
- [Available Listeners](#release_notes_supported_listeners)
- [Available Waitsets](#release_notes_supported_waitsets)
- [Filters and Queries](#release_notes_filters_and_queries)
- [DataReader](#release_notes_datareader631)
- [DataWriter](#release_notes_datawriter631)

Available Listeners                                                                {#release_notes_supported_listeners}
--------------------

Component   | Name
------------|------------------------
DataWriter  | on_offered_deadline_missed
DataWriter  | on_offered_incompatible_qos
DataWriter  | on_liveliness_lost
DataWriter  | on_publication_matched
DataReader  | on_requested_deadline_missed
DataReader  | on_requested_incompatible_qos
DataReader  | on_sample_rejected
DataReader  | on_liveliness_changed
DataReader  | on_data_available
DataReader  | on_subscription_matched


Available WaitSet Conditions                                                      {#release_notes_supported_waitsets}
--------------------------

Component   | Name
------------|------------------------
DataReader  | dds::sub::cond::ReadCondition
DataReader  | dds::sub::cond::QueryCondition
DataReader  | dds::core::cond::StatusCondition
DataReader  | dds::core::cond::GuardCondition
DataWriter  | dds::sub::cond::ReadCondition
DataWriter  | dds::sub::cond::QueryCondition
DataWriter  | dds::core::cond::StatusCondition
DataWriter  | dds::core::cond::GuardCondition


Filters and Queries                                                              {#release_notes_filters_and_queries}
-----------------------
- Content Filtered Topics
~~~~~~~~~~~~~~~{.cpp}
dds::topic::Topic<Foo::Bar> topic(dp, "CFTopic", topicQos);
dds::topic::Filter filter = "Some String to filter"
dds::topic::ContentFilteredTopic<Foo::Bar> cftopic(topic, "CFFooBar", filter);
dds::sub::DataReader<Foo::Bar> dr(sub, cftopic, drqos);
~~~~~~~~~~~~~~~

- Query ReadCondtion
~~~~~~~~~~~~~~~{.cpp}
std::vector<std::string> params;
params.push_back("0");
dds::sub::Query query(reader, "long_1 > %0", params);
dds::sub::cond::QueryCondition queryCond(query, anyDataState);
~~~~~~~~~~~~~~~

DataReader                                                                      {#release_notes_datareader631}
------------------------
- Instance handle registration/de-registration
- Selector e.g.
~~~~~~~~~~~~~~~{.cpp}
dds::core::InstanceHandle Sinstance(reader.lookup_instance(someSample));
dds::sub::LoanedSamples<SAMPLE> ddssamples = reader.select().
                                              instance(Sinstance).
                                              state(dds::sub::status::DataState::any_data())
                                              .read();
~~~~~~~~~~~~~~~

- User provided types/containers -
    - Forward Inserting Iterator (any iterable type)
~~~~~~~~~~~~~~~{.cpp}
          std::vector<dds::sub::Sample<SAMPLE> > ddssamples;
          ddssamples.resize(MAXSAMPLES);
          uint32_t readSamples = reader.read(ddssamples.begin(), MAXSAMPLES);
~~~~~~~~~~~~~~~
    - Backwards Inserting Iterator (any iterable type with push_back function)
~~~~~~~~~~~~~~~{.cpp}
std::vector<dds::sub::Sample<SAMPLE> > ddssamples;
ddssamples.resize(MAXSAMPLES);
uint32_t readSamples = reader.read(back_inserter(ddssamples));
~~~~~~~~~~~~~~~

DataWriter                                                                      {#release_notes_datawriter631}
------------------------
- Instance handle registration/de-registration
- User provided types/containers -
    - Forward Inserting Iterator (any iterable type)
~~~~~~~~~~~~~~~{.cpp}
dw.write(samples.begin(), samples.end());
~~~~~~~~~~~~~~~

Available in 6.3.0                                                                    {#release_notes_2}
====================

- [Available Qos](#release_notes_supported_qos)
- [Provided types](#release_notes_new_types)
- [Fluent accessors](#release_notes_new_accessors)
- [DataReader](#release_notes_datareader)
- [DataWriter](#release_notes_datawriter)
- [Content Filtered Topics] (#release_notes_contentfilteredtopics)

Available QoS                                             {#release_notes_supported_qos}
--------------------

QoS                                  | Kind values
-------------------------------------|-----------
dds::core::policy::Durability        | DurabilityKind:: TRANSIENT_LOCAL, TRANSIENT, PERSISTENT
dds::core::policy::DurabilityService | N/A
dds::core::policy::Presentation      | PresentationAccessScopeKind:: GROUP, INSTANCE, TOPIC
dds::core::policy::Deadline          | N/A
dds::core::policy::LatencyBudget     | N/A
dds::core::policy::Ownership         | OwnershipKind:: EXCLUSIVE, SHARED
dds::core::policy::OwnershipStrength | N/A
dds::core::policy::Liveliness        | LivelinessKind:: AUTOMATIC, MANUAL_BY_PARTICIPANT, MANUAL_BY_PARTICIPANT
dds::core::policy::TimeBasedFilter   | N/A
dds::core::policy::Partition         | N/A
dds::core::policy::Reliability       | ReliabilityKind:: BEST_EFFORT, RELIABLE
dds::core::policy::TransportPriority | N/A
dds::core::policy::Lifespan          | N/A
dds::core::policy::DestinationOrder  | DestinationOrderKind:: BY_RECEPTION_TIMESTAMP, BY_SOURCE_TIMESTAMP
dds::core::policy::History           | HistoryKind:: KEEP_ALL, KEEP_LAST
dds::core::policy::ResourceLimits    | N/A
dds::core::policy::EntityFactory     | N/A

Provided types                                                                  {#release_notes_new_types}
------------------------
- dds::sub::Sample
~~~~~~~~~~~~~~~{.cpp}
std::vector<dds::sub::Sample<SAMPLE> > ddssamples;
~~~~~~~~~~~~~~~
- dds::sub::LoanedSamples
~~~~~~~~~~~~~~~{.cpp}
dds::sub::LoanedSamples<SAMPLE> ddssamples = reader.take();
~~~~~~~~~~~~~~~
- dds::core::Time
~~~~~~~~~~~~~~~{.cpp}
dds::core::Time t(10,12345)
~~~~~~~~~~~~~~~
- dds::core::Duration
~~~~~~~~~~~~~~~{.cpp}
dds::core::Duration d(10,12345)
~~~~~~~~~~~~~~~

Fluent accessors                                                                {#release_notes_new_accessors}
------------------------
~~~~~~~~~~~~~~~{.cpp}
DataWriterQos dwqos;
dw >> dwqos;
dwqos << History::KeepAll(200)
            << dwqos.policy<ResourceLimits>()
                     .max_samples(p)
                     .max_instances(q)
                     .max_samples_per_instance(r);
dw << dwqos;
~~~~~~~~~~~~~~~

<br/>

~~~~~~~~~~~~~~~{.cpp}
dds::core::InstanceHandle Sinstance(reader.lookup_instance(someSample));
dds::sub::LoanedSamples<SAMPLE> ddssamples = reader.select().
					      instance(Sinstance).
					      state(dds::sub::status::DataState::any_data())
					      .read();
~~~~~~~~~~~~~~~

DataReader                                                                      {#release_notes_datareader}
------------------------
- Read with >> operator
~~~~~~~~~~~~~~~{.cpp}
dds::sub::LoanedSamples<SAMPLE> samples;
dr >> samples;
//OR
dr >> dds::sub::read >> samples;
~~~~~~~~~~~~~~~

- Take with >> operator
~~~~~~~~~~~~~~~{.cpp}
dds::sub::LoanedSamples<SAMPLE> samples;
dr >> dds::sub::take >> samples;
~~~~~~~~~~~~~~~

DataWriter                                                                      {#release_notes_datawriter}
------------------------
- Write with << operator
~~~~~~~~~~~~~~~{.cpp}
dw << sample;
~~~~~~~~~~~~~~~

Known Issues and Currently Unsupported Features                                                                    {#release_notes_known_issues}
=====================================

Although the ISO C++ DCPS API implements the majority of commonly used features, some functionality
of the OMG DDS specification and DDS-PSM-Cxx is not currently supported. Below is a list of known
issues and features that are not currently supported:

- [Listeners](#release_notes_us_listeners)
- [Find and Discovery functions](#release_notes_us_find_discovery)
- [New IDL type mapping for built-in topics](#release_notes_us_IDLtypemapping)
- [OpenSplice API extensions](#release_notes_us_extensions)
- [Other known issues] (#release_notes_other_known_issues)


Unsupported Listeners                                                                   {#release_notes_us_listeners}
---------------------
The listeners on the following entities are not currently supported:

- DomainParticipant
- Publisher
- Subscriber
- Topic

Find and Discovery functions                                                {#release_notes_us_find_discovery}
-------------------------
Currently, when using the find api to retrieve DataReaders or DataWriters based on their topic name,
only a single DataReader or DataWriter will be returned even though there may be more than one. It
is not specified which one will be returned. A future release will address this and allow multiple
DataReaders and DataWriters to be returned. As well as this the following discovery functions are
not currently supported:

- [dds/domain/discovery.hpp](@ref dds/domain/discovery.hpp)
- [dds/topic/discovery.hpp](@ref dds/topic/discovery.hpp)

New IDL type mapping for built-in topics                                                             {#release_notes_us_IDLtypemapping}
------------------------------------
The _new_ mapping of primitives and container types in the DDS-PSM-CXX specification (7.4.2) is not yet implemented for built-in topics. The DataTypes
generated for these still follow the original OMG IDL to C++ mapping specification.

OpenSplice API extensions                                                                        {#release_notes_us_extensions}
-------------------------
The implementation contains only API features that are present in the OMG DCPS specification at this time.
Other OpenSplice language bindings contain extensions to the specification such as additional QoS,
Entity operations, and Entities. These features are not yet available.

Other Known Issues                                                                        {#release_notes_other_known_issues}
------------------
- dds::sub::DataReader<T> >> dds::sub::DataReaderQos
    - Attempting to extract a QoS from a DataReader using the >> operator will cause an exception.

- dds::sub::AnyDataReader::parent()
    - The above function is not currently supported.

- dds::topic::MultiTopic
    - MultiTopics are not currently supported.

- dds::core::policy::UserData, dds::core::policy::GroupData, dds::core::policy::TopicData
    - The above policy classes are only partially supported. For each class, the constructor that
    takes either two pointers or two iterators is not currently supported, nor are the begin or end
    functions if present.

- Release/Debug DLL mismatch issue
    - In order to build applications in DEBUG mode on Windows the ISO C++ library must match, this can be done by recompiling the custom lib as documented @ref isocpp_customlibs "here"
