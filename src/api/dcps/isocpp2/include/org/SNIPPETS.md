ISO C++ example code snippets                                                          {#snippets}
=============================

Below you will find an assortment of small code examples which show you how to do various
things with the ISO C++ API. Most of the snippets are written in a way that they would be compilable
as one program and as such concepts already demonstrated are not repeated in subsequent snippets.

Using the DataWriter                                                                   {#datawriter}
====================

Creating a DataWriter with default QoS
--------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
    dds::pub::Publisher pub(dp);
    dds::topic::Topic<Foo::Type> topic(dp, "TypeTopic");
    dds::pub::DataWriter<Foo::Type> writer(pub, topic);
~~~~~~~~~~~~~~~
Creating a DataWriter with your own QoS
---------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::pub::qos::DataWriterQos dwqos = topic.qos();
    dwqos << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances();
    dds::pub::DataWriter<Foo::Type> writer2(pub, topic, dwqos);
~~~~~~~~~~~~~~~
Getting and setting DataWriter QoS
----------------------------------
~~~~~~~~~~~~~~~{.cpp}
    writer >> dwqos;
    dwqos << dds::core::policy::Deadline(dds::core::Duration::from_microsecs(10))
        << dds::core::policy::Durability::Volatile();

    writer << dwqos;
    //or
    writer.qos(dwqos);
~~~~~~~~~~~~~~~
Writing a sample
----------------
~~~~~~~~~~~~~~~{.cpp}
    Foo::Type sample;
    sample.long_1()=1;
    sample.long_2()=2;

    writer << sample;
    //or
    writer.write(sample);
~~~~~~~~~~~~~~~
Writing with other items (timestamps, instance handles)
-------------------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::core::Time t1(20,0);
    writer.write(sample, t1);
    //or
    writer << std::make_pair(sample, t1);
~~~~~~~~~~~~~~~
Writing from your own container
-------------------------------
~~~~~~~~~~~~~~~{.cpp}
    std::vector<Foo::Type> userSamples;
    userSamples.push_back(sample);
    writer.write(userSamples.begin(), userSamples.end());
~~~~~~~~~~~~~~~
Writing from your own samples container with your own instance handles
----------------------------------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    std::vector<dds::core::InstanceHandle> handles;
    handles.push_back(writer.register_instance(sample));
    writer.write(userSamples.begin(), userSamples.end(), handles.begin(), handles.end());
~~~~~~~~~~~~~~~
Register and instance handle and a write sample using it
--------------------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::core::InstanceHandle ih(writer.register_instance(sample));
    writer << std::make_pair(sample, ih);
~~~~~~~~~~~~~~~
Using the DataReader                                                                   {#datareader}
====================

Creating a DataReader with the default QoS
------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::sub::Subscriber sub(dp);
    dds::sub::DataReader<Foo::Type> reader(sub, topic);
~~~~~~~~~~~~~~~
Creating a DataReader with your own QoS
---------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::sub::qos::DataReaderQos drqos(topic.qos());
    drqos << dds::core::policy::ResourceLimits(1000);
    dds::sub::DataReader<Foo::Type> reader2(sub, topic, drqos);
~~~~~~~~~~~~~~~
Reading a sample
----------------
~~~~~~~~~~~~~~~{.cpp}
    dds::sub::LoanedSamples<Foo::Type> samples = reader.read();
    //Or
    reader >> samples;
~~~~~~~~~~~~~~~
Reading samples into your own containers (Forward inserting Iterator)
---------------------------------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    std::vector<dds::sub::Sample<Foo::Type> > mySamples;
    mySamples.resize(30);
    unsigned int numRead = reader.read(mySamples.begin(), 30);
~~~~~~~~~~~~~~~
Reading samples into your own containers (Back inserting Iterator)
------------------------------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    numRead = reader.read(back_inserter(mySamples));
~~~~~~~~~~~~~~~
Reading with a InstanceHandle (lookup key from sample)
------------------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    ih = reader.lookup_instance(sample);
    samples = reader.select().
        instance(ih).
        state(dds::sub::status::DataState::any_data())
        .read();
~~~~~~~~~~~~~~~
Reading samples with the Selector
---------------------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::sub::LoanedSamples<Foo::Type> FooSamples = reader.select()
        .instance(ih)
        .state(dds::sub::status::DataState::any_data())
        .read();
~~~~~~~~~~~~~~~
Reading samples with the Manipulator Selector
---------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    reader >> dds::sub::instance(ih) >> dds::sub::state(dds::sub::status::DataState::any_data()) >> samples;

    //Take
    reader.take();
    //Or with the ManipulatorSelector
    reader >> dds::sub::take >> samples;
~~~~~~~~~~~~~~~
Accessing the samples
---------------------
~~~~~~~~~~~~~~~{.cpp}
    for (dds::sub::LoanedSamples<Foo::Type>::const_iterator sample = samples.begin();
        sample < samples.end(); ++sample)
    {
        if (sample->info().valid())
        {
            std::cout << sample->data().long_1() << std::endl;
        }
    }
~~~~~~~~~~~~~~~
Accessing the samples (C++11)
-----------------------------
~~~~~~~~~~~~~~~{.cpp}
    for(auto s : reader.read())
    {
        std::cout << s->data().long_1() << std::endl;
    }
~~~~~~~~~~~~~~~
Accessing the samples (C++11 lambda)
------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    auto s = reader.read();
    std::for_each(s.begin(), s.end(), [&]{ std::cout << s->data().long_1() << std::endl; });
~~~~~~~~~~~~~~~
Miscellaneous                                                                       {#miscellaneous}
=============
Getting QoS values
----------------------
~~~~~~~~~~~~~~~{.cpp}
    dds::core::Duration lease_duration = drqos.policy<dds::core::policy::Liveliness>().lease_duration();
~~~~~~~~~~~~~~~
Finding entities (retained or built in)
--------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    //Finding one DataReader (Forward inserting iterator)
    std::vector<dds::sub::AnyDataReader> rv;
    rv.push_back(dds::sub::AnyDataReader(dds::core::null));
    dds::sub::find<dds::sub::DataReader<Foo::Type> >(sub, topic.name(), rv.begin(), 1);
    samples = dds::sub::DataReader<Foo::Type>(rv[0]).take();

    //Finding all the DataReaders (Back inserting iterator)
    dds::sub::find<dds::sub::DataReader<Foo::Type> >(sub, topic.name(), back_inserter(rv));
    dds::sub::DataReader<Foo::Type> adr(rv[0]);
    adr >> dds::sub::take >> samples;
~~~~~~~~~~~~~~~
Using a subset of listeners
---------------------------
~~~~~~~~~~~~~~~{.cpp}
    class ExampleDataReaderListener :
        public virtual dds::sub::DataReaderListener<Foo::Type>,
        public virtual dds::sub::NoOpDataReaderListener<Foo::Type>
    {
    ...
~~~~~~~~~~~~~~~
Extending/inheriting a entity
-----------------------------
~~~~~~~~~~~~~~~{.cpp}
    template <class T>
    class MyExtendedDataReader : public dds::sub::DataReader<T>
    {
        public:
        MyExtendedDataReader() : dds::sub::DataReader<T>(dds::core::null) {}
    ...
~~~~~~~~~~~~~~~
