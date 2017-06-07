Java 5 example code snippets                                                          {#snippets}
=============================

Below you will find an assortment of small code examples which show you how to do various
things with the Java 5 API. Most of the snippets are written in a way that they would be compilable
as one program and as such concepts already demonstrated are not repeated in subsequent snippets.

The following datatype is used for these code snipplets:
~~~~~~~~~~~~~~~{.idl}
module Test
{
   struct Foo
   {
      long userID;    // User ID
      string message; // message
   };
   #pragma keylist Foo userID
};
~~~~~~~~~~~~~~~

Using the DataWriter                                                                   {#datawriter}
====================

Creating a DataWriter with default QoS
--------------------------------------
~~~~~~~~~~~~~~~{.java}
    import Test.Foo;
    System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,"org.opensplice.dds.core.OsplServiceEnvironment");
    ServiceEnvironment env = ServiceEnvironment.createInstance(SnippetClassName.class.getClassLoader());
    DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);
    DomainParticipant p = dpf.createParticipant();
    Topic<Foo> topic = p.createTopic("TypeTopic", Foo.class);
    Publisher publisher = p.createPublisher()
    DataWriter<Foo> writer = publisher.createDataWriter(topic)
~~~~~~~~~~~~~~~
Creating a DataWriter with your own QoS
---------------------------------------
~~~~~~~~~~~~~~~{.java}
    Reliability r = PolicyFactory.getPolicyFactory(env).Reliability().withReliable();
    Durability d = PolicyFactory.getPolicyFactory(env).Durability().withTransient();
    DataWriter<Foo> writer = publisher.createDataWriter(topic, publisher.getDefaultDataWriterQos().withPolicies(r, d));
~~~~~~~~~~~~~~~
Getting and setting DataWriter QoS
----------------------------------
~~~~~~~~~~~~~~~{.java}
    DataWriterQos dwq = writer.getQos();
    Duration deadline = Duration.newDuration(5, TimeUnit.SECONDS, env);
    dwq = dwq.withPolicy(dwq.getDeadline().withPeriod(deadline));
    writer.setQos(dwq);
~~~~~~~~~~~~~~~
Writing a sample
----------------
~~~~~~~~~~~~~~~{.java}
    Foo foo = new Foo(1, "Hello World");
    writer.write(foo);
~~~~~~~~~~~~~~~
Writing with timestamps
-------------------------------------------------------
~~~~~~~~~~~~~~~{.java}
    Time t = Time.newTime(5, TimeUnit.SECONDS, env);
    writer.write(foo, t);
~~~~~~~~~~~~~~~
Register an instance handle and a write sample using it
-------------------------------------------------------
~~~~~~~~~~~~~~~{.java}
    InstanceHandle ih = dataWriter.registerInstance(foo);
    writer.write(foo, ih);
~~~~~~~~~~~~~~~

Using the DataReader                                                                   {#datareader}
====================

Creating a DataReader with the default QoS
------------------------------------------
~~~~~~~~~~~~~~~{.cpp}
    Subscriber subscriber = p.createSubscriber()
    DataReader<Foo> reader = subscriber.createDataReader(topic)
~~~~~~~~~~~~~~~
Creating a DataReader with your own QoS
---------------------------------------
~~~~~~~~~~~~~~~{.java}
    Reliability r = PolicyFactory.getPolicyFactory(env).Reliability().withReliable();
    Durability d = PolicyFactory.getPolicyFactory(env).Durability().withTransient();
    DataReader<Foo> reader = subscriber.createDataReader(topic, subscriber.getDefaultDataReaderQos().withPolicies(r, d));
~~~~~~~~~~~~~~~
Reading a sample
----------------
~~~~~~~~~~~~~~~{.java}
    Iterator<Sample<Foo>> messages = reader.read();
~~~~~~~~~~~~~~~
Reading samples into your own container
---------------------------------------------------------------------
~~~~~~~~~~~~~~~{.java}
    List<Sample<Foo>> samples = new ArrayList<Sample<Foo>>(10);
    List<Sample<Msg>> result = reader.read(samples);
~~~~~~~~~~~~~~~
Reading with a InstanceHandle (lookup key from sample)
------------------------------------------------------
~~~~~~~~~~~~~~~{.java}
    Selector<Foo> selector = reader.select();
    selector.instance(reader.lookupInstance(foo));
    List<Sample<Foo>> result = dataReader.read(samples, selector);
~~~~~~~~~~~~~~~
Reading samples with the Selector
---------------------------------
~~~~~~~~~~~~~~~{.java}
    DataState ds = subscriber.createDataState().withAnyInstanceState().withAnySampleState().withAnyViewState();
    Selector<Foo> selector = reader.select().dataState(ds);
    List<Sample<Foo>> result = dataReader.read(samples, selector);
~~~~~~~~~~~~~~~

Accessing the samples
---------------------
~~~~~~~~~~~~~~~{.java}
    while (result.hasNext()) {
        Foo foo = result.next().getData();
        System.out.println("Id: " + foo.UserID + " Message: " + foo.message);
    }
~~~~~~~~~~~~~~~
