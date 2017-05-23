.. _`Reading and Writing Data`:

########################
Reading and Writing Data
########################


The previous chapter covered the definition and semantics of DDS
topics, topic-instances and samples. It also described domains and
partitions and the roles they play in organizing application data flows.
This chapter examines the mechanisms provided by DDS for reading
and writing data.

************
Writing Data
************

As already illustrated, writing data with DDS is as simple
as calling the ``write`` method on the ``DataWriter``. Yet to 
be able to take full advantage of DDS it is necessary to understand
the relationship between writers and topic-instances life-cycles.

To explain the difference between topics and the instances of a topic's
datatype, this Tutorial made the analogy between topics/topic datatypes and 
classes/objects in an Object-Oriented Programming language, such 
as Java or C++. Like objects, the instances of the topic's datatype have:

  +  an identity provided by their unique key value, and 
  +  a life-cycle.


The instance life-cycle of a topic's datatype can be *implicitly* managed through
the semantics implied by the ``DataWriter``, or it can
be *explicitly* controlled *via* the ``DataWriter`` API. The
instance life-cycle transition can have implications for
local and remote resource usage, thus it is important to
understand this aspect.



Topic-Instances Life-cycle
==========================

Before getting into the details of how the life-cycle is managed, let's
see which are the possible states. 

+ An instance of a topic's datatype is ``ALIVE`` if there is at least one 
  ``DataWriter`` that has explicitly or implicitly (through a write) 
  registered it. A ``DataWriter`` that has registered an instance declares
  that it is committed to publishing potential updates for that instance 
  as soon as they occur. For that reason, the ``DataWriter`` has reserved
  resources to hold the administration for the instances and at least one
  of its samples. ``DataReaders`` for this topic will also maintain a similar 
  resource reservation for each registered instance. As long as an instance
  is registered by at least one ``DataWriter``, it will be considered ``ALIVE``.   
+ An instance is in the ``NOT_ALIVE_NO_WRITERS`` state when there are no 
  more ``DataWriters`` that have registered the instance. That means no more
  ``DataWriters`` have an intent to update the instance state and all of them
  released the resources they had previously claimed for it. In this state
  ``DataReaders`` no longer expect any incoming updates and so they may release 
  their resources for the instance as well. Be aware that when a Writer forgets
  to unregister an instance it no longer intends to update, it does not only
  leak away the resources it had locally reserved for it, but it also leaks 
  away the resources that all subscribing ``DataReaders`` still have reserved 
  for it in the expectation of future updates.  
+ Finally, the instance is ``NOT_ALIVE_DISPOSED`` if it was disposed either 
  implicitly, due to some default QoS settings, or explicitly by means of a 
  specific ``DataWriter`` API call. The ``NOT_ALIVE_DISPOSED`` state indicates 
  that the instance is no more relevant for the system and should basically be
  wiped from all storage. The big difference with the ``NOT_ALIVE_NO_WRITERS``
  state is that the latter only indicates that nobody intends to update the 
  instance and does not say anything about the validity of the last known state.
  
  
As an example, when a publishing application crashes it might want to restart 
on another node and obtain its last known state from the domain in which it 
resides. In the mean time it has no intention to invalidate the last known 
state for each of its instances or to wipe them from all storage in its domain. 
Quite the opposite, it wants the last known state to remain available for 
late-joiners, so that it can pick back up where it left off as soon as it is 
restarted. So in this case the Writer needs to make sure its instances go from 
``ALIVE`` to ``NOT_ALIVE_NO_WRITERS`` after the crash, which may then go back 
to ``ALIVE`` after the publishing application has been restarted.
  
On the other hand, if the application gracefully terminates and wants to indicate
that its instances are no longer a concern to the DDS global data space, it may 
want the state of its instances to go to ``NOT_ALIVE_DISPOSED`` so that the rest of 
the domain knows it can safely wipe away all of its samples in all of its storages. 


Automatic Life-cycle Management
===============================

We will illustrate the instances life-cycle management with an
example. 

If we look at the code in `Automatic management of Instance life-cycle`_
and assume this is the only application writing data, the result of the 
three ``write`` operations is to create three new topic instances in the 
system for the key values associated with the ``id = 1``, ``2``, ``3`` 
(the ``TempSensorType`` was defined in the  
:ref:`first chapter <IDL definition of a Temperature Sensor>`
as having a single attribute key named ``id``). These instances will be 
in the ``ALIVE`` state as long as this application is running, and will be 
automatically registered (we could say ‘associated’) with the writer. 
The default behavior for DDS is to then dispose the topic instances once 
the ``DataWriter`` object is destroyed, thus leading those instances to the 
``NOT_ALIVE_DISPOSED`` state. The default settings can be overridden to 
simply induce instances’ unregistration, causing in this case a transition from
``ALIVE`` to ``NOT_ALIVE_NO_WRITERS``.

.. _`Automatic management of Instance life-cycle`:

| **Automatic management of Instance life-cycle**

.. literalinclude:: ./code/isocpp2/ch3/alifecycle.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end

Explicit Life-cycle Management
==============================

Topic-instances life-cycle can also be managed explicitly *via* the API
defined on the ``DataWriter``. 

In this case the application programmer has full control of when instances  
are registered, unregistered and disposed. 

Topic-instance registration is a good practice to follow when
an application writes an instance very often and requires the
lowest-latency write. In essence the act of explicitly registering an
instance allows the middleware to reserve resources as well as optimize
the instance lookup. Topic-instance unregistration provides a means for
telling DDS that an application is done with writing a specific
topic-instance, thus all the resources locally associated with can be
safely released. Finally, disposing topic-instances gives a way of
communicating to DDS that the instance is no longer relevant for the
distributed system, thus whenever possible resources allocated with the
specific instances should be released both locally and remotely. 
`Explicit management of topic-instances life-cycle`_ shows an example of 
how the DataWriter API can be used to register,
unregister and dispose topic-instances. 

In order to show the full life-cycle management, the default DataWriter 
behavior has been changed so that instances are *not* automatically 
disposed when unregistered. In addition, to keep the code compact it 
takes advantage of the new C++11 ``auto`` feature which leaves it to 
the the compiler to infer the left-hand-side types from the right-hand-side 
return-type. 

`Explicit management of topic-instances life-cycle`_ shows an
application that writes four samples belonging to four different
topic-instances, respectively those with ``id = 1, 2, 3``. The
instances with ``id = 1, 2, 3`` are explicitly registered by calling the
``DataWriter::register_instance`` method, while the instance with
``id=0`` is automatically registered as result of the write on the
``DataWriter``. 

To show the different possible state transitions, the
topic-instance with ``id=1`` is explicitly unregistered, thus causing
it to transition to the ``NOT_ALIVE_NO_WRITER`` state; the
topic-instance with ``id=2`` is explicitly disposed, thus causing it
to transition to the ``NOT_ALIVE_DISPOSED`` state. Finally, the
topic-instance with ``id=0,3`` will be automatically unregistered,
as a result of the destruction of the objects ``dw`` and
``dwi3`` respectively, thus transitioning to the state
``NOT_ALIVE_NO_WRITER``. 

Once again, as mentioned above, in this example
the writer has been configured to ensure that topic-instances are not
automatically disposed upon unregistration.

.. literalinclude:: ./code/isocpp2/ch3/lifecycle.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end


Keyless Topics
==============

Most of the discussion above has focused on keyed topics, but what about
keyless topics? As explained in
:ref:`Topics, Domains and Partitions <Distinguish keyless from keyed topics>`
keyless topics are like singletons, in the sense that there is only one 
instance. As a result for keyless topics the state transitions are 
tied to the lifecycle of the data-writer.

.. _`Explicit management of topic-instances life-cycle`:

| **Explicit management of topic-instances life-cycle**


Blocking or Non-Blocking Write?
===============================

One question that might arise at this point is whether the write is
blocking or not. The short answer is that the write is non-blocking;
however, as will be seen later on, there are cases in which, depending on
settings, the write *might* block. In these cases, the blocking behaviour
is necessary to avoid data-loss.


**************
Accessing Data
**************

DDS provides a mechanism to select the samples based on their *content* and
*state*, and another to control whether samples have to be *read* or *taken*
(removed from the cache).

Read *vs.* Take
===============

The DDS provides data access through the ``DataReader`` class which exposes
two semantics for data access: *read* and *take*. 

The *read* semantics, implemented by the ``DataReader::read`` method, gives 
access to the data received by the ``DataReader`` without removing it from 
its cache. This means that the data will remain readable *via* an appropriate 
read call. 

The *take* semantics, implemented by the ``DataReader::take`` method, 
allows DDS to access the data received by the ``DataReader`` by removing it 
from its local cache. This means that once the data is taken, it is no longer 
available for subsequent read or take operations. 

The semantics provided by the ``read`` and ``take`` operations enable
you to use DDS as either a distributed cache or like a queuing system, or
both. This is a powerful combination that is rarely found in the same
middleware platform. This is one of the reasons why DDS is used in a variety
of systems sometimes as a high-performance distributed cache, or like
a high-performance messaging technology, and at yet other times as a
combination of the two. In addition, the *read* semantics is useful when
using topics to model distributed *states*, and the *take* semantics when
modeling distributed *events*.

Data and Meta-Data
==================

The first part of this chapter showed how the ``DataWriter`` can be
used to control the life-cycle of topic-instances. The topic-instance
life-cycle along with other information describing properties of
received data samples are made available to ``DataReader`` and can be
used to select the data access *via* either a ``read`` or ``take``. 
Specifically, each data sample received by a ``DataWriter`` has an associated
``SampleInfo`` describing the property of that sample. These properties
includes information on:

+ **Sample State**. The sample state can be ``READ`` or ``NOT_READ`` 
  depending on whether the sample has already been read or not.

+ **Instance State.** As explained above, this indicates the status of
  the instance as being either ``ALIVE``, ``NOT_ALIVE_NO_WRITERS``, or
  ``NOT_ALIVE_DISPOSED``.

+ **View State.** The view state can be ``NEW`` or ``NOT_NEW`` 
  depending on whether this is the first sample ever received for the
  given topic-instance or not.

The ``SampleInfo`` also contains a set of counters that allow you to
determine the number of times that a topic-instance has performed
certain status transitions, such as becoming alive after being disposed.

Finally, the ``SampleInfo`` contains a ``timestamp`` for the data and a
flag that tells wether the associated data sample is valid or not. This
latter flag is important since DDS might generate valid samples info
with invalid data to inform about state transitions such as an instance
being disposed.

Selecting Samples
=================

Regardless of whether data are read or taken from DDS, the same mechanism
is used to express the sample selection. Thus, for brevity, the following
examples use the ``read`` operation; to use the ``take`` operation,
simply replace each occurrence of a ``read`` with a ``take``.

DDS allows the selection of data based on *state* and *content*. 

+ State-based selection is based on the values of the *view* state, 
  *instance* state and *sample* state.
+ Content-based selection is based on the content of the sample.


State-based Selection
---------------------

For instance, to get *all* of the data received, no
matter what the view, instance and sample state, issue a ``read`` 
(or a ``take``) as follows:

.. literalinclude:: ./code/isocpp2/ch3/sbsub.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end

On the other hand, to read (or take) only samples that have not been 
read yet, issue a ``read`` (or a ``take``) as follows:

.. literalinclude:: ./code/isocpp2/ch3/sbsub.cpp
     :language: cpp
     :start-after: segment2-start
     :end-before: segment2-end

To read new valid data, meaning no samples with only a valid
``SampleInfo``, issue a ``read`` (or a ``take``) as follows:

.. literalinclude:: ./code/isocpp2/ch3/sbsub.cpp
     :language: cpp
     :start-after: segment3-start
     :end-before: segment3-end

Finally, to only read data associated to instances that are
making their appearance in the system for the first time, issue 
a ``read`` (or a ``take``) as follows:

.. literalinclude:: ./code/isocpp2/ch3/sbsub.cpp
     :language: cpp
     :start-after: segment4-start
     :end-before: segment4-end


Notice that this kind of read *only* and *always* gets 
*the first sample written for each instance*. 

Although it might seem a strange use case, this is quite useful for all those 
applications that need to do something special whenever a new instance makes 
its first appearance in the system. An example could be a new
airplane entering a new region of control; in this case the system 
would have to do quite a few things that are unique to this specific state
transition.

It is also worth mentioning that if the status is omitted, 
a ``read`` (or a ``take``) can be used like this:

.. literalinclude:: ./code/isocpp2/ch3/sbsub.cpp
     :language: cpp
     :start-after: segment5-start
     :end-before: segment5-end

This is equivalent to selecting samples with the
``NOT_READ_SAMPLE_STATE``, ``ALIVE_INSTANCE_STATE`` and
``ANY_VIEW_STATE``.

finally, it should be noted that statuses enable
data to be selected based on its meta-information.

Content-based Selection
-----------------------

Content-based selection is supported through *queries*. Although the
concept of a query might seem to overlap with that of 
:ref:`content filtering <Content Filtering>`,
the underlying idea is different. 

*Filtering* is about controlling the data received by the data
reader: the data that does not match the filter is not inserted into
the data reader cache. On the other hand, *queries* are about selecting
the data that is (already) in the data reader cache.

.. _`Content Query`:

| **Content Query**

.. literalinclude:: ./code/isocpp2/ch3/cbsub.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end

The syntax supported by query expressions is identical to that used to
define filter expressions; for convenience this is summarized in 
the table.



.. _`Legal operators for content query`:

| **Legal operators for content query**

.. tabularcolumns:: | p{4.5cm} | p{7.5cm} |
 
+------------------------+-------------------------------+
| **Constructed Type**   | **Example**                   |
+========================+===============================+
| =                      | equal                         |
+------------------------+-------------------------------+
| <>                     | not equal                     |
+------------------------+-------------------------------+
| >                      | greater than                  |
+------------------------+-------------------------------+
| <                      | less than                     |
+------------------------+-------------------------------+
| >=                     | greater than or equal         |
+------------------------+-------------------------------+
| <=                     | less than or equal            |
+------------------------+-------------------------------+
| BETWEEN                | between and inclusive range   |
+------------------------+-------------------------------+
| LIKE                   | matches a string pattern      |
+------------------------+-------------------------------+


The execution of the query is completely under user control and is
performed in the context of a ``read`` or ``take`` operation as shown in
ListingB [Listing:DDS:Query].


Instance-based Selection
------------------------

In some instances you may want to only look at the data coming from a
specific topic instance. As instances are identified by the values of their 
key attributes you may be tempted to use content filtering to discriminate
between them. Although this would work perfectly well, it is not the most
efficient way of selecting an instance. DDS provides another mechanism that
allows you to pinpoint the instance you are interested in more
efficiently than content filtering. In essence, each instance has
an associated *instance handle*; this can be used to access the data from
a given instance in a very efficient manner.

The listing `Instance-based selection`_ shows how this can be done.

.. _`Instance-based selection`:

| **Instance-based selection**

.. literalinclude:: ./code/isocpp2/ch3/ibsub.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end


Iterators or Containers?
========================

The examples shown so far were ‘loaning’ the data from DDS: in
other words, you did not have to provide the storage for the samples. The
advantage of this style of read is that it allows ‘zero copy’ reads. 
However, if you want to store the data in a container of your choice 
you can use iterator-based read and take operations.

The iterator-based read/take API supports both forward iterators as well
as back-inserting iterators. The API allows you to read (or take) data
into whatever structure you'd like, so long as you can get a forward or a
back-inserting iterator for it. Here we will focus on the forward-iterator-based 
API; back-inserting is pretty similar. you should be able
to read data as follows:

.. literalinclude:: ./code/isocpp2/ch3/itersub.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end


Blocking or Non-Blocking Read/Take?
===================================

The DDS read and take are always non-blocking. If no data is available
to read then the call will return immediately. Likewise if there is less
data than requested the call will gather what *is* available and return right
away. The non-blocking nature of read/take operations ensures that these
can be safely used by applications that poll for data.

**************************
Waiting and being Notified
**************************

One way of coordinating with DDS is to have the application poll for
data by performing either a read or a take every so often. Polling might
be the best approach for some classes of applications, the most common
example being control applications that execute a control loop or a
cyclic executive. In general, however, applications might want to be
notified of the availability of data or perhaps be able to wait for its
availability, as opposed to polling for it. DDS supports both synchronous and
asynchronous coordination by means of wait-sets and listeners.

Waitsets
========

DDS provides a generic mechanism for waiting on conditions. One of the
supported kind of conditions are ``ReadConditions`` which can be used to
wait for the availability data on one or more ``DataReaders``. This
functionality is provided by the ``Waitset`` class, which can be
regarded as an object-oriented version of the Unix ``select``.

.. _`Using WaitSet to wait for data availability`:

| **Using WaitSet to wait for data availability**

.. literalinclude:: ./code/isocpp2/ch3/wstssub.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end

If we wanted to wait for temperature samples to be available we could
create a ``ReadCondition`` on our ``DataReader`` and make it wait for new
data by creating a ``WaitSet`` and attaching the ``ReadCondition`` to it 
as shown in `Using WaitSet to wait for data availability`_. 

At this point, we can synchronize on the
availability of data, and there are two ways of doing it. One approach
is to invoke the ``Waitset::wait`` method, which returns the list of
active conditions. These active conditions can then be iterated upon and
their associated datareaders can be accessed. The other approach is to
invoke the ``Waitset::dispatch``, which is demonstrated in a separate example.

As an alternative to iterating through the conditions yourself, DDS conditions 
can be associated with functor objects which are then used to execute 
application-specific logic when the condition is triggered. The DDS event-handling 
mechanism allows you to bind anything you want to an event, meaning that you 
could bind a function, a class method, or even a lambda-function as a functor 
to the condition. You then attach the condition to the waitset in the same way, 
but in this case you would invoke the ``Waitset::dispatch`` function, that causes 
the infrastructure to automatically invoke the functor associated with each triggered 
conditions before unblocking, as is shown in `Using WaitSet to dispatch to incoming data`_.
Notice that the execution of the functor happens in the context of the application 
thread, prior to returning from the ``Waitset::dispatch`` function.


.. _`Using WaitSet to dispatch to incoming data`:

| **Using WaitSet to dispatch to incoming data**

.. literalinclude:: ./code/isocpp2/ch3/wsdispatch.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end


Listeners
=========

Another way of finding out when there is data to be read is to take
advantage of the events raised by DDS and notified asynchronously to
registered handlers. Thus, if we wanted a handler to be notified of the
availability of data, we would connect the appropriate handler with the
``on_data_available`` event raised by the ``DataReader``.

.. _`Using a listener to receive notification of data availability`:

| **Using a listener to receive notification of data availability**

.. literalinclude:: ./code/isocpp2/ch3/ltssub.cpp
     :language: cpp
     :start-after: segment1-start
     :end-before: segment1-end

.. literalinclude:: ./code/isocpp2/ch3/ltssub.cpp
     :language: cpp
     :start-after: segment2-start
     :end-before: segment2-end

The listing `Using a listener to receive notification of data availability`_ 
shows how this can be done. The ``NoOpDataReaderListener`` is a utility 
class provided by the API that provides a trivial implementation for all 
of the operations defined as part of the listener. This way, you can override 
only those that are relevant for your application.

Something worth pointing out is that the handler code will execute in a 
middleware thread. As a result, when using listeners you should try to 
minimize the time spent in the listener itself.

*******
Summary
*******

This chapter has presented the various aspects involved in writing
and reading data with DDS. It described the topic-instance life-cycle,
explained how that can be managed *via* the ``DataWriter`` and showcased
all the meta-information available to ``DataReader``. It explained
wait-sets and listeners and how these can be used to receive
indication of when data is available.

It is recommended again that the reader compiles and runs the examples 
and experiments with the programs developed so far.

