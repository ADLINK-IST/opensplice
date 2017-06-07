ISO/IEC C++ Streams DCPS API                                                            {#streams_mainpage}
============================

[TOC]

The ISO C++ Streams API is an extension of ISO C++ and provides a mechanism for batching samples
together into a stream. This has the effect of increasing data throughput due to a reduced overhead
which occurs when transmitting samples individually. The samples are appended to a stream
and then later flushed which causes the samples to be written together as one batch. It is performant
when a large number of samples need to be sent continuously.


Getting Started                                                                 {#streams_mainpage_getting_started}
=================
If you are new to ISO C++ Streams we would recommend that you start by looking at the example code provided in your OpenSplice installation directory as well as the documentation in the sections below.

A summary of the Throughput example, along with build instructions can be found on the \subpage isocpp_streams_examples page.

Using Streams                                                               {#streams_mainpage_using}
=============

To enable the use of the Streams API on a data-model and automatically include the required API headers, you should annotate the data-model in your IDL file with the `#pragma stream` directive.

~~~~~~~~~~~~~~~~~~{.idl}
module Foo
{
    struct Bar
    {
        unsigned long long count;
        sequence<octet> payload;
    };
    #pragma stream Bar
    #pragma keylist Bar
};
~~~~~~~~~~~~~~~~~~

The inclusion of the `FooSteams_DCPS.hpp` file is also required, this also implicitly includes `Foo_DCPS.hpp` and facilitates combined API usage.

Configuring Streams
-------------------

Due to the way streams batches samples together it has a higher memory footprint than standard dcps. As a result it is possible to reach the limits of the default 10mb database size eg. if you have a large (in terms of bytesize) data-model or you have a large number of samples per batch, either by having a large flushMaxSamples or large flushTimeOut.

You can change the database by altering the Database Size parameter either with the osplconf tool or by altering the xml:
~~~~~~~~~~~~~~~~~~~~~~{.xml}
<Database>
    <Size>20485760</Size>
</Database>
~~~~~~~~~~~~~~~~~~~~~~

API documentation                                                               {#streams_mainpage_api}
=================

The namespaces below contain all of the major functionality of the ISO C++ Streams API:

- \subpage dds::streams

    + dds::streams::pub
        + The dds::streams::pub namespace provides facilities for writing samples.
    + dds::streams::sub
        + The dds::streams::sub namespace provides facilities for reading samples.
    + dds::streams::core
        + The dds::streams::core namespace provides core facilities such as QoS policies.

Platform requirements                                                       {#streams_mainpage_build}
=====================

The ISO C++ PSM specification, and therefore the ISO C++ Streams API have the same requirements which can be found in the @ref mainpage_build "ISO C++ requirements section"

Supported features                                                          {#streams_mainpage_supported}
===================

Streams is an extension of ISO C++ and so can be used along side the ISO C++ API and make
use of features such as WaitSets. It does not, however, support:

- @ref DCPS_Modules_Infrastructure_Listener "Listeners"
- @ref DCPS_Modules_Subscription_DataReader "DataReader QoS"
- @ref DCPS_Modules_Publication_DataWriter "DataWriter QoS"
- @ref DCPS_Modules_Topic "Topic QoS"
- Status polling
