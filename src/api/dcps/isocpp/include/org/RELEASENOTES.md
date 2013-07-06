Release Notes                                              {#release_notes}
====================

[TOC]

Supported DDS features                                                          {#release_notes_toc}
=======================

- [Available Qos](#release_notes_supported_qos)
- [Available Listeners (new in 6.3.1)](#release_notes_supported_listeners)
- [Available Waitsets (new in 6.3.1)](#release_notes_supported_waitsets)
- [Filters and Querys (new in 6.3.1)](#release_notes_filters_and_querys)


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


Available Waitset Conditions                                                      {#release_notes_supported_waitsets}
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


Filter and Queries                                                              {#release_notes_filters_and_querys}
-----------------------
- Content Filtered Topics
~~~~~~~~~~~~~~~{.cpp}
dds::topic::Topic<Foo::Bar> topic(dp, "CFTopic", topicQos);
dds::topic::Filter filter = "Some String to filter"
dds::topic::ContentFilteredTopic<Foo::Bar> cftopic(topic, "CFFooBar", filter);
dds::sub::DataReader<Foo::Bar> dr(sub, cftopic, drqos);
~~~~~~~~~~~~~~~

- QueryReadCondtion
~~~~~~~~~~~~~~~{.cpp}
std::vector<std::string> params;
params.push_back("0");
dds::sub::Query query(reader, "long_1 > %0", params);
dds::sub::cond::QueryCondition queryCond(query, anyDataState);
~~~~~~~~~~~~~~~




Available in 6.3.0                                                                    {#release_notes_2}
====================

- [Provided types](#release_notes_new_types)
- [Fluent accessors](#release_notes_new_accessors)
- [DataReader](#release_notes_datareader)
- [DataWriter](#release_notes_datawriter)
- [Content Filtered Topics] (#release_notes_contentfilteredtopics)

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
- Read methods with >> operator
~~~~~~~~~~~~~~~{.cpp}
dds::sub::LoanedSamples<SAMPLE> samples;
dr >> samples;
//OR
dr >> dds::sub::read >> samples;
~~~~~~~~~~~~~~~

- Take methods with >> operator
~~~~~~~~~~~~~~~{.cpp}
dds::sub::LoanedSamples<SAMPLE> samples;
dr >> dds::sub::take >> samples;
~~~~~~~~~~~~~~~

DataWriter                                                                      {#release_notes_datawriter}
------------------------
- write methods with and << operator
~~~~~~~~~~~~~~~{.cpp}
dw << sample;
~~~~~~~~~~~~~~~




Available in 6.3.1                                                                            {#release_notes_3}
====================
- [DataReader](#release_notes_datareader631)
- [DataWriter](#release_notes_datawriter631)

DataReader                                                                      {#release_notes_datareader631}
------------------------
- Instance handle registration/de-registration
- Selector eg.
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
    - Backwards Inserting Iterator (any iterable type with push_back method)
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






Unsupported Features                                                                    {#release_notes_4}
====================

As ISO C++ DCPS is currently in a beta stage some functionality of the OMG DDS specification and DDS-PSM-Cxx are not covered, below is a list of features that might be expected but are not currently supported.

- [Listeners](#release_notes_us_listeners)
- [Find and lookups](#release_notes_us_findandlookups)
- [IDL type mapping](#release_notes_us_IDLtypemapping)

Unsupported Listeners                                                                   {#release_notes_us_listeners}
----------------------
The listener methods on the following entities are not yet implemented

- Domain Participant
- Publisher
- Subscriber
- Topic

Find and lookups                                                                       {#release_notes_us_findandlookups}
----------------------
Find and lookup methods on some of the entities are not yet implemented

- Domain Participant
    - lookup_topic_description
    - lookup_participant
    - find_topic

- Publisher
    - lookup_datawriter

IDL type mapping                                                                        {#release_notes_us_IDLtypemapping}
----------------------
Although mappings exist and are usable, the full mapping of primitives and container types in the DDS-PSM-CXX specification (7.4.2) are not yet implemented.
