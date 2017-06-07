.. _`API Reference`:


#############
API Reference
#############

Introduction
************

*As described in :ref:`Getting Started <Getting Started>`, the OpenSplice
IDL preprocessor generates typed Streams API classes for each type that
is annotated with a streams pragma.*

As in the OpenSplice DDS *C++ Reference Guide*, the fictional type ``Foo``,
defined in module Space, is used as an example. When the ``Foo`` type is
annotated with a pragma streams, ``FooStreamDataWriter`` and
``FooStreamDataReader`` classes will be generated.

This section describes the usage of all operations on these classes.



QoS Policies
************

+---------------------------+-------------------------------+
| StreamDataWriterQos       | StreamDataReaderQos           |
+===========================+===============================+
| ``StreamFlushQosPolicy``  |                               |
+---------------------------+-------------------------------+

.. need a force a gap here somehow (for the HTML at least)

+-----------------------+---------------------+----------------------------+
| StreamFlushQosPolicy  | Type                | Default value              |
+=======================+=====================+============================+
| ``max_delay``         | ``DDS::Duration_t`` | ``DDS::DURATION_INFINITE`` |
+-----------------------+---------------------+----------------------------+
| ``max_samples``       | ``long``            | ``0``                      |
+-----------------------+---------------------+----------------------------+



StreamDataWriterQos
===================

StreamFlushQosPolicy
--------------------

**Scope**

``DDS::Streams``

**Synopsis**

::

   #include <streams_ccpp.h>

   struct StreamFlushQosPolicy {
      Duration_t  max_delay;
      long max_samples;
   };


**Description**

   The ``StreamFlushQosPolicy`` can be used to set limits on the stream(s) of
   the ``StreamDataWriter`` it is applied to.

**Attributes**

   ``Duration_t max_delay``
      Time-based limit.
      The StreamDataWriter will automatically flush all of its
      streams each ``max_delay`` period.

   |caution| *Note*: ``max_delay`` is not yet implemented.
   It is scheduled for a future release.

   ``long max_samples``
      Samples-per-stream based limit.
      The StreamDataWriter will automatically flush a stream when,
      after appending a sample, the number of samples in that stream
      equals ``max_samples``.

**Detailed Description**

   By setting the ``StreamFlushQosPolicy``, the ``StreamDataWriter`` will
   automatically flush its stream(s) based on a particular limit. The
   attributes can be combined, for example a ``max_delay`` of *1* second and a
   ``max_samples`` of *100* will result in a flush at least each second or sooner
   if 100 samples are appended to a stream.

   The ``max_delay`` limit applies to all streams in case a ``StreamDataWriter``
   manages more than one stream. It is initialized when the first stream is
   created, and applied to all streams created after that.

   In case of a manual flush (when the application calls the `flush`_
   operation), the ``max_samples`` limit is reinitialized.

StreamDataReaderQos
-------------------

Currently no QoS properties for a ``StreamDataReader`` have been identified,
but the ``StreamDataReaderQos`` is defined in the API to maintain
consistency with the ``StreamDataWriter``; it is reserved for future use.

StreamDataWriter Class
**********************

Constructors
============

**Scope**

``Space::FooStreamDataWriter``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   ooStreamDataWriter(
      DDS::Publisher_ptr publisher,
      DDS::Streams::StreamDataWriterQos &sqos,
      const char* streamName);

   FooStreamDataWriter(
      DDS::DomainId_t domainId,
         DDS::Streams::StreamDataWriterQos &sqos,
         const char* streamName);

   FooStreamDataWriter(
      DDS::Streams::StreamDataWriterQos &sqos,
         const char* streamName);

   FooStreamDataWriter(
      DDS::Publisher_ptr publisher,
         const char* streamName);

   FooStreamDataWriter(
         DDS::DomainId_t domainId,
         const char* streamName);

**Description**

   Multiple constructors are available to create a ``FooStreamDataWriter``.
   Depending on which parameters are supplied by the application, one of
   the overloaded constructors will be selected to create a new instance of
   the ``FooStreamDataWriter`` class.

**Parameters**

   ``in DDS::Publisher_ptr publisher``
      A pointer to a pre-created DDS Publisher.
      This parameter is optional; if a publisher is not supplied
      the ``FooStreamDataWriter`` will create an internal publisher.

   ``in DDS::DomainId_t domainId``
      The id of the DDS domain to attach to.
      The ``DDS::DOMAIN_ID_DEFAULT`` macro can be used to connect to the
      default domain, which is also used if the parameter is omitted.

   ``in DDS::Streams::StreamDataWriterQos &sqos``
      The QoS settings that are applied to the ``FooStreamDataWriter``.

   ``in const char* streamName``
      The system-wide unique name of the stream that is used to create
      a DDS (container-)topic for the stream(s) that are handled by
      the ``FooStreamDataWriter``.

**Exceptions**

   Constructors cannot return a value, therefore they throw exceptions when
   the object cannot be constructed. Besides exceptions, the regular
   OpenSplice error logging framework is used to report additional
   information when a constructor fails.

   The constructors throw a ``StreamsException`` if an error occurs. The
   application may catch these exceptions to detect when creation of a
   StreamDataWriter doesn’t succeed.

   ::

      DDS::Streams::StreamsException {
         out const char *message;
         out DDS::ReturnCode_t id
      }

   The message contains a description of the error. The ``id`` field
   contains a DDS error code that represents the error condition.

**Detailed Description**

   When a pre-created publisher is not supplied, the ``FooStreamDataWriter``
   will create an internal DDS participant and DDS publisher. This will
   naturally consume some resources, so when a lot of streams need to be
   created it is recommended to supply a publisher that can be re-used for
   each ``FooStreamDataWriter`` instance.

   The ``streamName`` is a required parameter. The ``FooStreamDataWriter``
   will create a DDS topic of the correct type and name it after the supplied
   ``streamName``.

append
======

**Scope**

``Space::FooStreamDataWriter``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   append(
      StreamId id,
      const Foo &data)

**Description**

   Write a sample to the stream with the supplied ``id``.

**Parameters**

   ``in StreamId id``
      The stream id.

   ``in Foo &data``
      The data to write to the stream.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_PRECONDITION_NOT_MET``.

**Detailed Description**

   Using the ``append`` operation, the application can write data to a stream.
   Note that for each stream of a certain type, multiple *instances* of
   this stream-type can be created by assigning unique ids to each of
   streams. Each id then represents an *instance* of the stream of the
   associated type. So the actual stream instance is selected based on the
   supplied ``StreamId``.

   When the stream doesn’t exist it is automatically created based on the
   current QoS settings.

**Return Code**

   When the operation returns:

   ``RETCODE_OK``
      The data was successfully appended to the stream.

   ``RETCODE_PRECONDITION_NOT_MET``
      A precondition failed, data was not appended.

   If the ``StreamDataWriter`` QoS specifies an auto-flush maximum samples
   limit, an ``append`` may trigger a `flush`_. In that case the ``append``
   call forwards the return code of the flush to the application, so any return
   code that is specified in the next section may also be returned by
   ``append``.

flush
=====

**Scope**

``Space::FooStreamDataWriter``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   flush(
      DDS::Streams::StreamId id)

**Description**

   Write all data in a stream to the DDS subsystem.

**Parameters**

   ``in StreamId id``
      The id of the stream.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_PRECONDITION_NOT_MET``.

**Detailed Description**

   When a stream is flushed, all data in the stream is delivered to DDS and
   the stream is emptied. The memory allocated will be reused the next time
   data is appended to the stream.

   The ``flush`` operation results in a write call on the underlying DDS
   subsystem. Depending on the result of the write, this result is returned
   back to the application.

**Return Code**

   ``RETCODE_OK``
      The stream was successfully flushed.

   ``RETCODE_PRECONDITION_NOT_MET``
      A precondition failed; most likely the stream doesn’t exist.

   See the OpenSplice DDS *C++ Reference Guide* for
   possible result codes returned by a DDS ``write`` operation.

get_qos
=======

**Scope**

``Space::FooStreamDataWriter``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   get_qos(
      DDS::Streams::StreamDataWriterQos &qos)

**Description**

   This operation allows access to the existing set of QoS policies for a
   ``FooStreamDataWriter``.

**Parameters**

   ``inout StreamDataWriterQos &qos``
      A pointer to a ``StreamDatatWriterQos`` object to which the current
      QoS settings will be copied.

**Return Value**

   ``ReturnCode_t``
      Possible return code of the operation is:
      ``DDS::RETCODE_OK``.

**Detailed Description**

   The existing list of QoS settings of the ``FooStreamDataWriter`` is copied
   to the object pointed to by ``qos``. The application can then inspect and,
   if necessary, modify the settings and apply the settings using the
   ``set_qos`` operation.

**Return Code**

   ``RETCODE_OK``
      The QoS settings were successfully copied to the supplied ``qos`` object.

set_qos
=======

**Scope**

``Space::FooStreamDataWriter``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   set_qos(
      DDS::Streams::StreamDataWriterQos &qos)

**Description**

   This operation allows replacing the existing set of QoS policies for a
   ``FooStreamDataWriter``.

**Parameters**

   ``in StreamDataWriterQos &qos``
      A pointer to a ``qos`` object with the new policies.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_UNSUPPORTED``.

**Detailed Description**

   This operation allows replacing the set of QoS policies of a
   ``FooStreamDataWriter``.

   |caution| *Note*: A new ``StreamFlushQosPolicy`` may decrease the
   value of ``max_samples``, but existing streams are not allowed to
   violate this limit. Any streams that contain data that exceeds the
   new ``max_samples`` value are automatically flushed before the new
   policy is applied.

**Return Code**

   ``RETCODE_OK``
      The QoS settings were successfully applied to the
      ``FooStreamDataWriter``.

   ``RETCODE_UNSUPPORTED``
      The application attempted to set QoS policies or values that are
      not (yet) supported.

StreamDataReader Class
**********************

Constructors
============

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   FooStreamDataReader(
      DDS::Subscriber_ptr subscriber,
      DDS::Streams::StreamDataReaderQos &sqos,
      const char* streamName);

   FooStreamDataReader(
      DDS::DomainId_t domainId,
         DDS::Streams::StreamDataReaderQos &sqos,
         const char* streamName);

   FooStreamDataReader(
      DDS::Streams::StreamDataReaderQos &sqos,
         const char* streamName);

   FooStreamDataReader(
      DDS::Subscriber_ptr subscriber,
         const char* streamName);

   FooStreamDataReader(
         DDS::DomainId_t domainId,
         const char* streamName);

**Description**

   Multiple constructors are available to create a ``FooStreamDataReader``.
   Depending on which parameters are supplied by the application, one of
   the overloaded constructors will be selected to create a new instance of
   a ``FooStreamDataReader`` class.

**Parameters**

   ``in DDS::Subscriber_ptr subscriber``
      A pointer to a pre-created DDS Subscriber.
      This parameter is optional; if a subscriber is not supplied the
      ``FooStreamDataReader`` will create an internal subscriber.

   ``in DDS::DomainId_t domainId``
      The id of the DDS domain to attach to. The ``DDS::DOMAIN_ID_DEFAULT``
      macro can be used to connect to the default domain, which is also
      used if the parameter is omitted.

   ``in DDS::Streams::StreamDataReaderQos &sqos``
      The QoS settings that are applied to the ``FooStreamDataReader``.

   ``in const char* streamName``
      The system-wide unique name of the stream which is also used to create
      a DDS (container-)topic for the stream(s) that are handled by
      the ``FooStreamDataReader``.

**Exceptions**

   Constructors cannot return a value, therefore they throw exceptions when
   the object cannot be constructed. Besides exceptions, the regular
   OpenSplice error logging framework is used to report additional
   information when a constructor fails.

   The constructors throw a ``StreamsException`` if an error occurs. The
   application may catch these exceptions to detect when creation of a
   ``StreamDataReader`` doesn’t succeed.

   ::

      DDS::Streams::StreamsException {
         out const char *message;
         out DDS::ReturnCode_t id
      }

   The message contains a description of the error. The ``id`` field
   contains a DDS error code that represents the error condition.

**Detailed Description**

   When a pre-created subscriber is not supplied, the ``FooStreamDataReader``
   will create an internal DDS participant and DDS subscriber. This will
   naturally consume some resources, so when a lot of instances need to be
   created it is recommended to supply a subscriber that can be re-used for
   each ``FooStreamDataReader`` instance.

   The ``streamName`` is a required parameter. The ``FooStreamDataReader``
   will create a DDS topic of the correct type and name it after the supplied
   ``streamName``.

get
===

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   get(
      DDS::Streams::StreamId id,
      Space::FooStreamBuf data_values,
      long max_samples,
      DDS::Duration_t timeout);

**Description**

   Check if any data is available in a stream and retrieve it, emptying the
   stream.

**Parameters**

   ``in StreamId id``
      The ``id`` of the stream instance from which to retrieve
      the data.

   ``inout FooStreamBuf data_values``
      The buffer in which the data is stored.

   ``in long max_samples``
      The maximum amount of data samples retrieved.
      Default is ``DDS::LENGTH_UNLIMITED``.

   ``in Duration_t timeout``
      Blocking time, in case no data is immediately available.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_PRECONDITION_NOT_MET``.

**Detailed Description**

   Using the ``get`` operation, the application can retrieve data from a
   stream. The stream is selected based on the supplied ``StreamId``.

   If no data is available initially, the ``get`` operation blocks for a
   maximum period specified in the ``timeout`` parameter. If data becomes
   available during the ``timeout`` period the ``FooStreamDataReader``
   proceeds to retrieve the data and return it to the application.
   To return immediately, the application can use the special value
   ``DDS::DURATION_ZERO`` as a ``timeout`` parameter. To block indefinitely
   until data is available, the value ``DDS::DURATION_INFINITE`` should
   be passed.

   The data is returned in a buffer that is to be supplied by the
   application. The application is responsible for allocating a buffer that
   is large enough to contain the available data. If more data is available
   than will fit in the buffer, the excess data will be stored by the
   StreamDataReader and returned to the application during the next call to
   `get`_ (or `get_w_filter`_). In this state, the ``StreamDataReader``
   will only attempt to retrieve new data after all data that was stored
   internally is returned to the application.

   Since allocating memory for the buffer is an expensive operation, it is
   recommended to re-use the same buffer for each subsequent call to `get`_
   or `get_w_filter`_. The ``max_samples`` parameter can be used to limit
   the amount of data that is returned with each `get`_ or `get_w_filter`_
   call.

   |caution| *Note*: Internal pre-allocation of buffers, using a loans
   registry similar to the DCPS API, will be implemented in a future version.

**Return Code**

   ``DDS::RETCODE_OK``
      Data is returned in the ``data_values`` buffer.

   ``DDS::RETCODE_NO_DATA``
      There is currently no data available.

   ``DDS::RETCODE_PRECONDITION_NOT_MET``
      The operation could not be performed because a precondition is not
      met; most likely the ``data_values`` buffer is not preallocated.

   The list of possible return codes includes all possible return codes of
   ``waitset.wait()`` and ``take_instance()`` calls. These DCPS calls are
   used internally by the Streams API. There is one exception: if the
   ``waitset.wait()`` returns a ``DDS::RETCODE_TIMEOUT``, this return
   code is translated to a ``DDS::RETCODE_NO_DATA`` return code.

   See the OpenSplice DDS *C++ Reference Guide* for possible result
   codes returned by a DDS ``take_instance`` operation and
   for ``waitset.wait()``.

get_w_filter
============

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   get_w_filter(
      DDS::Streams::StreamId id,
      Space::FooStreamBuf data_values,
      long max_samples,
      DDS::Duration_t timeout
      Space::FooStreamFilterCallback a_filter);

**Description**

   Check if any data is available in a stream and retrieve it if it matches
   the filter, discard otherwise.

**Parameters**

   ``in StreamId id``
      The ``id`` of the stream instance of which to retrieve the data.

   ``inout FooStreamBuf data_values``
      The buffer in which the data is stored.

   ``in long max_samples``
      The maximum amount of data samples retrieved.

   ``in Duration_t timeout``
      Blocking time, in case no data is immediately available.

   ``in FooStreamFilterCallback a_filter``
      Pointer to a function that implements a filter for the data.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_PRECONDITION_NOT_MET``.

**Detailed Description**

   The ``get_w_filter`` operation is equivalent to the `get`_ operation,
   the description of `get`_ also applies to ``get_w_filter``.

   The difference is that ``get_w_filter`` allows the application to
   supply a ``FooStreamFilterCallback`` instance that implements the
   ``match_data()`` operation. Each data sample is matched against the
   filter and only data for which the filter returns ``true`` is returned
   to the application.

   Samples that do not match the filter are not considered in relation to
   ``max_samples`` and the ``data_values`` buffer length; the buffer does
   *not* need to be capable of holding *all* available samples, just the
   samples that pass the filter.

   Samples are only evaluated once and are discarded if not matched.

**Return Code**

   ``DDS::RETCODE_OK``
      Data is returned in the ``data_values`` buffer.

   ``DDS::RETCODE_NO_DATA``
      There is no data available during the period specified by ``timeout``.

   ``DDS::RETCODE_PRECONDITION_NOT_MET``
      The operation could not be performed because a precondition is not
      met; most likely the ``data_values`` buffer is not preallocated.

   The list of possible return codes includes all possible return codes of
   ``waitset.wait()`` and ``take_instance()`` calls. These DCPS calls are
   used internally by the Streams API. There's one exception: If the
   ``waitset.wait()`` returns a ``DDS::RETCODE_TIMEOUT``, this return code
   is translated to a ``DDS::RETCODE_NO_DATA`` return code.

   See the OpenSplice DDS *C++ Reference Guide* for possible result codes
   returned by a DDS ``take_instance`` operation and ``waitset.wait()``.

return_loan
===========

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   return_loan(
      Space::FooStreamBuf data_values)

**Description**

   The application should use this operation to indicate that it has
   finished accessing the sequence of ``data_values``.

**Parameters**

   ``inout FooStreamBuf data_values``
      The data sequence which was loaned from the ``FooStreamDataReader``.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_PRECONDITION_NOT_MET``.

**Detailed Description**

   When the application does not pre-allocate a buffer to hold the data,
   the ``FooStreamDataReader`` will do so itself when a `get`_ operation is
   invoked. The application calls ``return_loan`` to indicate that it has
   finished accessing this buffer so the ``FooStreamDataReader`` can reclaim
   the resources allocated for the buffer.

   |caution| *Note*: Internal pre-allocation will be implemented in a
   future release. This operation has no effect on buffers allocated by
   the application.

get_qos
=======

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   get_qos(
      DDS::Streams::StreamDataReaderQos &qos)

**Description**

   This operation allows access to the existing set of QoS policies for a
   ``FooStreamDataReader``.

**Parameters**

   ``inout StreamDataReaderQos &qos``
      A pointer to a ``StreamDataReaderQos`` object to which the current
      QoS settings will be copied.

**Return Value**

   ``ReturnCode_t``
      Possible return code of the operation is:
      ``DDS::RETCODE_OK``.

**Detailed Description**

   The existing list of QoS settings of the ``FooStreamDataReader`` is copied
   to the object pointed to by ``qos``. The application can then inspect and,
   if necessary, modify the settings and apply the settings using the
   ``set_qos`` operation.

**Return Code**

   ``RETCODE_OK``
      The QoS settings were successfully copied to the supplied ``qos`` object.

set_qos
=======

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   set_qos(
      DDS::Streams::StreamDataReaderQos &qos)

**Description**

   This operation allows replacing the existing set of QoS policies for a
   ``FooStreamDataReader``.

**Parameters**

   ``in StreamDataReaderQos &qos``
      A pointer to a ``qos`` object with the new policies.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_UNSUPPORTED``.

**Detailed Description**

   This operation allows replacing the set of QoS policies of a
   ``FooStreamDataReader``.

**Return Code**

   ``RETCODE_OK``
      The QoS settings were successfully applied to the ``FooStreamDataWriter``.

   ``RETCODE_UNSUPPORTED``
      The application attempted to set QoS policies or values that are not
      (yet) supported.

interrupt
=========

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   DDS::ReturnCode_t
   interrupt();

**Description**

   Interrupt a blocking `get`_ operation from a different thread.

**Return Value**

   ``ReturnCode_t``
      Possible return codes of the operation are:
      ``DDS::RETCODE_OK``, ``DDS::RETCODE_ERROR``.

**Detailed Description**

   The `get`_ operation accepts a ``timeout`` parameter which causes the
   ``FooStreamDataReader`` to block until data becomes available. It can block
   indefinitely when an infinite timeout is supplied and data never becomes
   available because there are simply no compatible writers.

   In such cases it can be desirable to interrupt the `get`_ operation from the
   application, i.e. for termination or reclaiming of resources.

   The ``interrupt`` call triggers an internal ``GuardCondition`` by calling
   ``DDS::GuardCondition::set_trigger_value(true)``. This causes the `get`_
   operation to return with a ``DDS::RETCODE_NO_DATA`` result.

**Return Code**

   The return code of this operation is determined by the result of ``DDS::GuardCondition::set_trigger_value()``

   ``DDS::RETCODE_OK``
      The ``GuardCondition`` was triggered successfully

   ``DDS::RETCODE_ERROR``
      An internal error occurred

FooStreamFilterCallback Interface
*********************************

**Scope**

``Space::FooStreamDataReader``

**Synopsis**

::

   #include <SpaceStreamsApi.h>

   boolean
   a_filter(
      const Space::Foo &data)

**Description**

   Function interface for filters that are passed to the `get_w_filter`_
   and/or ``peek_w_filter`` operations.

**Parameters**

   ``in const Foo &data``
      A data sample.

**Return Value**

   ``boolean``
      Return ``true`` if the supplied data matches, ``false`` if it
      doesn’t match.

**Detailed Description**

   The application can supply any function that adheres to the
   ``FooStreamFilterCallback`` interface, to filter data that is retrieved by
   the `get_w_filter`_ operation. If the data matches the filter, the function
   returns ``true`` and the data is added to the ``data_values`` buffer that is
   returned by the `get_w_filter`_ operation. Data that doesn’t match the
   filter is discarded.



.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm
